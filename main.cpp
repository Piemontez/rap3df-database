/*
 * This file is part of the OpenKinect Project. http://www.openkinect.org
 *
 * Copyright (c) 2010 individual OpenKinect contributors. See the CONTRIB file
 * for details.
 *
 * This code is licensed to you under the terms of the Apache License, version
 * 2.0, or, at your option, the terms of the GNU General Public License,
 * version 2.0. See the APACHE20 and GPL2 files for the text of the licenses,
 * or the following URLs:
 * http://www.apache.org/licenses/LICENSE-2.0
 * http://www.gnu.org/licenses/gpl-2.0.txt
 *
 * If you redistribute this file in source form, modified or unmodified, you
 * may:
 *   1) Leave this header intact and distribute it under the same terms,
 *      accompanying it with the APACHE20 and GPL20 files, or
 *   2) Delete the Apache 2.0 clause and accompany it with the GPL2 file, or
 *   3) Delete the GPL v2 clause and accompany it with the APACHE20 file
 * In all cases you must keep the copyright notice intact and include a copy
 * of the CONTRIB file.
 *
 * Binary distributions must follow the binary distribution requirements of
 * either License.
 */

#include <cstdlib>
#include <iostream>
#include <vector>

#if defined(__APPLE__)
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif

#include "context.h"
#include "camera.h"
#include "freenectdevice.h"

int minX = 640;
int minY = 640;
int maxX = 0;
int maxY = 0;

void checkRegions(const int& y, const int& x) {
    if (y > maxY) {
        maxY = y;
    }
    if (y < minY) {
        minY = y;
    }
    if (x > maxX) {
        maxX = x;
    }
    if (x < minX) {
        minX = x;
    }
}

void MakeVertex(int pos, uint16_t depth) {
    glVertex3f( (pos%640 - (640-1)/2.f) * depth / Context::instance()->f,  // X = (x - cx) * d / fx
                (pos/640 - (480-1)/2.f) * depth / Context::instance()->f,  // Y = (y - cy) * d / fy
                depth );
}

class BoxCamViewPort: public ContextViewPort
{
public:
    void update(std::vector<uint8_t> &rgb, std::vector<uint16_t> &depth) {

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

class InfoViewPort: public ContextViewPort
{
public:
    void update(std::vector<uint8_t> &rgb, std::vector<uint16_t> &depth) {

        glViewport(0, 0, context->width, context->height);

        glMatrixMode(GL_PROJECTION);
        glPushMatrix();
        {
          glLoadIdentity();

          gluOrtho2D(0.0, 640, 0.0, 480);
          glMatrixMode(GL_MODELVIEW);
          glPushMatrix();
          {
              glLoadIdentity();

              glRasterPos2i(10, 10);
              std::string s =
                      "Box Check" + std::to_string(minY) + 'x' + std::to_string(minX) + " | "
                                  + std::to_string(maxY) + 'x' + std::to_string(maxX)
                      + " rx" + std::to_string(context->cam->getXRot())
                      + " ry" + std::to_string(context->cam->getYRot())
                      + " rz" + std::to_string(context->cam->getZRot())

                      + " x" + std::to_string(context->cam->getXPos())
                      + " y" + std::to_string(context->cam->getYPos())
                      + " z" + std::to_string(context->cam->getZPos())

                      + " size" + std::to_string(context->rgb.size());
              void * font = GLUT_BITMAP_9_BY_15;
              for (std::string::iterator i = s.begin(); i != s.end(); ++i)
              {
                char c = *i;
                glColor3d(1.0, 0.0, 0.0);
                glutBitmapCharacter(font, c);
              }

              glMatrixMode(GL_MODELVIEW);
          }
          glPopMatrix();

          glMatrixMode(GL_PROJECTION);
        }
        glPopMatrix();
    }
};

class PointCamViewPort: public ContextViewPort
{
public:
    void update(std::vector<uint8_t> &rgb, std::vector<uint16_t> &depth) {

        glViewport(0, context->height/2, context->width/2, context->height/2);

        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
        {
            glRotatef(context->cam->getXRot(), 1.0f, 0.0f, 0.0f); // Rotate our camera on the x-axis (looking up and down)
            glRotatef(context->cam->getYRot(), 0.0f, 1.0f, 0.0f); // Rotate our camera on the  y-axis (looking left and right)
            glRotatef(context->cam->getZRot(), 0.0f, 0.0f, 1.0f);
            glTranslatef( -context->cam->getXPos(), -context->cam->getYPos(), -context->cam->getZPos() );

            glBegin(GL_POINTS);
            for (int i = 0; i < 480*640; ++i)
            {
                glColor3ub( rgb[3*i+0],    // R
                            rgb[3*i+1],    // G
                            rgb[3*i+2] );  // B

                MakeVertex(i, depth[i]);
            }
            glEnd();
        }
    }
};


class TrianguleCamViewPort: public ContextViewPort
{
public:
    void update(std::vector<uint8_t> &rgb, std::vector<uint16_t> &depth) {

        glViewport(context->width/2, context->height/2, context->width/2, context->height/2);

        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
        {
            glRotatef(context->cam->getXRot(), 1.0f, 0.0f, 0.0f); // Rotate our camera on the x-axis (looking up and down)
            glRotatef(context->cam->getYRot(), 0.0f, 1.0f, 0.0f); // Rotate our camera on the  y-axis (looking left and right)
            glRotatef(context->cam->getZRot(), 0.0f, 0.0f, 1.0f);
            glTranslatef( -context->cam->getXPos(), -context->cam->getYPos(), -context->cam->getZPos() );

            glBegin(GL_TRIANGLES);
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
            glEnd();
        }
    }
};


class BoxExtractViewPort: public ContextViewPort
{
public:
    void update(std::vector<uint8_t> &rgb, std::vector<uint16_t> &depth) {

        context->rgbModified.clear();
        context->depthModified.clear();

        std::vector<uint8_t>::iterator itRgb = rgb.begin();
        std::vector<uint16_t>::iterator itDepth = depth.begin();

        int i;
        for (int j = 0; j < 480; j++)
            for (int k = 0; k < 640; k++)
//        for (int i = 0; i < 480*640; ++i)
            {
                i = (j * 640) + k; // 480 linhas de 640 pixels

                float x = (k - 319.5f) * (*itDepth) / Context::instance()->f;
                float y = (j - 239.5f) * (*itDepth) / Context::instance()->f;

                if (x > (context->boxPos->getX() - context->boxDim->getX())
                    && x < (context->boxPos->getX() + context->boxDim->getX())

                    && y > (context->boxPos->getY() - context->boxDim->getY())
                    && y < (context->boxPos->getY() + context->boxDim->getY())

                    && (*itDepth) > (context->boxPos->getZ() - context->boxDim->getZ())
                    && (*itDepth) < (context->boxPos->getZ() + context->boxDim->getZ())
                        )
                {
                    checkRegions(j, k);

                    context->rgbModified.push_back(*itRgb++);
                    context->rgbModified.push_back(*itRgb++);
                    context->rgbModified.push_back(*itRgb++);

                    context->depthModified.push_back(*itDepth++);
                } else {
                    context->rgbModified.push_back(0);
                    context->rgbModified.push_back(0);
                    context->rgbModified.push_back(0);

                    context->depthModified.push_back(0);

                    itRgb+=3;
                    itDepth++;
                }
            }
    }
};

class FrontCamViewPort: public ContextViewPort
{
public:
    void update(std::vector<uint8_t> &rgb, std::vector<uint16_t> &depth) {

        glViewport(context->width/3, 0, context->width/3, context->height/3);

        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
        {
            glRotatef(180, 1.0f, 0.0f, 0.0f); // Rotate our camera on the x-axis (looking up and down)
            //glRotatef(-90, 0.0f, 1.0f, 0.0f); // Rotate our camera on the  y-axis (looking left and right)
            glTranslatef( 0, 0, -500 );

            glBegin(GL_POINTS);
            for (int i = 0; i < 480*640; ++i)
            {
                if (!context->depthModified[i]) continue;

                glColor3ub( context->rgbModified[3*i+0],    // R
                            context->rgbModified[3*i+1],    // G
                            context->rgbModified[3*i+2] );  // B

                MakeVertex(i, context->depthModified[i]);
            }
            glEnd();
        }
    }
};


class LeftCamViewPort: public ContextViewPort
{
public:
    void update(std::vector<uint8_t> &rgb, std::vector<uint16_t> &depth) {

        glViewport(0, 0, context->width/3, context->height/3);

        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
        {
            glRotatef(180, 1.0f, 0.0f, 0.0f); // Rotate our camera on the x-axis (looking up and down)
            glRotatef(-90, 0.0f, 1.0f, 0.0f); // Rotate our camera on the  y-axis (looking left and right)
            glTranslatef( 500, 0, -1000 );

            glBegin(GL_POINTS);
            for (int i = 0; i < 480*640; ++i)
            {
                if (!context->depthModified[i]) continue;

                glColor3ub( context->rgbModified[3*i+0],    // R
                            context->rgbModified[3*i+1],    // G
                            context->rgbModified[3*i+2] );  // B

                MakeVertex(i, context->depthModified[i]);
            }
            glEnd();
        }
    }
};

class RightCamViewPort: public ContextViewPort
{
public:
    void update(std::vector<uint8_t> &rgb, std::vector<uint16_t> &depth) {

        glViewport(context->width/3*2, 0, context->width/3, context->height/3);

        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
        {
            glRotatef(180, 1.0f, 0.0f, 0.0f); // Rotate our camera on the x-axis (looking up and down)
            glRotatef(90, 0.0f, 1.0f, 0.0f); // Rotate our camera on the  y-axis (looking left and right)
            glTranslatef( -500, 0, -1000 );

            glBegin(GL_POINTS);
            for (int i = 0; i < 480*640; ++i)
            {
                if (!context->depthModified[i]) continue;

                glColor3ub( context->rgbModified[3*i+0],    // R
                            context->rgbModified[3*i+1],    // G
                            context->rgbModified[3*i+2] );  // B

                MakeVertex(i, context->depthModified[i]);
            }
            glEnd();
        }
    }
};

int main(int argc, char **argv)
{
    Context* context = Context::instance();
    context->init();

    context->addViewport(new InfoViewPort);
    context->addViewport(new PointCamViewPort);
    {
        context->addViewport(new TrianguleCamViewPort);
        context->addViewport(new BoxCamViewPort);
    }

    context->addViewport(new BoxExtractViewPort);

    context->addViewport(new LeftCamViewPort);
    context->addViewport(new FrontCamViewPort);
    context->addViewport(new RightCamViewPort);

    context->initGlLoop(argc, argv);

    return 0;
}
