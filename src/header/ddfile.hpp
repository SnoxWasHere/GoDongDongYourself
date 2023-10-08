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
    virtual void write(input container) {} //writes one
    virtual std::vector<output> read() {return std::vector<output>();}   //reads all
    void flush() {_file.flush();}
    ~DDFile() {_file.flush(); _file.close();}
};


/***********************************************************************************/
/***********************************************************************************/


class DDInfo : public DDFile<util::hRen&, util::iRen>
{
protected:
    void writeHeader(int count) override;
public:
    //read
    DDInfo(std::string d, std::string n) :        DDFile(d, n) {_file.open(_dir + _name, std::ios::in  | std::ios::binary);}
    //write
    DDInfo(std::string d, std::string n, int c) : DDFile(d, n) {_file.open(_dir + _name, std::ios::out | std::ios::binary); writeHeader(c);}

    void write(util::hRen &container) override;
    std::vector<util::iRen> read() override; 
};


/***********************************************************************************/
/***********************************************************************************/


class DDGrid : public DDFile<util::mRen&, util::mRen>
{
protected:
void writeHeader(int count) override;
public:
    //read
    DDGrid(std::string d, std::string n) :        DDFile(d, n) {_file.open(_dir + _name, std::ios::in  | std::ios::binary);}
    //write
    DDGrid(std::string d, std::string n, int c) : DDFile(d, n) {_file.open(_dir + _name, std::ios::out | std::ios::binary); writeHeader(c);}
    void write(util::mRen &container) override;
    std::vector<util::mRen> read() override;
};


/***********************************************************************************/
/***********************************************************************************/


class DDSound : public DDFile<util::sRen, util::sRen>
{
protected:
    void writeHeader(int count) override;
public:
    //read
    DDSound(std::string d, std::string n) :        DDFile(d, n) {_file.open(_dir + _name, std::ios::in  | std::ios::binary);}
    //write
    DDSound(std::string d, std::string n, int c) : DDFile(d, n) {_file.open(_dir + _name, std::ios::out | std::ios::binary); writeHeader(c);}
    void write(util::sRen val) override;
    std::vector<util::sRen> read() override;
};

#endif