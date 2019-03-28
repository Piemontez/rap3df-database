#ifndef CONTEXT_H
#define CONTEXT_H

#include <list>
#include <vector>

template <class T> class Vec3;

#include <libfreenect2/libfreenect2.hpp>
#include <libfreenect2/frame_listener_impl.h>
#include <libfreenect2/registration.h>
#include <libfreenect2/packet_pipeline.h>
#include <libfreenect2/logger.h>

class Camera;
class Context;
class ContextAction;

const int STEP_FINISHED    = -1;
const int STEP_NONE        = 1;
const int STEP_START       = 2;
const int STEP_CACHE_IMAGE = 4;
const int STEP_DEMOGRAPHI  = 8;
const int STEP_IMAGES_STEPS = STEP_NONE | STEP_START | STEP_CACHE_IMAGE;

const int STEP_DEMOG_NONE   = 0;
const int STEP_DEMOG_YEAR   = 1;
const int STEP_DEMOG_GENDER = 2;
const int STEP_DEMOG_COLOR  = 3;
const int STEP_DEMOG_WEIGHT = 4;
const int STEP_DEMOG_HEIGHT = 5;

const int IMAGE_TYPE_FRONT  = 1;
const int IMAGE_TYPE_UP     = 2;
const int IMAGE_TYPE_DOWN   = 4;
const int IMAGE_TYPE_LEFT   = 8;
const int IMAGE_TYPE_RIGHT  = 16;
const int IMAGE_TYPE_BURNED = 32;

class ContextViewPort
{
protected:
    Context* context;
    int window{0};
    int flags;
public:
    explicit ContextViewPort(int _flags = 0);
    virtual void update(int window) = 0;

    friend class Context;
    friend class ContextAction;
};

class ContextAction
{
protected:
    Context* context;
    char key;
    int step;
public:
    virtual void exec(char key) = 0;

    friend class Context;
};


class Context
{
    static Context *_instance;
    static int window1;//TODO: REVER
    static int window2;//TODO: REVER    
    Context();
public:
    std::string uuid;
    int step{STEP_NONE};
    int imagesSaved{0};
    int currImageType{0};
    int errorCode{0}; //1 ,2: Nenhuma imagem capturada

    int demographicStep{STEP_DEMOG_NONE};
    std::string demographicYaers;
    std::string demographicGender;
    std::string demographicColor;
    std::string demographicWeight;
    std::string demographicHeight;

    double freenect_angle;
    Camera* cam;
    Vec3<int>* boxPos;
    Vec3<int>* boxDim;

    int width(int window);
    int height(int window);
    int width1;
    int height1;
    int width2;
    int height2;
    float f;

    libfreenect2::Freenect2 freenect2;
    libfreenect2::Freenect2Device *dev = 0;
    libfreenect2::PacketPipeline *pipeline = 0;

    std::string serial;

    libfreenect2::SyncMultiFrameListener* listener{0};
    libfreenect2::Registration* registration;

    libfreenect2::FrameMap frames;
    libfreenect2::Frame* undistorted;
    libfreenect2::Frame* registered;

    libfreenect2::Frame *_rgb{0};
    libfreenect2::Frame *_ir{0};
    libfreenect2::Frame *_depth{0};

    std::vector<uint16_t> depth;

    std::vector<uint8_t> rgbImage;
    std::vector<uint8_t> irImageBgRm;
    std::vector<uint8_t> depthImageBgRm;
    std::vector<uint8_t> rgbImageBgRm;
    std::vector<uint16_t> irDataBgRm;
    std::vector<uint16_t> depthDataBgRm;

    std::vector<uint8_t> sel_rgbImage;
    std::vector<uint8_t> sel_irImageBgRm;
    std::vector<uint8_t> sel_depthImageBgRm;
    std::vector<uint8_t> sel_rgbImageBgRm;
    std::vector<uint16_t> sel_irDataBgRm;
    std::vector<uint16_t> sel_depthDataBgRm;

    std::list<ContextViewPort*> viewports;
    std::list<ContextAction*> actions;

    static Context *instance()
    {
        if (!_instance)
          _instance = new Context;
        return _instance;
    }

    void init(int argc, char **argv);
    int initWindow(const char *title);
    void start();
    void clearSelected();

    void addViewport(const int window, ContextViewPort*);
    void notify(int window);

    void addAction(const unsigned char k, ContextAction* action, int step = 0);

    void keyPressed(int key, int x, int y);
    void keyPressed(unsigned char key, int x, int y);
};


#endif // CAMERA_H
