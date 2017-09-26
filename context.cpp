#include "freenectdevice.h"
#include "camera.h"

#include "context.h"

#if defined(__APPLE__)
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif

Context *Context::_instance = 0;

Context::Context()
{
    device = FreenectDevice::createDevice();

    cam = new Camera;

    boxPos = new Vec3<int>(0,0, 1500);
    boxDim = new Vec3<int>(200, 300, 300);

    width = 640;
    height = 480;

    window = 0;
}

void Context::init()
{
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

        glMatrixMode(GL_MODELVIEW);
    });

    glutKeyboardFunc([] (unsigned char key, int x, int y) {
        if (key == 0x1B) {// ESC
            glutDestroyWindow(_instance->window);
            _instance->device->stopDepth();
            _instance->device->stopVideo();
            exit(0);
        }
        _instance->cam->keyPressed(key,x,y);
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
    viewports.push_back(viewport);
}

void Context::notify() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    for (std::list<ContextViewPort*>::iterator it=viewports.begin(); it!=viewports.end(); ++it)
        (*it)->update();

    glFlush();
    glutSwapBuffers();
}
