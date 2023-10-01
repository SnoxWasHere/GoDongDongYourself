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
    DDInfo *ddi = new DDInfo(wDir, "full.ddi");
    //read ddi to vector dlst
    vector<util::iRen> dlst = ddi->read();
    //remove ones moved to unchanged 
    //bad implementation
    unsigned iSize = dlst.size();
    for (unsigned idx = 0, vPos = 0; idx < iSize; idx++) {
        if (!std::filesystem::exists(wDir + string("main/") + to_string(idx) + ".png")) {
            dlst.erase(dlst.begin() + vPos);
        } else {vPos++;}
    }
    delete ddi;

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

void partFour(string pfile) 
{
    util::Status* status = new util::Status("Opening files");
    string wDir = string("./") + pfile + "/";
    string tDir = wDir + "new/";
    
    vector<uint8_t> changed; //don't want bool vec

    //open ddi
    DDInfo *ddi = new DDInfo(wDir, "full.ddi");
    //read ddi to vector dlst
    vector<util::iRen> dlst = ddi->read();
    //remove ones moved to unchanged
    for (int ij = 0; ij < dlst.size(); ij++) {
        if (fs::exists(tDir + std::to_string(ij) + "e.idx")) {
            changed.push_back(1); //check if changed
        } else {
            changed.push_back(0);
        }
    }
    delete ddi;
    PlayerFile player(pfile);
    player.readHeader();    

    delete status;
    status = new util::Status("Initializing character file");

    ofstream modded(player.outputDir + player.fileName + "modded.player", std::ios::binary);
    unsigned pLen = util::getLength(player._playerFile, player.getPosition());
    
    //copy all unchanged information
    uint32_t headstart = player.getPosition();
    char* buffer = new char[headstart];
    memset(buffer, 0, headstart);
    player._playerFile.seekg(0, ios::beg);
    player._playerFile.read(buffer, headstart);

    modded.write(buffer, headstart);
    modded.flush();
    delete[] buffer;

    char* blankbuf = new char[20]; 
    memset(blankbuf, 0, 20); //for when we just need 20 zeros

    delete status;
    status = new util::Status("Writing images", dlst.size());

    //begin going through each image
    uint16_t count = 0;
    while (count < dlst.size())
    {
        if (player.getPosition() != dlst[count].ofs) { //skip over blank space in player file
            //copy whats there just incase, although probably just 0
            unsigned blankSpace = dlst[count].ofs - player.getPosition();
            char* buffer = new char[blankSpace];
            memset(buffer, 0, blankSpace);
            player._playerFile.read(buffer, blankSpace);

            modded.write(buffer, blankSpace);
            delete[] buffer;
        }
        
        else if (changed[count] == 1) //image was found in grid
        {
            ifstream idx ((tDir + to_string(dlst[count].num) + "e.idx").c_str(), ifstream::binary);
            int ddlength = util::getLength(idx);
            
            char* buffer = new char[20];
            memset(buffer, 0, 20);
            player._playerFile.read(buffer, 20);

            if (util::isEmpty(buffer, 20)) //don't increase count
            {
                modded.write(blankbuf, 20);
                delete[] buffer;
            }  
            else
            {
                modded.write(buffer, 16);
                modded.write(blankbuf, 4); //set size equal to 0
                //this allows us to write the uncompressed data 1:1 without trouble 

                uint32_t sizzle = 0;
                //TODO - see if i work with just dlst[i].size
                //weird negative positive stuff happens without cast
                sizzle |= static_cast<uint8_t>(buffer[16]);
                sizzle |= static_cast<uint8_t>(buffer[17]) << 8;
                sizzle |= static_cast<uint8_t>(buffer[18]) << 16;
                sizzle |= static_cast<uint8_t>(buffer[19]) << 24;
                

                if (buffer[12] != 0) //checking for palette
                {
                    delete[] buffer;
                    ifstream prga ((tDir + to_string(dlst[count].num) + "p.RGBA").c_str(), ios::binary);
                    int prglength = util::getLength(prga);
                    buffer = new char[1024];
                    memset(buffer, 0, 1024);

                    prga.read(buffer, prglength);
                    for (int ip = 0; ip < 1024; ip+= 4) 
                    {   //rearrange RGBA to BGRA
                        if(ip < prglength) {
                            modded.put(buffer[ip + 2]);
                            modded.put(buffer[ip + 1]);
                            modded.put(buffer[ip + 0]);
                            modded.put(buffer[ip + 3]);
                        }
                        else {modded.write(blankbuf, 4);}
                    }
                    //as of v1.1, a 1024 byte palette is presumed
                    //thankfully all of the 512 images are usually just particle effects anyways
                    //purposely untested
                    modded.flush();
                    prga.close();
                }
                delete[] buffer;
                //copy-paste entire .idx
                buffer = new char[ddlength];
                memset(buffer, 0, ddlength);
                idx.read(buffer, ddlength);

                modded.write(buffer, ddlength);
                delete[] buffer;
                //jump ahead by normal image size
                player._playerFile.seekg(sizzle, ios::cur);

                count++;
                status->update(count);
                idx.close();
            }
        }
        else //unchanged image
        {
            buffer = new char[20];
            memset(buffer, 0, 20);
            player._playerFile.read(buffer, 20);

            if (util::isEmpty(buffer, 20)) //count not increased
            {
                modded.write(blankbuf, 20);
                delete[] buffer;
            }  
            else
            {
                modded.write(buffer, 20);
                delete[] buffer;
                if (count + 1 < dlst.size()) //overflow protection
                {
                    //dist between where we are now and where the next header is
                    //TODO technically could be optimized to go to next changed
                    uint32_t highwater = dlst[count + 1].ofs - dlst[count].ofs - 20;
                    buffer = new char[highwater];
                    memset(buffer, 0, highwater);
                    player._playerFile.read(buffer, highwater);
                    
                    modded.write(buffer, highwater);
                    delete[] buffer;
                }
                //it's fine to just ignore the last image if it's unchanged because of the next step
                count++;
                status->update(count);
            }
        }
        
        modded.flush();
    }
    //sound time!
    string sDir = wDir + "snd/";
    DDSound dds(sDir, "full.dds");
    vector<pair<uint32_t, uint32_t>> sounds = dds.read();
    vector<uint8_t> sChanged;
    for (int ij = 0; ij < sounds.size(); ij++) {
        if (fs::exists(sDir + std::to_string(ij) + ".wav")) {
            sChanged.push_back(1); //check if changed
        } else {
            sChanged.push_back(0);
        }
    }

    unsigned pToSound = sounds[0].second - player.getPosition();
    //copy everything between where we are now and the first sound
    buffer = new char[pToSound];
    memset(buffer, 0, pToSound);
    player._playerFile.read(buffer, pToSound);
    modded.write(buffer, pToSound);
    delete[] buffer;

    //TODO - generalize image framework to work with sounds
    //same loop structure as before
    /* vague layout
    * template <typename T> class Merger
    * unsigned count = 0;
    * Merger(ifstream player, ofstream modded, vector<T> list, vector<uint32_t> changed)
    * void run()
    * {
    *   while (count < list.size())
    *   {
    *       if(pos != next changed pos) {
    *           skipToNext(); //can probably be non-virtual even
    *       }
    *       else 
    *       {
    *           player.read(buffer, 20);
    *           if(blank)   {doNothing();} //non-virtual
    *           else if(changed) {changedCopy();} //virtual but stil pretty similar
    *           else        {unchangedCopy();} //hopefully non-virtual if create sound struct
    *       }
    *
    *   }
    * }
    */
    //maybe change changedVect to store indexes instead of bools
    //allow for perfect skipping over changed members

    //copy everything else
    unsigned pRemainder = pLen - player.getPosition();
    buffer = new char[pRemainder];
    memset(buffer, 0, pRemainder);
    player._playerFile.read(buffer, pRemainder);
    
    modded.write(buffer, pRemainder);
    delete[] buffer;
    delete status;
    cout << "Rename modded player file to the original and replace it in the game directory." << endl;
    //thank you!
    // <3 snox
}


int main()
{
    string pfileName;
    cout << "    ______         ____                       ____                   "  << endl <<  "   / ____/___     / __ \\____  ____  ____ _   / __ \\____  ____  ____ _"  << endl <<   "  / / __/ __ \\   / / / / __ \\/ __ \\/ __ `/  / / / / __ \\/ __ \\/ __ `/"  << endl <<  " / /_/ / /_/ /  / /_/ / /_/ / / / / /_/ /  / /_/ / /_/ / / / / /_/ / "  << endl <<  " \\____/\\____/  /_____/\\____/_/ /_/\\__, /  /_____/\\____/_/_/_/\\__, /  "  << endl <<  "                    \\ \\/ /___  __/____/___________  / / __/ /____/   "  << endl <<  "                     \\  / __ \\/ / / / ___/ ___/ _ \\/ / /_            "  << endl <<  "                     / / /_/ / /_/ / /  (__  )  __/ / __/            "  << endl <<  "                    /_/\\____/\\__,_/_/  /____/\\___/_/_/               "  << endl
    << endl << "* v2.0 - snox - 08/2023 *" << endl << endl;
    cout << "Enter character name without file extension (case sensitive): "; cin >> pfileName;
    unsigned modeNum;
    MODE: //go back if given an invalid input
    cout << endl << "* MODE SELECT *" << endl;
    cout << "(1) - PlayerToPng" << endl;
    cout << "(2) - PngToGrid" << endl;
    cout << "(3) - GridToPng" << endl;
    cout << "(4) - PngToPlayer" << endl;
    cout << "For help, please check README on github." << endl << endl;
    cout << "Select mode number: "; cin >> modeNum; cout << endl;
    switch (modeNum)
    {
    case 1:
        partOne(pfileName);
        break;
    case 2:
        cout << "Before continuing, move all sprites you don't wish to change to /unchanged/" << endl;
        cout << endl << "Press any key to continue." << endl;
        system("pause >nul");
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