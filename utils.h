#ifndef UTILS_H
#define UTILS_H

struct Pixel
{
    unsigned char Blue;
    unsigned char Green;
    unsigned char Red;
    unsigned char Alpha;
};

void WriteBMPFile(std::vector<uint8_t> pixelVec, std::string fileName, uint32_t WIDTH, uint32_t HEIGHT);

#endif // CAMERA_H
