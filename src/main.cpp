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

            util::mRen header = {gridNum, bigW/10, bigH/10, cgrid.size(), *cImgs};
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

void partThree(string pfile) {}
void partFour(string pfile) {}


int main()
{
    string pfileName;
    cout << "    ______         ____                       ____                   "  << endl <<  "   / ____/___     / __ \\____  ____  ____ _   / __ \\____  ____  ____ _"  << endl <<   "  / / __/ __ \\   / / / / __ \\/ __ \\/ __ `/  / / / / __ \\/ __ \\/ __ `/"  << endl <<  " / /_/ / /_/ /  / /_/ / /_/ / / / / /_/ /  / /_/ / /_/ / / / / /_/ / "  << endl <<  " \\____/\\____/  /_____/\\____/_/ /_/\\__, /  /_____/\\____/_/_/_/\\__, /  "  << endl <<  "                    \\ \\/ /___  __/____/___________  / / __/ /____/   "  << endl <<  "                     \\  / __ \\/ / / / ___/ ___/ _ \\/ / /_            "  << endl <<  "                     / / /_/ / /_/ / /  (__  )  __/ / __/            "  << endl <<  "                    /_/\\____/\\__,_/_/  /____/\\___/_/_/               "  << endl
    << endl << "* v2.0 - snox - 08/2023 " << endl << endl;
    cout << "Enter character name without file extension (case sensitive): "; cin >> pfileName;
    unsigned modeNum;
    MODE:
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
    
    partTwo(pfileName);
    cout << endl << "Press any key to close." << endl;
    system("pause >nul");
    return 0;
}