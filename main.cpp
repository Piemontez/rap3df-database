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

Context* context;
//FreenectDevice* device;
//Camera* cam;
//Vec3<int>* boxPos;
//Vec3<int>* boxDim;

//int window(0);                // Glut window identifier
//int mWidth;
//int mHeight;

void MakeVertex(int pos, uint16_t depth) {
    float f = 595.f;

    glVertex3f( (pos%640 - (640-1)/2.f) * depth / f,  // X = (x - cx) * d / fx
                (pos/640 - (480-1)/2.f) * depth / f,  // Y = (y - cy) * d / fy
                depth );
}

void DrawStereoCamPoints(bool triangles)
{
    static std::vector<uint8_t> rgb(640*480*3);
    static std::vector<uint16_t> depth(640*480);

    context->device->getRGB(rgb);
    context->device->getDepth(depth);

    if (triangles) {
        glBegin(GL_TRIANGLES);
        for (int j, i = 0; i < 480*638; ++i)
        {
            glColor3ub( rgb[3*i+0],    // R
                        rgb[3*i+1],    // G
                        rgb[3*i+2] );  // B

                             // Z = d

            if (depth[i] > 0 && depth[i+1] > 0 && depth[i+640] > 0 && depth[i+641] > 0) {
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
    } else {
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

void DrawBoxForCapture()
{
    // Draw the world coordinate frame
    //Todo: improve
    Vec3<int>* boxPos = context->boxPos;
    Vec3<int>* boxDim = context->boxDim;

    glBegin(GL_QUADS);
        glColor4f(0, 0,  1, 0.4);
        glVertex3f(boxPos->getX() + boxDim->getY(), boxPos->getY() + boxDim->getY(), boxPos->getZ() - boxDim->getZ());
        glVertex3f(boxPos->getX() + boxDim->getY(), boxPos->getY() - boxDim->getY(), boxPos->getZ() - boxDim->getZ());
        glVertex3f(boxPos->getX() + boxDim->getY(), boxPos->getY() - boxDim->getY(), boxPos->getZ() + boxDim->getZ());
        glVertex3f(boxPos->getX() + boxDim->getY(), boxPos->getY() + boxDim->getY(), boxPos->getZ() + boxDim->getZ());

        glVertex3f(boxPos->getX() - boxDim->getY(), boxPos->getY() + boxDim->getY(), boxPos->getZ() - boxDim->getZ());
        glVertex3f(boxPos->getX() - boxDim->getY(), boxPos->getY() - boxDim->getY(), boxPos->getZ() - boxDim->getZ());
        glVertex3f(boxPos->getX() - boxDim->getY(), boxPos->getY() - boxDim->getY(), boxPos->getZ() + boxDim->getZ());
        glVertex3f(boxPos->getX() - boxDim->getY(), boxPos->getY() + boxDim->getY(), boxPos->getZ() + boxDim->getZ());

        glColor4f(0, 0,  0.5, 0.2);
        glVertex3f(boxPos->getX() + boxDim->getY(), boxPos->getY() + boxDim->getY(), boxPos->getZ() - boxDim->getZ());
        glVertex3f(boxPos->getX() - boxDim->getY(), boxPos->getY() + boxDim->getY(), boxPos->getZ() - boxDim->getZ());
        glVertex3f(boxPos->getX() - boxDim->getY(), boxPos->getY() + boxDim->getY(), boxPos->getZ() + boxDim->getZ());
        glVertex3f(boxPos->getX() + boxDim->getY(), boxPos->getY() + boxDim->getY(), boxPos->getZ() + boxDim->getZ());

        glVertex3f(boxPos->getX() + boxDim->getY(), boxPos->getY() - boxDim->getY(), boxPos->getZ() - boxDim->getZ());
        glVertex3f(boxPos->getX() - boxDim->getY(), boxPos->getY() - boxDim->getY(), boxPos->getZ() - boxDim->getZ());
        glVertex3f(boxPos->getX() - boxDim->getY(), boxPos->getY() - boxDim->getY(), boxPos->getZ() + boxDim->getZ());
        glVertex3f(boxPos->getX() + boxDim->getY(), boxPos->getY() - boxDim->getY(), boxPos->getZ() + boxDim->getZ());

        glColor4f(0, 1,  0, 0.8);
        glVertex3f(boxPos->getX() + boxDim->getY(), boxPos->getY() - boxDim->getY(), boxPos->getZ() + boxDim->getZ());
        glVertex3f(boxPos->getX() - boxDim->getY(), boxPos->getY() - boxDim->getY(), boxPos->getZ() + boxDim->getZ());
        glVertex3f(boxPos->getX() - boxDim->getY(), boxPos->getY() + boxDim->getY(), boxPos->getZ() + boxDim->getZ());
        glVertex3f(boxPos->getX() + boxDim->getY(), boxPos->getY() + boxDim->getY(), boxPos->getZ() + boxDim->getZ());
    glEnd();
//    glColor3ub(0, 255, 0);  // X-axis
}

class InfoViewPort: public ContextViewPort
{
public:
    void update() {
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
              std::string s = "rx" + std::to_string(context->cam->getXRot())
                      + " ry" + std::to_string(context->cam->getYRot())
                      + " rz" + std::to_string(context->cam->getZRot())
                      + " x" + std::to_string(context->cam->getXPos())
                      + " y" + std::to_string(context->cam->getYPos())
                      + " z" + std::to_string(context->cam->getZPos());
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
    void update() {

        context->cam->move(0.2);

        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
        {
            glViewport(0, context->height/2, context->width/2, context->height/2);

            glRotatef(context->cam->getXRot(), 1.0f, 0.0f, 0.0f); // Rotate our camera on the x-axis (looking up and down)
            glRotatef(context->cam->getYRot(), 0.0f, 1.0f, 0.0f); // Rotate our camera on the  y-axis (looking left and right)
            glRotatef(context->cam->getZRot(), 0.0f, 0.0f, 1.0f);
            glTranslatef( -context->cam->getXPos(), -context->cam->getYPos(), -context->cam->getZPos() );

            DrawStereoCamPoints(false);

            glViewport(context->width/2, context->height/2, context->width/2, context->height/2);
            DrawStereoCamPoints(true);
    //        DrawBoxForCapture();
        }

        // Place the camera

    //    glScalef(zoom, zoom, 1);


    }
};



int main(int argc, char **argv)
{
    context = Context::instance();
    context->init();

    context->addViewport(new PointCamViewPort);
    context->addViewport(new InfoViewPort);

    context->initGlLoop(argc, argv);

    return 0;
}
