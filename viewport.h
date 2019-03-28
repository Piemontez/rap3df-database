#ifndef VIEWPORT_H
#define VIEWPORT_H

#include "context.h"

class BoxCamViewPort: public ContextViewPort
{
public:
    void update(int window);
};

/*
 * Display info
 */
class InfoViewPort: public ContextViewPort
{
public:
    void update(int window);
};

/*
 * Display info 2
 */
class InfoVoluntaryViewPort: public ContextViewPort
{
public:
    void update(int window);
};

/*
 * Display stereo cam points with rgb color
 */
class PointCamViewPort: public ContextViewPort
{
public:
    void update(int window);
};


/*
 * Display stereo cam with basic algorithm for triangles
 */
class TriangleCamViewPort: public ContextViewPort
{
public:
    explicit TriangleCamViewPort(int _flags): ContextViewPort(_flags) {}
    void update(int window);
};


class BoxExtractViewPort: public ContextViewPort
{
public:
    void update(int window);
};

class FrontCamViewPort: public ContextViewPort
{
public:
    explicit FrontCamViewPort(int _flags): ContextViewPort(_flags) {}
    void update(int window);
};


class LeftCamViewPort: public ContextViewPort
{
public:
    explicit LeftCamViewPort(int _flags): ContextViewPort(_flags) {}

    void update(int window);
};

class RightCamViewPort: public ContextViewPort
{
public:
    explicit RightCamViewPort(int _flags): ContextViewPort(_flags) {}
    void update(int window);
};

#endif // VIEWPORT_H
