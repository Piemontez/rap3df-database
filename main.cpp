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

#include "camera.h"
#include "freenectdevice.h"


FreenectDevice* device;
Camera* cam;
Vec3<int>* boxPos;
Vec3<int>* boxDim;

int window(0);                // Glut window identifier

void MakeVertex(int pos, uint16_t depth) {
    float f = 595.f;

    glVertex3f( (pos%640 - (640-1)/2.f) * depth / f,  // X = (x - cx) * d / fx
                (pos/640 - (480-1)/2.f) * depth / f,  // Y = (y - cy) * d / fy
                depth );
}

void DrawStereoCamPoints()
{
    static std::vector<uint8_t> rgb(640*480*3);
    static std::vector<uint16_t> depth(640*480);

    device->getRGB(rgb);
    device->getDepth(depth);

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

}

void DrawBoxForCapture()
{
    // Draw the world coordinate frame
    glLineWidth(2.0f);
    glBegin(GL_LINES);
    glColor3ub(255, 0, 0);  // X-axis
    glVertex3f(  0, 0, 0);
    glVertex3f( 50, 0, 0);
    glColor3ub(0, 255, 0);  // Y-axis
    glVertex3f(0,   0, 0);
    glVertex3f(0,  50, 0);
    glColor3ub(0, 0, 255);  // Z-axis
    glVertex3f(0, 0,   0);
    glVertex3f(0, 0,  50);
    glEnd();

    glLineWidth(5.0f);
    glNormal3f(0.0, 0.0, 1.0);

    int x,y,z;
    for (int k = 0; k < 3; k++) {
        x=0; y=0; z=0;
        for (int j = -1; j < 2; j+=2)
        {
            glBegin(GL_LINE_LOOP);
                glVertex3f(-300, -100, boxPos->getZ() + (boxDim->getZ() * j));
                glVertex3f(+300, -100, boxPos->getZ() + (boxDim->getZ() * j));
                glVertex3f(+300, +100, boxPos->getZ() + (boxDim->getZ() * j));
                glVertex3f(-300, +100, boxPos->getZ() + (boxDim->getZ() * j));
            glEnd();
        }
    }
//    glColor3ub(0, 255, 0);  // X-axis
}

void DrawPainel() {
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
          std::string s = "rx" + std::to_string(cam->getXRot())
                  + " ry" + std::to_string(cam->getYRot())
                  + " rz" + std::to_string(cam->getZRot())
                  + " x" + std::to_string(cam->getXPos())
                  + " y" + std::to_string(cam->getYPos())
                  + " z" + std::to_string(cam->getZPos());
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

void DrawGLScene()
{
    cam->move(0.2);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    {
        glRotatef(cam->getXRot(), 1.0f, 0.0f, 0.0f); // Rotate our camera on the x-axis (looking up and down)
        glRotatef(cam->getYRot(), 0.0f, 1.0f, 0.0f); // Rotate our camera on the  y-axis (looking left and right)
        glRotatef(cam->getZRot(), 0.0f, 0.0f, 1.0f);
        glTranslatef( -cam->getXPos(), -cam->getYPos(), -cam->getZPos() );

        DrawStereoCamPoints();
        DrawBoxForCapture();
    }

    DrawPainel();

    glFlush();
    // Place the camera

//    glScalef(zoom, zoom, 1);

    glutSwapBuffers();
}



void resizeGLScene(int width, int height)
{
    glViewport(0, 0, width, height);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(50.0, (float)width / height, 900.0, 11000.0);

    glMatrixMode(GL_MODELVIEW);
}


void idleGLScene()
{
    glutPostRedisplay();
}


int main(int argc, char **argv)
{
    device = FreenectDevice::createDevice();
    device->startVideo();
    device->startDepth();

    cam = new Camera;
    boxPos = new Vec3<int>(0,0, 1400);
    boxDim = new Vec3<int>(200, 300, 300);

    glutInit(&argc, argv);

    glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);
    glutInitWindowSize(640, 480);
    glutInitWindowPosition(0, 0);

    window = glutCreateWindow("RAP3DF");
    glClearColor(0.45f, 0.45f, 0.45f, 0.0f);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_ALPHA_TEST);
    glAlphaFunc(GL_GREATER, 0.0f);

    glMatrixMode(GL_PROJECTION);
    gluPerspective(50.0, 1.0, 900.0, 11000.0);
        
    glutDisplayFunc(&DrawGLScene);
    glutIdleFunc(&idleGLScene);
    glutReshapeFunc(&resizeGLScene);

    //Mov events

    glutKeyboardFunc([] (unsigned char key, int x, int y) {
        if (key == 0x1B) {// ESC
            glutDestroyWindow(window);
            device->stopDepth();
            device->stopVideo();
            exit(0);
        }
        cam->keyPressed(key,x,y);
    });

    glutMotionFunc([] (int x, int y) {
        cam->handleMouseMove(x, y);
    });

    glutMouseFunc([] (int button, int state, int x, int y)
    {
        cam->mouseButtonPressed(button, state, x, y);
    });

    glutMainLoop();

    return 0;
}
