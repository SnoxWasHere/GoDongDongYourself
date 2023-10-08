#include "header/merger.hpp"

template <typename T, unsigned int S> void Merger<T,S>::skipToNext()
{
    //skip over blank space in player file
    //copy whats there just incase, although probably just 0
    unsigned blankSpace = list->at(_count).ofs - player->tellg();
    
    char* buffer = new char[blankSpace];
    memset(buffer, 0, blankSpace);
    player->read(buffer, blankSpace);

    modded->write(buffer, blankSpace);
    delete[] buffer;
}

template <typename T, unsigned int S> void Merger<T,S>::unchangedCopy()
{
    if (_count + 1 < list->size()) //overflow protection
    {
        //dist between where we are now and where the next header is
        uint32_t highwater = list->at(_count + 1).ofs - list->at(_count).ofs - S;
        //come hell and highwater
        char* buffer = new char[highwater];
        memset(buffer, 0, highwater);
        player->read(buffer, highwater);
        modded->write(buffer, highwater);
        delete[] buffer;
    }
    _count++;
    status->update(_count);
}

template <typename T, unsigned int S> void Merger<T,S>::run()
{
    status = new util::Status("Writing images", list->size());
    while (this->_count < list->size())
    {
        if (player->tellg() != list->at(_count).ofs) {  
            this->skipToNext(); //skip to next header
        }
        else {
            headerbuf = new char[S];
            memset(headerbuf, 0, S);
            player->read(headerbuf, S);

            if (util::isEmpty(headerbuf, S)) {modded->write(blankbuf, S);}
            else {
                if (changed->at(_count) == 1) {
                    //implement changed header case-by-case
                    this->changedCopy(); //copy from .idx
                }
                else { //unchanged
                    modded->write(headerbuf, S);
                    this->unchangedCopy(); //copy from .player
                }
            }
            delete[] headerbuf;
        }
    }
    modded->flush();
    delete status;
    delete[] blankbuf;
}

/***********************************************************************************/
/***********************************************************************************/


std::string ImageMerger::dir = "";

uint32_t ImageMerger::getSize(char buffer[20]){
    uint32_t out = 0;
    //weird negative positive stuff happens without cast
    out |= static_cast<uint8_t>(buffer[16]);
    out |= static_cast<uint8_t>(buffer[17]) << 8;
    out |= static_cast<uint8_t>(buffer[18]) << 16;
    out |= static_cast<uint8_t>(buffer[19]) << 24;
    return out;
}

void ImageMerger::changedCopy()
{
    std::ifstream idx ((dir + std::to_string(list->at(_count).num) + "e.idx").c_str(), std::ios::binary);
    int ddlength = util::getLength(idx);

    //set size equal to 0
    //this allows us to write the uncompressed data 1:1 without trouble 
    modded->write(headerbuf, 16);
    modded->write(blankbuf, 4);

    uint32_t sizzle = getSize(headerbuf);

    if (headerbuf[12] != 0) //checking for palette
    {
        std::ifstream prga ((dir + std::to_string(list->at(_count).num) + "p.RGBA").c_str(), std::ios::binary);
        int prglength = util::getLength(prga);
        char* buffer = new char[1024];
        memset(buffer, 0, 1024);

        prga.read(buffer, prglength);
        //prglength % 4 should == 0 always
        for (int ip = 0; ip < 1024; ip+= 4) 
        {   //rearrange RGBA to BGRA
            if(ip < prglength) {
                modded->put(buffer[ip + 2]);
                modded->put(buffer[ip + 1]);
                modded->put(buffer[ip + 0]);
                modded->put(buffer[ip + 3]);
            }
            else {modded->write(blankbuf, 4);}
        }
        //as of v2.1, a 1024 byte palette is presumed
        //thankfully all of the 512 images are usually just particle effects anyways
        //purposely untested
        modded->flush();
        prga.close();
        delete[] buffer;
    }
    //copy-paste entire .idx
    char* buffer = new char[ddlength];
    memset(buffer, 0, ddlength);
    idx.read(buffer, ddlength);

    modded->write(buffer, ddlength);
    delete[] buffer;
    //jump ahead by normal image size
    player->seekg(sizzle, std::ios::cur);

    _count++;
    status->update(_count);
    idx.close();
}


/***********************************************************************************/
/***********************************************************************************/

std::string SoundMerger::dir = "";

void SoundMerger::changedCopy(){
    std::ifstream wav ((dir + std::to_string(list->at(_count).num) + ".wav").c_str(), std::ios::binary);
    uint32_t ddlength = util::getLength(wav);

    //copy whole header
    modded->write(headerbuf, 42);

    //it's unclear if i can exceed this size
    uint32_t sizzle = list->at(_count).size;
    
    //copy-paste entire .wav
    char* buffer = new char[sizzle];
    //fill with neutral sound value
    memset(buffer, 128, sizzle);
    wav.read(buffer, std::min(ddlength, sizzle));
    modded->write(buffer, sizzle);
    delete[] buffer;
    //jump ahead by normal image size
    player->seekg(sizzle, std::ios::cur);

    _count++;
    status->update(_count);
    wav.close();
}