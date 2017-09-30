#include "freenectdevice.h"
#include "camera.h"

#include "context.h"

#if defined(__APPLE__)
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif

Context *Context::_instance = 0;

Context::Context():
    rgb(640*480*3),
    depth(640*480)
{
    device = FreenectDevice::createDevice();

    cam = new Camera;

    boxPos = new Vec3<int>(0,0, 1100);
    boxDim = new Vec3<int>(200, 300, 300);

    width = 640;
    height = 480;
    f = 595.f;

    window = 0;
}

void Context::init()
{
    freenect_angle = device->getState().getTiltDegs();
    device->startVideo();
    device->startDepth();
}

void Context::initGlLoop(int argc, char **argv)
{
    glutInit(&argc, argv);

    glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);
    glutInitWindowSize(width, height);
    glutInitWindowPosition(0, 0);

    window = glutCreateWindow("RAP3DF");
    glClearColor(0.45f, 0.45f, 0.45f, 0.0f);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_ALPHA_TEST);
    glAlphaFunc(GL_GREATER, 0.0f);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glMatrixMode(GL_PROJECTION);
    gluPerspective(50.0, 1.0, 900.0, 11000.0);

    glutDisplayFunc([] () {
        _instance->notify();
    });

    glutIdleFunc([] () {
        glutPostRedisplay();
    });

    glutReshapeFunc([] (int width, int height) {
        _instance->width = width;
        _instance->height = height;

        glViewport(0, 0, width, height);
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        gluPerspective(50.0, (float)width / height, 1.0, 12000.0);

//        glMatrixMode(GL_MODELVIEW);
    });

    glutKeyboardFunc([] (unsigned char key, int x, int y) {
        if (key == 0x1B) {// ESC
            glutDestroyWindow(_instance->window);
            _instance->device->stopDepth();
            _instance->device->stopVideo();
            exit(0);
        }
        _instance->keyPressed(key,x,y);
    });

    glutMotionFunc([] (int x, int y) {
        _instance->cam->handleMouseMove(x, y);
    });

    glutMouseFunc([] (int button, int state, int x, int y)
    {
        _instance->cam->mouseButtonPressed(button, state, x, y);
    });

    glutMainLoop();
}

void Context::addViewport(ContextViewPort* viewport)
{
    viewport->context = this;
    viewports.push_back(viewport);
}



void Context::notify() {
    cam->move(0.2);

    device->getRGB(rgb);
    device->getDepth(depth);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    for (std::list<ContextViewPort*>::iterator it=viewports.begin(); it!=viewports.end(); ++it) {
        (*it)->update(rgb, depth);
    }

    glFlush();
    glutSwapBuffers();
}

void Context::keyPressed(unsigned char key, int x, int y)
{
    cam->holdingForward = false;
    cam->holdingBackward = false;
    cam->holdingRightStrafe = false;
    cam->holdingLeftStrafe = false;

    switch (key)
    {
            case  'Q':
            case  'q':
                cam->rotateLeft();
                break;
            case  'e':
            case  'E':
                cam->rotateRight();
                break;
            case 'W':
            case 'w':
                cam->holdingForward = true;
                break;
            case 'S':
            case 's':
                cam->holdingBackward = true;
                break;
            case 'A':
            case 'a':
                cam->holdingLeftStrafe = true;
                break;
            case 'D':
            case 'd':
                cam->holdingRightStrafe= true;
                break;
            case 'R':
            case 'r':
                freenect_angle++;
                if (freenect_angle > 30)
                    freenect_angle = 30;
                device->setTiltDegrees(freenect_angle);
                break;
            case 'F':
            case 'f':
                freenect_angle = 0;
                device->setTiltDegrees(freenect_angle);
                break;
            case 'V':
            case 'v':
                freenect_angle--;
                if (freenect_angle < -30)
                    freenect_angle = -30;
                device->setTiltDegrees(freenect_angle);
                break;
    }
}
