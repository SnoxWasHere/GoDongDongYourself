#include <iostream>
#include <fstream>
#include <stdint.h>
using namespace std;

class PixMap
{
protected:
    string _name;
    static string _dir;
    string _ext;
    uint32_t _w;
    uint32_t _h;
    ios::openmode _mode;
    
public:
    ofstream file;
    static inline void setDir(string str) {PixMap::_dir = str;}
    PixMap(string n, string e, uint32_t w, uint32_t h, ios::openmode m) : _name(n), _ext(e), _w(w), _h(h), _mode(m) {this->file.open(_dir + _name + _ext, _mode);}
    virtual void writeHeader() {}
    string get() {return (_dir + _name + _ext);}
    ~PixMap() {this->file.close();}
};
string PixMap::_dir;

class PPM : public PixMap
{
public:
    
    PPM(string n, uint32_t w, uint32_t h) : PixMap(n,".ppm",w,h,ios::binary) {}
    void writeHeader() override 
    {
        this->file << "P3"; //P3 - binary full color
        this->file.put(0x0a);
        this->file << _w << " " << _h;
        this->file.put(0x0a);
        this->file << "255";
        this->file.put(0x0a);
    }
};

class PBM : public PixMap
{
public:
    PBM(string n, uint32_t w, uint32_t h) : PixMap(n,"a.pbm",w,h,ios::out) {}
    void writeHeader() override 
    {
        this->file << "P1" << endl; //P1 - ascii black and white
        this->file << _w << " " << _h << endl;
    }
};

int main()
{
    PixMap::setDir(".///");
    PixMap test("dave", ".ball", 5, 3, ios::out);
    cout << test.get() << endl;

    PPM test2("dave", 7, 12);
    cout << test2.get() << endl;
    test2.writeHeader();

    PBM test3("dave", 6, 4);
    cout << test3.get() << endl;
    test3.writeHeader();
    return 0;
}