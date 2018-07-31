#ifndef UTILS_H
#define UTILS_H

#define IMAGES_DIR "rap3df_data"
#define JSON_IMAGES_INFO "database.json"

#define KINECT_1_XY_FILE "rgb_xy.bmp"
#define KINECT_1_XYZ_IR_VIEW_FILE "ir_xy.bmp"
#define KINECT_1_XYZ_DEPTH_VIEW_FILE "depth_xyz.bmp"

#define KINECT_1_XY_DATA_FILE "depth_xy.data"
#define KINECT_1_XYZ_DATA_FILE "depth_xyz.data"
#define KINECT_1_XYZ_IR_FILE "ir_xyz.data"

struct Pixel
{
    unsigned char Blue;
    unsigned char Green;
    unsigned char Red;
    unsigned char Alpha;
};

void WriteBMPFile(std::vector<uint8_t> pixelVec, std::string fileName, uint32_t WIDTH, uint32_t HEIGHT);
void WriteFile(std::vector<uint16_t> dataVec, std::string fileName, uint32_t WIDTH, uint32_t HEIGHT);

#endif // CAMERA_H
