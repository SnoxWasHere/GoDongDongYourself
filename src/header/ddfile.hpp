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
    std::fstream _file;
    virtual void writeHeader(int count) {}
public:
    //blank constructor because too lazy for crtp
    DDFile(std::string d, std::string n) : _dir(d), _name(n) {}
    virtual void write(input &container) {} //writes one
    virtual std::vector<output> read() {return std::vector<output>();}   //reads all
    void flush() {_file.flush();}
    ~DDFile() {_file.flush(); _file.close();}
};



class DDInfo : public DDFile<util::hRen, util::iRen>
{
protected:
    void writeHeader(int count) override {
        const uint16_t mWord = 0x10DD;
        _file.write(reinterpret_cast<const char*>(&mWord), 2);
        _file.write(charptr(&count), 2);
    }
public:
    //read
    DDInfo(std::string d, std::string n) :        DDFile(d, n) {_file.open(_dir + _name, std::ios::in  | std::ios::binary);}
    //write
    DDInfo(std::string d, std::string n, int c) : DDFile(d, n) {_file.open(_dir + _name, std::ios::out | std::ios::binary); writeHeader(c);}

    void write(util::hRen &container) override {
        _file.write(charptr(&container.ofs), 4);
        _file.write(charptr(&container.w), 2);
        _file.write(charptr(&container.h), 2);
    }

    std::vector<util::iRen> read() override 
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
    
};

class DDGrid : public DDFile<util::mRen, util::mRen>
{
protected:
void writeHeader(int count) override {
        const uint16_t mWord = 0x60DD;
        _file.write(reinterpret_cast<const char*>(&mWord), 2);
        _file.write(charptr(&count), 2);
    }
public:
    //read
    DDGrid(std::string d, std::string n) :        DDFile(d, n) {_file.open(_dir + _name, std::ios::in  | std::ios::binary);}
    //write
    DDGrid(std::string d, std::string n, int c) : DDFile(d, n) {_file.open(_dir + _name, std::ios::out | std::ios::binary); writeHeader(c);}
    void write(util::mRen &container) override {
        _file.put(container.num);
        _file.put(container.w);
        _file.put(container.h);
        _file.put(container.count);
        for(auto img : container.imgs) {_file.write(charptr(&img), 2);}
    }
    std::vector<util::mRen> read() {
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
};

class DDSound : public DDFile<uint32_t, uint32_t>
{
protected:
    void writeHeader(int count) override {
            const uint16_t mWord = 0x50DD;
            _file.write(reinterpret_cast<const char*>(&mWord), 2);
            _file.write(charptr(&count), 2);
    }
public:
    //read
    DDSound(std::string d, std::string n) :        DDFile(d, n) {_file.open(_dir + _name, std::ios::in  | std::ios::binary);}
    //write
    DDSound(std::string d, std::string n, int c) : DDFile(d, n) {_file.open(_dir + _name, std::ios::out | std::ios::binary); writeHeader(c);}
    void write(uint32_t &size) override {
        _file.write(charptr(size), 4);
    }
    std::vector<uint32_t> read() {
        std::vector<uint32_t> output;
        _file.seekg(2); //magic word
        uint16_t numSounds = 0;
        _file.read(charptr(&numSounds), 2);

        for (uint16_t i = 0; i < numSounds; i++) {
            uint32_t sSize;
            memset(&sSize, 0, 4);
            _file.read(charptr(&sSize), 4);

            output.push_back(sSize);
        }
        return output;
    }
};

#endif