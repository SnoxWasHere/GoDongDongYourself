/**********************************************
* █▀▀ █▀█   █▀▄ █▀█ █▄ █ █▀▀   █▀▄ █▀█ █▄ █ █▀▀
* █▄█ █▄█   █▄█ █▄█ █ ▀█ █▄█   █▄█ █▄█ █ ▀█ █▄█
*            █▄█ █▀█ █ █ █▀█ █▀ █▀▀ █   █▀▀
*             █  █▄█ █▄█ █▀▄ ▄█ ██▄ █▄▄ █▀ 
***********************************************
* STEP 1
* Converts .player file into .png and .wav
* v1.1 - snox - 08/2023
*
* Large portions of this code is translated from the 2DK_2nd_Ripper
* uploaded by pomba_gira on the Mugen Archive, author unknown
*/

#include <iostream>
#include <fstream>
#include <vector>
#include <stdint.h>
#include <Windows.h>
#include <filesystem>
#include <cstdlib>
#include <bitset>
#include <assert.h>

namespace fs = std::filesystem;
using namespace std;


struct hRen
{
    uint32_t ofs = 0;
    uint32_t w = 0;
    uint32_t h = 0;
    uint32_t pal = 0;
    uint32_t size = 0;
    uint8_t* unk = nullptr;
};

struct Sound
{
    char name[0x24];
    uint32_t size;
    uint16_t unk;
};

struct BGRA
{
    uint8_t b;
    uint8_t g;
    uint8_t r;
    uint8_t a;
};

BGRA pals[0x100];

HANDLE fil, fil2;
OVERLAPPED ovr, ovr2;
string str;

string char_name;

Sound qqww;

uint32_t snd_n;
uint8_t* snd_m;

uint32_t num_mov;

uint32_t cho_num;

uint8_t* pkd = nullptr;
uint8_t* unpkd = nullptr;

uint32_t tm_num;

hRen zap;
hRen* zps;

uint32_t new_size = 0;
uint8_t progbar = 0;

uint8_t* filp;

BGRA* tmppal;

BGRA* pallt[8];

uint8_t* mempal;

bool est;

uint32_t* bigunk;
bool beefedup;
uint32_t beefval = 0xBEEFFACE;

void memset32( void * dest, uint32_t value, uintptr_t size )
{
    //for detecting unwritten memory with poison value
    uintptr_t i;
    for( i = size - 4; i > 4; i -= 4 )
    {
        memcpy(((char*)dest) + i, &value, 4 );
    }  
    for( ; i > 0; i-- )
    {
        ((char*)dest)[i] = 0xFF;
    }  
}


void toChar(ofstream &file, uint8_t chars[], uint8_t size, uint32_t val)
{
    //using little endian
    for (uint8_t i = 0; i < size; i++)
    {
        chars[i] = (val >> (8 * i)) & 0xFF;
    }
    
    file.write(reinterpret_cast<const char*>(chars), size);
}

void PPMWrite(string directory, string name, uint32_t height, uint32_t width, uint8_t* start, BGRA* pallete)
{
    //creates RGB .ppm and Alpha .pbm and uses ImageMagick to combine them into a .png 
    
    ofstream imgppm (directory + name + string(".ppm"), ios::binary);
    uint8_t pbt;
    
    //ppm header info
    imgppm << "P3"; //P3 - binary full color
    imgppm.put(0x0a);
    imgppm << width << " " << height;
    imgppm.put(0x0a);
    imgppm << "255";
    imgppm.put(0x0a);
    
    ofstream imgpbm (directory + name + string("a.pbm"));

    //pbm header info
    imgpbm << "P1" << endl; //P1 - ascii black and white
    imgpbm << width << " " << height << endl;


    for (uint32_t j = 0; j < height; j++)
    {
        for (uint32_t i = 0; i < width; i++)
        {
            pbt = *(start + i + (j * width));
            imgppm << +pallete[pbt].r << " " << +pallete[pbt].g << " " << +pallete[pbt].b << endl;
            
            //.player files use #00000000 as the "true" transparency value 
            if(pallete[pbt].r == 0 && pallete[pbt].g == 0 && pallete[pbt].b == 0)
            {
                imgpbm << "1";
            }
            else
            {
                imgpbm << "0";
            }
        }
        imgpbm << endl;
    }
    imgppm.close();
    imgpbm.close();

    //copies alpha from .pbm to .ppm
    string com = string("magick ") 
                + directory + name + string("a.pbm ") + string("-alpha off ") 
                + directory + name + string(".ppm ") 
                + string("+swap  -compose copyalpha -composite ") 
                + directory + name + string(".png");
    
    system(com.c_str());
    
    remove((directory + name + string(".ppm")).c_str());
    remove((directory + name + string("a.pbm")).c_str());
}




void Extract(uint8_t* dst, uint8_t* src, uint32_t size, uint32_t newsize = 0, uint32_t pij = 0)
{
    //Extracts RLE encoded images
    uint32_t pos = 0;
    uint32_t pos2 = 0;
    uint32_t tmp = 0;
    uint32_t tmp2 = 0;
    uint32_t tmp3 = 0;
    uint32_t teax = 0;
    uint8_t bt = 0;

    while (pos < size)
    {
        if (pos2 >= newsize && newsize != 0)
        {
            cout << "overflow" << size << endl;
            break;
        }
        
        //takes 2 most significant bytes as type of encoding
        tmp = *(src + pos);
        tmp2 = tmp >> 6;
        tmp = tmp & 0x3F;


        if (tmp == 0) //if lower 6 are empty, read the next 1-2 bytes
        {
            pos++;
            tmp = *(src + pos);
            if (tmp != 0) //read +1 bytes
            {
                tmp = tmp + 0x3F; //0x3F = 00111111
            } 
            else //read +2 bytes
            {
                tmp = *(reinterpret_cast<uint16_t*>(src + pos + 1));
                tmp3 = (*(src + pos + 3)) << 0x10;
                tmp = tmp + tmp3 + 0x13F; //0x13F = 100111111
                pos = pos + 3;
            }
            
        }

       if(tmp2 == 0) //fill certain number of bytes with 0
       {
            teax = tmp;
            if(teax != 0)
            {
                tmp3 = tmp;
                for (size_t i = 0; i < tmp3; i++)
                {
                    if (pos2 + i > newsize && newsize != 0)
                    {
                        cout << "error0" << endl;
                        break;
                    }
                    *(dst + pos2 + i) = 0; //fill 0
                }
                pos2 = pos2 + tmp3;
            }
       } 
       else if (tmp2 == 1) //transfer the next bytes 1:1
       {
            tmp3 = tmp;
            if(tmp3 != 0)
            {
                //cout << tmp << endl;
                while (tmp > 0)
                {
                    bt = *(src + pos + 1); //copy
                    pos++;
                    *(dst + pos2) = bt; //paste
                    pos2++;
                    if (pos2 > newsize && newsize != 0)
                    {
                        cout << "error1" << endl;
                        break;
                    }
                    tmp = tmp - 1;
                } 
            }
       }
       else if (tmp2 == 2) //copy the next byte a certain number of times
       {
            bt = *(src + pos + 1); //copy
            pos++;
            tmp3 = tmp;
        
            if (tmp3 != 0)
            {
                tmp = tmp;
                for (size_t i = 0; i < tmp3; i++)
                {
                    if (pos2 + i > newsize && newsize != 0)
                    {
                        cout << "error2" << endl;
                        break;
                    }
                    *(dst + pos2 + i) = bt; //paste
                }
                pos2 = pos2 + tmp;
            }  
       }
       else if (tmp2 == 3) //repeat a section of bytes from earlier in dst
       {
            pos++;
            tmp3 = *(src + pos);
            if(tmp3 == 0)
            {
                pos++; 
                teax = *(src + pos);
                teax = ((teax + 1) << 8);
                tmp3 = teax;
                pos++;
            }
            teax = pos2 - tmp3; //find starting position
            tmp3 = tmp;
            if(tmp3 != 0)
            {
                while (tmp > 0)
                {
                    *(dst + pos2) = *(dst + teax); //copy from earlier
                    teax++;
                    pos2++;
                    if (pos2 > newsize && newsize != 0)
                    {
                        cout << "error3" << endl;
                        break;
                    }
                    tmp = tmp - 1;
                }
                
            }
        }
        pos++;
    }
}



int main()
{
    string pfile_name;
    cout << "    ______         ____                       ____                   "  << endl <<  "   / ____/___     / __ \\____  ____  ____ _   / __ \\____  ____  ____ _"  << endl <<   "  / / __/ __ \\   / / / / __ \\/ __ \\/ __ `/  / / / / __ \\/ __ \\/ __ `/"  << endl <<  " / /_/ / /_/ /  / /_/ / /_/ / / / / /_/ /  / /_/ / /_/ / / / / /_/ / "  << endl <<  " \\____/\\____/  /_____/\\____/_/ /_/\\__, /  /_____/\\____/_/_/_/\\__, /  "  << endl <<  "                    \\ \\/ /___  __/____/___________  / / __/ /____/   "  << endl <<  "                     \\  / __ \\/ / / / ___/ ___/ _ \\/ / /_            "  << endl <<  "                     / / /_/ / /_/ / /  (__  )  __/ / __/            "  << endl <<  "                    /_/\\____/\\__,_/_/  /____/\\___/_/_/               "  << endl <<
    "* STEP 1 " << endl <<
    "* Converts .player file into .png and .wav " << endl <<
    "* v1.1 - snox - 08/2023 " << endl << endl;
    cout << "Enter character name without file extension (case sensitive): "; cin >> pfile_name; cout << endl;
    
    
    vector<hRen*> slst;
    vector<BGRA*> plst;
    
    ovr.Offset = 0;
    ovr.OffsetHigh = 0;
    ovr.Internal = 0;
    ovr.InternalHigh = 0;
    cout << "Opening file... ";
    str = string("./") + pfile_name + string(".player");
    //cout << str.c_str() << endl;
    fil = CreateFile(str.c_str(), GENERIC_ALL, 
                                FILE_SHARE_READ, 
                                nullptr, 
                                OPEN_EXISTING, 
                                FILE_ATTRIBUTE_NORMAL, 
                                nullptr);
    if(fil == INVALID_HANDLE_VALUE) //error usually caused by lack of admin access
    {
        DWORD errorCode = GetLastError();
        cerr << "Error opening file." << errorCode << endl;
        if (errorCode == ERROR_ACCESS_DENIED) {
            cout << "Access denied. Check file permissions or run as administrator." << endl;
        }
        CloseHandle(fil);
        system("pause >nul");
        return 1;
    }

    //file magic word 2DKGT2K/2DKGT2G
    ReadFileEx(fil, const_cast<char*>(str.c_str()), 0x10, &ovr, nullptr);
    ovr.Offset += 0x10;
    //cout << "0+16: " << str << endl;


    //internal char name
    ReadFileEx(fil, const_cast<char*>(char_name.c_str()), 0x100, &ovr, nullptr);
    ovr.Offset += 0x100;
    //cout << "16+255: " << char_name << endl;


    //num of moves
    num_mov = 0;
    ReadFileEx(fil, &num_mov, 4, &ovr, nullptr);
    ovr.Offset += 4;
    //cout << "num moves: " << num_mov << endl;


    ovr.Offset = 0x114 + (num_mov * 0x27);
    //cout << "offset" << ovr.Offset << endl;

    cho_num = 0;
    ReadFileEx(fil, &cho_num, 4, &ovr, nullptr);
    ovr.Offset += 4;

    ovr.Offset += (cho_num << 4);
    
    tm_num = 0;
    ReadFileEx(fil, &tm_num, 4, &ovr, nullptr);
    ovr.Offset += 4;
    cout << "Done!" << endl;

    cout << "Creating directories... ";

    string dir = string("./") + pfile_name;
    fs::create_directories(dir);
    
    dir = dir + string("/");

    /* TODO for global pallette images 
    for (uint8_t i = 1; i <= 7; i++)
    {
        fs::create_directories(dir + to_string(i));
    }
    */

    fs::create_directories(dir + string("snd"));

    cout << "Done!" << endl;
    //reading and expanding begin
    cout << "Reading images... ";
    progbar = 0;
    for (uint32_t ij = 0; ij < tm_num; ij++)
    {
        zps = new hRen;
        memset(reinterpret_cast<void*>(zps), 0, sizeof(hRen));

        est = false;

        //fills zps
        ReadFileEx(fil, reinterpret_cast<char*>(zps), 0x14, &ovr, nullptr);
        zps->ofs = ovr.Offset;

        ovr.Offset += 0x14;
            
        if(zps->size != 0) //compressed
        {
            
            pkd = new uint8_t[zps->size];
            memset(pkd, 0, zps->size);
            
            if(zps->pal == 1) //create space for palette
            {
                new_size = zps->w * zps->h + 0x400;
            } 
            else
            {
                new_size = zps->w * zps->h;
            }
            
            unpkd = new uint8_t[new_size];
            memset32(unpkd, beefval, new_size);
            ReadFileEx(fil, pkd, zps->size, &ovr, nullptr);
            // cout << zps->size << endl; 
            // cout << new_size << endl; 
            

            ovr.Offset += zps->size;
            

            Extract(unpkd, pkd, zps->size, new_size, ij);

            delete[] pkd;


            est = true;
        }
        else if (zps->w != 0 && zps->h != 0) //uncompressed but not blank
        {
            if(zps->pal == 1)
            {
                new_size = zps->w * zps->h + 0x400;
            } 
            else
            {
                new_size = zps->w * zps->h;
            }
            unpkd = new uint8_t[new_size+0x10];
            ReadFileEx(fil, unpkd, new_size, &ovr, nullptr);
            ovr.Offset += new_size;

            est = true;
        }
        if(est)
        {
            zps->unk = (unpkd);
            zps->size = new_size;
            slst.push_back(zps);
            // cout << slst.size() << endl;
        }
        else  //if blank
        {
            delete zps;
        }
    }
    
    cout << "Done!" << endl;
    cout << "Extracting pallettes... ";
    
    mempal = new uint8_t[0x2100]; //8 palletes
    ReadFileEx(fil, mempal, 0x2100, &ovr, nullptr);
    ovr.Offset += 0x2100;
    cout << ovr.Offset << endl;
    cerr << "dave";
    ReadFileEx(fil, &snd_n, 4, &ovr, nullptr);

    ovr.Offset += 4;
    cout << "Done!" << endl;
    cout << "Creating sounds... 0/" << snd_n;

    for (uint32_t ij = 0; ij < snd_n; ij++)
    {
        //sounds are stored as uncompressed .wavs
        memset(&qqww, 0, 0x2A);
        ReadFileEx(fil, &qqww, 0x2A, &ovr, nullptr);
        ovr.Offset += 0x2A;

        if(qqww.size != 0)
        {
            snd_m = new uint8_t[qqww.size];
            ReadFileEx(fil, snd_m, qqww.size, &ovr, nullptr);
            ovr.Offset += qqww.size;

            ovr2.Offset = 0;
            ovr2.OffsetHigh = 0;
            ovr2.Internal = 0;
            ovr2.InternalHigh = 0;

            str = dir + string("snd/") + to_string(ij) + string(".wav");

            fil2 = CreateFile(str.c_str(), GENERIC_ALL, FILE_SHARE_READ || FILE_SHARE_WRITE, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
            WriteFileEx(fil2, snd_m, qqww.size, &ovr2, nullptr);
            CloseHandle(fil2);
            delete snd_m;
            cout << "\r" << "Creating sounds... " << (ij + 1) << "/" << (snd_n);
        }
        
    }
    
    cout << "\r" << "Creating sounds... Done!" << endl;

    CloseHandle(fil);

    //creating images
    
    for (int ij = 0; ij <= 7; ij++)
    {
        pallt[ij] = reinterpret_cast<BGRA*>(mempal + ij * 0x420);
    }
    fs::create_directories(dir + "main");
    fs::create_directories(dir + "unchanged");

    uint8_t nibble[4];
    uint8_t bytes[2];
    //creating DongDongImage file
    //stores information about each image to be referenced later
    ofstream ddi (dir + "full.ddi", ios::binary);
    toChar(ddi, nibble, 4, 0x000010DD);
    toChar(ddi, bytes, 2, slst.size());
    ddi.flush();
    cout << "Converting images... 0/" << slst.size();
    for (uint32_t ij = 0; ij < slst.size(); ij++)
    {
        zps = slst[ij];

        if (zps->pal == 1) //if image contains it's own palette
        {
            
            tmppal = reinterpret_cast<BGRA*>(zps->unk);

            //some images only have a 512 byte palette before the image begins as opposed to the presumed 1024
            //this doesn't cause issues upon reading because of how size is stored within the header
            //however this 512 vs 1024 information is located at an unknown location earlier in the file
            //and so we check for 512 bytes of a poison value (0xBEEFFACE) at the end of image to determine it
            bigunk = reinterpret_cast<uint32_t*>(zps->unk + (zps->h * zps->w) + 0x200);
            if (*(bigunk) == beefval) 
            {
                beefedup = true;
                for (int i = 1; i < ((0x200 / 4) - 2) ; i++)
                {
                    if (*(bigunk + i) != beefval)
                    {
                        beefedup = false;
                    }
                }
                if (*(zps->unk + 0x200) != 0xCE) //poison value is written back to front so this will always align
                {
                    beefedup = false;
                }
                if (beefedup)
                {
                    filp = (zps->unk + 0x200);
                    //cout << "so beefy!";
                }
                else
                {
                    filp = (zps->unk + 0x400);
                } 
            }
            else
            {
                filp = (zps->unk + 0x400);
            }
            
            
            PPMWrite(dir + "main/", to_string(ij), zps->h, zps->w, filp, tmppal);

            //writes all necessary zps information to .ddi
            toChar(ddi, bytes, 2, ij);
            toChar(ddi, nibble, 4, zps->ofs);
            toChar(ddi, bytes, 2, zps->w);
            toChar(ddi, bytes, 2, zps->h);
            ddi.flush();
        }
        else //image uses global palette
        {
            //DDND doesn't really make use of this feature, so this should never really be an issue
            //technically untested
            for (int pk = 0; pk < 7; pk++)
            {
                tmppal = pallt[pk];
                filp = zps->unk;
                PPMWrite((dir + "main/"), (to_string(pk) + "p" + to_string(ij)), zps->h, zps->w, filp, tmppal);
            }
        }
        cout << "\r" << "Converting images... " << (ij + 1) << "/" << (slst.size());    
    }
    ddi.close();

    cout << "\r" << "Converting images... Done!     " << endl;
    cout << endl << "Press any key to close." << endl;
    system("pause >nul");
    return 0;
}