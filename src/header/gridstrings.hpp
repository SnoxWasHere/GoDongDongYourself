#ifndef GRIDSTR_HPP
#define GRIDSTR_HPP
#include "util.hpp"

struct gridStrings
{
    std::string graphics     = ""; //plain images
    std::string tmplGraphics = ""; //template images
    std::string bigCap       = ""; //big caption
    std::string smallCap     = ""; //little caption
    static uint16_t bW; //TODO change these to pointers maybe why dontcha
    static uint16_t bH;
    void writeGraphics(const std::string &d, const unsigned n) {
        graphics.append(d + std::to_string(n) + ".png "); //adds image
    }
    void writeTemplate(const unsigned w, const unsigned h) {
        tmplGraphics.append("xc:White[" + std::to_string(w) + std::string("x") + std::to_string(h) + std::string("!] ")); //adds template
    }

    inline std::string getCaption(const unsigned x, const unsigned y, const unsigned ofs, const unsigned val){
        return (" -annotate +" + std::to_string(bW * x) + "+" + std::to_string(bH * y + ofs) + " \"" + std::to_string(val) + "\" ");
    }
    void writeCaptions(const unsigned x, const unsigned y, const util::iRen &img){
        bigCap.append(getCaption(x, y, 12, img.num));
        smallCap.append(getCaption(x, y, 22, img.w));
        smallCap.append(getCaption(x, y, 30, img.h));
    }

    void makeImages(const std::string &dir, const unsigned num){
        std::string tileInfo = " -gravity NorthWest -geometry " + std::string("\"") + std::to_string(bW) + std::string("x") + std::to_string(bH) + "+0+0<>\" -tile 4x -background none " + dir + std::to_string(num);
        system((std::string("magick montage ") + graphics     + tileInfo + "g.png" ).c_str());
        system((std::string("magick montage ") + tmplGraphics + tileInfo + "gt.png").c_str());
        //overlay captions on top
        system((std::string("magick convert ") + dir + std::to_string(num) + std::string("gt.png -pointsize 14 ") + bigCap + " -pointsize 10 " + smallCap + dir + std::to_string(num) + std::string("gt.png")).c_str());
    }

};
uint16_t gridStrings::bW = 0;
uint16_t gridStrings::bH = 0;

#endif