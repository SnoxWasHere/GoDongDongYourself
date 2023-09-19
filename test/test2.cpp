#include <fstream>
#include <iostream>

class DDFile
{
protected:
    std::string _dir;
    std::string _name;
    std::fstream _file;
public:
    DDFile(std::string d, std::string n) : _dir(d), _name(n) {std::cout << "hi!";}
    virtual void writeHeader() {}
    virtual void write() {}
    ~DDFile() {std::cout << "bye!";}
};

class DDInfo : public DDFile
{
protected:

public:
    DDInfo(std::string d, std::string n) : DDFile(d, n) {}
};

int main()
{
    for (size_t i = 0; i < 5; i++)
    {
        char chars[i];
        std::cout << sizeof(chars);
    }
    
}