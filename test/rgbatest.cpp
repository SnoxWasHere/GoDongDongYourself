#include <iostream>
#include <fstream>
#include <stdint.h>
#include <windows.h>
using namespace std;

namespace util
{
    struct BGRA;
    void rawWrite();
}
void util::rawWrite(string name, uint16_t width, uint16_t height, uint8_t* start, util::BGRA* pallete)
{
    //uses RGBA imageMagick file format
    RawImage* image = new RawImage(name, width, height);
    uint8_t pbt;
    for (uint32_t j = 0; j < height; j++)
    {
        for (uint32_t i = 0; i < width; i++)
        {
            pbt = *(start + i + (j * width));
            image->writeColor(pallete[pbt]);
        }
    }
    image->createPNG();
    delete image;
}

struct util::BGRA
{
    uint8_t b;
    uint8_t g;
    uint8_t r;
    uint8_t a;
};

class RawImage
{
private:
    static string _dir;
    string _name;
    uint16_t _w;
    uint16_t _h;
public:
    ofstream file;
    static inline void setDir(string str) {RawImage::_dir = str;}
    RawImage(string n, uint16_t w, uint16_t h) : _name(n), _w(w), _h(h) {this->file.open(_dir + _name + ".RGBA", ios::binary);}
    void writeColor(util::BGRA &color);
    void createPNG();
    ~RawImage();
};
string RawImage::_dir = "./";

void RawImage::writeColor(util::BGRA &color)
{
    file.put(color.r);
    file.put(color.g);
    file.put(color.b);
    if (color.r == 0 && color.g == 0 && color.b == 0) {file.put(0xFF);}
    else                                              {file.put(0x00);}
}

void RawImage::createPNG()
{
    string com = string("magick ")
                + string("-size ") + to_string(_w) + "x" + to_string(_h)
                + string (" -depth 8 ") 
                + _dir + _name + string(".RGBA ")
                + _dir + _name + string(".png");
    
    system(com.c_str());
}

RawImage::~RawImage()
{
    file.flush();
    file.close();
    remove((_dir + _name + ".RGBA").c_str());
}

int main()
{

}