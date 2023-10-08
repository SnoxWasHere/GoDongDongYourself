#ifndef MERGER_HPP
#define MERGER_HPP
#include "util.hpp"
#include "playerfile.hpp"

template <typename T, unsigned int S> class Merger
{
protected:
    unsigned _count = 0;
    //have to do pointers to these
    std::ifstream* player;
    std::ofstream* modded;
    std::vector<T>* list; 
    std::vector<uint8_t>* changed;
    util::Status* status;
    char* blankbuf; //please no memory leaks
    char* headerbuf; //you too
    void skipToNext();
    virtual void changedCopy() {}
    void unchangedCopy();
public:
    Merger(std::ifstream* p, std::ofstream* m, std::vector<T>* l, std::vector<uint8_t>* c) {
        this->player = p;
        this->modded = m;
        this->list = l;
        this->changed = c;
        //blank buffer to be used later
        this->blankbuf = new char[20];
        memset(this->blankbuf, 0, 20);
    }
    void run();
};

/***********************************************************************************/
/***********************************************************************************/

class ImageMerger : public Merger<util::iRen, 20>
{
protected:
    void changedCopy() override;
    static uint32_t getSize(char buffer[20]);
public:
    static std::string dir;
    ImageMerger(std::ifstream* p, std::ofstream* m, std::vector<util::iRen>* l, std::vector<uint8_t>* c) : Merger(p,m,l,c) {}
};

#endif

/***********************************************************************************/
/***********************************************************************************/

class SoundMerger : public Merger<util::sRen, 42>
{
protected:
    void changedCopy() override;
public:
    static std::string dir; 
    SoundMerger(std::ifstream* p, std::ofstream* m, std::vector<util::sRen>* l, std::vector<uint8_t>* c) : Merger(p,m,l,c) {}
};