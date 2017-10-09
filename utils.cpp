#include "stdint.h"
#include <vector>
#include <cstring>
#include <string>
#include <iostream>

#include <FreeImagePlus.h>

#include "utils.h"

void WriteBMPFile(std::vector<uint8_t> pixelVec, std::string fileName, uint32_t WIDTH, uint32_t HEIGHT)
{
    BYTE pixels [3*WIDTH*HEIGHT+ sizeof(BITMAPINFOHEADER) + 1 * sizeof(RGBQUAD)];

    for(int i=0; i<3*WIDTH*HEIGHT; i+=3)
    {
        pixels[i+2] = pixelVec[i+0];
        pixels[i+1] = pixelVec[i+1];
        pixels[i+0] = pixelVec[i+2];
    }

    FIBITMAP* Image = FreeImage_ConvertFromRawBits(pixels, WIDTH, HEIGHT, 3 * WIDTH, 24, 0,0,0, true);
    FreeImage_Save(FIF_BMP, Image, fileName.c_str(), 0);
    return;
}


void WriteFile(std::vector<uint16_t> dataVec, std::string fileName, uint32_t WIDTH, uint32_t HEIGHT)
{
       FILE* filePtr = fopen(fileName.c_str(), "wb");

       for(int i=0; i<WIDTH*HEIGHT; i++)
       {
           fwrite(&dataVec[i], 1, sizeof(uint16_t), filePtr);
       }
       std::cout << std::endl;

       fclose(filePtr);
}
