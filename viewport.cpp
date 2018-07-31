#if defined(__APPLE__)
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif

#include <libfreenect2/libfreenect2.hpp>
#include <libfreenect2/frame_listener_impl.h>
#include <libfreenect2/registration.h>
#include <libfreenect2/packet_pipeline.h>
#include <libfreenect2/logger.h>

#include "freenectdevice.h"
#include "viewport.h"
#include "camera.h"
#include "utils.h"

#define depthScale 1.0f

void MakeVertex(float x, float y, float depth, int w, int h) {
    glVertex3f(x-(w/2), y-(h/2), depth * depthScale);
}

void BoxCamViewPort::update() {

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

void InfoViewPort::update() {
    glViewport(0, context->height-30, context->width, context->height);

    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    {
        glLoadIdentity();


        gluOrtho2D(0, context->width, 0.0, context->height);
        glMatrixMode(GL_MODELVIEW);
        glColor3f(0, 0.0, 0.0);
        glPushMatrix();
        {
            glLoadIdentity();

            glRasterPos2i(5, 15);
            std::string s;
            if (this->context->step == 0) {
                s = "Type 1: Start.";
            }
            if (this->context->step != 0) {
                s = "UUID:" + this->context->uuid;
            }
            if (this->context->step == -1) {
                s += " Liberado para coletar novos dados. Type 1: start";
            }
            if (this->context->step == 1) {
                if (this->context->currImageType == 0) {
                    s += " Defina o tipo de imagem. Type [f,t,d,l,r,b]";
                } else {
                    s += (context->sel_rgbImage.size() ? " Com imagens." : " Sem imagens.");
                    s += " Type 2: capture images";
                }
            }

            for (std::string::iterator i = s.begin(); i != s.end(); ++i)
            {
                char c = *i;
                glutBitmapCharacter(GLUT_BITMAP_9_BY_15, c);
            }

            glRasterPos2i(5, 0);
            s = "Images:";
            s += ((this->context->currImageType & 1)  ? " X " : (this->context->imagesSaved & 1 > 0) ? " F " : " - "); //Front
            s += ((this->context->currImageType & 2) ? " X " : (this->context->imagesSaved & 2 > 0) ? " T " : " - "); //Top
            s += ((this->context->currImageType & 4) ? " X " : (this->context->imagesSaved & 4 > 0) ? " D " : " - "); //Down
            s += ((this->context->currImageType & 8)  ? " X " : (this->context->imagesSaved & 8 > 0) ? " L " : " - "); //Left
            s += ((this->context->currImageType & 16) ? " X " : (this->context->imagesSaved & 16 > 0) ? " R " : " - "); //Right
            s += ((this->context->currImageType & 32) ? " X " : (this->context->imagesSaved & 32 > 0) ? " B " : " - "); //Pocket Lighter
            if (context->errorCode) {
                switch (context->errorCode) {
                case 1:
                    s+= "Nenhum tipo de imagem informado.";
                    break;
                case 2:
                    s+= "Nenhum dado (BMP,DATA) capturado";
                    break;
                }
            } else {
                s += "Box:" + std::to_string(this->context->boxDim->getX()) + 'x' + std::to_string(this->context->boxDim->getY()) + 'x' + std::to_string(this->context->boxDim->getZ());
                s += "CamDepth:" + std::to_string(this->context->boxPos->getZ());
            }
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

void PointCamViewPort::update() {

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

        if (context->_depth)
        {
            int w = context->_depth->width;
            int h = context->_depth->height;

            for (int y = 0; y < (h -1); ++y)
                for (int x = 0; x < (w -1); ++x)
                {
                    int i = ((y * w) + x) * 4;

                    if (!context->_rgb->status)
                        glColor3ub( context->registered->data[i+2],    // R
                                context->registered->data[i+1],    // G
                                context->registered->data[i+0]);  // A

                    if (!context->_depth->status && context->_depth->data[i+2] > 0)
                        MakeVertex(x, y, context->_depth->data[i+2], w, h);

                }
        }
        glEnd();
    }
}

void TriangleCamViewPort::update() {

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
        if (context->_depth)
        {
            int w = context->_depth->width;
            int h = context->_depth->height;
            int j;

            for (int y = 0; y < (h -1); ++y)
                for (int x = 0; x < (w -1); ++x)
                {
                    int i = ((y * w) + x) * 4;

                    if (!context->_rgb->status)
                        glColor3ub( context->registered->data[i+2],    // R
                                context->registered->data[i+1],    // G
                                context->registered->data[i+0]);  // A

                    if (!context->_depth->status)
                    {
                        if (context->_depth->data[i+2] > 0
                                && context->_depth->data[i+6] > 0
                                && context->_depth->data[i+2+(w * 4)] > 0) {
                            j = i;
                            MakeVertex(x, y, context->_depth->data[j+2], w, h);
                            j = i+4;
                            MakeVertex(x+1, y, context->_depth->data[j+2], w, h);
                            j = i+(w * 4);
                            MakeVertex(x, y+1, context->_depth->data[j+2], w, h);
                        }

                        if (context->_depth->data[i+6] > 0
                                && context->_depth->data[i+2+(w * 4)] > 0
                                && context->_depth->data[i+6+(w * 4)] > 0) {
                            j = i+4;
                            MakeVertex(x+1, y, context->_depth->data[j+2], w, h);
                            j = i+(w * 4);
                            MakeVertex(x, y+1, context->_depth->data[j+2], w, h);
                            j = i+(w * 4)+4;
                            MakeVertex(x+1, y+1, context->_depth->data[j+2], w, h);
                        }

                    }

                }
        }
        glEnd();
    }
}

void BoxExtractViewPort::update() {
    context->rgbImageBgRm.clear();

    context->depth.clear();
    context->depthImageBgRm.clear();
    context->depthDataBgRm.clear();

    context->irDataBgRm.clear();
    context->irImageBgRm.clear();

    if (context->_depth) {
        int w = context->_depth->width;
        int h = context->_depth->height;

        for (int y = 0; y < (h -1); ++y)
            for (int x = 0; x < (w -1); ++x)
            {

                if ((y-(h/2)) > -context->boxDim->getY()
                        && (y-(h/2)) < +context->boxDim->getY()
                        && (x-(w/2)) > (context->boxPos->getX() -context->boxDim->getX())
                        && (x-(w/2)) < (context->boxPos->getX() +context->boxDim->getX())) {

                    int i = ((y * w) + x) * 4;

                    //RGB
                    context->rgbImage.push_back(context->registered->data[i+2]);
                    context->rgbImage.push_back(context->registered->data[i+1]);
                    context->rgbImage.push_back(context->registered->data[i+0]);

                    uint16_t depth = context->_depth->data[i+2];
                    context->depth.push_back(depth);

                    if ((depth * depthScale) > (context->boxPos->getZ() - context->boxDim->getZ())
                            && (depth * depthScale) < (context->boxPos->getZ() + context->boxDim->getZ()))
                    {
                        //Depth extract
                        uint16_t depth = context->_depth->data[i+2];
                        context->depthImageBgRm.push_back(depth & 0xff);
                        context->depthImageBgRm.push_back(depth >> 8);
                        context->depthImageBgRm.push_back(0);

                        context->depthDataBgRm.push_back(depth);

                        //IR extract
                        uint16_t ir = context->_ir->data[i+2];
                        context->irImageBgRm.push_back(ir % 255);
                        context->irImageBgRm.push_back(ir / 255);
                        context->irImageBgRm.push_back(0);

                        context->irDataBgRm.push_back(ir);
                        //RGB
                        context->rgbImageBgRm.push_back(context->registered->data[i+2]);
                        context->rgbImageBgRm.push_back(context->registered->data[i+1]);
                        context->rgbImageBgRm.push_back(context->registered->data[i+0]);

                    } else {
                        //DEPTH
                        context->depthImageBgRm.push_back(0);
                        context->depthImageBgRm.push_back(0);
                        context->depthImageBgRm.push_back(0);
                        context->depthDataBgRm.push_back(0);

                        //IR
                        context->irImageBgRm.push_back(0);
                        context->irImageBgRm.push_back(0);
                        context->irImageBgRm.push_back(0);
                        context->irDataBgRm.push_back(0);

                        //RGB
                        context->rgbImageBgRm.push_back(0);
                        context->rgbImageBgRm.push_back(0);
                        context->rgbImageBgRm.push_back(0);
                    }
                }
            }
    }
}

void FrontCamViewPort::update() {

    if (flags == 1)
        glViewport(context->width/3, context->height/2, context->width/3, context->height/3);
    if (flags == 2)
        glViewport(context->width/3, 0, context->width/3, context->height);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    {
        glRotatef(180, 1.0f, 0.0f, 0.0f); // Rotate our camera on the x-axis (looking up and down)
        glTranslatef( 0, -0, context->boxDim->getZ() +100);

        glPointSize(1.0f);
        glBegin(GL_POINTS);
        int w = context->boxDim->getX() * 2 - 1;
        int h = context->boxDim->getY() * 2 - 1;

        if (context->depthDataBgRm.size() > (h*w))
            for (int y = 0; y < (h -1); ++y)
                for (int x = 0; x < (w -1); ++x)
                {
                    int i = ((y * w) + x);

                    if (!context->depthDataBgRm[i]) continue;

                    glColor3ub( context->rgbImageBgRm[3*i+0],    // R
                            context->rgbImageBgRm[3*i+1],    // G
                            context->rgbImageBgRm[3*i+2]);  // B

                    MakeVertex(x, y, context->depthDataBgRm[i], w, h);
                }
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

void LeftCamViewPort::update() {

    if (flags == 1)
        glViewport(0, context->height/2, context->width/3, context->height/3);
    if (flags == 2)
        glViewport(0, 0, context->width/3, context->height);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    {
        glRotatef(180, 1.0f, 0.0f, 0.0f); // Rotate our camera on the x-axis (looking up and down)
        glRotatef(-90, 0.0f, 1.0f, 0.0f); // Rotate our camera on the  y-axis (looking left and right)
        glTranslatef( context->boxPos->getX() + (context->boxDim->getX() * 2), 0, -context->boxPos->getZ() + context->boxDim->getZ() );

        glPointSize(2.0f);
        glBegin(GL_POINTS);
        int w = context->boxDim->getX() * 2 - 1;
        int h = context->boxDim->getY() * 2 - 1;

        if (context->depthDataBgRm.size() > (h*w))
            for (int y = 0; y < (h -1); ++y)
                for (int x = 0; x < (w -1); ++x)
                {
                    int i = ((y * w) + x);

                    if (!context->depthDataBgRm[i]) continue;

                    glColor3ub( context->rgbImageBgRm[3*i+0],    // R
                            context->rgbImageBgRm[3*i+1],    // G
                            context->rgbImageBgRm[3*i+2]);  // A

                    MakeVertex(x, y, context->depthDataBgRm[i], w, h);
                }
        glEnd();

        glBegin(GL_LINES);
        glColor3f(1, 0,  0);
        glVertex3f(context->boxPos->getX(), context->boxPos->getY() + context->boxDim->getY(), context->boxPos->getZ() - (0.95f * context->boxDim->getZ()));
        glVertex3f(context->boxPos->getX(), context->boxPos->getY() - context->boxDim->getY(), context->boxPos->getZ() - (0.95f * context->boxDim->getZ()));
        glEnd();
    }
}

void RightCamViewPort::update() {

    if (flags == 1)
        glViewport(context->width/3*2, context->height/2, context->width/3, context->height/3);
    if (flags == 2)
        glViewport(context->width/3*2, 0, context->width/3, context->height);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    {
        glRotatef(180, 1.0f, 0.0f, 0.0f); // Rotate our camera on the x-axis (looking up and down)
        glRotatef(90, 0.0f, 1.0f, 0.0f); // Rotate our camera on the  y-axis (looking left and right)
        glTranslatef( -context->boxPos->getX() - (context->boxDim->getX() * 2), 0, -context->boxPos->getZ() + context->boxDim->getZ() );


        glPointSize(2.0f);
        glBegin(GL_POINTS);
        int w = context->boxDim->getX() * 2 - 1;
        int h = context->boxDim->getY() * 2 - 1;

        if (context->depthDataBgRm.size() > (h*w))
            for (int y = 0; y < (h -1); ++y)
                for (int x = 0; x < (w -1); ++x)
                {
                    int i = ((y * w) + x);

                    if (!context->depthDataBgRm[i]) continue;

                    glColor3ub( context->rgbImageBgRm[3*i+0],    // R
                            context->rgbImageBgRm[3*i+1],    // G
                            context->rgbImageBgRm[3*i+2]);  // A

                    MakeVertex(x, y, context->depthDataBgRm[i], w, h);
                }
        glEnd();

        glBegin(GL_LINES);
        glColor3f(1, 0,  0);
        glVertex3f(context->boxPos->getX(), context->boxPos->getY() + context->boxDim->getY(), context->boxPos->getZ() - (0.95f * context->boxDim->getZ()));
        glVertex3f(context->boxPos->getX(), context->boxPos->getY() - context->boxDim->getY(), context->boxPos->getZ() - (0.95f * context->boxDim->getZ()));
        glEnd();
    }
}
