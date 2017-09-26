#include "camera.h"

#if defined(__APPLE__)
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif

const double Camera::TO_RADS = 3.141592654 / 180.0; // The value of 1 degree in radians

Camera::Camera()
{
    initCamera();
}

Camera::~Camera()
{
}

void Camera::initCamera()
{
    lastX = 0;
    lastY = 0;

    // Set position, rotation and speed values to zero
    position.zero();
    rotation.zero();
    speed.zero();
    rotation.addX(180);

    // How fast we move (higher values mean we move and strafe faster)
    movementSpeedFactor = 100.0;

    pitchSensitivity = 0.2; // How sensitive mouse movements affect looking up and down
    yawSensitivity   = 0.2; // How sensitive mouse movements affect looking left and right

    // To begin with, we aren't holding down any keys
    holdingForward     = false;
    holdingBackward    = false;
    holdingLeftStrafe  = false;
    holdingRightStrafe = false;
}

// Function to convert degrees to radians
const double Camera::toRads(const double &theAngleInDegrees) const
{
    return theAngleInDegrees * TO_RADS;
}

// Function to deal with mouse position changes
void Camera::handleMouseMove(int mouseX, int mouseY)
{
    rotation.addY((lastX - mouseX) * yawSensitivity);
    rotation.addX((lastY - mouseY) * pitchSensitivity);

    lastX = mouseX;
    lastY = mouseY;
}

void Camera::rotateLeft()
{
    rotation.addZ(5*pitchSensitivity);
}

void Camera::rotateRight()
{
    rotation.addZ(-5*pitchSensitivity);
}

// Function to calculate which direction we need to move the camera and by what amount
void Camera::move(double deltaTime)
{
    // Vector to break up our movement into components along the X, Y and Z axis
    Vec3<double> movement;

    // Get the sine and cosine of our X and Y axis rotation
    double sinXRot = sin( toRads( rotation.getX() ) );
    double cosXRot = cos( toRads( rotation.getX() ) );

    double sinYRot = sin( toRads( rotation.getY() ) );
    double cosYRot = cos( toRads( rotation.getY() ) );

    double pitchLimitFactor = cosXRot; // This cancels out moving on the Z axis when we're looking up or down

    if (holdingForward)
    {
        movement.addX(sinYRot * pitchLimitFactor);
        movement.addY(-sinXRot);
        movement.addZ(-cosYRot * pitchLimitFactor);
    }

    if (holdingBackward)
    {
        movement.addX(-sinYRot * pitchLimitFactor);
        movement.addY(sinXRot);
        movement.addZ(cosYRot * pitchLimitFactor);
    }

    if (holdingLeftStrafe)
    {
        movement.addX(-cosYRot);
        movement.addZ(-sinYRot);
    }

    if (holdingRightStrafe)
    {
        movement.addX(cosYRot);
        movement.addZ(sinYRot);
    }

    // Normalise our movement vector
    movement.normalise();

    // Calculate our value to keep the movement the same speed regardless of the framerate...
    double framerateIndependentFactor = movementSpeedFactor * deltaTime;

    // .. and then apply it to our movement vector.
    movement *= framerateIndependentFactor;

    // Finally, apply the movement to our position
    position += movement;
}


void Camera::keyPressed(unsigned char key, int x, int y)
{
    holdingForward = false;
    holdingBackward = false;
    holdingRightStrafe = false;
    holdingLeftStrafe = false;

    switch (key)
    {
            case  'Q':
            case  'q':
                rotateLeft();
                break;
            case  'e':
            case  'E':
                rotateRight();
                break;
            case 'W':
            case 'w':
                holdingForward = true;
                break;
            case 'S':
            case 's':
                holdingBackward = true;
                break;
            case 'A':
            case 'a':
                holdingLeftStrafe = true;
                break;
            case 'D':
            case 'd':
                holdingRightStrafe= true;
                break;
    }
}

void Camera::mouseButtonPressed(int button, int state, int x, int y)
{
    if (state == GLUT_DOWN)
    {
        switch (button)
        {
            case GLUT_LEFT_BUTTON:
                lastX = x;
                lastY = y;
                break;
//            case 3:
//                zoom *= 1.2f;
//                break;

//            case 4:
//                zoom /= 1.2f;
//                break;
        }
    }
    else if (state == GLUT_UP && button == GLUT_LEFT_BUTTON)
    {
        lastX = -1;
        lastY = -1;
    }
}
