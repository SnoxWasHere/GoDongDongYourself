#include <filesystem>
#include <iostream>
#include "header\playerfile.hpp"

using std::cout;
using std::endl;

//private
void PlayerFile::extract(uint8_t* dst, uint8_t* src, uint32_t size, uint32_t newsize, uint32_t pij)
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

//public
PlayerFile::PlayerFile(std::string file)
{
    util::Status status("Opening file");
    fileName = file;
    std::string fileDir = "./";
    _ovr.Offset = 0;
    _ovr.OffsetHigh = 0;
    _ovr.Internal = 0;
    _ovr.InternalHigh = 0;
    _playerFile = CreateFile((fileDir + fileName + ".player").c_str(), 
                            GENERIC_ALL, 
                            FILE_SHARE_READ, 
                            nullptr, 
                            OPEN_EXISTING, 
                            FILE_ATTRIBUTE_NORMAL, 
                            nullptr);
    if(_playerFile == INVALID_HANDLE_VALUE) //error usually caused by lack of admin access
    {
        DWORD errorCode = GetLastError();
        std::cerr << "Error opening file - " << errorCode << endl;
        if (errorCode == ERROR_ACCESS_DENIED) {
            cout << "Access denied. Check file permissions or run as administrator." << endl;
        }
        CloseHandle(_playerFile);
        system("pause >nul");
        exit(errorCode);
    }
}
void PlayerFile::readHeader()
{
    util::Status status("Reading header");
    //file magic word 2DKGT2K/2DKGT2G
    _ovr.Offset += 0x10;

    //internal char name
    ReadFileEx(_playerFile, &charName, 0x100, &_ovr, nullptr);
    _ovr.Offset += 0x100;

    //num of moves
    uint32_t numMoves = 0;
    ReadFileEx(_playerFile, &numMoves, 4, &_ovr, nullptr);
    _ovr.Offset += 4;

    _ovr.Offset += numMoves * 0x27;

    uint32_t choNum = 0;
    ReadFileEx(_playerFile, &choNum, 4, &_ovr, nullptr);
    _ovr.Offset += 4;

    _ovr.Offset += (choNum << 4);
    
    ReadFileEx(_playerFile, &headerCount, 4, &_ovr, nullptr);
    _ovr.Offset += 4;
}
void PlayerFile::createDir()
{
    util::Status status("Creating directories");
    outputDir = "./";
    outputDir += fileName;
    std::filesystem::create_directories(outputDir);
    
    outputDir += "/";
    std::filesystem::create_directories(outputDir + "snd");
    std::filesystem::create_directories(outputDir + "main");
    std::filesystem::create_directories(outputDir + "unchanged");
}
void PlayerFile::readImages()
{
    util::Status status("Reading images");
    uint8_t* unpkd;
    uint8_t* pkd;
    uint32_t new_size = 0;
    for (uint32_t ij = 0; ij < headerCount; ij++)
    {
        zps = new util::hRen;
        memset(reinterpret_cast<void*>(zps), 0, sizeof(util::hRen));

        bool est = false;
        //fills zps
        ReadFileEx(_playerFile, reinterpret_cast<char*>(zps), 0x14, &_ovr, nullptr);
        zps->ofs = _ovr.Offset;

        _ovr.Offset += 0x14;
            
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
            util::memset32(unpkd, BEEFVAL, new_size);
            ReadFileEx(_playerFile, pkd, zps->size, &_ovr, nullptr);
            // cout << zps->size << endl; 
            // cout << new_size << endl; 
            _ovr.Offset += zps->size;
            extract(unpkd, pkd, zps->size, new_size, ij);
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
            ReadFileEx(_playerFile, unpkd, new_size, &_ovr, nullptr);
            _ovr.Offset += new_size;

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
}
void PlayerFile::readPallettes()
{
    util::Status status("Extracting pallettes");
    mempal = new uint8_t[0x2100]; //8 palletes
    ReadFileEx(_playerFile, mempal, 0x2100, &_ovr, nullptr);
    _ovr.Offset += 0x2100;
}
void PlayerFile::createSounds()
{
    uint32_t snd_n;
    ReadFileEx(_playerFile, &snd_n, 4, &_ovr, nullptr);
    util::Status status("Creating sounds", snd_n);
    _ovr.Offset += 4;
    for (uint32_t ij = 0; ij < snd_n; ij++)
    {
        //sounds are stored as uncompressed .wavs
        memset(&qqww, 0, 0x2A);
        ReadFileEx(_playerFile, &qqww, 0x2A, &_ovr, nullptr);
        _ovr.Offset += 0x2A;

        if(qqww.size != 0)
        {
            uint8_t* snd_m = new uint8_t[qqww.size];
            ReadFileEx(_playerFile, snd_m, qqww.size, &_ovr, nullptr);
            _ovr.Offset += qqww.size;

            OVERLAPPED ovr2;
            ovr2.Offset = 0;
            ovr2.OffsetHigh = 0;
            ovr2.Internal = 0;
            ovr2.InternalHigh = 0;

            std::string str = outputDir + std::string("snd/") + std::to_string(ij) + std::string(".wav");

            HANDLE fil2 = CreateFile(str.c_str(), GENERIC_ALL, FILE_SHARE_READ || FILE_SHARE_WRITE, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
            WriteFileEx(fil2, snd_m, qqww.size, &ovr2, nullptr);
            CloseHandle(fil2);
            delete[] snd_m;
            
        }
        status.update(ij + 1);
    }
}
void PlayerFile::createImages()
{
    util::Status status("Creating images", slst.size());
    util::BGRA* pallt[8];

    for (int ij = 0; ij <= 7; ij++)
    {
        pallt[ij] = reinterpret_cast<util::BGRA*>(mempal + ij * 0x420);
    }
    uint8_t nibble[4];
    uint8_t bytes[2];
    //creating DongDongImage file
    //stores information about each image to be referenced later
    std::ofstream ddi (outputDir + "full.ddi", std::ios::binary);
    util::toChar(ddi, nibble, 4, 0x000010DD);
    util::toChar(ddi, bytes, 2, slst.size());
    ddi.flush();

    bool beefedup;
    uint8_t* filp;
    util::BGRA* tmppal;
    RawImage::setDir("./KEN/main/");

    for (uint32_t ij = 0; ij < slst.size(); ij++)
    {
        zps = slst[ij];

        if (zps->pal == 1) //if image contains it's own palette
        {
            tmppal = reinterpret_cast<util::BGRA*>(zps->unk);

            //some images only have a 512 byte palette before the image begins as opposed to the presumed 1024
            //this doesn't cause issues upon reading because of how size is stored within the header
            //however this 512 vs 1024 information is located at an unknown location earlier in the file
            //and so we check for 512 bytes of a poison value (0xBEEFFACE) at the end of image to determine it
            uint32_t* bigunk = reinterpret_cast<uint32_t*>(zps->unk + (zps->h * zps->w) + 0x200);
            if (*bigunk == BEEFVAL) 
            {
                beefedup = true;
                for (int i = 1; i < ((0x200 / 4) - 2) ; i++)
                {
                    if (*(bigunk + i) != BEEFVAL)
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
            
            
            util::rawWrite(std::to_string(ij), zps->w, zps->h, filp, tmppal);

            //writes all necessary zps information to .ddi
            util::toChar(ddi, bytes, 2, ij);
            util::toChar(ddi, nibble, 4, zps->ofs);
            util::toChar(ddi, bytes, 2, zps->w);
            util::toChar(ddi, bytes, 2, zps->h);
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
                util::rawWrite(std::to_string(pk) + "p" + std::to_string(ij), zps->w, zps->h, filp, tmppal);
            }
        }
        status.update(ij + 1);
    }
    ddi.close();
}


PlayerFile::~PlayerFile()
{
    CloseHandle(_playerFile);
}

