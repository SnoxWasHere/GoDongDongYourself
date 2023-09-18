#include "header\util.hpp"
#include "header\rawimg.hpp"

//annoying circular dependency



void util::memset32(void* dest, uint32_t value, uintptr_t size)
{
    //for detecting unwritten memory with poison value
    uintptr_t i;
    for( i = size - 4; i > 4; i -= 4 ) //back to front
    {
        memcpy(((char*)dest) + i, &value, 4 );
    }  
    for( ; i > 0; i-- )
    {
        ((char*)dest)[i] = 0xFF;
    }  
}

void util::toChar(std::ofstream &file, uint8_t chars[], uint8_t size, uint32_t val)
{
    //using little endian
    for (uint8_t i = 0; i < size; i++)
    {
        chars[i] = (val >> (8 * i)) & 0xFF;
    }
    
    file.write(reinterpret_cast<const char*>(chars), size);
}

void util::rawWrite(std::string name, uint16_t width, uint16_t height, uint8_t* start, util::BGRA* pallete)
{
    //uses RGBA imageMagick file format
    //TODO - smart pointer implementation
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
