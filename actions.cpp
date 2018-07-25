#include <algorithm>

#include "actions.h"
#include "camera.h"
#include "freenectdevice.h"
#include "utils.h"

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


void GenerateUUIDAction::exec() {
    this->context->uuid = UUID();
}

void CreateImagesCacheAction::exec() {
    context->curr_rgbImageXY = context->rgbImageXY;
    context->curr_irImageXYZ = context->irImageXYZ;
    context->curr_depthImageXYZ = context->depthImageXYZ;
    context->curr_depthXY = context->depthXY;
    context->curr_depthXYZ = context->depthXYZ;
    context->curr_irXYZ = context->irXYZ;
}

void SaveImagesAction::exec() {
#ifdef KINECT1
    int w = extractEndX - extractBegX;
    int h = extractEndY - extractBegY;
#else
    int w = context->boxDim->getX() * 2 - 1;
    int h = context->boxDim->getY() * 2 - 1;
#endif

    std::string path;
    path.append("mkdir ").append(IMAGES_DIR);
    system(path.c_str());

    path.clear();
    path.append("mkdir ").append(IMAGES_DIR).append("/").append(this->context->uuid);
    system(path.c_str());

    std::string csvFilePath;
    csvFilePath.append(IMAGES_DIR).append("/").append(CSV_IMAGES_INFO);

    //Save csv database info
    std::FILE * csvFile;
    csvFile = std::fopen(csvFilePath.c_str(),"a");
    if (csvFile)
    {
        std::string info;

        info += this->context->uuid + ";";
        info += std::to_string(w)+ ";";
        info += std::to_string(h)+ ";";
        info += "\r\n";

        std::fputs(info.c_str(),csvFile);
        std::fclose (csvFile);
    }

    //
    // KINECT 1
    //
    //Save original rgb image
    path.clear();
    path.append(IMAGES_DIR).append("/").append(this->context->uuid).append("/").append(KINECT_1_XY_FILE);
    WriteBMPFile(context->curr_rgbImageXY, path, w, h);

    //Save original bitmap deth image for view.
    path.clear();
    path.append(IMAGES_DIR).append("/").append(this->context->uuid).append("/").append(KINECT_1_XYZ_IR_VIEW_FILE);
    WriteBMPFile(context->curr_irImageXYZ, path, w, h);

    //Save original bitmap deth image for view.
    path.clear();
    path.append(IMAGES_DIR).append("/").append(this->context->uuid).append("/").append(KINECT_1_XYZ_DEPTH_VIEW_FILE);
    WriteBMPFile(context->curr_depthImageXYZ, path, w, h);


    //Save original bitmap deth image for view.
    path.clear();
    path.append(IMAGES_DIR).append("/").append(this->context->uuid).append("/").append(KINECT_1_XY_DATA_FILE);
    WriteFile(context->curr_depthXY, path, w, h);


    //Save original bitmap deth image for view.
    path.clear();
    path.append(IMAGES_DIR).append("/").append(this->context->uuid).append("/").append(KINECT_1_XYZ_DATA_FILE);
    WriteFile(context->curr_depthXYZ, path, w, h);

    //Save original bitmap deth image for view.
    path.clear();
    path.append(IMAGES_DIR).append("/").append(this->context->uuid).append("/").append(KINECT_1_XYZ_IR_FILE);
    WriteFile(context->curr_irXYZ, path, w, h);
}

void SaveTestImagesAction::exec() {
    int w = context->boxDim->getX() * 2 - 1;
    int h = context->boxDim->getY() * 2 - 1;

    std::string path;
    path.append("mkdir ").append(IMAGES_DIR);
    system(path.c_str());

    path.clear();
    path.append("mkdir ").append(IMAGES_DIR).append("/").append(this->context->uuid);
    system(path.c_str());

    path.clear();
    path.append("mkdir ").append(IMAGES_DIR).append("/").append(this->context->uuid).append("/").append(TEST_DIR);
    system(path.c_str());


    std::string prefixFilesName = UUID();
    std::string folderTest;
    folderTest.append(IMAGES_DIR).append("/").append(this->context->uuid).append("/").append(TEST_DIR).append("/");

    std::string csvFilePath;
    csvFilePath.append(folderTest).append(CSV_IMAGES_INFO);


    //Save csv database info
    std::FILE * csvFile;
    csvFile = std::fopen(csvFilePath.c_str(),"a");
    if (csvFile)
    {
        std::string info;

        info += prefixFilesName + ";";
        info += std::to_string(w)+ ";";
        info += std::to_string(h)+ ";";
        info += "\r\n";

        std::fputs(info.c_str(),csvFile);
        std::fclose (csvFile);
    }

    //
    // KINECT 1
    //
    //Save original rgb image
    path.clear(); path.append(folderTest).append(prefixFilesName).append(KINECT_1_XY_FILE);
    WriteBMPFile(context->curr_rgbImageXY, path, w, h);

    //Save original bitmap deth image for view.
    path.clear(); path.append(folderTest).append(prefixFilesName).append(KINECT_1_XYZ_IR_VIEW_FILE);
    WriteBMPFile(context->curr_irImageXYZ, path, w, h);

    //Save original bitmap deth image for view.
    path.clear(); path.append(folderTest).append(prefixFilesName).append(KINECT_1_XY_DATA_FILE);
    //        WriteFile(context->depthInBoxXY, path, w, h);

    //Save original bitmap deth image for view.
    path.clear(); path.append(folderTest).append(prefixFilesName).append(KINECT_1_XYZ_DEPTH_VIEW_FILE);
    WriteBMPFile(context->curr_depthImageXYZ, path, w, h);

    //Save original bitmap deth image for view.
    path.clear(); path.append(folderTest).append(prefixFilesName).append(KINECT_1_XYZ_DATA_FILE);
    WriteFile(context->curr_depthXYZ, path, w, h);
}
