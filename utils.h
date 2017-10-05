#ifndef UTILS_H
#define UTILS_H

#define IMAGES_DIR "rap3df_data"
#define TEST_DIR "tests"
#define CSV_IMAGES_INFO "database.csv"

#define KINECT_1_XY_FILE "k1_box_xy.bmp"
#define KINECT_1_XY_DEPTH_VIEW_FILE "k1_box_xy_depth_view.bmp"
#define KINECT_1_XY_DATA_FILE "k1_box_xy_depth.data"

#define KINECT_1_XYZ_DEPTH_VIEW_FILE "k1_box_xyz_depth_view.bmp"
#define KINECT_1_XYZ_DATA_FILE "k1_box_xyz_depth.data"

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
