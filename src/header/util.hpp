#ifndef UTIL_HPP
#define UTIL_HPP

#include <stdint.h>
#include <fstream>
#include <iostream>
#define charptr(x) reinterpret_cast<char*>(x)

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
    inline int getLength(std::ifstream &f) {
        f.seekg (0, f.end);
        int flen = f.tellg();
        f.seekg (0, f.beg);
        return flen;
    }
    void memset32(void* dest, uint32_t value, uintptr_t size); //fills back-to-front with 32 bit int
    void rawWrite(std::string name, uint16_t width, uint16_t height, uint8_t* start, util::BGRA* pallete); //writes .RGBA from .player
}

#endif