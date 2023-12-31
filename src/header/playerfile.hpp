#ifndef PLAYERFILE_HPP
#define PLAYERFILE_HPP

#include "rawimg.hpp"
#include "util.hpp"
#include <Windows.h>
#include <vector>

#define BEEFVAL 0xBEEFFACE

void partFour(std::string pfile);
class PlayerFile
{
private:
    std::ifstream _playerFile;

    std::vector<util::hRen> slst;
    uint8_t* mempal;
    util::Sound qqww;

    void extract(uint8_t* dst, uint8_t* src, uint32_t size, uint32_t newsize = 0, uint32_t pij = 0);
public:
    friend void partFour(std::string pfile); //allows direct file manip
    std::string fileName;
    std::string outputDir;
    char charName[0x100];
    uint32_t headerCount = 0;

    PlayerFile(std::string file);
    void readHeader();
    void createDir();
    void readImages();
    void readPallettes();
    void createSounds();
    inline void closePlayerFile() {_playerFile.close();}
    void createImages();
    ~PlayerFile();
    inline int getPosition() {return _playerFile.tellg();}
};

#endif