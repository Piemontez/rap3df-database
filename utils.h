#ifndef UTILS_H
#define UTILS_H

#define IMAGES_DIR "rap3df_data_02"
#define JSON_IMAGES_INFO "database.json"

#define FILE_BMP_RGB "rgb_ID.bmp"
#define FILE_BMP_IR_BG_RM "ir_rmBg_ID.bmp"
#define FILE_BMP_DEPTH_BG_RM "depth_bgRm_ID.bmp"
#define FILE_BMP_DEPTH_RGB_BG_RM "depth_rgb_bgRm_ID.bmp"

#define FILE_DATA_DEPTH "depth_ID.data"
#define FILE_DATA_DEPTH_BG_REM "depth_bgRm_ID.data"
#define FILE_DATA_IR_BG_REM "ir_bgRm_ID.data"

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
