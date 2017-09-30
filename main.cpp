#include <algorithm>

#if defined(__APPLE__)
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif

#include "context.h"
#include "camera.h"
#include "freenectdevice.h"

#include "utils.h"

#define IMAGES_DIR "rap3df_data"
#define CSV_IMAGES_INFO "database.csv"

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

/*
 * Display cam position info
 */
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
              glColor3ub(0, 0.0, 0.0);

              std::string s =
                      "UUID " + uuidFolderName
                      + " Box Check" + std::to_string(minX) + 'x' + std::to_string(minY) + "|"
                                  + std::to_string(maxX) + 'x' + std::to_string(maxY)
                      + " rx" + std::to_string(context->cam->getXRot())
                      + " ry" + std::to_string(context->cam->getYRot())
                      + " rz" + std::to_string(context->cam->getZRot())

                      + " x" + std::to_string(context->cam->getXPos())
                      + " y" + std::to_string(context->cam->getYPos())
                      + " z" + std::to_string(context->cam->getZPos())

                      + " size" + std::to_string(context->rgb.size());

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
    void update(std::vector<uint8_t> &rgb, std::vector<uint16_t> &depth) {

        glViewport(0, context->height/2, context->width/2, context->height/2);

        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
        {
            glRotatef(context->cam->getXRot(), 1.0f, 0.0f, 0.0f); // Rotate our camera on the x-axis (looking up and down)
            glRotatef(context->cam->getYRot(), 0.0f, 1.0f, 0.0f); // Rotate our camera on the  y-axis (looking left and right)
            glRotatef(context->cam->getZRot(), 0.0f, 0.0f, 1.0f);
            glTranslatef( -context->cam->getXPos(), -context->cam->getYPos(), -context->cam->getZPos() );

            glPointSize(1.5f);
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


/*
 * Display stereo cam basic region triangles
 */
class TriangleCamViewPort: public ContextViewPort
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
        context->rgbInBox.clear();
        context->depthInBox.clear();

        std::vector<uint8_t>::iterator itRgb = rgb.begin();
        std::vector<uint16_t>::iterator itDepth = depth.begin();

        for (int j = 0; j < 480; j++) //Rows
            for (int k = 0; k < 640; k++) //Cols
            {
//                if (k >= extractBegX && k <= extractEndX
//                 && j >= extractBegY && j <= extractEndY) {
//                    context->rgbInBox.push_back(*itRgb++);
//                    context->rgbInBox.push_back(*itRgb++);
//                    context->rgbInBox.push_back(*itRgb);
//                    itRgb--;itRgb--;

////                    context->depthInBox.push_back(*itDepth++);
//                }

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

            glPointSize(1.0f);
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

            glPointSize(1.0f);
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

            glPointSize(1.0f);
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

class GenerateUUIDAction: public ContextAction
{
    void exec() {
        srand (time(NULL));

        const char* charmap = "ABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890";
        const size_t charmapLength = 36;
        auto generator = [&](){ return charmap[rand()%charmapLength]; };
        uuidFolderName.clear(); uuidFolderName.reserve(7);
        std::generate_n(std::back_inserter(uuidFolderName), 7, generator);
    }
};

class SaveImagesAction: public ContextAction
{
    void exec() {
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
          std::fputs((uuidFolderName+";\r\n").c_str(),csvFile);
          std::fclose (csvFile);

        }

        //Save original rgb image
        path.clear();
        path.append(IMAGES_DIR).append("/").append(uuidFolderName).append("/").append("original.bmp");
        WriteBMPFile(context->rgb, path, 640, 480);

        //Save bitmap deth image.


        //In box
        int w = extractEndX - extractBegX;
        int h = extractEndY - extractBegY;

        //Save original rgb image
        path.clear();
        path.append(IMAGES_DIR).append("/").append(uuidFolderName).append("/").append("box.bmp");
        WriteBMPFile(context->rgbInBox, path, w, h);

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

    context->initGlLoop(argc, argv);

    return 0;
}
