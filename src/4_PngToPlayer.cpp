/**********************************************
* █▀▀ █▀█   █▀▄ █▀█ █▄ █ █▀▀   █▀▄ █▀█ █▄ █ █▀▀
* █▄█ █▄█   █▄█ █▄█ █ ▀█ █▄█   █▄█ █▄█ █ ▀█ █▄█
*            █▄█ █▀█ █ █ █▀█ █▀ █▀▀ █   █▀▀
*             █  █▄█ █▄█ █▀▄ ▄█ ██▄ █▄▄ █▀ 
***********************************************
* STEP 4
* Writes .ddp and .BGRA files into the new player file
* v1.1 - snox - 08/2023
*/

#include <cstdlib>
#include <stdint.h>
#include <iostream>
#include <fstream>
#include <filesystem>
#include <Windows.h>
#include <vector>
namespace fs = std::filesystem;
using namespace std;

struct iRen //stores info from .ddi
{
    uint16_t num;
    uint16_t ofsa;
    uint16_t ofsb;
    uint16_t w;
    uint16_t h;
    uint32_t ofs;
};

iRen dat;
iRen* dts; 

HANDLE fil;
OVERLAPPED ovr;

char* buffer;

int getLength(ifstream &filed)
{
    filed.seekg (0, filed.end);
    int length = filed.tellg();
    filed.seekg (0, filed.beg);
    return length;
}

bool isEmpty(char* buf, int size) 
{
    for(int i = 0; i < size; i++) 
    {
        if(buf[i] != 0) return false;
    }
    return true;
}



int main()
{
    string pfile_name;
    cout << "    ______         ____                       ____                   "  << endl <<  "   / ____/___     / __ \\____  ____  ____ _   / __ \\____  ____  ____ _"  << endl <<   "  / / __/ __ \\   / / / / __ \\/ __ \\/ __ `/  / / / / __ \\/ __ \\/ __ `/"  << endl <<  " / /_/ / /_/ /  / /_/ / /_/ / / / / /_/ /  / /_/ / /_/ / / / / /_/ / "  << endl <<  " \\____/\\____/  /_____/\\____/_/ /_/\\__, /  /_____/\\____/_/_/_/\\__, /  "  << endl <<  "                    \\ \\/ /___  __/____/___________  / / __/ /____/   "  << endl <<  "                     \\  / __ \\/ / / / ___/ ___/ _ \\/ / /_            "  << endl <<  "                     / / /_/ / /_/ / /  (__  )  __/ / __/            "  << endl <<  "                    /_/\\____/\\__,_/_/  /____/\\___/_/_/               "  << endl <<
    "* STEP 4 " << endl <<
    "* Writes .ddp and .BGRA files into the new player file " << endl <<
    "* v1.1 - snox - 08/2023 " << endl << endl;
    
    cout << "Enter character name without file extension (case sensitive): "; cin >> pfile_name; cout << endl;
    
    string dir = ".\\" + pfile_name + string("\\");
    string dirt = dir + "temp\\";
    
    vector<iRen*> dlst;
    vector<bool> changed;

    ovr.Offset = 0;
    ovr.OffsetHigh = 0;
    ovr.Internal = 0;
    ovr.InternalHigh = 0;
    cout << "Opening file... ";

    fil = CreateFile((dir + "full.ddi").c_str(), GENERIC_ALL, FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
    if(fil == INVALID_HANDLE_VALUE)
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

    ovr.Offset += 4;
    uint16_t num_img = 0;
    ReadFileEx(fil, &num_img, 2, &ovr, nullptr);
    ovr.Offset += 2;
    
    for (uint16_t i = 0; i < num_img; i++)
    {
        dts = new iRen;
        memset(dts, 0, sizeof(iRen));
        ReadFileEx(fil, reinterpret_cast<char*>(dts), 0xA, &ovr, nullptr);
        
        //reading to a struct of different types of ints is difficult so this is the solution
        dts->ofs = (dts->ofsa) | (dts->ofsb << 16);
        
        ovr.Offset += 0xA;

        dlst.push_back(dts);

        if (fs::exists(dirt + to_string(i) + "e.ddp")) //check if changed
        {
            changed.push_back(1);
        }
        else
        {
            changed.push_back(0);
        }
    }
    
    CloseHandle(fil);

    ovr.Offset = 0;
    ovr.OffsetHigh = 0;
    ovr.Internal = 0;
    ovr.InternalHigh = 0;
    
    fil = CreateFile((".\\" + pfile_name + string(".player")).c_str(), GENERIC_ALL, FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
    if(fil == INVALID_HANDLE_VALUE)
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

    ovr.Offset += 0x10;

    ovr.Offset += 0x100;

    //partially reread player file like in step 1
    uint32_t pum_pov = 0;
    ReadFileEx(fil, &pum_pov, 4, &ovr, nullptr);
    ovr.Offset += 4;

    ovr.Offset = 0x114 + (pum_pov * 0x27);

    uint32_t cho_num = 0;
    ReadFileEx(fil, &cho_num, 4, &ovr, nullptr);
    ovr.Offset += 4;

    ovr.Offset += (cho_num << 4);
    
    uint32_t tm_num = 0;
    ReadFileEx(fil, &tm_num, 4, &ovr, nullptr);
    ovr.Offset += 4;

    uint32_t headstart = ovr.Offset;

    CloseHandle(fil);

    cout << "Done!" << endl;

    cout << "Initializing character file... ";


    ifstream player ((".\\" + pfile_name + string(".player")).c_str(), ifstream::binary);
    int plength = getLength(player);
    
    ofstream dest ((dir + pfile_name + string("modded.player")).c_str(), ios::binary);

    //copy all unchanged information
    buffer = new char[headstart];
    memset(buffer, 0, headstart);
    player.read(buffer, headstart);
    dest.write(buffer, headstart);
    dest.flush();

    delete[] buffer;
    uint32_t ploc = headstart;

    player.seekg(ploc, player.beg);
    uint16_t count = 0;

    char* blankbuf = new char[20];
    memset(blankbuf, 0, 20);

    cout << "Done!" << endl;

    cout << "Writing images... 0/" << to_string(dlst.size());

    //begin going through each image
    while (count < dlst.size())
    {
        if (ploc != dlst[count]->ofs) //skip over blank space in player file
        {
            buffer = new char[dlst[count]->ofs - ploc];
            memset(buffer, 0, dlst[count]->ofs - ploc);
            player.read(buffer, dlst[count]->ofs - ploc);

            dest.write(buffer, dlst[count]->ofs - ploc);
            delete[] buffer;
            ploc = dlst[count]->ofs;
        }
        
        else if (changed[count] == 1)
        {
            ifstream ddp ((dirt + to_string(dlst[count]->num) + "e.ddp").c_str(), ifstream::binary);
            int ddlength = getLength(ddp);
            
            buffer = new char[20];
            memset(buffer, 0, 20);
            
            player.read(buffer, 20);
            ploc += 20;
            player.seekg(ploc, player.beg);
            

            if (isEmpty(buffer, 20)) //don't increase count
            {
                dest.write(blankbuf, 20);
                delete[] buffer;
            }  
            else
            {
                dest.write(buffer, 16);
                dest.write(blankbuf, 4); //set size equal to 0
                //this allows us to write the uncompressed data 1:1 without trouble 

                uint32_t sizzle = 0;
                sizzle |= static_cast<uint8_t>(buffer[16]);
                sizzle |= static_cast<uint8_t>(buffer[17]) << 8;
                sizzle |= static_cast<uint8_t>(buffer[18]) << 16;
                sizzle |= static_cast<uint8_t>(buffer[19]) << 24;
                

                if (buffer[12] != 0) //checking for palette
                {
                    delete[] buffer;
                    ifstream prga ((dirt + to_string(dlst[count]->num) + "p.RGBA").c_str(), ifstream::binary);
                    int prglength = getLength(prga);
                    buffer = new char[1024];
                    memset(buffer, 0, 1024);
                    prga.read(buffer, prglength);

                    for (int ip = 0; ip < 1024; ip+= 4) //rearrange RGBA to BGRA
                    {
                        if(ip < prglength)
                        {
                            dest.put(buffer[ip + 2]);
                            dest.put(buffer[ip + 1]);
                            dest.put(buffer[ip + 0]);
                            dest.put(buffer[ip + 3]);
                        }
                        else
                        {  
                            dest.write(blankbuf, 4);
                        }
                    }

                    dest.flush();
                    prga.close();
                    //as of v1.1, a 1024 byte palette is presumed
                    //thankfully all of the 512 images are usually just particle effects anyways
                }
                delete[] buffer;
                //copy-paste entire .ddp
                buffer = new char[ddlength];
                memset(buffer, 0, ddlength);
                ddp.read(buffer, ddlength);

                dest.write(buffer, ddlength);
                delete[] buffer;
                
                ploc += sizzle;

                count++;
                cout << "\r" << "Writing images... " << (count) << "/" << (dlst.size());
                ddp.close();
            }
        }
        else //unchanged image
        {
            buffer = new char[20];
            memset(buffer, 0, 20);
            
            player.read(buffer, 20);
            ploc += 20;
            player.seekg(ploc, player.beg);

            if (isEmpty(buffer, 20)) //count not increased
            {
                dest.write(blankbuf, 20);
                delete[] buffer;
            }  
            else
            {
                dest.write(buffer, 20);
                delete[] buffer;
                if (count + 1 < dlst.size()) //overflow protection
                {
                    uint32_t highwater = dlst[count + 1]->ofs - dlst[count]->ofs - 20;
                    buffer = new char[highwater];
                    memset(buffer, 0, highwater);
                    player.read(buffer, highwater);
                    
                    dest.write(buffer, highwater);
                    ploc += highwater;
                    delete[] buffer;
                }
                //it's fine to just ignore the last image if it's unchanged because of the next step
                count++;
                cout << "\r" << "Writing images... " << (count) << "/" << (dlst.size());
            }
        }
        
        dest.flush();
        player.seekg(ploc, player.beg);
    }
    //copy everything else
    buffer = new char[plength - ploc];
    memset(buffer, 0, plength - ploc);
    player.read(buffer, plength - ploc);
    
    dest.write(buffer, plength - ploc);
    delete[] buffer;

    cout << "\r" << "Writing images... Done!       " << endl << endl;
    cout << "Rename modded player file to the original and replace it in the game directory." << endl;
    cout << "Press any key to close." << endl;
    system("pause >nul");

    //thank you for using my program!
    // <3 snox
    return 0;
}