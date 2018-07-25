#ifndef VIEWPORT_H
#define VIEWPORT_H

#include "context.h"

class BoxCamViewPort: public ContextViewPort
{
public:
    void update();
};

/*
 * Display cam position info
 */
class InfoViewPort: public ContextViewPort
{
public:
    void update();
};

/*
 * Display stereo cam points with rgb color
 */
class PointCamViewPort: public ContextViewPort
{
public:
    void update();
};


/*
 * Display stereo cam with basic algorithm for triangles
 */
class TriangleCamViewPort: public ContextViewPort
{
public:
    void update();
};


class BoxExtractViewPort: public ContextViewPort
{
public:
    void update();
};

class FrontCamViewPort: public ContextViewPort
{
public:
    explicit FrontCamViewPort(int _flags): ContextViewPort(_flags) {}
    void update();
};


class LeftCamViewPort: public ContextViewPort
{
public:
    explicit LeftCamViewPort(int _flags): ContextViewPort(_flags) {}

    void update();
};

class RightCamViewPort: public ContextViewPort
{
public:
    explicit RightCamViewPort(int _flags): ContextViewPort(_flags) {}
    void update();
};

#endif // VIEWPORT_H
