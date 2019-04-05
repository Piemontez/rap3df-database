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

    srand (static_cast<unsigned int>(time(nullptr)));

    const char* charmap = "ABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890";
    const size_t charmapLength = 36;
    auto generator = [&](){ return charmap[static_cast<unsigned int>(rand())%charmapLength]; };

    std::generate_n(std::back_inserter(uuid), size, generator);

    return uuid;
}


void GenerateUUIDAction::exec(char) {
    if (this->context->step == STEP_NONE || this->context->step == STEP_FINISHED) {
        this->context->step = STEP_START;
        this->context->uuid = UUID();
    }
}

void CreateImagesCacheAction::exec(char) {
    if (context->rgbImageBgRm.size()) {
        this->context->step = STEP_CACHE_IMAGE;

        context->sel_rgbImage = context->rgbImage;
        context->sel_irImageBgRm = context->irImageBgRm;
        context->sel_depthImageBgRm = context->depthImageBgRm;
        context->sel_rgbImageBgRm = context->rgbImageBgRm;

        context->sel_depthDataBgRm = context->depthDataBgRm;
        context->sel_irDataBgRm = context->irDataBgRm;
    } else
        this->context->step = STEP_START;
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
            while (std::fgets(buf, sizeof buf, csvFile) != nullptr) {
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

        images["demography"];
        images["front"];
        images["up"];
        images["down"];
        images["left"];
        images["right"];
    } else {
        images = root[uuid];
    }


    switch (context->currImageType) {
    case IMAGE_TYPE_FRONT://Front
        images["front"].append(curr);
        break;
    case IMAGE_TYPE_UP://Up
        images["up"].append(curr);
        break;
    case IMAGE_TYPE_DOWN://Down
        images["down"].append(curr);
        break;
    case IMAGE_TYPE_LEFT://Left
        images["left"].append(curr);
        break;
    case IMAGE_TYPE_RIGHT://Right
        images["right"].append(curr);
        break;
    case IMAGE_TYPE_BURNED://Burned
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

    //Limpar informações
    this->context->step = STEP_START;
    this->context->sel_depthImageBgRm.clear();
    this->context->sel_rgbImage.clear();
    this->context->sel_irImageBgRm.clear();
    this->context->sel_depthImageBgRm.clear();
    this->context->sel_rgbImageBgRm.clear();
    this->context->sel_depthDataBgRm.clear();
    this->context->sel_irDataBgRm.clear();

    std::cout << this->context->imagesSaved << std::endl;

}

void SetImageTypeAction::exec(char key)
{
    switch (key)
    {
        case 'f':
            this->context->currImageType = IMAGE_TYPE_FRONT;
            break;
        case 'u':
            this->context->currImageType = IMAGE_TYPE_UP;
            break;
        case 'd':
            this->context->currImageType = IMAGE_TYPE_DOWN;
            break;
        case 'l':
            this->context->currImageType = IMAGE_TYPE_LEFT;
            break;
        case 'r':
            this->context->currImageType = IMAGE_TYPE_RIGHT;
            break;
        case 'b':
            this->context->currImageType = IMAGE_TYPE_BURNED;
            break;
    }
    if (this->context->step == STEP_CACHE_IMAGE) {
        this->context->step = STEP_START;
        this->context->clearSelected();
    }
}

void SetDemograpichInfoAction::exec(char key)
{
    if (key == 0x0d) {
        this->context->demographicStep++;
    } else if (key == 0x08 || key == 0x1b) {
        switch (this->context->demographicStep) {
        case STEP_DEMOG_YEAR:
            this->context->demographicYaers = "";
            break;
        case STEP_DEMOG_WEIGHT:
            this->context->demographicWeight = "";
            break;
        case STEP_DEMOG_HEIGHT:
            this->context->demographicHeight = "";
            break;

        }
    } else {
        switch (this->context->demographicStep) {
        case STEP_DEMOG_YEAR:
            this->context->demographicYaers += key;
            break;
        case STEP_DEMOG_GENDER:
            if (key == 'f' || key == 'm') {
                this->context->demographicGender = key;
            }
            break;
        case STEP_DEMOG_COLOR:
            switch (key) {
            case '1':
                this->context->demographicColor = "branca";
                break;
            case '2':
                this->context->demographicColor = "preta";
                break;
            case '3':
                this->context->demographicColor = "parda";
                break;
            case '4':
                this->context->demographicColor = "amarela";
                break;
            case '5':
                this->context->demographicColor = "indigena";
                break;
            }
            break;
        case STEP_DEMOG_WEIGHT:
            this->context->demographicWeight += key;
            break;
        case STEP_DEMOG_HEIGHT:
            this->context->demographicHeight += key;
            break;
        }
    }
}



void EnableNewDataCollectionAction::exec(char)
{
    if (this->context->demographicStep != STEP_DEMOG_NONE) {
        this->context->demographicStep = STEP_DEMOG_NONE;


        const std::string &uuid = this->context->uuid;

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

        {//Carrega o arquivo json
            std::FILE * csvFile;
            csvFile = std::fopen(csvFilePath.c_str(),"r");

            std::string json;
            if (csvFile) {
                char buf[2];
                while (std::fgets(buf, sizeof buf, csvFile) != nullptr) {
                    json.push_back(buf[0]);
                }
                std::fclose(csvFile);
            }
            std::cout << json << std::endl;

            reader.parse(json, root);
        }

        Json::Value demographi;
        if (root[uuid].empty() || root[uuid]["demography"].empty())
        {
            root[uuid]["demography"]["yaers"];
            root[uuid]["demography"]["gender"];
            root[uuid]["demography"]["color"];
            root[uuid]["demography"]["weight"];
            root[uuid]["demography"]["height"];
        } else {
            demographi= root[uuid]["demography"];
        }

        root[uuid]["demography"]["yaers"] = this->context->demographicYaers;
        root[uuid]["demography"]["gender"] = this->context->demographicGender;
        root[uuid]["demography"]["color"] = this->context->demographicColor;
        root[uuid]["demography"]["weight"] = this->context->demographicWeight;
        root[uuid]["demography"]["height"] = this->context->demographicHeight;

        {//Save json database info
            std::cout << root << std::endl;

            std::FILE * csvFile;
            csvFile = std::fopen(csvFilePath.c_str(),"w+");
            if (csvFile) {
                std::fputs(writer.write( root ).c_str(),csvFile);
                std::fclose(csvFile);
            }
        }

        this->context->demographicYaers = "";
        this->context->demographicGender = "";
        this->context->demographicColor = "";
        this->context->demographicWeight = "";
        this->context->demographicHeight = "";

        std::string viewer = "./rap3df-viewer " + uuid;
        system(viewer.c_str());
    }

    this->context->step = STEP_FINISHED;
    this->context->currImageType = 0;
    this->context->imagesSaved = 0;
}


void StartDemographicDataAction::exec(char) {
    if (this->context->step > STEP_NONE) {
        this->context->step = STEP_DEMOGRAPHI;
        this->context->demographicStep = STEP_DEMOG_YEAR;

        this->context->sel_depthImageBgRm.clear();
        this->context->sel_rgbImage.clear();
        this->context->sel_irImageBgRm.clear();
        this->context->sel_depthImageBgRm.clear();
        this->context->sel_rgbImageBgRm.clear();
        this->context->sel_depthDataBgRm.clear();
        this->context->sel_irDataBgRm.clear();
    }
}

void CloseDemographicDataAction::exec(char)
{
    this->context->step = STEP_START;
}
