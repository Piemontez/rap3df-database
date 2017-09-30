#include "stdint.h"
#include <vector>
#include <cstring>
#include <string>

#include "utils.h"

uint16_t const BITMAP_FILE_TYPE = 0x4d42; //BM, little-endian
static uint32_t const BITS_PER_BYTE = 8;
static uint32_t const BITS_PER_PIXEL = 24;

#pragma pack(1)
struct BITMAPFILEHEADER {
       //uint16_t   bfType;
       uint32_t   bfSize;
       uint16_t   bfReserved1;
       uint16_t   bfReserved2;
       uint32_t   bfOffBits;
};

struct BITMAPINFOHEADER {
       uint32_t      biSize;
       int32_t       biWidth;
       int32_t       biHeight;
       uint16_t      biPlanes;
       uint16_t      biBitCount;
       uint32_t      biCompression;
       uint32_t      biSizeImage;
       int32_t       biXPelsPerMeter;
       int32_t       biYPelsPerMeter;
       uint32_t      biClrUsed;
       uint32_t      biClrImportant;
};
#pragma pack()

inline uint64_t CalcBitmapFileSize(uint32_t widthBytes, uint32_t height) {
       uint64_t fileSize = widthBytes * height;

       uint32_t padSize = (widthBytes) % sizeof(uint32_t);
       if (padSize > 0) {
              padSize = sizeof(uint32_t) - padSize;
              padSize *= height;
       }
       fileSize += padSize;
       fileSize += sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);


       return fileSize;
}

void WriteBMPFile(std::vector<uint8_t> pixelVec, std::string fileName, uint32_t WIDTH, uint32_t HEIGHT)
{
       FILE             *filePtr;          // file pointer
       BITMAPFILEHEADER bitmapFileHeader;  // bitmap file header
       BITMAPINFOHEADER bitmapInfoHeader;  // bitmap info header
       memset(&bitmapFileHeader, 0, sizeof(bitmapFileHeader));
       memset(&bitmapInfoHeader, 0, sizeof(bitmapInfoHeader));

                                                                     // open file for writing binary mode
       filePtr = fopen(fileName.c_str(), "wb");

       // define the bitmap file header
       //bitmapFileHeader.bfType = BITMAP_FILE_TYPE;
       unsigned short bfType=0x4d42;
       bitmapFileHeader.bfSize = static_cast<uint32_t>(CalcBitmapFileSize(WIDTH * (BITS_PER_PIXEL / BITS_PER_BYTE), HEIGHT));
       bitmapFileHeader.bfReserved1 = 0;
       bitmapFileHeader.bfReserved2 = 0;
       bitmapFileHeader.bfOffBits = sizeof(bitmapFileHeader) + sizeof(bitmapInfoHeader);

       // define the bitmap information header
       bitmapInfoHeader.biSize = sizeof(bitmapInfoHeader);
       bitmapInfoHeader.biWidth = WIDTH;                      // bitmap width
       bitmapInfoHeader.biHeight = HEIGHT;                    // bitmap height
       bitmapInfoHeader.biPlanes = 1;
       bitmapInfoHeader.biBitCount = BITS_PER_PIXEL;
       bitmapInfoHeader.biCompression = 0;               // no compression
       bitmapInfoHeader.biSizeImage = (WIDTH * HEIGHT) * 4;          // width * height
       bitmapInfoHeader.biXPelsPerMeter = 5000;
       bitmapInfoHeader.biYPelsPerMeter = 5000;
       bitmapInfoHeader.biClrUsed = 0;
       bitmapInfoHeader.biClrImportant = 0;

       fwrite(&bfType,1,sizeof(bfType),filePtr);
                                                                                                   // write the bitmap file header
       fwrite(&bitmapFileHeader, 1, sizeof(BITMAPFILEHEADER), filePtr);

       // write the bitmap info header
       fwrite(&bitmapInfoHeader, 1, sizeof(BITMAPINFOHEADER), filePtr);

       for(int i=WIDTH*HEIGHT*3; i>0; i-=3)
       {
           fwrite(&pixelVec[i+2], 1, 1, filePtr);
           fwrite(&pixelVec[i+1], 1, 1, filePtr);
           fwrite(&pixelVec[i], 1, 1, filePtr);
       }

       // close our file
       fclose(filePtr);
}
