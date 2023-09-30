#include "header\playerfile.hpp"
#include "header\ddfile.hpp"
#include "header\gridstrings.hpp"

using namespace std;
namespace fs = std::filesystem;

void partOne(string pfile)
{
    PlayerFile player(pfile);
    player.readHeader();
    player.createDir();
    player.readImages();
    player.readPallettes();
    player.createSounds();
    player.closePlayerFile();
    player.createImages();
}


void partTwo(string pfile)
{
    util::Status* status = new util::Status("Opening files");
    string wDir = string("./") + pfile + "/";

    //open ddi
    DDInfo ddi(wDir, "full.ddi");
    //read ddi to vector dlst
    vector<util::iRen> dlst = ddi.read();
    //remove ones moved to unchanged
    for (int ij = 0; ij < dlst.size(); ij++) {
        if (!std::filesystem::exists(wDir + std::string("main/") + std::to_string(ij) + ".png")) {
            dlst.erase(dlst.begin() + ij);
        }
    }

    //create ddg
    unsigned imgNum = dlst.size();
    //round up from 16
    uint16_t maxGrids = imgNum / 16;
    if(imgNum % 16 != 0) {maxGrids++;}
    DDGrid ddg(wDir, "full.ddg", maxGrids);
    delete status;

    status = new util::Status("Creating directories");
    //create directories
    fs::create_directories(wDir + string("grids"));
    string gDir = wDir + "grids/";
    wDir += "main/";
    delete status;
    
    status = new util::Status("Creating grids", maxGrids);
    //begin loop
    unsigned gridNum = 0;
    vector<util::iRen> cgrid;
    for (int ij = 0; ij < dlst.size(); ij++) //loops through every image found in main
    {
        cgrid.push_back(dlst[ij]); 
        if (cgrid.size() >= 16 || ij == (dlst.size() - 1)) //grids have max size of 4x4
        {
            gridStrings magickText;
            uint16_t bigW = 0;
            uint16_t bigH = 0;

            uint16_t cImgs[16] = {0}; 
            for (int idx = 0; const auto &img : cgrid) {
                //creating list of imgs & shapes
                magickText.writeGraphics(wDir, img.num);
                magickText.writeTemplate(img.w, img.h);
                //checks to see which image is the biggest
                if (img.w > bigW) {bigW = img.w;}
                if (img.h > bigH) {bigH = img.h;}
                //preparing ddg info
                cImgs[idx] = img.num;
                idx++;
            }
            //arbitrary buffer
            bigW = (bigW / 10) * 10 + 20;
            bigH = (bigH / 10) * 10 + 20;

            util::mRen header = {gridNum, bigW/10, bigH/10, cgrid.size()};
            memcpy(header.imgs, cImgs, sizeof(cImgs));
            ddg.write(header);
            
            gridStrings::bW = bigW;
            gridStrings::bH = bigH;
            for (uint8_t ix = 0, iy = 0; const auto &img : cgrid) { 
                //have to loop second time because first time determined grid cell size
                magickText.writeCaptions(ix, iy, img);
                if (ix == 3) {ix = 0; iy++;}
                else         {ix++;}
            }
            //create image grid and template grid
            magickText.makeImages(gDir, gridNum);
            gridNum++;
            status->update(gridNum);
            cgrid.clear();
            ddg.flush();
        }
    }
    delete status;
}

void partThree(string pfile) 
{
    util::Status* status = new util::Status("Opening files");
    string wDir = string("./") + pfile + "/";
    
    //open ddi & ddg
    DDInfo* ddi = new DDInfo(wDir, "full.ddi");
    //don't remove the ones that don't exist anymore
    vector<util::iRen> dlst = ddi->read();
    delete ddi;

    DDGrid* ddg = new DDGrid(wDir, "full.ddg");
    vector<util::mRen> mlst = ddg->read();
    delete ddg;
    
    fs::create_directories(wDir + string("new"));
    string gDir = wDir + "grids/";
    wDir += "new/";

    delete status;
    status = new util::Status("Extracting grids", mlst.size() * 16);
    gridStrings::setRGBAInfo(wDir);
    for (unsigned inum = 0; auto &grid : mlst)
    {
        gridStrings::setGridInfo(gDir, wDir, grid.num, grid.w, grid.h);
        for (uint8_t i = 0, ix = 0, iy = 0; i < grid.count; i++)
        {
            util::iRen img = dlst[grid.imgs[i]];
            gridStrings::cropGrid(img.w, img.h, ix, iy, img.num);
            
            //open pallete
            ifstream palFile ((wDir + to_string(img.num) + string("p.RGBA")).c_str(), ifstream::binary);

            char* palbuf = new char [1024];
            memset(palbuf, 0, 1024); 
            palFile.read (palbuf, 1024);
            palFile.close();
            
            //open image
            ifstream imgFile ((wDir + to_string(img.num) + string("e.RGBA")).c_str(), ifstream::binary);
            int imgLen = util::getLength(imgFile);

            char* imgbuf = new char [imgLen];
            memset(imgbuf, 0, imgLen);
            imgFile.read (imgbuf,imgLen);
            imgFile.close();

            uint32_t* imgptr = reinterpret_cast<uint32_t*>(imgbuf);
            uint32_t* palptr = reinterpret_cast<uint32_t*>(palbuf);

            ofstream idx ((wDir + to_string(img.num) + string("e.idx")).c_str(), ios::binary);

            //compare each 4 byte color in image to palette and write down the index
            for (uint32_t j = 0; j < (imgLen / 4); j++) {
                bool done = false; //in case color isn't found in palette
                for (uint16_t jx = 0; jx < (1024 / 4); jx++) {
                    if (*(imgptr + j) == *(palptr + jx)) 
                    {
                        idx.put(jx);
                        done = true;
                        break;
                    }
                }
                if(!done) {idx.put(0x00);}
            }
            idx.flush();
            idx.close();
            delete[] palbuf;
            delete[] imgbuf;
            
            gridStrings::removeRaw(wDir, img.num);
            
            if (ix == 3) {ix = 0; iy++;}
            else         {ix++;}
            inum++;
            status->update(inum);
        }
        
    }
    delete status;
}

void partFour(string pfile) {}


int main()
{
    string pfileName;
    cout << "    ______         ____                       ____                   "  << endl <<  "   / ____/___     / __ \\____  ____  ____ _   / __ \\____  ____  ____ _"  << endl <<   "  / / __/ __ \\   / / / / __ \\/ __ \\/ __ `/  / / / / __ \\/ __ \\/ __ `/"  << endl <<  " / /_/ / /_/ /  / /_/ / /_/ / / / / /_/ /  / /_/ / /_/ / / / / /_/ / "  << endl <<  " \\____/\\____/  /_____/\\____/_/ /_/\\__, /  /_____/\\____/_/_/_/\\__, /  "  << endl <<  "                    \\ \\/ /___  __/____/___________  / / __/ /____/   "  << endl <<  "                     \\  / __ \\/ / / / ___/ ___/ _ \\/ / /_            "  << endl <<  "                     / / /_/ / /_/ / /  (__  )  __/ / __/            "  << endl <<  "                    /_/\\____/\\__,_/_/  /____/\\___/_/_/               "  << endl
    << endl << "* v2.0 - snox - 08/2023 *" << endl << endl;
    cout << "Enter character name without file extension (case sensitive): "; cin >> pfileName;
    unsigned modeNum;
    MODE: //go back if given an invalid input
    cout << "Select mode: "; cin >> modeNum; cout << endl;
    switch (modeNum)
    {
    case 1:
        partOne(pfileName);
        break;
    case 2:
        partTwo(pfileName);
        break;
    case 3:
        partThree(pfileName);
        break;
    case 4:
        partFour(pfileName);
        break;
    default:
        cout << "Invalid mode number." << endl;
        goto MODE;
        break;
    }

    cout << endl << "Press any key to close." << endl;
    system("pause >nul");
    return 0;
}