#include <algorithm>

#if defined(__APPLE__)
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif

#include "context.h"
#include "camera.h"
#include "freenectdevice.h"

#include <libfreenect2/libfreenect2.hpp>
#include <libfreenect2/frame_listener_impl.h>
#include <libfreenect2/registration.h>
#include <libfreenect2/packet_pipeline.h>
#include <libfreenect2/logger.h>

#include "utils.h"

std::string uuidFolderName;

//Region tested for context->box
int extractBegX = 215;
int extractEndX = 424;
int extractBegY = 100;
int extractEndY = 379;

int minX = 640;
int minY = 640;
int maxX = 0;
int maxY = 0;

#ifdef KINECT1
void MakeVertex(int pos, float depth, int w, int h) {
    glVertex3f( (pos%w- (w-1)/2.f) * depth / Context::instance()->f,  // X = (x - cx) * d / fx
                (pos/w - (h-1)/2.f) * depth / Context::instance()->f,  // Y = (y - cy) * d / fy
                depth );
}
#else

#define depthScale 1.0f

void MakeVertex(float x, float y, float depth, int w, int h) {
    glVertex3f(x-(w/2), y-(h/2), depth * depthScale);
}
#endif

class BoxCamViewPort: public ContextViewPort
{
public:
    void update() {

        Vec3<int>* boxPos = Context::instance()->boxPos;
        Vec3<int>* boxDim = Context::instance()->boxDim;

        glBegin(GL_QUADS);
            glColor4f(0, 0,  1, 0.4);
            glVertex3f(boxPos->getX() + boxDim->getX(), boxPos->getY() + boxDim->getY(), boxPos->getZ() - boxDim->getZ());
            glVertex3f(boxPos->getX() + boxDim->getX(), boxPos->getY() - boxDim->getY(), boxPos->getZ() - boxDim->getZ());
            glVertex3f(boxPos->getX() + boxDim->getX(), boxPos->getY() - boxDim->getY(), boxPos->getZ() + boxDim->getZ());
            glVertex3f(boxPos->getX() + boxDim->getX(), boxPos->getY() + boxDim->getY(), boxPos->getZ() + boxDim->getZ());

            glVertex3f(boxPos->getX() - boxDim->getX(), boxPos->getY() + boxDim->getY(), boxPos->getZ() - boxDim->getZ());
            glVertex3f(boxPos->getX() - boxDim->getX(), boxPos->getY() - boxDim->getY(), boxPos->getZ() - boxDim->getZ());
            glVertex3f(boxPos->getX() - boxDim->getX(), boxPos->getY() - boxDim->getY(), boxPos->getZ() + boxDim->getZ());
            glVertex3f(boxPos->getX() - boxDim->getX(), boxPos->getY() + boxDim->getY(), boxPos->getZ() + boxDim->getZ());

            glColor4f(0, 0,  0.5, 0.2);
            glVertex3f(boxPos->getX() + boxDim->getX(), boxPos->getY() + boxDim->getY(), boxPos->getZ() - boxDim->getZ());
            glVertex3f(boxPos->getX() - boxDim->getX(), boxPos->getY() + boxDim->getY(), boxPos->getZ() - boxDim->getZ());
            glVertex3f(boxPos->getX() - boxDim->getX(), boxPos->getY() + boxDim->getY(), boxPos->getZ() + boxDim->getZ());
            glVertex3f(boxPos->getX() + boxDim->getX(), boxPos->getY() + boxDim->getY(), boxPos->getZ() + boxDim->getZ());

            glVertex3f(boxPos->getX() + boxDim->getX(), boxPos->getY() - boxDim->getY(), boxPos->getZ() - boxDim->getZ());
            glVertex3f(boxPos->getX() - boxDim->getX(), boxPos->getY() - boxDim->getY(), boxPos->getZ() - boxDim->getZ());
            glVertex3f(boxPos->getX() - boxDim->getX(), boxPos->getY() - boxDim->getY(), boxPos->getZ() + boxDim->getZ());
            glVertex3f(boxPos->getX() + boxDim->getX(), boxPos->getY() - boxDim->getY(), boxPos->getZ() + boxDim->getZ());

            glColor4f(0, 1,  0, 0.8);
            glVertex3f(boxPos->getX() + boxDim->getX(), boxPos->getY() - boxDim->getY(), boxPos->getZ() + boxDim->getZ());
            glVertex3f(boxPos->getX() - boxDim->getX(), boxPos->getY() - boxDim->getY(), boxPos->getZ() + boxDim->getZ());
            glVertex3f(boxPos->getX() - boxDim->getX(), boxPos->getY() + boxDim->getY(), boxPos->getZ() + boxDim->getZ());
            glVertex3f(boxPos->getX() + boxDim->getX(), boxPos->getY() + boxDim->getY(), boxPos->getZ() + boxDim->getZ());
        glEnd();
    }
};

/*
 * Display cam position info
 */
class InfoViewPort: public ContextViewPort
{
public:
    void update() {
        int w = extractEndX - extractBegX;
        int h = extractEndY - extractBegY;

        glViewport(0, context->height-30, context->width, context->height);

        glMatrixMode(GL_PROJECTION);
        glPushMatrix();
        {
          glLoadIdentity();


          gluOrtho2D(0.0, context->width, 0.0, context->height);
          glMatrixMode(GL_MODELVIEW);
          glColor3f(0, 0.0, 0.0);
          glPushMatrix();
          {
              glLoadIdentity();

              glRasterPos2i(10, 10);
              std::string s =
                      "UUID " + uuidFolderName
                      + " Box Check" + std::to_string(minX) + 'x' + std::to_string(minY) + "|"
                                  + std::to_string(maxX) + 'x' + std::to_string(maxY)
                      + " width:" + std::to_string(w)
                      + " heigh:" + std::to_string(h);

              for (std::string::iterator i = s.begin(); i != s.end(); ++i)
              {
                char c = *i;
                glutBitmapCharacter(GLUT_BITMAP_9_BY_15, c);
              }

              glMatrixMode(GL_MODELVIEW);
          }
          glPopMatrix();

          glMatrixMode(GL_PROJECTION);
        }
        glPopMatrix();
    }
};

/*
 * Display stereo cam points with rgb color
 */
class PointCamViewPort: public ContextViewPort
{
public:
    void update() {

        glViewport(0, 0, context->width/2, context->height/2);
//        glViewport(0, 0, context->width, context->height);

        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
        {
            glRotatef(context->cam->getXRot(), 1.0f, 0.0f, 0.0f); // Rotate our camera on the x-axis (looking up and down)
            glRotatef(context->cam->getYRot(), 0.0f, 1.0f, 0.0f); // Rotate our camera on the  y-axis (looking left and right)
            glRotatef(context->cam->getZRot(), 0.0f, 0.0f, 1.0f);
//            glTranslatef( -context->cam->getXPos()-(context->width/2), -context->cam->getYPos()-(context->height/3), -context->cam->getZPos()+500 );
            glTranslatef( -context->cam->getXPos(), -context->cam->getYPos(), -context->cam->getZPos() );

            glPointSize(1.0f);
            glBegin(GL_POINTS);

#ifdef KINECT1
            for (int i = 0; i < 480*640; ++i)
            {
                glColor3ub( rgb[3*i+0],    // R
                            rgb[3*i+1],    // G
                            rgb[3*i+2] );  // B

                MakeVertex(i, depth[i]);
            }
#else
            int w = context->depth2->width;
            int h = context->depth2->height;

            for (int y = 0; y < (h -1); ++y)
                for (int x = 0; x < (w -1); ++x)
                {
                    int i = ((y * w) + x) * 4;

                    if (!context->rgb2->status)
                        glColor3ub( context->registered->data[i+2],    // R
                                    context->registered->data[i+1],    // G
                                    context->registered->data[i+0]);  // A

                    if (!context->depth2->status && context->depth2->data[i+2] > 0)
                        MakeVertex(x, y, context->depth2->data[i+2], w, h);

                }
#endif
            glEnd();
        }
    }
};


/*
 * Display stereo cam with basic algorithm for triangles
 */
class TriangleCamViewPort: public ContextViewPort
{
public:
    void update() {

        glViewport(context->width/2, 0, context->width/2, context->height/2);

        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
        {
            glRotatef(context->cam->getXRot(), 1.0f, 0.0f, 0.0f); // Rotate our camera on the x-axis (looking up and down)
            glRotatef(context->cam->getYRot(), 0.0f, 1.0f, 0.0f); // Rotate our camera on the  y-axis (looking left and right)
            glRotatef(context->cam->getZRot(), 0.0f, 0.0f, 1.0f);
//            glTranslatef( -context->cam->getXPos()-(context->width/2), -context->cam->getYPos()-(context->height/3), -context->cam->getZPos()+500 );
            glTranslatef( -context->cam->getXPos(), -context->cam->getYPos(), -context->cam->getZPos() );

            glBegin(GL_TRIANGLES);
#ifdef KINECT1
            for (int j, i = 0; i < 480*638; ++i)
            {
                if (depth[i] > 0 && depth[i+1] > 0 && depth[i+640] > 0 && depth[i+641] > 0) {
                    glColor3ub( rgb[3*i+0],    // R
                                rgb[3*i+1],    // G
                                rgb[3*i+2] );  // B

                    j = i;
                    MakeVertex(j, depth[j]);
                    j = i+1;
                    MakeVertex(j, depth[j]);
                    j = i+640;
                    MakeVertex(j, depth[j]);

                    j = i+1;
                    MakeVertex(j, depth[j]);
                    j = i+640;
                    MakeVertex(j, depth[j]);
                    j = i+641;
                    MakeVertex(j, depth[j]);
                }
            }
#else
            int w = context->depth2->width;
            int h = context->depth2->height;
            int j;

            for (int y = 0; y < (h -1); ++y)
                for (int x = 0; x < (w -1); ++x)
                {
                    int i = ((y * w) + x) * 4;

                    if (!context->rgb2->status)
                        glColor3ub( context->registered->data[i+2],    // R
                                    context->registered->data[i+1],    // G
                                    context->registered->data[i+0]);  // A

                    if (!context->depth2->status)
                    {
                        if (context->depth2->data[i+2] > 0
                            && context->depth2->data[i+6] > 0
                            && context->depth2->data[i+2+(w * 4)] > 0) {
                            j = i;
                            MakeVertex(x, y, context->depth2->data[j+2], w, h);
                            j = i+4;
                            MakeVertex(x+1, y, context->depth2->data[j+2], w, h);
                            j = i+(w * 4);
                            MakeVertex(x, y+1, context->depth2->data[j+2], w, h);
                        }

                        if (context->depth2->data[i+6] > 0
                            && context->depth2->data[i+2+(w * 4)] > 0
                            && context->depth2->data[i+6+(w * 4)] > 0) {
                            j = i+4;
                            MakeVertex(x+1, y, context->depth2->data[j+2], w, h);
                            j = i+(w * 4);
                            MakeVertex(x, y+1, context->depth2->data[j+2], w, h);
                            j = i+(w * 4)+4;
                            MakeVertex(x+1, y+1, context->depth2->data[j+2], w, h);
                        }

                    }

                }

#endif
            glEnd();
        }
    }
};


class BoxExtractViewPort: public ContextViewPort
{
public:
    void update() {
        context->rgbInBoxXY.clear();

        context->depthInBoxXY.clear();
        context->depthImageInBoxXYZ.clear();
        context->depthInBoxXYZ.clear();

        context->irImageInBoxXYZ.clear();
        context->irInBoxXYZ.clear();

#ifdef KINECT1
        std::vector<uint8_t>::iterator itRgb = rgb.begin();
        std::vector<uint16_t>::iterator itDepth = depth.begin();

        for (int j = 0; j < 480; j++) //Rows
            for (int k = 0; k < 640; k++) //Cols
            {
                if (k >= extractBegX && k < extractEndX
                 && j >= extractBegY && j < extractEndY) {
                    context->rgbInBoxXY.push_back(*itRgb++);
                    context->rgbInBoxXY.push_back(*itRgb++);
                    context->rgbInBoxXY.push_back(*itRgb++);

                    context->depthImageInBoxXY.push_back((*itDepth) & 0xff);
                    context->depthImageInBoxXY.push_back((*itDepth) >> 8);
                    context->depthImageInBoxXY.push_back(0);

                    context->depthInBoxXY.push_back(*itDepth);

                    if ((*itDepth) > (context->boxPos->getZ() - context->boxDim->getZ())
                            && (*itDepth) < (context->boxPos->getZ() + context->boxDim->getZ()))
                    {
                        context->depthImageInBoxXYZ.push_back((*itDepth) & 0xff);
                        context->depthImageInBoxXYZ.push_back((*itDepth) >> 8);
                        context->depthImageInBoxXYZ.push_back(0);

                        context->depthInBoxXYZ.push_back(*itDepth);
                    } else {
                        context->depthImageInBoxXYZ.push_back(0);
                        context->depthImageInBoxXYZ.push_back(0);
                        context->depthImageInBoxXYZ.push_back(0);
                        context->depthInBoxXYZ.push_back(0);
                    }
                } else {
                    itRgb+=3;
                }
                itDepth++;
            }
#else
        int w = context->depth2->width;
        int h = context->depth2->height;

        for (int y = 0; y < (h -1); ++y)
            for (int x = 0; x < (w -1); ++x)
            {

                if ((y-(h/2)) > -context->boxDim->getY()
                 && (y-(h/2)) < +context->boxDim->getY()
                 && (x-(w/2)) > (context->boxPos->getX() -context->boxDim->getX())
                 && (x-(w/2)) < (context->boxPos->getX() +context->boxDim->getX())) {

                    int i = ((y * w) + x) * 4;

                    context->rgbInBoxXY.push_back(context->registered->data[i+2]);
                    context->rgbInBoxXY.push_back(context->registered->data[i+1]);
                    context->rgbInBoxXY.push_back(context->registered->data[i+0]);

                    uint16_t depth = context->depth2->data[i+2];
                    context->depthInBoxXY.push_back(depth);

                    if ((depth * depthScale) > (context->boxPos->getZ() - context->boxDim->getZ())
                     && (depth * depthScale) < (context->boxPos->getZ() + context->boxDim->getZ()))
                    {
                        uint16_t depth = context->depth2->data[i+2];
                        context->depthImageInBoxXYZ.push_back(depth & 0xff);
                        context->depthImageInBoxXYZ.push_back(depth >> 8);
                        context->depthImageInBoxXYZ.push_back(0);

                        context->depthInBoxXYZ.push_back(depth);

                        uint16_t ir = context->ir2->data[i+2];
                        context->irImageInBoxXYZ.push_back(ir % 255);
                        context->irImageInBoxXYZ.push_back(ir / 255);
                        context->irImageInBoxXYZ.push_back(0);

                        context->irInBoxXYZ.push_back(ir);
                    } else {
                        context->depthImageInBoxXYZ.push_back(0);
                        context->depthImageInBoxXYZ.push_back(0);
                        context->depthImageInBoxXYZ.push_back(0);
                        context->depthInBoxXYZ.push_back(0);

                        context->irImageInBoxXYZ.push_back(0);
                        context->irImageInBoxXYZ.push_back(0);
                        context->irImageInBoxXYZ.push_back(0);
                        context->irInBoxXYZ.push_back(0);
                    }
                }
            }
#endif
    }
};

class FrontCamViewPort: public ContextViewPort
{
public:
    void update() {

        glViewport(context->width/3, context->height/2, context->width/3, context->height/3);

        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
        {
            glRotatef(180, 1.0f, 0.0f, 0.0f); // Rotate our camera on the x-axis (looking up and down)
            glTranslatef( 0, -0, context->boxDim->getZ());

            glPointSize(1.0f);
            glBegin(GL_POINTS);
#ifdef KINECT1
            int w = extractEndX - extractBegX;
            int h = extractEndY - extractBegY;
            for (int i = 0; i < w*h; ++i)
            {
                if (!context->depthInBoxXYZ[i]) continue;

                glColor4f(1/(((context->depthInBoxXYZ[i]/2) & 0xff) / 100.f),
                          1/((context->depthInBoxXYZ[i] & 0xff) / 100.f),
                          1/(((context->depthInBoxXYZ[i]/4) & 0xff) / 100.f),
                          0.7f);

                glVertex3f( (i%w- (w-1)/2.f) * context->depthInBoxXYZ[i] / Context::instance()->f,  // X = (x - cx) * d / fx
                            (i/w- (h-1)/2.f) * context->depthInBoxXYZ[i] / Context::instance()->f,  // Y = (y - cy) * d / fy
                            context->depthInBoxXYZ[i] );

            }
#else
            int w = context->boxDim->getX() * 2 - 1;
            int h = context->boxDim->getY() * 2 - 1;

            for (int y = 0; y < (h -1); ++y)
                for (int x = 0; x < (w -1); ++x)
                {
                    int i = ((y * w) + x);

                    if (!context->depthInBoxXYZ[i]) continue;

                    glColor3ub( context->rgbInBoxXY[3*i+0],    // R
                                context->rgbInBoxXY[3*i+1],    // G
                                context->rgbInBoxXY[3*i+2]);  // B

                    MakeVertex(x, y, context->depthInBoxXYZ[i], w, h);
                }
#endif
            glEnd();

            glBegin(GL_LINES);
                glColor3f(1, 0,  0);
                glVertex3f(context->boxPos->getX(), context->boxPos->getY() + context->boxDim->getY(), context->boxPos->getZ() - context->boxDim->getZ());
                glVertex3f(context->boxPos->getX(), context->boxPos->getY() - context->boxDim->getY(), context->boxPos->getZ() - context->boxDim->getZ());

                glVertex3f(context->boxPos->getX() + context->boxDim->getX(), context->boxPos->getY(), context->boxPos->getZ() - context->boxDim->getZ());
                glVertex3f(context->boxPos->getX() - context->boxDim->getX(), context->boxPos->getY(), context->boxPos->getZ() - context->boxDim->getZ());
            glEnd();
        }
    }
};


class LeftCamViewPort: public ContextViewPort
{
public:
    void update() {

        glViewport(0, context->height/2, context->width/3, context->height/3);

        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
        {
            glRotatef(180, 1.0f, 0.0f, 0.0f); // Rotate our camera on the x-axis (looking up and down)
            glRotatef(-90, 0.0f, 1.0f, 0.0f); // Rotate our camera on the  y-axis (looking left and right)
            glTranslatef( context->boxPos->getX() + (context->boxDim->getX() * 2), 0, -context->boxPos->getZ() + context->boxDim->getZ() );

            glPointSize(2.0f);
            glBegin(GL_POINTS);
#ifdef KINECT1
            int w = extractEndX - extractBegX;
            int h = extractEndY - extractBegY;
            for (int i = 0; i < w*h; ++i)
            {
                if (!context->depthInBoxXYZ[i]) continue;

                glColor4f(1/((context->depthInBoxXYZ[i] & 0xff) / 100.f),
                          1/((context->depthInBoxXYZ[i] & 0xff) / 100.f),
                          1/((context->depthInBoxXYZ[i] >> 8) / 50.f),
                          0.7f);

                glVertex3f( (i%w- (w-1)/2.f) * context->depthInBoxXYZ[i] / Context::instance()->f,  // X = (x - cx) * d / fx
                            (i/w- (h-1)/2.f) * context->depthInBoxXYZ[i] / Context::instance()->f,  // Y = (y - cy) * d / fy
                            context->depthInBoxXYZ[i] );

            }
#else
            int w = context->boxDim->getX() * 2 - 1;
            int h = context->boxDim->getY() * 2 - 1;

            for (int y = 0; y < (h -1); ++y)
                for (int x = 0; x < (w -1); ++x)
                {
                    int i = ((y * w) + x);

                    if (!context->depthInBoxXYZ[i]) continue;

                    glColor3ub( context->rgbInBoxXY[3*i+0],    // R
                                context->rgbInBoxXY[3*i+1],    // G
                                context->rgbInBoxXY[3*i+2]);  // A

                    MakeVertex(x, y, context->depthInBoxXYZ[i], w, h);
                }
#endif
            glEnd();

            glBegin(GL_LINES);
                glColor3f(1, 0,  0);
                glVertex3f(context->boxPos->getX(), context->boxPos->getY() + context->boxDim->getY(), context->boxPos->getZ() - (0.95f * context->boxDim->getZ()));
                glVertex3f(context->boxPos->getX(), context->boxPos->getY() - context->boxDim->getY(), context->boxPos->getZ() - (0.95f * context->boxDim->getZ()));
            glEnd();
        }
    }
};

class RightCamViewPort: public ContextViewPort
{
public:
    void update() {

        glViewport(context->width/3*2, context->height/2, context->width/3, context->height/3);

        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
        {
            glRotatef(180, 1.0f, 0.0f, 0.0f); // Rotate our camera on the x-axis (looking up and down)
            glRotatef(90, 0.0f, 1.0f, 0.0f); // Rotate our camera on the  y-axis (looking left and right)
            glTranslatef( -context->boxPos->getX() - (context->boxDim->getX() * 2), 0, -context->boxPos->getZ() + context->boxDim->getZ() );


            glPointSize(2.0f);
            glBegin(GL_POINTS);
#ifdef KINECT1
            int w = extractEndX - extractBegX;
            int h = extractEndY - extractBegY;
            for (int i = 0; i < w*h; ++i)
            {
                if (!context->depthInBoxXYZ[i]) continue;

                glColor4f(1/((context->depthInBoxXYZ[i] & 0xff) / 100.f),
                          1/((context->depthInBoxXYZ[i] & 0xff) / 100.f),
                          1/((context->depthInBoxXYZ[i] >> 8) / 50.f), 0.7f);

                glVertex3f( (i%w- (w-1)/2.f) * context->depthInBoxXYZ[i] / Context::instance()->f,  // X = (x - cx) * d / fx
                            (i/w- (h-1)/2.f) * context->depthInBoxXYZ[i] / Context::instance()->f,  // Y = (y - cy) * d / fy
                            context->depthInBoxXYZ[i] );

            }
#else
            int w = context->boxDim->getX() * 2 - 1;
            int h = context->boxDim->getY() * 2 - 1;

            for (int y = 0; y < (h -1); ++y)
                for (int x = 0; x < (w -1); ++x)
                {
                    int i = ((y * w) + x);

                    if (!context->depthInBoxXYZ[i]) continue;

                    glColor3ub( context->rgbInBoxXY[3*i+0],    // R
                                context->rgbInBoxXY[3*i+1],    // G
                                context->rgbInBoxXY[3*i+2]);  // A

                    MakeVertex(x, y, context->depthInBoxXYZ[i], w, h);
                }
#endif
            glEnd();

            glBegin(GL_LINES);
                glColor3f(1, 0,  0);
                glVertex3f(context->boxPos->getX(), context->boxPos->getY() + context->boxDim->getY(), context->boxPos->getZ() - (0.95f * context->boxDim->getZ()));
                glVertex3f(context->boxPos->getX(), context->boxPos->getY() - context->boxDim->getY(), context->boxPos->getZ() - (0.95f * context->boxDim->getZ()));
            glEnd();
        }
    }
};

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

class GenerateUUIDAction: public ContextAction
{
    void exec() {
        uuidFolderName = UUID();
    }
};

class SaveImagesAction: public ContextAction
{
    void exec() {
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
        path.append("mkdir ").append(IMAGES_DIR).append("/").append(uuidFolderName);
        system(path.c_str());

        std::string csvFilePath;
        csvFilePath.append(IMAGES_DIR).append("/").append(CSV_IMAGES_INFO);

        //Save csv database info
        std::FILE * csvFile;
        csvFile = std::fopen(csvFilePath.c_str(),"a");
        if (csvFile)
        {
          std::string info;

          info += uuidFolderName + ";";
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
        path.append(IMAGES_DIR).append("/").append(uuidFolderName).append("/").append(KINECT_1_XY_FILE);
        WriteBMPFile(context->rgbInBoxXY, path, w, h);

        //Save original bitmap deth image for view.
        path.clear();
        path.append(IMAGES_DIR).append("/").append(uuidFolderName).append("/").append(KINECT_1_XYZ_IR_VIEW_FILE);
        WriteBMPFile(context->irImageInBoxXYZ, path, w, h);

        //Save original bitmap deth image for view.
        path.clear();
        path.append(IMAGES_DIR).append("/").append(uuidFolderName).append("/").append(KINECT_1_XYZ_DEPTH_VIEW_FILE);
        WriteBMPFile(context->depthImageInBoxXYZ, path, w, h);


        //Save original bitmap deth image for view.
        path.clear();
        path.append(IMAGES_DIR).append("/").append(uuidFolderName).append("/").append(KINECT_1_XY_DATA_FILE);
        WriteFile(context->depthInBoxXY, path, w, h);


        //Save original bitmap deth image for view.
        path.clear();
        path.append(IMAGES_DIR).append("/").append(uuidFolderName).append("/").append(KINECT_1_XYZ_DATA_FILE);
        WriteFile(context->depthInBoxXYZ, path, w, h);

        //Save original bitmap deth image for view.
        path.clear();
        path.append(IMAGES_DIR).append("/").append(uuidFolderName).append("/").append(KINECT_1_XYZ_IR_FILE);
        WriteFile(context->irInBoxXYZ, path, w, h);
    }
};

class SaveTestImagesAction: public ContextAction
{
    void exec() {
        int w = context->boxDim->getX() * 2 - 1;
        int h = context->boxDim->getY() * 2 - 1;

        std::string path;
        path.append("mkdir ").append(IMAGES_DIR);
        system(path.c_str());

        path.clear();
        path.append("mkdir ").append(IMAGES_DIR).append("/").append(uuidFolderName);
        system(path.c_str());

        path.clear();
        path.append("mkdir ").append(IMAGES_DIR).append("/").append(uuidFolderName).append("/").append(TEST_DIR);
        system(path.c_str());


        std::string prefixFilesName = UUID();
        std::string folderTest;
        folderTest.append(IMAGES_DIR).append("/").append(uuidFolderName).append("/").append(TEST_DIR).append("/");

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
        WriteBMPFile(context->rgbInBoxXY, path, w, h);

        //Save original bitmap deth image for view.
        path.clear(); path.append(folderTest).append(prefixFilesName).append(KINECT_1_XYZ_IR_VIEW_FILE);
        WriteBMPFile(context->irImageInBoxXYZ, path, w, h);

        //Save original bitmap deth image for view.
        path.clear(); path.append(folderTest).append(prefixFilesName).append(KINECT_1_XY_DATA_FILE);
//        WriteFile(context->depthInBoxXY, path, w, h);

        //Save original bitmap deth image for view.
        path.clear(); path.append(folderTest).append(prefixFilesName).append(KINECT_1_XYZ_DEPTH_VIEW_FILE);
        WriteBMPFile(context->depthImageInBoxXYZ, path, w, h);

        //Save original bitmap deth image for view.
        path.clear(); path.append(folderTest).append(prefixFilesName).append(KINECT_1_XYZ_DATA_FILE);
        WriteFile(context->depthInBoxXYZ, path, w, h);
    }
};


int main(int argc, char **argv)
{
    Context* context = Context::instance();
    context->init();

    context->addViewport(new InfoViewPort);
    context->addViewport(new PointCamViewPort);
    {
        context->addViewport(new TriangleCamViewPort);
        context->addViewport(new BoxCamViewPort);
    }

    context->addViewport(new BoxExtractViewPort);

    context->addViewport(new LeftCamViewPort);
    context->addViewport(new FrontCamViewPort);
    context->addViewport(new RightCamViewPort);

    context->addAction('1', new GenerateUUIDAction);
//    context->addAction('2', new CreateImagesCacheAction);
    context->addAction('3', new SaveImagesAction);
    context->addAction('5', new SaveTestImagesAction);

    context->initGlLoop(argc, argv);

    return 0;
}
