#ifndef CONTEXT_H
#define CONTEXT_H

#include <list>
#include <vector>

template <class T> class Vec3;
class FreenectDevice;
class Camera;
class Context;

class ContextViewPort
{
protected:
    Context* context;
public:
    virtual void update(std::vector<uint8_t> &rgb, std::vector<uint16_t> &depth) = 0;

    friend class Context;
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
    Context();
public:

    int window;
    FreenectDevice* device;
    double freenect_angle;
    Camera* cam;
    Vec3<int>* boxPos;
    Vec3<int>* boxDim;

    int width;
    int height;
    float f;

    std::list<ContextViewPort*> viewports;
    std::list<ContextAction*> actions;

    std::vector<uint8_t> rgb; //RGB Kinect captured
    std::vector<uint16_t> depth; //Depth Image Kinect captured

    std::vector<uint8_t> rgbInBoxXY; //RGB in the BOX
    std::vector<uint8_t> depthImageInBoxXY; //Depth Image in the BOX
    std::vector<uint16_t> depthInBoxXY; //Depth in the BOX

    std::vector<uint8_t> depthImageInBoxXYZ; //Depth Image in the BOX
    std::vector<uint16_t> depthInBoxXYZ; //Depth in the BOX


    static Context *instance()
    {
        if (!_instance)
          _instance = new Context;
        return _instance;
    }

    void init();
    void initGlLoop(int argc, char **argv);

    void addViewport(ContextViewPort*);
    void notify();

    void addAction(const unsigned char k, ContextAction* action);

    void keyPressed(unsigned char key, int x, int y);
};


#endif // CAMERA_H
