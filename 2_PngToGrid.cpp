/**********************************************
* █▀▀ █▀█   █▀▄ █▀█ █▄ █ █▀▀   █▀▄ █▀█ █▄ █ █▀▀
* █▄█ █▄█   █▄█ █▄█ █ ▀█ █▄█   █▄█ █▄█ █ ▀█ █▄█
*            █▄█ █▀█ █ █ █▀█ █▀ █▀▀ █   █▀▀
*             █  █▄█ █▄█ █▀▄ ▄█ ██▄ █▄▄ █▀ 
***********************************************
* STEP 2
* Combines images into more convenient spritesheets with templates 
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

void toChar(ofstream &file, uint8_t chars[], uint8_t size, uint32_t val)
{
    //using little endian
    for (uint8_t i = 0; i < size; i++)
    {
        chars[i] = (val >> (8 * i)) & 0xFF;
    }
    
    file.write(reinterpret_cast<const char*>(chars), size);
}

int maxGrid(int numToRound)
{
    //returns how many grids are going to be needed
    int remainder = numToRound % 16;
    if (remainder == 0)
        return numToRound / 16;

    return (numToRound + 16 - remainder) / 16;
}


int main()
{
    string pfile_name;
    cout << "    ______         ____                       ____                   "  << endl <<  "   / ____/___     / __ \\____  ____  ____ _   / __ \\____  ____  ____ _"  << endl <<   "  / / __/ __ \\   / / / / __ \\/ __ \\/ __ `/  / / / / __ \\/ __ \\/ __ `/"  << endl <<  " / /_/ / /_/ /  / /_/ / /_/ / / / / /_/ /  / /_/ / /_/ / / / / /_/ / "  << endl <<  " \\____/\\____/  /_____/\\____/_/ /_/\\__, /  /_____/\\____/_/_/_/\\__, /  "  << endl <<  "                    \\ \\/ /___  __/____/___________  / / __/ /____/   "  << endl <<  "                     \\  / __ \\/ / / / ___/ ___/ _ \\/ / /_            "  << endl <<  "                     / / /_/ / /_/ / /  (__  )  __/ / __/            "  << endl <<  "                    /_/\\____/\\__,_/_/  /____/\\___/_/_/               "  << endl <<
    "* STEP 2 " << endl <<
    "* Combines images into more convenient spritesheets with templates" << endl <<
    "* v1.1 - snox - 08/2023 " << endl << endl;
    cout << "Please move any images which you don't wish to replace to the 'unchanged' folder before continuing." << endl;
    cout << "Enter character name without file extension (case sensitive): "; cin >> pfile_name; cout << endl;
    
    string dir = ".\\" + pfile_name + string("\\");
    uint8_t nibble[4];
    uint8_t bytes[2];
    //creates DongDongGrid file to store info about each grid
    ofstream ddg (dir + "full.ddg", ios::binary);
    toChar(ddg, nibble, 4, 0x000060DD);
    
    vector<iRen*> dlst;

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

    cout << "Done!" << endl;

    cout << "Creating directories... ";
    fs::create_directories(dir + string("grids"));
    string dirg = dir + "grids\\";
    dir = dir + "main\\";

    for (uint16_t i = 0; i < num_img; i++)
    {
        dts = new iRen;
        memset(dts, 0, sizeof(iRen));
        ReadFileEx(fil, reinterpret_cast<char*>(dts), 0xA, &ovr, nullptr);
        
        //reading to a struct of different types of ints is difficult so this is the solution
        dts->ofs = (dts->ofsa) | (dts->ofsb << 16); 
        
        ovr.Offset += 0xA;

        if (fs::exists(dir + to_string(i) + ".png")) //checks if file has been moved to unchanged
        {
            dlst.push_back(dts);
        }
    }
    

    CloseHandle(fil);

    vector<iRen*> cgrid;
    string imgs;    //plain images
    string timgs;   //template images
    string cimgs;   //big caption
    string csimgs;  //little caption
    uint16_t bigw;
    uint16_t bigh;
    uint8_t gridnum = 0;
    

    toChar(ddg, nibble, 4, maxGrid(dlst.size()));
    ddg.flush();

    cout << "Done!" << endl;

    cout << "Creating grids... 0/" << maxGrid(dlst.size());

    for (int ij = 0; ij < dlst.size(); ij++) //loops through every image found in main
    {
        if (cgrid.size() >= 16 || ij == (dlst.size() - 1)) //grids have max size of 4x4
        {
            if (ij == (dlst.size() - 1)) //if last one, append last image
            {
                cgrid.push_back(dlst[ij]); 
            }
            for (int i = 0; i < cgrid.size(); i++)
            {
                imgs.append(dir + to_string(cgrid[i]->num) + ".png "); //adds image

                timgs.append("xc:White[" + to_string(cgrid[i]->w) + string("x") + to_string(cgrid[i]->h) + string("!] ")); //adds template

                //checks to see which image is the biggest
                if (cgrid[i]->w > bigw)
                {
                    bigw = cgrid[i]->w;
                }
                if (cgrid[i]->h > bigh)
                {
                    bigh = cgrid[i]->h;
                }
            }
            //arbitrary buffer
            bigw = (bigw / 10) * 10 + 20;
            bigh = (bigh / 10) * 10 + 20;

            uint8_t ix = 0;
            uint8_t iy = 0;

            ddg.put(gridnum);
            ddg.put(bigw / 10);
            ddg.put(bigh / 10);
            ddg.put(cgrid.size());
            ddg.flush();

            for (int i = 0; i < cgrid.size(); i++) //have to loop second time because first time determined grid cell size
            {
                cimgs.append(" -annotate +" + to_string(bigw * ix) + "+" + to_string(bigh * iy + 12) + " \"" + to_string(cgrid[i]->num) + "\" ");
                
                csimgs.append(" -annotate +" + to_string(bigw * ix) + "+" + to_string(bigh * iy + 22) + " \"" + to_string(cgrid[i]->w) + "\" ");
                csimgs.append(" -annotate +" + to_string(bigw * ix) + "+" + to_string(bigh * iy + 30) + " \"" + to_string(cgrid[i]->h) + "\" ");

                toChar(ddg, bytes, 2, cgrid[i]->num);

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
            ddg.flush();
            //create image grid and template grid
            system((string("montage ") + imgs +  " -gravity NorthWest -geometry " + string("\"") + to_string(bigw) + string("x") + to_string(bigh) + "+0+0<>\" -tile 4x -background none " + dirg + to_string(gridnum) + string("g.png")).c_str());
            system((string("montage ") + timgs +  " -gravity NorthWest -geometry " + string("\"") + to_string(bigw) + string("x") + to_string(bigh) + "+0+0<>\" -tile 4x -background none " + dirg + to_string(gridnum) + string("gt.png")).c_str());
            //overlay captions on top
            system((string("convert ") + dirg + to_string(gridnum) + string("gt.png -pointsize 14 ") + cimgs + " -pointsize 10 " + csimgs + dirg + to_string(gridnum) + string("gt.png")).c_str());
            imgs = "";
            timgs = "";
            cimgs = "";
            csimgs = "";
            bigw = 0;
            bigh = 0;
            gridnum++;
            cgrid.clear();
            cout << "\r" << "Creating grids... " << to_string(gridnum) << "/" << maxGrid(dlst.size());
        }
        cgrid.push_back(dlst[ij]);
        
    }
    ddg.close();
    
    cout << "\r" << "Creating images... Done!     " << endl;
    cout << endl << "Press any key to close." << endl;
    system("pause >nul");
    
    return 0;
}