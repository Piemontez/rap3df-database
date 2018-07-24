#include "freenectdevice.h"
#include "camera.h"

#include "context.h"

#if defined(__APPLE__)
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif

Context *Context::_instance = 0;
int Context::window1 = 0;
int Context::window2 = 0;

ContextViewPort::ContextViewPort(int _flags): flags(_flags)
{

}

Context::Context()
#ifdef KINECT1
    :rgb(640*480*3),
    depth(640*480)
#endif
{
    cam = new Camera;

#ifdef KINECT1
    width = 640;
    height = 480;

    boxPos = new Vec3<int>(0,0, 1000);
    boxDim = new Vec3<int>(150, 200, 150);
#else
    width = 512;
    height = 424;

    boxPos = new Vec3<int>(0,0, 90);
    boxDim = new Vec3<int>(60, 75, 60);
#endif
    f = 595.f;

}

void Context::init(int argc, char **argv)
{
    //GLUT START
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);
    glutInitWindowSize(width, height);
    glutInitWindowPosition(0, 0);

#ifdef KINECT1
    device = FreenectDevice::createDevice();

    device->startVideo();
    device->startDepth();

    freenect_angle = device->getState().getTiltDegs();
#else
    libfreenect2::setGlobalLogger(libfreenect2::createConsoleLogger(libfreenect2::Logger::Debug));

    serial = freenect2.getDefaultDeviceSerialNumber();

    if(freenect2.enumerateDevices() == 0)
    {
      std::cout << "no device connected!" << std::endl;
      return;
    }

//    pipeline = new libfreenect2::CpuPacketPipeline();
//    pipeline = new libfreenect2::OpenGLPacketPipeline();
//    dev = freenect2.openDevice(serial, pipeline);

    dev = freenect2.openDevice(serial);

    libfreenect2::Freenect2Device::Config conff;
    conff.MaxDepth = 2;
    dev->setConfiguration(conff);

    dev->start();

    int types  = 0;
    types |= libfreenect2::Frame::Color;
    types |= libfreenect2::Frame::Ir;
    types |= libfreenect2::Frame::Depth;

    listener = new libfreenect2::SyncMultiFrameListener(types);

    dev->setColorFrameListener(listener);
    dev->setIrAndDepthFrameListener(listener);

    std::cout << "device serial: " << dev->getSerialNumber() << std::endl;
    std::cout << "device firmware: " << dev->getFirmwareVersion() << std::endl;

    registration = new libfreenect2::Registration(dev->getIrCameraParams(), dev->getColorCameraParams());

    undistorted = new libfreenect2::Frame(width, height, 4);
    registered = new libfreenect2::Frame(width, height, 4);
#endif
}


int Context::initWindow(const char* title)
{
//    glutTimerFunc( 10, TimeEvent, 1);
//    glutInitWindowSize(1000, 600);
//    glutInitWindowPosition(100, 0);

    int currwindow = glutCreateWindow(title);
    if (!Context::window1) //Todo REVER
        Context::window1 = currwindow;
    else if (!Context::window2) //Todo REVER
        Context::window2 = currwindow;

    glClearColor(0.8f, 0.8f, 0.8f, 0.0f);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_ALPHA_TEST);
    glAlphaFunc(GL_GREATER, 0.0f);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    if (Context::window2)
        glutDisplayFunc([] (void) {
            _instance->notify(Context::window2);
            glutPostRedisplay();
        });
    else if (Context::window1)
        glutDisplayFunc([] (void) {
            _instance->notify(Context::window1);
            glutPostRedisplay();
        });

//    glutIdleFunc([] () {
//        glutPostRedisplay();
//    });

    glutReshapeFunc([] (int width, int height) {
        _instance->width = width;
        _instance->height = height;

        glViewport(0, 0, width, height);
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        gluPerspective(50.0, (float)width / height, 1, 5000.0);

//        glMatrixMode(GL_MODELVIEW);
    });

    glutKeyboardFunc([] (unsigned char key, int x, int y) {
        if (key == 0x1B) {// ESC
            //Todo destroy windows
//            glutDestroyWindow(_instance->window);

#ifdef KINECT1
            _instance->device->stopDepth();
            _instance->device->stopVideo();       
#else
            _instance->dev->stop();
            _instance->dev->close();
#endif
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

    return currwindow;
}

void Context::start()
{
    glutMainLoop();
}

void Context::addViewport(const int window, ContextViewPort* viewport)
{
    viewport->context = this;
    viewport->window = window;
    viewports.push_back(viewport);
}


void Context::notify(int window) {
    cam->move(0.2);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

#ifdef KINECT1
    device->getRGB(rgb);
    device->getDepth(depth);
#else

    bool hasFrame = listener && listener->waitForNewFrame(frames, 10*1000);
    if(hasFrame)
    {
      rgb2 = frames[libfreenect2::Frame::Color];
      ir2 = frames[libfreenect2::Frame::Ir];
      depth2 = frames[libfreenect2::Frame::Depth];

      registration->apply(rgb2, depth2, undistorted, registered);
    }


    for (std::list<ContextViewPort*>::iterator it=viewports.begin(); it!=viewports.end(); ++it) {
        if (window == (*it)->window)
            (*it)->update();
    }


    if(hasFrame)
    {
      listener->release(frames);
      /** libfreenect2::this_thread::sleep_for(libfreenect2::chrono::milliseconds(100)); */
    }

#endif

    glFlush();
    glutSwapBuffers();
}

void Context::addAction(const unsigned char key, ContextAction* action)
{
    action->context = this;
    action->key = key;
    actions.push_back(action);
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

            case 'Z':
            case 'z':
                boxPos->addX(-1);
                break;
            case 'C':
            case 'c':
                boxPos->addX(1);
                break;

            case 'R':
            case 'r':
                freenect_angle++;
                if (freenect_angle > 30)
                    freenect_angle = 30;
#ifdef KINECT1
                device->setTiltDegrees(freenect_angle);
                break;
#else

#endif
            case 'F':
            case 'f':
                freenect_angle = 0;
#ifdef KINECT1
                device->setTiltDegrees(freenect_angle);
#else

#endif
                break;
            case 'V':
            case 'v':
                freenect_angle--;
                if (freenect_angle < -30)
                    freenect_angle = -30;
#ifdef KINECT1
                device->setTiltDegrees(freenect_angle);
#else

#endif
                break;
    }
    for (std::list<ContextAction*>::iterator it=actions.begin(); it!=actions.end(); it++) {
        if ((*it)->key == key)
            (*it)->exec();
    }
}

