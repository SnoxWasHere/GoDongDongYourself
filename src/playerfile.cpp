#include <filesystem>
#include <iostream>
#include "header\playerfile.hpp"
#include "header\ddfile.hpp"

using std::cout;
using std::endl;

//private
void PlayerFile::extract(uint8_t* dst, uint8_t* src, uint32_t size, uint32_t newsize, uint32_t pij)
{
    //Extracts RLE encoded images
    //don't try to edit this unless you want to go insane
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
    this->fileName = file;
    this->outputDir = "./";

    _playerFile.open(outputDir + fileName + ".player", std::ios::binary);

    if(!_playerFile.is_open()) //error probably won't happen anymore
    {
        std::cerr << "Error opening file!" << endl;
        system("pause >nul");
        exit(1);
    }
}
void PlayerFile::readHeader()
{
    util::Status status("Reading header");
    //file magic word 2DKGT2K/2DKGT2G
    _playerFile.seekg(0x10);
    //internal char name
    _playerFile.read(charptr(&charName), 0x100);

    //num of moves
    uint32_t numMoves = 0;
    _playerFile.read(charptr(&numMoves), 4);
    //skip over move data
    _playerFile.seekg((numMoves * 0x27), std::ios::cur);

    uint32_t choNum = 0;
    _playerFile.read(charptr(&choNum), 4);
    _playerFile.seekg((choNum << 4), std::ios::cur);
    
    _playerFile.read(charptr(&headerCount), 4);
}
void PlayerFile::createDir()
{
    util::Status status("Creating directories");
    this->outputDir += fileName;
    std::filesystem::create_directories(outputDir);
    
    this->outputDir += "/";
    std::filesystem::create_directories(outputDir + "snd");
    std::filesystem::create_directories(outputDir + "main");
    std::filesystem::create_directories(outputDir + "unchanged");
}
void PlayerFile::readImages() 
{
    //the scary part of the program
    util::Status status("Reading images");
    uint8_t* unpkd;
    uint8_t* pkd;
    uint32_t new_size = 0;
    for (uint32_t ij = 0; ij < headerCount; ij++)
    {
        util::hRen zps = {0};
        bool est = false;

        _playerFile.read(charptr(&zps), 0x14);
        zps.ofs = (int)_playerFile.tellg() - 0x14;
        
        //TODO - make these if statements not bad
        if(zps.size != 0) //compressed
        {
            pkd = new uint8_t[zps.size];
            memset(pkd, 0, zps.size);
            
            new_size = zps.w * zps.h;
            //create space for palette
            if(zps.pal == 1) {new_size += 0x400;} 
            
            unpkd = new uint8_t[new_size];
            //more on beefiness later
            util::memset32(unpkd, BEEFVAL, new_size);
            _playerFile.read(charptr(pkd), zps.size);

            extract(unpkd, pkd, zps.size, new_size, ij); //decompress from pkd to unpkd
            delete[] pkd;
            est = true;
        }
        else if (zps.w != 0 && zps.h != 0) //uncompressed but not blank
        {
            new_size = zps.w * zps.h;
            //create space for palette
            if(zps.pal == 1) {new_size += 0x400;} 
            
            //removed the +0x10, if this breaks put it back, also added beefcheck
            unpkd = new uint8_t[new_size];
            util::memset32(unpkd, BEEFVAL, new_size);
            _playerFile.read(charptr(unpkd), new_size);

            est = true;
        }
        if(est)
        {
            zps.unk = (unpkd);
            zps.size = new_size;
            slst.push_back(zps);
        }
    }
}
void PlayerFile::readPallettes()
{
    util::Status status("Extracting pallettes");
    mempal = new uint8_t[0x2100]; //8 palletes
    memset(mempal, 0, 0x2100); 
    _playerFile.read(charptr(mempal), 0x2100);
}
void PlayerFile::createSounds()
{
    uint32_t snd_n;
    _playerFile.read(charptr(&snd_n), 4);
    util::Status status("Creating sounds", snd_n);
    //DDSound* dds = new DDSound(outputDir + "snd/", "full.dds", snd_n);

    for (uint32_t ij = 0; ij < snd_n; ij++)
    {
        //sounds are stored as uncompressed .wavs
        memset(&qqww, 0, sizeof(qqww));
        _playerFile.read(charptr(&qqww), 0x2A);
        //dds->write(qqww.size);
        if(qqww.size != 0)
        {
            uint8_t* snd_m = new uint8_t[qqww.size];
            _playerFile.read(charptr(snd_m), qqww.size);

            std::string str = outputDir + std::string("snd/") + std::to_string(ij) + std::string(".wav");

            std::ofstream fil2(str, std::ios::binary);
            //test me!
            fil2.write(charptr(snd_m), qqww.size);
            fil2.close();
            delete[] snd_m;
            
        }
        status.update(ij + 1);
    }
    //delete dds;
}
void PlayerFile::createImages()
{
    util::Status status("Creating images", slst.size());
    util::BGRA* pallt[8];

    for (int ij = 0; ij <= 7; ij++)
    {
        pallt[ij] = reinterpret_cast<util::BGRA*>(mempal + ij * 0x420);
    }

    //creating DongDongImage file
    //stores information about each image to be referenced later
    DDInfo ddi(outputDir, "full.ddi", slst.size());

    bool beefedup;
    uint8_t* imgStart;
    util::BGRA* tmppal;
    outputDir += "main/";
    RawImage::setDir(outputDir);

    for (uint32_t ij = 0; ij < slst.size(); ij++)
    {
        util::hRen zps = slst[ij];

        if (zps.pal == 1) //if image contains it's own palette
        {
            tmppal = reinterpret_cast<util::BGRA*>(zps.unk);
            
            imgStart = (zps.unk + 0x400);

            //some images only have a 512 byte palette before the image begins as opposed to the presumed 1024
            //this doesn't cause issues upon reading because of how size is stored within the header
            //however this 512 vs 1024 information is located at an unknown location
            //so we check for 512 bytes of a poison value (0xBEEFFACE) at the end of image to determine it
            uint32_t* bigunk = reinterpret_cast<uint32_t*>(zps.unk + (zps.h * zps.w) + 0x200); //start + 1 byte per pixel + 512
            if (*bigunk == BEEFVAL) 
            {
                beefedup = true;
                //checking for false positives (might be excessive)
                for (int i = 1; i < ((0x200 / 4) - 2) ; i++)
                {
                    if (*(bigunk + i) != BEEFVAL) {beefedup = false;}
                }
                //poison value is written back to front so this will always align
                if (*(zps.unk + 0x200) != 0xCE)  {beefedup = false;}

                if (beefedup) {imgStart = (zps.unk + 0x200);} //otherwise 0x400
            }
            //creates img
            util::rawWrite(std::to_string(ij), zps.w, zps.h, imgStart, tmppal);
            //writes all necessary zps information to .ddi
            ddi.write(zps); 
        }
        else //image uses global palette
        {
            //DDND doesn't really make use of this feature, so this should never really be an issue
            //technically untested
            for (int pk = 0; pk < 7; pk++)
            {
                tmppal = pallt[pk];
                imgStart = zps.unk;
                util::rawWrite(std::to_string(pk) + "p" + std::to_string(ij), zps.w, zps.h, imgStart, tmppal);
            }
        }
        status.update(ij + 1);
    }
}


PlayerFile::~PlayerFile()
{
    this->closePlayerFile();
}

