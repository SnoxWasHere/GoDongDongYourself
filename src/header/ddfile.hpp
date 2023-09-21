#ifndef DDFILE_HPP
#define DDFILE_HPP

#include "util.hpp"
#include <vector>
#include <Windows.h>
#include <filesystem>


template <typename input, typename output> class DDFile
{
protected:
    std::string _dir;
    std::string _name;
    std::fstream _file; //could arguably be templated
public:
    //constructor opens file then calls writeHeader
    DDFile(std::string d, std::string n, int c) : _dir(d), _name(n) {_file.open(_dir + _name, std::ios::binary); writeHeader(c);}
    virtual void writeHeader(int count) {}
    virtual void write(input &container) {} //writes one
    virtual std::vector<output> read() {}   //reads all
    ~DDFile() {_file.flush(); _file.close();}
};



class DDInfo : public DDFile<util::hRen, util::iRen>
{
protected:
public:
    DDInfo(std::string d, std::string n, int c) : DDFile(d, n, c) {}
    void writeHeader(int count) override {
        util::toChar(_file, 2, 0x10DD);
        util::toChar(_file, 2, count);
    }
    void write(util::hRen &container) override {
        util::toChar(_file, 4, container.ofs);
        util::toChar(_file, 2, container.w);
        util::toChar(_file, 2, container.h);
    }

    std::vector<util::iRen> read() override 
    {
        std::vector<util::iRen> output;

        //switch this to fstream pls
        HANDLE fil = CreateFile((_dir + _name).c_str(), GENERIC_ALL, FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
        OVERLAPPED ovr;
        ovr.Offset = 0;
        ovr.OffsetHigh = 0;
        ovr.Internal = 0;
        ovr.InternalHigh = 0;
        
        ovr.Offset += 2;
        uint16_t num_img = 0;
        ReadFileEx(fil, &num_img, 2, &ovr, nullptr);
        ovr.Offset += 2;
        util::iRen img;
        for (uint16_t i = 0; i < num_img; i++)
        {
            if (std::filesystem::exists(_dir + std::string("main/") + std::to_string(i) + ".png")) //checks if file has been moved to unchanged
            {
                util::iRen img;
                memset(&img, 0, sizeof(util::iRen));
                ReadFileEx(fil, &img, 8, &ovr, nullptr);
            
                //reading to a struct of different types of ints is difficult so this is the solution
                img.ofs = (img.ofsa) | (img.ofsb << 16); 
                output.push_back(img);
            }
            ovr.Offset += 8;
        }
        CloseHandle(fil);
        return output;
    }
    
};

class DDGrid : public DDFile<util::mRen, util::mRen>
{
    DDGrid(std::string d, std::string n, int c) : DDFile(d, n, c) {}
    void writeHeader(int count) override {
        util::toChar(_file, 2, 0x60DD);
        util::toChar(_file, 2, count);
    }
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
        OVERLAPPED ovr;
        ovr.Offset = 0;
        ovr.OffsetHigh = 0;
        ovr.Internal = 0;
        ovr.InternalHigh = 0;    

        HANDLE fil = CreateFile((_dir + _name).c_str(), GENERIC_ALL, FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);


        ovr.Offset += 2;
        uint16_t num_grids = 0;
        ReadFileEx(fil, &num_grids, 2, &ovr, nullptr);
        ovr.Offset += 2;

        for (uint16_t i = 0; i < num_grids; i++)
        {
            //header
            util::mRen mts;
            memset(&mts, 0, sizeof(mRen));
            uint16_t inum = 0;

            ReadFileEx(fil, &mts, 4, &ovr, nullptr);
            ovr.Offset += 4;

            //img numbers
            for (uint8_t i = 0; i < mts->count; i++)
            {
                ReadFileEx(fil, &inum, 2, &ovr, nullptr);
                ovr.Offset += 2;
                mts->imgs[i] = inum;
            }
            
            mlst.push_back(mts);
        }
        CloseHandle(fil);
    }
}

#endif