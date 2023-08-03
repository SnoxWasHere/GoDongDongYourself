/**********************************************
* █▀▀ █▀█   █▀▄ █▀█ █▄ █ █▀▀   █▀▄ █▀█ █▄ █ █▀▀
* █▄█ █▄█   █▄█ █▄█ █ ▀█ █▄█   █▄█ █▄█ █ ▀█ █▄█
*            █▄█ █▀█ █ █ █▀█ █▀ █▀▀ █   █▀▀
*             █  █▄█ █▄█ █▀▄ ▄█ ██▄ █▄▄ █▀ 
***********************************************
* STEP 3
* Converts grids into separate .RGBA image and palette files, along with a preview .png 
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

struct mRen //stores info from .ddg
{
    uint8_t num;
    uint8_t w;
    uint8_t h;
    uint8_t count;
    uint16_t imgs[16];
};

mRen mon;
mRen* mts;

HANDLE fil;
OVERLAPPED ovr;

int main()
{
    vector<iRen*> dlst;
    vector<mRen*> mlst;

    string pfile_name;
    cout << "    ______         ____                       ____                   "  << endl <<  "   / ____/___     / __ \\____  ____  ____ _   / __ \\____  ____  ____ _"  << endl <<   "  / / __/ __ \\   / / / / __ \\/ __ \\/ __ `/  / / / / __ \\/ __ \\/ __ `/"  << endl <<  " / /_/ / /_/ /  / /_/ / /_/ / / / / /_/ /  / /_/ / /_/ / / / / /_/ / "  << endl <<  " \\____/\\____/  /_____/\\____/_/ /_/\\__, /  /_____/\\____/_/_/_/\\__, /  "  << endl <<  "                    \\ \\/ /___  __/____/___________  / / __/ /____/   "  << endl <<  "                     \\  / __ \\/ / / / ___/ ___/ _ \\/ / /_            "  << endl <<  "                     / / /_/ / /_/ / /  (__  )  __/ / __/            "  << endl <<  "                    /_/\\____/\\__,_/_/  /____/\\___/_/_/               "  << endl <<
    "* STEP 3 " << endl <<
    "* Converts grids into separate .RGBA image and palette files, along with a preview .png " << endl <<
    "* v1.1 - snox - 08/2023 " << endl << endl;
    
    cout << "Enter character name without file extension (case sensitive): "; cin >> pfile_name; cout << endl;
    
    string dir = ".\\" + pfile_name + string("\\");



    ovr.Offset = 0;
    ovr.OffsetHigh = 0;
    ovr.Internal = 0;
    ovr.InternalHigh = 0;
    cout << "Opening files... ";
    
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
    }
    

    CloseHandle(fil);

    ovr.Offset = 0;
    ovr.OffsetHigh = 0;
    ovr.Internal = 0;
    ovr.InternalHigh = 0;    

    fil = CreateFile((dir + "full.ddg").c_str(), GENERIC_ALL, FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
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
    uint32_t num_grids = 0;
    ReadFileEx(fil, &num_grids, 4, &ovr, nullptr);
    ovr.Offset += 4;

    
    uint16_t* inum;

    for (uint16_t i = 0; i < num_grids; i++)
    {
        //header
        mts = new mRen;
        memset(mts, 0, sizeof(mRen));
        inum = new uint16_t;
        memset(inum, 0, sizeof(uint16_t));
        ReadFileEx(fil, reinterpret_cast<char*>(mts), 4, &ovr, nullptr);
        ovr.Offset += 4;

        //img numbers
        for (uint8_t i = 0; i < mts->count; i++)
        {
            ReadFileEx(fil, reinterpret_cast<char*>(inum), 2, &ovr, nullptr);
            ovr.Offset += 2;
            mts->imgs[i] = *inum;
        }
        
        mlst.push_back(mts);
        delete inum;
    }
    
    CloseHandle(fil);

    cout << "Done!" << endl;

    cout << "Creating directories... ";

    fs::create_directories(dir + string("temp"));

    string dirg = dir + "grids\\";
    dir = dir + "temp\\";

    cout << "Done!" << endl;

    cout << "Extracting grids... 0/" << num_grids;


    for (uint8_t ij = 0; ij < num_grids; ij++)
    {
        mts = mlst[ij];
        
        uint8_t ix = 0;
        uint8_t iy = 0;

        for (uint8_t i = 0; i < mts->count; i++) 
        {
            dts = dlst[mts->imgs[i]];
            //crop out each cell, apply an alpha threshold, and quantize the colors
            //png
            string com = (string("convert ") + dirg + to_string(mts->num) + string("g.png ") 
                        + string(" -crop ") + to_string(dts->w) + "x" + to_string(dts->h) 
                        + string("+") + to_string(mts->w * 10 * ix) + string("+") + to_string(mts->h * 10 * iy)
                        + string(" -channel alpha -threshold 50\% -channel RGBA -colors 254 ")
                        + dir + to_string(mts->imgs[i]) + string("e.png"));
            system(com.c_str());
            
            //raw
            system((string("convert ") + dir + to_string(mts->imgs[i]) + string("e.png ")
                    + dir + to_string(mts->imgs[i]) + string("e.RGBA")).c_str());
            //palette
            system((string("convert ") + dir + to_string(mts->imgs[i]) + string("e.png ")
                    + string(" -unique-colors ") + dir + to_string(mts->imgs[i]) + string("p.RGBA")).c_str());

            
            
            ifstream pargb ((dir + to_string(mts->imgs[i]) + string("p.RGBA")).c_str(), ifstream::binary);
            pargb.seekg (0, pargb.end);
            int plength = pargb.tellg();
            pargb.seekg (0, pargb.beg);

            char* palbuf = new char [1024];
            memset(palbuf, 0, 1024);

            pargb.read (palbuf,plength);
            //fill remainder of palette with black (00,00,00,00)

            
            ifstream eimg ((dir + to_string(mts->imgs[i]) + string("e.RGBA")).c_str(), ifstream::binary);
            eimg.seekg (0, eimg.end);
            int elength = eimg.tellg();
            eimg.seekg (0, eimg.beg);

            char* imgbuf = new char [elength];
            memset(imgbuf, 0, elength);


            eimg.read (imgbuf,elength);

            uint32_t* imgp = reinterpret_cast<uint32_t*>(imgbuf);
            uint32_t* palp = reinterpret_cast<uint32_t*>(palbuf);

            ofstream ddp ((dir + to_string(mts->imgs[i]) + string("e.ddp")).c_str(), ios::binary);

            //compare each 4 byte color in image to palette and write down the index
            for (uint32_t j = 0; j < (elength / 4); j++)
            {
                for (uint16_t jx = 0; jx < (plength / 4); jx++)
                {
                    if (*(imgp + j) == *(palp + jx))
                    {
                        ddp.put(jx);
                        break;
                    }
                    
                }
                
            }
            ddp.flush();
            ddp.close();
            delete[] palbuf;
            delete[] imgbuf;
            pargb.close();
            eimg.close();
            
            remove((dir + to_string(mts->imgs[i]) + string("e.RGBA")).c_str());
            
            if (ix == 3) //width is always 4
            {
                ix = 0;
                iy++;
            }
            else
            {
                ix++;
            }
        }
        cout << "\r" << "Extracting grids... " << to_string(ij + 1) << "/" << to_string(num_grids);
            
    }
    cout << "\r" << "Extracting grids... Done!     " << endl;
    cout << endl << "Press any key to close." << endl;
    system("pause >nul");
    return 0;
}