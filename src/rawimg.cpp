#include "header\rawimg.hpp"

std::string RawImage::_dir = "./";
void RawImage::writeColor(util::BGRA &color)
{
    file.put(color.r);
    file.put(color.g);
    file.put(color.b);
    //.player files use 0,0,0,0 as "true" transparency
    if (color.r == 0 && color.g == 0 && color.b == 0) {file.put(0x00);} 
    else                                              {file.put(0xFF);}
}
void RawImage::createPNG()
{
    file.flush();
    //have to declare size & depth when using RGBA w/ magick
    //TO-DO - async with functioning file deletion
    std::string com = std::string("magick ")
                + std::string("-size ") + std::to_string(_w) + "x" + std::to_string(_h)
                + std::string (" -depth 8 ") 
                + _dir + _name + std::string(".RGBA ")
                + _dir + _name + std::string(".png");
    system(com.c_str());
}
RawImage::~RawImage()
{
    file.flush();
    file.close();
    this->deleteFile();
}