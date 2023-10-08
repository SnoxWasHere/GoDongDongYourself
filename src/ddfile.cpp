#include "header/ddfile.hpp"

void DDInfo::writeHeader(int count) {
    const uint16_t mWord = 0x10DD;
    _file.write(reinterpret_cast<const char*>(&mWord), 2);
    _file.write(charptr(&count), 2);
}
void DDInfo::write(util::hRen &container) {
    _file.write(charptr(&container.ofs), 4);
    _file.write(charptr(&container.w), 2);
    _file.write(charptr(&container.h), 2);
}

std::vector<util::iRen> DDInfo::read() 
{
    std::vector<util::iRen> output;
    _file.seekg(2); //magic word
    uint16_t numImg = 0;
    _file.read(charptr(&numImg), 2);

    util::iRen img;
    for (uint16_t i = 0; i < numImg; i++)
    {
        //removing directories from this because not always
        util::iRen img;
        memset(&img, 0, sizeof(util::iRen));
        _file.read(charptr(&img), 8);
        //reading to a struct of different types of ints is difficult so this is the solution
        //check if this is still necessary now that switched to fstream
        img.ofs = (img.ofsa) | (img.ofsb << 16); 
        img.num = i;
        output.push_back(img);
    }
    return output; 
}


/***********************************************************************************/
/***********************************************************************************/

void DDGrid::writeHeader(int count) {
    const uint16_t mWord = 0x60DD;
    _file.write(reinterpret_cast<const char*>(&mWord), 2);
    _file.write(charptr(&count), 2);
}

void DDGrid::write(util::mRen &container)  {
    _file.put(container.num);
    _file.put(container.w);
    _file.put(container.h);
    _file.put(container.count);
    for(auto img : container.imgs) {_file.write(charptr(&img), 2);}
}

std::vector<util::mRen> DDGrid::read() {
    std::vector<util::mRen> output;
    _file.seekg(2); //magic word
    uint16_t numGrids = 0;
    _file.read(charptr(&numGrids), 2);

    for (uint16_t i = 0; i < numGrids; i++)
    {
        //header
        util::mRen mts;
        memset(&mts, 0, sizeof(util::mRen));
        _file.read(charptr(&mts), 4);

        //img numbers
        uint16_t inum = 0;
        for (uint8_t i = 0; i < mts.count; i++)
        {
            _file.read(charptr(&inum), 2);
            mts.imgs[i] = inum;
        }
        output.push_back(mts);
    }
    return output;
}


/***********************************************************************************/
/***********************************************************************************/

void DDSound::writeHeader(int count) {
    if(!_file.is_open()) {std::cerr << "help";}
    const uint16_t mWord = 0x50DD;
    _file.write(reinterpret_cast<const char*>(&mWord), 2);
    _file.write(charptr(&count), 2);
    _file.flush();
}

void DDSound::write(util::sRen val) {
    //this ampersand costed like half an hour
    _file.write(charptr(&val.size), 4);  //size
    _file.write(charptr(&val.ofs), 4); //ofs
}

std::vector<util::sRen> DDSound::read() {
    std::vector<util::sRen> output;
    _file.seekg(2); //magic word
    uint16_t numSounds = 0;
    _file.read(charptr(&numSounds), 2);

    for (uint16_t i = 0; i < numSounds; i++) {
        util::sRen snd;
        _file.read(charptr(&snd.size), 4);
        _file.read(charptr(&snd.ofs), 4);

        output.push_back(snd);
    }
    return output;
}

