#include <algorithm>

#include "actions.h"
#include "camera.h"
#include "freenectdevice.h"
#include "utils.h"

#include "json/value.h"
#include "json/reader.h"
#include "json/writer.h"

std::string UUID(int size = 7) {
    std::string uuid;
    uuid.reserve(size);

    srand (time(NULL));

    const char* charmap = "ABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890";
    const size_t charmapLength = 36;
    auto generator = [&](){ return charmap[rand()%charmapLength]; };

    std::generate_n(std::back_inserter(uuid), size, generator);

    return uuid;
}


void GenerateUUIDAction::exec(char key) {
    if (!this->context->step || this->context->step == -1) {
        this->context->step = 1;
        this->context->uuid = UUID();
    }
}

void CreateImagesCacheAction::exec(char key) {
    if (context->rgbImageBgRm.size()) {
        this->context->step = 2;

        context->sel_rgbImage = context->rgbImage;
        context->sel_irImageBgRm = context->irImageBgRm;
        context->sel_depthImageBgRm = context->depthImageBgRm;
        context->sel_rgbImageBgRm = context->rgbImageBgRm;

        context->sel_depthDataBgRm = context->depthDataBgRm;
        context->sel_irDataBgRm = context->irDataBgRm;
    } else
        this->context->step = 1;
}

void SaveImagesAction::exec(char key) {
    int w = context->boxDim->getX() * 2 - 1;
    int h = context->boxDim->getY() * 2 - 1;

    if (!context->currImageType) {
        context->errorCode = 1;
        return;
    }

    const std::string &uuid = this->context->uuid;
    const std::string &fileID = UUID(3);

    std::string path;
    path.append(IMAGES_DIR).append("/").append(uuid);
    std::string csvFilePath;
    csvFilePath.append(IMAGES_DIR).append("/").append(JSON_IMAGES_INFO);
    std::string file;

    Json::Value root;
    Json::Reader reader;
    Json::StyledWriter writer;

    {//Cria o diretório
        std::string mkdir = "mkdir -p ";
        mkdir.append(path);
        system(mkdir.c_str());
    }

    Json::Value curr;
    if (context->sel_rgbImage.size()) {
        file = path; file.append("/").append(FILE_BMP_RGB); file.replace(file.find("ID"), 2, fileID.c_str());
        curr["rgb"] = file;
        WriteBMPFile(context->sel_rgbImage, file, w, h);
    }
    if (context->sel_irImageBgRm.size()) {
        file = path; file.append("/").append(FILE_BMP_IR_BG_RM); file.replace(file.find("ID"), 2, fileID.c_str());
        curr["ir_with_bg"] = file;
        WriteBMPFile(context->sel_irImageBgRm, file, w, h);
    }
    if (context->sel_depthImageBgRm.size()) {
        file = path; file.append("/").append(FILE_BMP_DEPTH_BG_RM); file.replace(file.find("ID"), 2, fileID.c_str());
        curr["depth_with_bg"] = file;
        WriteBMPFile(context->sel_depthImageBgRm, file, w, h);
    }
    if (context->sel_rgbImageBgRm.size()) {
        file = path; file.append("/").append(FILE_BMP_DEPTH_RGB_BG_RM); file.replace(file.find("ID"), 2, fileID.c_str());
        curr["rgb_with_bg"] = file;
        WriteBMPFile(context->sel_rgbImageBgRm, file, w, h);
    }
    if (context->sel_irDataBgRm.size()) {
        file = path; file.append("/").append(FILE_DATA_IR_BG_REM); file.replace(file.find("ID"), 2, fileID.c_str());
        curr["ir_data_with_bg"] = file;
        WriteFile(context->sel_irDataBgRm, file, w, h);
    }
    if (context->sel_depthDataBgRm.size()) {
        file = path; file.append("/").append(FILE_DATA_DEPTH_BG_REM); file.replace(file.find("ID"), 2, fileID.c_str());
        curr["depth_data_with_bg"] = file;
        WriteFile(context->sel_depthDataBgRm, file, w, h);
    }

    if (curr.empty()) {
        context->errorCode = 2;
        return;
    }

    {//Carrega o arquivo json
        std::FILE * csvFile;
        csvFile = std::fopen(csvFilePath.c_str(),"r");

        std::string json;
        if (csvFile) {
            char buf[2];
            while (std::fgets(buf, sizeof buf, csvFile) != NULL) {
                json.push_back(buf[0]);
            }
            std::fclose(csvFile);
        }
        std::cout << json << std::endl;

        reader.parse(json, root);
    }

    Json::Value images;
    if (root[uuid].empty())
    {
        root["_faces"].append(uuid);

        images["front"];
        images["left"];
        images["right"];
        images["top"];
        images["down"];
    } else {
        images = root[uuid];
    }


    switch (context->currImageType) {
    case 1://Front
        images["front"].append(curr);
        break;
    case 2://Top
        images["top"].append(curr);
        break;
    case 4://Down
        images["down"].append(curr);
        break;
    case 8://Left
        images["left"].append(curr);
        break;
    case 16://Right
        images["right"].append(curr);
        break;
    case 32://Burned
        images["burned"].append(curr);
        break;
    }
    root[uuid] = images;


    {//Save json database info
        std::cout << root << std::endl;

        std::FILE * csvFile;
        csvFile = std::fopen(csvFilePath.c_str(),"w+");
        if (csvFile) {
            std::fputs(writer.write( root ).c_str(),csvFile);
            std::fclose(csvFile);
        }
    }

    this->context->imagesSaved |= this->context->currImageType;
    std::cout << this->context->imagesSaved << std::endl;
}

void SetImageTypeAction::exec(char key)
{
    switch (key)
    {
        case 'f':
            this->context->currImageType = 1;
            break;
        case 't':
            this->context->currImageType = 2;
            break;
        case 'd':
            this->context->currImageType = 4;
            break;
        case 'l':
            this->context->currImageType = 8;
            break;
        case 'r':
            this->context->currImageType = 16;
            break;
        case 'b':
            this->context->currImageType = 32;
            break;
    }
    if (this->context->step == 2) {
        this->context->step = 1;
        this->context->clearSelected();
    }
}

void EnableNewDataCollectionAction::exec(char key)
{
    this->context->step = -1;
    this->context->currImageType = 0;
}

