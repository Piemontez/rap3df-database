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
    int step{0};
    int imagesSaved{0};
    int currImageType{0};
    int errorCode{0}; //1 ,2: Nenhuma imagem capturada

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

    void addViewport(const int window, ContextViewPort*);
    void notify(int window);

    void addAction(const unsigned char k, ContextAction* action);

    void keyPressed(int key, int x, int y);
    void keyPressed(unsigned char key, int x, int y);
};


#endif // CAMERA_H
