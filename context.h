#ifndef CONTEXT_H
#define CONTEXT_H

#include <list>
#include <vector>

template <class T> class Vec3;
#ifdef KINECT1
class FreenectDevice;
#else
#include <libfreenect2/libfreenect2.hpp>
#include <libfreenect2/frame_listener_impl.h>
#include <libfreenect2/registration.h>
#include <libfreenect2/packet_pipeline.h>
#include <libfreenect2/logger.h>
#endif

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
    virtual void update() = 0;

    friend class Context;
    friend class ContextAction;
};

class ContextAction
{
protected:
    Context* context;
    char key;
public:
    virtual void exec() = 0;

    friend class Context;
};


class Context
{
    static Context *_instance;
    static int window1;//TODO: REVER
    static int window2;//TODO: REVER    
    Context();
public:

    double freenect_angle;
    Camera* cam;
    Vec3<int>* boxPos;
    Vec3<int>* boxDim;

    int width;
    int height;
    float f;

    std::string uuidFolderName;

#ifdef KINECT1
    FreenectDevice* device;

    std::vector<uint8_t> rgb; //RGB Kinect captured
    std::vector<uint16_t> depth; //Depth Image Kinect captured

    std::vector<uint8_t> rgbInBoxXY; //RGB in the BOX
    std::vector<uint8_t> depthImageInBoxXY; //Depth Image in the BOX
    std::vector<uint16_t> depthInBoxXY; //Depth in the BOX
#else
    libfreenect2::Freenect2 freenect2;
    libfreenect2::Freenect2Device *dev = 0;
    libfreenect2::PacketPipeline *pipeline = 0;

    std::string serial;

    libfreenect2::SyncMultiFrameListener* listener{0};
    libfreenect2::Registration* registration;

    libfreenect2::FrameMap frames;
    libfreenect2::Frame* undistorted;
    libfreenect2::Frame* registered;

    libfreenect2::Frame *rgb2{0};
    libfreenect2::Frame *ir2{0};
    libfreenect2::Frame *depth2{0};

    std::vector<uint8_t> rgbInBoxXY; //RGB in the BOX
    std::vector<uint16_t> depthInBoxXY; //Depth in the BOX

    std::vector<uint8_t> irImageInBoxXYZ; //Depth Image in the BOX
    std::vector<uint16_t> irInBoxXYZ; //Depth Image in the BOX
#endif

    std::list<ContextViewPort*> viewports;
    std::list<ContextAction*> actions;

    std::vector<uint8_t> depthImageInBoxXYZ; //Depth Image in the BOX
    std::vector<uint16_t> depthInBoxXYZ; //Depth in the BOX


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

    void keyPressed(unsigned char key, int x, int y);
};


#endif // CAMERA_H
