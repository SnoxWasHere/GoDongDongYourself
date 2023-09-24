#include <iostream>
#include <fstream>
#include <vector>
#include <Windows.h>
#include <filesystem>
#include <stdint.h>

namespace util
{
    class Status { //small class for managing status messages
    private:
        std::string _msg;
        unsigned _count;
    public:
        //don't include "... " in constructor arg
        Status(std::string m, unsigned c = 0) : _msg(m + "... "), _count(c) {
            std::cout << _msg;
            if(_count != 0) {std::cout << "0/" << _count;}
        }
        inline void update(unsigned val) {std::cout << "\r" << _msg << val << "/" << _count;}
        ~Status() {std::cout << "\r" << _msg << "Done!   " << std::endl;}
    };
    struct BGRA {
        uint8_t b;
        uint8_t g;
        uint8_t r;
        uint8_t a;
    };
    struct hRen { //img header info
        uint32_t ofs = 0;
        uint32_t w = 0;
        uint32_t h = 0;
        uint32_t pal = 0;
        uint32_t size = 0;
        uint8_t* unk = nullptr;
    };
    struct Sound { //sound header info 
        char name[0x24];
        uint32_t size;
        uint16_t unk;
    };
    struct iRen { //stores info from .ddi
        uint16_t ofsa;
        uint16_t ofsb;
        uint16_t w;
        uint16_t h;
        uint16_t num;
        uint32_t ofs;
    };
    struct mRen //stores info from .ddg
    {
        uint8_t num;
        uint8_t w;
        uint8_t h;
        uint8_t count;
        uint16_t imgs[16];
    };
    void memset32(void* dest, uint32_t value, uintptr_t size); //fills back-to-front with 32 bit int
    void toChar(std::ofstream &file, uint8_t size, uint32_t val); //32 bit int to 4 chars
    void toChar(std::fstream &file, uint8_t size, uint32_t val);
}
void util::memset32(void* dest, uint32_t value, uintptr_t size)
{
    //for detecting unwritten memory with poison value
    uintptr_t i;
    for( i = size - 4; i > 4; i -= 4 ) //back to front
    {
        memcpy(((char*)dest) + i, &value, 4 );
    }  
    for( ; i > 0; i-- )
    {
        ((char*)dest)[i] = 0xFF;
    }  
}
void util::toChar(std::ofstream &file, uint8_t size, uint32_t val)
{
    //using little endian
    char* chars = new char[size];
    for (uint8_t i = 0; i < size; i++)
    {
        chars[i] = (val >> (8 * i)) & 0xFF;
    }
    
    file.write(chars, size);
    delete[] chars;
}
void util::toChar(std::fstream &file, uint8_t size, uint32_t val)
{
    //using little endian
    char* chars = new char[size];
    for (uint8_t i = 0; i < size; i++)
    {
        chars[i] = (val >> (8 * i)) & 0xFF;
    }
    
    file.write(chars, size);
    delete[] chars;
}


template <typename input, typename output> class DDFile
{
protected:
    std::string _dir;
    std::string _name;
    std::fstream _file; //could arguably be templated
    virtual void writeHeader(int count) {}
public:
    //empty because of 
    DDFile(std::string d, std::string n) : _dir(d), _name(n) {}
    virtual void write(input &container) {} //writes one
    virtual std::vector<output> read() {return std::vector<output>();}   //reads all
    void flush() {_file.flush();}
    ~DDFile() {_file.flush(); _file.close(); std::cout<<"f";}
};

class DDGrid : public DDFile<util::mRen, util::mRen>
{
protected:
    void writeHeader(int count) override {
        std::cout << _file.is_open();
        util::toChar(_file, 2, 0x60DD);
        util::toChar(_file, 2, count);
    }
public:
    DDGrid(std::string d, std::string n) :        DDFile(d, n) {_file.open(_dir + _name, std::ios::in  | std::ios::binary);}
    DDGrid(std::string d, std::string n, int c) : DDFile(d, n) {_file.open(_dir + _name, std::ios::out | std::ios::binary); writeHeader(c);}
    void write(util::mRen &container) override {
        _file.put(container.num);
        _file.put(container.w);
        _file.put(container.h);
        _file.put(container.count);
        for(auto img : container.imgs)
        {
            util::toChar(_file, 2, img);
        }
    }
    std::vector<util::mRen> read() {
        std::vector<util::mRen> output;
        _file.seekg(2);
        uint16_t numGrids = 0;
        _file.read(reinterpret_cast<char*>(&numGrids), 2);

        for (uint16_t i = 0; i < numGrids; i++)
        {
            //header
            util::mRen mts;
            memset(&mts, 0, sizeof(util::mRen));
            _file.read(reinterpret_cast<char*>(&mts), 4);

            //img numbers
            uint16_t inum = 0;
            for (uint8_t i = 0; i < mts.count; i++)
            {
                _file.read(reinterpret_cast<char*>(&inum), 2);
                mts.imgs[i] = inum;
            }
            output.push_back(mts);
        }
        return output;
    }
};

int main()
{
    std::cout << "hi!" << std::endl;
    DDGrid* gridfile = new DDGrid("./", "dave.ddg", 2);
    util::mRen ren = {10, 23, 22, 16, {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16}};
    std::cout << "hi!" << std::endl;
    gridfile->write(ren);
    util::mRen ren2 = {11, 15, 16, 16, {200, 300, 400, 500, 600, 700, 800, 900, 1000, 1100, 1200, 1300, 1400, 1500, 1600, 1700}};
    std::cout << "hi!" << std::endl;
    gridfile->write(ren2);
    delete gridfile;

    gridfile = new DDGrid(".\\", "dave.ddg");
    
    std::vector<util::mRen> readVals = gridfile->read();
    std::cout << std::to_string(readVals[1].imgs[4]) << std::endl;

    return 0;
}