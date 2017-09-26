#ifndef CONTEXT_H
#define CONTEXT_H

#include <list>
#include <vector>

template <class T> class Vec3;
class FreenectDevice;
class Camera;

class ContextViewPort
{
public:
    virtual void update(std::vector<uint8_t> &rgb, std::vector<uint16_t> &depth) = 0;
};

class Context
{
    static Context *_instance;
    Context();
public:

    int window;
    FreenectDevice* device;
    Camera* cam;
    Vec3<int>* boxPos;
    Vec3<int>* boxDim;

    int width;
    int height;

    std::list<ContextViewPort*> viewports;

    std::vector<uint8_t> rgb;
    std::vector<uint16_t> depth;

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
};


#endif // CAMERA_H
