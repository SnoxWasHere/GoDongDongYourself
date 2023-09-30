#ifndef RAWIMG_HPP
#define RAWIMG_HPP

#include "util.hpp"

class RawImage
{
private:
    static std::string _dir;
    std::string _name;
    uint16_t _w;
    uint16_t _h;
public:
    std::ofstream file;
    static inline void setDir(std::string &str) {RawImage::_dir = str;}
    RawImage(std::string &n, uint16_t w, uint16_t h) : _name(n), _w(w), _h(h) {this->file.open(_dir + _name + ".RGBA", std::ios::binary);}
    void writeColor(util::BGRA &color);
    void createPNG();
    inline void deleteFile() {remove((_dir + _name + ".RGBA").c_str());}
    ~RawImage();
};

#endif