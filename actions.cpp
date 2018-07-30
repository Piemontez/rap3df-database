#include <algorithm>

#include "actions.h"
#include "camera.h"
#include "freenectdevice.h"
#include "utils.h"

#include "json/value.h"
#include "json/reader.h"
#include "json/writer.h"

std::string UUID() {
    std::string uuid;
    uuid.reserve(7);

    srand (time(NULL));

    const char* charmap = "ABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890";
    const size_t charmapLength = 36;
    auto generator = [&](){ return charmap[rand()%charmapLength]; };

    std::generate_n(std::back_inserter(uuid), 7, generator);

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
    std::string path;
    Json::Value root;
    Json::Reader reader;
    Json::StyledWriter writer;

    {//Cria o diretório
        path.append("mkdir ").append(IMAGES_DIR);
        system(path.c_str());

        path.clear();
        path.append("mkdir ").append(IMAGES_DIR).append("/").append(uuid);
        system(path.c_str());
    }

    {//Carrega o arquivo json
        std::string csvFilePath;
        csvFilePath.append(IMAGES_DIR).append("/").append(JSON_IMAGES_INFO);

        std::FILE * csvFile;
        csvFile = std::fopen(csvFilePath.c_str(),"a");
    }


    reader.parse("{ faces: [] }", root);

    if (root[uuid].empty())
    {
        root["_faces"].append(uuid);

        Json::Value images;
        images["front"];
        images["left"];
        images["right"];
        images["top"];
        images["down"];

        root[uuid] = images;
    }

    writer.write( root );

    switch (context->currImageType) {
    case value:

        break;
    default:
        break;
    }

    if (context->curr_rgbImageXY.size()) {
        std::string path;
        path.append(IMAGES_DIR).append("/").append(this->context->uuid).append("/").append(KINECT_1_XY_FILE);

//        WriteBMPFile(context->curr_rgbImageXY, path, w, h);
    }



    std::cout << root << std::endl;


//    //Save csv database info
//    std::FILE * csvFile;
//    csvFile = std::fopen(csvFilePath.c_str(),"a");
//    if (csvFile)
//    {
//        std::string info;

//        info += this->context->uuid + ";";
//        info += std::to_string(w)+ ";";
//        info += std::to_string(h)+ ";";
//        info += "\r\n";

//        std::fputs(info.c_str(),csvFile);
//        std::fclose (csvFile);
//    }

//    //
//    // KINECT 1
//    //
//    //Save original rgb image
//    if (context->curr_rgbImageXY.size()) {
//        path.clear();
//        path.append(IMAGES_DIR).append("/").append(this->context->uuid).append("/").append(KINECT_1_XY_FILE);
//        WriteBMPFile(context->curr_rgbImageXY, path, w, h);
//    }

//    //Save original bitmap deth image for view.
//    if (context->curr_irImageXYZ.size()) {
//        path.clear();
//        path.append(IMAGES_DIR).append("/").append(this->context->uuid).append("/").append(KINECT_1_XYZ_IR_VIEW_FILE);
//        WriteBMPFile(context->curr_irImageXYZ, path, w, h);
//    }

//    //Save original bitmap deth image for view.
//    if (context->curr_depthImageXYZ.size()) {
//        path.clear();
//        path.append(IMAGES_DIR).append("/").append(this->context->uuid).append("/").append(KINECT_1_XYZ_DEPTH_VIEW_FILE);
//        WriteBMPFile(context->curr_depthImageXYZ, path, w, h);
//    }


//    //Save original bitmap deth image for view.
//    if (context->curr_depthXY.size()) {
//        path.clear();
//        path.append(IMAGES_DIR).append("/").append(this->context->uuid).append("/").append(KINECT_1_XY_DATA_FILE);
//        WriteFile(context->curr_depthXY, path, w, h);
//    }


//    //Save original bitmap deth image for view.
//    if (context->curr_depthXYZ.size()) {
//        path.clear();
//        path.append(IMAGES_DIR).append("/").append(this->context->uuid).append("/").append(KINECT_1_XYZ_DATA_FILE);
//        WriteFile(context->curr_depthXYZ, path, w, h);
//    }

//    //Save original bitmap deth image for view.
//    if (context->curr_irXYZ.size()) {
//        path.clear();
//        path.append(IMAGES_DIR).append("/").append(this->context->uuid).append("/").append(KINECT_1_XYZ_IR_FILE);
//        WriteFile(context->curr_irXYZ, path, w, h);
//    }
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

