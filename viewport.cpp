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
void MakeTriangle(int r, int c, int w, int h, unsigned char* data, int canais) {
    int i = ((r * w) + c) * canais;
    int j;
    if (data[i+2] > 0
            && data[i+6] > 0
            && data[i+2+(w * 4)] > 0) {
        j = i;
        MakeVertex(c, r, data[j+2], w, h);
        j = i+4;
        MakeVertex(c+1, r, data[j+2], w, h);
        j = i+(w * 4);
        MakeVertex(c, r+1, data[j+2], w, h);
    }

    if (data[i+6] > 0
            && data[i+2+(w * 4)] > 0
            && data[i+6+(w * 4)] > 0) {
        j = i+4;
        MakeVertex(c+1, r, data[j+2], w, h);
        j = i+(w * 4);
        MakeVertex(c, r+1, data[j+2], w, h);
        j = i+(w * 4)+4;
        MakeVertex(c+1, r+1, data[j+2], w, h);
    }
}

void BoxCamViewPort::update(int window) {

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

void InfoViewPort::update(int window) {
    glViewport(0, context->height(window)-30, context->width(window), context->height(window));

    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    {
        glLoadIdentity();


        gluOrtho2D(0, context->width(window), 0.0, context->height(window));
        glMatrixMode(GL_MODELVIEW);
        glColor3f(0, 0.0, 0.0);
        glPushMatrix();
        {
            glLoadIdentity();

            glRasterPos2i(5, 15);
            std::string s;

            if (this->context->step != STEP_NONE) {
                s = "UUID:" + this->context->uuid;
            }

            switch (this->context->step) {
            case STEP_NONE:
                s = "Type 1: Start.";
                break;
            case STEP_START:
                if (this->context->currImageType == 0) {
                    s += " Defina o tipo de imagem. Type [f,t,d,l,r,b]";
                } else {
                    s += (context->sel_rgbImage.size() ? " With images." : " WithOUT images.");
                    s += " Type 2: capture images";
                }
                break;
            case STEP_CACHE_IMAGE:
                s += " Type 9 to save image, 3 to get demographic data, or 0 to finish.";
                break;
            case STEP_DEMOGRAPHI:
                s += " Collecting of demographic data. Type - to close.";
                break;
            case STEP_FINISHED:
                s += " Liberado para coletar novos dados. Type 1: start";
                break;
            }

            for (std::string::iterator i = s.begin(); i != s.end(); ++i)
            {
                char c = *i;
                glutBitmapCharacter(GLUT_BITMAP_9_BY_15, c);
            }

            //Informações das imagens coletadas
            if ((this->context->step & (STEP_START | STEP_CACHE_IMAGE)) > 0)
            {
                glRasterPos2i(5, 0);
                s = "Images:";
                if (this->context->imagesSaved == 63) {
                    s += "<       CONCLUDED      >";
                } else {
                    s += "F"; s += (this->context->imagesSaved & 1)  > 0 ? "x" : "-"; s += (this->context->currImageType & 1)  ? "< " : "  ";//Front
                    s += "T"; s += (this->context->imagesSaved & 2)  > 0 ? "x" : "-"; s += (this->context->currImageType & 2)  ? "< " : "  ";//Top
                    s += "D"; s += (this->context->imagesSaved & 4)  > 0 ? "x" : "-"; s += (this->context->currImageType & 4)  ? "< " : "  ";//Down
                    s += "L"; s += (this->context->imagesSaved & 8)  > 0 ? "x" : "-"; s += (this->context->currImageType & 8)  ? "< " : "  ";//Left
                    s += "R"; s += (this->context->imagesSaved & 16) > 0 ? "x" : "-"; s += (this->context->currImageType & 16) ? "< " : "  ";//Right
                    s += "B"; s += (this->context->imagesSaved & 32) > 0 ? "x" : "-"; s += (this->context->currImageType & 32) ? "< " : "  ";//Pocket Lighter
                }
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
            }
            if (this->context->step == STEP_DEMOGRAPHI)
            {
                glRasterPos2i(5, 0);
                s = "";
                switch(this->context->demographicStep)
                {
                case 4:
                    s+= "Height:" + this->context->demographicHeight;
                    break;
                case 3:
                    s+= "Weight:" + this->context->demographicWeight;
                    break;
                case 2:
                    s+= "Color:" + this->context->demographicColor;
                    break;
                case 1:
                    s+= "Gender:" + this->context->demographicGender;
                    break;
                case 0:
                    s+= "Years:" + this->context->demographicYaers;
                    break;
                }


                for (std::string::iterator i = s.begin(); i != s.end(); ++i)
                {
                    char c = *i;
                    glutBitmapCharacter(GLUT_BITMAP_9_BY_15, c);
                }
            }

            glMatrixMode(GL_MODELVIEW);
        }
        glPopMatrix();

        glMatrixMode(GL_PROJECTION);
    }
    glPopMatrix();
}

void PointCamViewPort::update(int window) {

    glViewport(0, 0, context->width(window)/2, context->height(window)/2);
    //        glViewport(0, 0, context->width(window), context->height(window));

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    {
        glRotatef(context->cam->getXRot(), 1.0f, 0.0f, 0.0f); // Rotate our camera on the x-axis (looking up and down)
        glRotatef(context->cam->getYRot(), 0.0f, 1.0f, 0.0f); // Rotate our camera on the  y-axis (looking left and right)
        glRotatef(context->cam->getZRot(), 0.0f, 0.0f, 1.0f);
        //            glTranslatef( -context->cam->getXPos()-(context->width(window)/2), -context->cam->getYPos()-(context->height(window)/3), -context->cam->getZPos()+500 );
        glTranslatef( -context->cam->getXPos(), -context->cam->getYPos(), -context->cam->getZPos() );

        glPointSize(1.0f);
        glBegin(GL_POINTS);

        if (context->_depth)
        {
            int w = context->_depth->width;
            int h = context->_depth->height;

            for (int r = 0; r < (h -1); ++r)
                for (int c = 0; c < (w -1); ++c)
                {
                    int i = ((r * w) + c) * 4;

                    const uint8_t *p = reinterpret_cast<uint8_t*>(&context->registered->data[i]);

                    if (!context->_rgb->status)
                        glColor3ub( p[2], p[1], p[0]);

                    if (!context->_depth->status && context->_depth->data[i+2] > 0)
                        MakeVertex(c, r, context->_depth->data[i+2], w, h);

                }
        }
        glEnd();
    }
}

void TriangleCamViewPort::update(int window) {

    glViewport(context->width(window)/2, 0, context->width(window)/2, context->height(window)/2);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    {
        glRotatef(context->cam->getXRot(), 1.0f, 0.0f, 0.0f); // Rotate our camera on the x-axis (looking up and down)
        glRotatef(context->cam->getYRot(), 0.0f, 1.0f, 0.0f); // Rotate our camera on the  y-axis (looking left and right)
        glRotatef(context->cam->getZRot(), 0.0f, 0.0f, 1.0f);
        //            glTranslatef( -context->cam->getXPos()-(context->width(window)/2), -context->cam->getYPos()-(context->height(window)/3), -context->cam->getZPos()+500 );
        glTranslatef( -context->cam->getXPos(), -context->cam->getYPos(), -context->cam->getZPos() );

        glBegin(GL_TRIANGLES);
        if (context->_depth)
        {
            int w = context->_depth->width;
            int h = context->_depth->height;
            int j;

            for (int r = 0; r < (h -1); ++r)
                for (int c = 0; c < (w -1); ++c)
                {
                    int i = ((r * w) + c) * 4;

                    const uint8_t *p = reinterpret_cast<uint8_t*>(&context->registered->data[i]);

                    if (!context->_rgb->status)
                        glColor3ub( p[2], p[1], p[0]);

                    if (!context->_depth->status)
                        MakeTriangle(r, c, w, h, context->_depth->data, 4);

                }
        }
        glEnd();
    }
}

void BoxExtractViewPort::update(int window) {
    context->rgbImage.clear();
    context->rgbImageBgRm.clear();

    context->depth.clear();
    context->depthImageBgRm.clear();
    context->depthDataBgRm.clear();

    context->irDataBgRm.clear();
    context->irImageBgRm.clear();

    if (context->_depth) {
        int w = context->_depth->width;
        int h = context->_depth->height;
        int bp = context->_depth->bytes_per_pixel;

        int r_w = context->_rgb->width;
        int r_h = context->_rgb->height;
        int r_bp = context->_rgb->bytes_per_pixel;


        for (int y = 0; y < (h -1); ++y)
            for (int x = 0; x < (w -1); ++x)
            {

                //Todo: mover para valor inical de y e x
                if ((y-(h/2)) > -context->boxDim->getY()
                        && (y-(h/2)) < +context->boxDim->getY()
                        && (x-(w/2)) > (context->boxPos->getX() -context->boxDim->getX())
                        && (x-(w/2)) < (context->boxPos->getX() +context->boxDim->getX()))
                {

                    int i  = ((y*w)  + x) * bp;
                    int ib = (((int)(y*r_h/(float)h) * r_w) + (int)(x*r_w/(float)w)) * r_bp;

                    context->rgbImage.push_back(context->_rgb->data[ib+2]);
                    context->rgbImage.push_back(context->_rgb->data[ib+1]);
                    context->rgbImage.push_back(context->_rgb->data[ib+0]);

                    uint16_t depth = context->_depth->data[i+2];
                    context->depth.push_back(depth);

//                    if ((depth * depthScale) > (context->boxPos->getZ() - context->boxDim->getZ())
//                            && (depth * depthScale) < (context->boxPos->getZ() + context->boxDim->getZ()))
                    {
                        //Depth extract
                        uint16_t depth = context->_depth->data[i+2];
                        context->depthImageBgRm.push_back(depth & 0xff);
                        context->depthImageBgRm.push_back(depth >> 8);
                        context->depthImageBgRm.push_back(depth & 0xff);

                        context->depthDataBgRm.push_back(depth);

                        //IR extract
                        uint16_t ir = context->_ir->data[i+2];

                        context->irImageBgRm.push_back(ir & 0xff);
                        context->irImageBgRm.push_back(ir >> 8);
                        context->irImageBgRm.push_back(0);

                        context->irDataBgRm.push_back(ir);
                        //RGB
                        context->rgbImageBgRm.push_back(context->registered->data[i+2]);
                        context->rgbImageBgRm.push_back(context->registered->data[i+1]);
                        context->rgbImageBgRm.push_back(context->registered->data[i+0]);

                    }/* else {
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
                    }*/
                }
            }
    }
}

void FrontCamViewPort::update(int window) {

    if (flags == 1)
        glViewport(context->width(window)/3, context->height(window)/2, context->width(window)/3, context->height(window)/3);
    if (flags == 2)
        glViewport(context->width(window)/12*3, context->height(window)/2, context->width(window)/2, context->height(window)/2);
    if (flags == 3)
        glViewport(context->width(window)/12*3, 0, context->width(window)/2, context->height(window)/2);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    {
        glRotatef(180, 1.0f, 0.0f, 0.0f); // Rotate our camera on the x-axis (looking up and down)
        glTranslatef( 0, -0, context->boxDim->getZ() - 100);

        glPointSize(1.0f);

        int w = context->boxDim->getX() * 2 - 1;
        int h = context->boxDim->getY() * 2 - 1;

        std::vector<uint8_t>* rgb;
        std::vector<uint16_t>* depth;

        if (context->sel_depthDataBgRm.size()) {
            depth = &context->sel_depthDataBgRm;
            rgb = &context->sel_rgbImageBgRm;
        } else {
            depth = &context->depthDataBgRm;
            rgb = &context->rgbImageBgRm;
        }

        if (depth->size() >= (h*w)) {
            if (flags == 3)
                glBegin(GL_TRIANGLES);
            else
                glBegin(GL_POINTS);

            for (int r = 0; r < (h -1); ++r)
                for (int c = 0; c < (w -1); ++c)
                {
                    int i = ((r * w) + c);

                    const uint8_t *p = reinterpret_cast<uint8_t*>(&(*rgb)[3*i]);
                    glColor3ubv(p); //glColor3ub( p[0], p[1], p[2]);

                    if (flags == 3) {
                        unsigned char* data = reinterpret_cast<uint8_t*>(depth->data());
                        MakeTriangle(r, c, w, h, data, 2);
                    } else {
                        MakeVertex(c, r, (*depth)[i], w, h);
                    }
                }
            glEnd();
        }

        glBegin(GL_LINES);
        glColor3f(1, 0,  0);
        glVertex3f(context->boxPos->getX(), context->boxPos->getY() + context->boxDim->getY(), context->boxPos->getZ() - context->boxDim->getZ());
        glVertex3f(context->boxPos->getX(), context->boxPos->getY() - context->boxDim->getY(), context->boxPos->getZ() - context->boxDim->getZ());

        glVertex3f(context->boxPos->getX() + context->boxDim->getX(), context->boxPos->getY(), context->boxPos->getZ() - context->boxDim->getZ());
        glVertex3f(context->boxPos->getX() - context->boxDim->getX(), context->boxPos->getY(), context->boxPos->getZ() - context->boxDim->getZ());
        glEnd();
    }
}

void LeftCamViewPort::update(int window) {

    if (flags == 1)
        glViewport(0, context->height(window)/2, context->width(window)/3, context->height(window)/3);
    if (flags == 2)
        glViewport(0, context->height(window)/3, context->width(window)/3, context->height(window)/3);

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

        std::vector<uint8_t>* rgb;
        std::vector<uint16_t>* depth;

        if (context->sel_depthDataBgRm.size()) {
            depth = &context->sel_depthDataBgRm;
            rgb = &context->sel_rgbImageBgRm;
        } else {
            depth = &context->depthDataBgRm;
            rgb = &context->rgbImageBgRm;
        }

        if (depth->size() >= (h*w))
            for (int y = 0; y < (h -1); ++y)
                for (int x = 0; x < (w -1); ++x)
                {
                    int i = ((y * w) + x);

                    if (!(*depth)[i]) continue;

                    const uint8_t *p = reinterpret_cast<uint8_t*>(&(*rgb)[3*i]);
                    glColor3ubv(p); //glColor3ub( p[0], p[1], p[2]);

                    MakeVertex(x, y, (*depth)[i], w, h);
                }
        glEnd();

        glBegin(GL_LINES);
        glColor3f(1, 0,  0);
        glVertex3f(context->boxPos->getX(), context->boxPos->getY() + context->boxDim->getY(), context->boxPos->getZ() - (0.95f * context->boxDim->getZ()));
        glVertex3f(context->boxPos->getX(), context->boxPos->getY() - context->boxDim->getY(), context->boxPos->getZ() - (0.95f * context->boxDim->getZ()));
        glEnd();
    }
}

void RightCamViewPort::update(int window) {

    if (flags == 1)
        glViewport(context->width(window)/3*2, context->height(window)/2, context->width(window)/3, context->height(window)/3);
    if (flags == 2)
        glViewport(context->width(window)/3*2, context->height(window)/3, context->width(window)/3, context->height(window)/3);

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

        std::vector<uint8_t>* rgb;
        std::vector<uint16_t>* depth;

        if (context->sel_depthDataBgRm.size()) {
            depth = &context->sel_depthDataBgRm;
            rgb = &context->sel_rgbImageBgRm;
        } else {
            depth = &context->depthDataBgRm;
            rgb = &context->rgbImageBgRm;
        }

        if (depth->size() >= (h*w))
            for (int y = 0; y < (h -1); ++y)
                for (int x = 0; x < (w -1); ++x)
                {
                    int i = ((y * w) + x);

                    if (!(*depth)[i]) continue;

                    const uint8_t *p = reinterpret_cast<uint8_t*>(&(*rgb)[3*i]);
                    glColor3ubv(p); //glColor3ub( p[0], p[1], p[2]);

                    MakeVertex(x, y, (*depth)[i], w, h);
                }

        glEnd();

        glBegin(GL_LINES);
        glColor3f(1, 0,  0);
        glVertex3f(context->boxPos->getX(), context->boxPos->getY() + context->boxDim->getY(), context->boxPos->getZ() - (0.95f * context->boxDim->getZ()));
        glVertex3f(context->boxPos->getX(), context->boxPos->getY() - context->boxDim->getY(), context->boxPos->getZ() - (0.95f * context->boxDim->getZ()));
        glEnd();
    }
}
