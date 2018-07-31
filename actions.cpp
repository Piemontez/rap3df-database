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
    if (context->rgbImageXY.size()) {
        this->context->step = 2;

        context->curr_rgbImageXY = context->rgbImageXY;
        context->curr_irImageXYZ = context->irImageXYZ;
        context->curr_depthImageXYZ = context->depthImageXYZ;
        context->curr_depthXY = context->depthXY;
        context->curr_depthXYZ = context->depthXYZ;
        context->curr_irXYZ = context->irXYZ;
    } else
        this->context->step = 1;
}

void SaveImagesAction::exec(char key) {
#ifdef KINECT1
    int w = extractEndX - extractBegX;
    int h = extractEndY - extractBegY;
#else
    int w = context->boxDim->getX() * 2 - 1;
    int h = context->boxDim->getY() * 2 - 1;
#endif
    if (!context->currImageType) return;

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

    Json::Value curr;
//    if (context->curr_rgbImageXY.size())
    {
        file = path; file.append("/").append(FILE_BMP_RGB); file.replace(file.find("ID"), 2, fileID.c_str());
        curr["rgbXY"] = file;
//        WriteBMPFile(context->curr_rgbImageXY, path, w, h);
    }
    if (context->curr_irImageXYZ.size()) {
        file = path; file.append("/").append(FILE_BMP_IR_BG_RM); file.replace(file.find("ID"), 2, fileID.c_str());
        curr["irXYZ"] = file;
//        WriteBMPFile(context->curr_irImageXYZ, path, w, h);
    }
    if (context->curr_depthImageXYZ.size()) {
        file = path; file.append("/").append(FILE_BMP_DEPTH_BG_RM); file.replace(file.find("ID"), 2, fileID.c_str());
        curr["depthXYZ"] = file;
//        WriteBMPFile(context->curr_depthImageXYZ, path, w, h);
    }
    if (context->curr_depthXY.size()) {
        file = path; file.append("/").append(FILE_DATA_DEPTH); file.replace(file.find("ID"), 2, fileID.c_str());
        curr["depthXY"] = file;
//        WriteFile(context->curr_depthXY, path, w, h);
    }
    if (context->curr_depthXYZ.size()) {
        file = path; file.append("/").append(FILE_DATA_DEPTH_BG_REM); file.replace(file.find("ID"), 2, fileID.c_str());
        curr["depthXYZ"] = file;
//        WriteFile(context->curr_depthXYZ, path, w, h);
    }
    if (context->curr_irXYZ.size()) {
        file = path; file.append("/").append(FILE_DATA_IR_BG_REM); file.replace(file.find("ID"), 2, fileID.c_str());
        curr["irXYZ"] = file;
//        WriteFile(context->curr_irXYZ, path, w, h);
    }

    if (curr.empty()) return;


    {//Cria o diretório
        std::string mkdir = "mkdir -p ";
        mkdir.append(path);
        system(mkdir.c_str());
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
}

void EnableNewDataCollectionAction::exec(char key)
{
    this->context->step = -1;
    this->context->currImageType = 0;
}

