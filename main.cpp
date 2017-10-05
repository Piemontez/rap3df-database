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
        int w = extractEndX - extractBegX;
        int h = extractEndY - extractBegY;

        glViewport(0, context->height-30, context->width, context->height);

        glMatrixMode(GL_PROJECTION);
        glPushMatrix();
        {
          glLoadIdentity();


          gluOrtho2D(0.0, context->width, 0.0, context->height);
          glMatrixMode(GL_MODELVIEW);
          glColor3f(0, 0.0, 0.0);
          glPushMatrix();
          {
              glLoadIdentity();

              glRasterPos2i(10, 10);
              std::string s =
                      "UUID " + uuidFolderName
                      + " Box Check" + std::to_string(minX) + 'x' + std::to_string(minY) + "|"
                                  + std::to_string(maxX) + 'x' + std::to_string(maxY)
                      + " width:" + std::to_string(w)
                      + " heigh:" + std::to_string(h);

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

        glViewport(0, 0, context->width/2, context->height/2);

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
 * Display stereo cam with basic algorithm for triangles
 */
class TriangleCamViewPort: public ContextViewPort
{
public:
    void update(std::vector<uint8_t> &rgb, std::vector<uint16_t> &depth) {

        glViewport(context->width/2, 0, context->width/2, context->height/2);

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
        context->rgbInBoxXY.clear();
        context->depthImageInBoxXY.clear();
        context->depthInBoxXY.clear();

        context->depthImageInBoxXYZ.clear();
        context->depthInBoxXYZ.clear();

        std::vector<uint8_t>::iterator itRgb = rgb.begin();
        std::vector<uint16_t>::iterator itDepth = depth.begin();

        for (int j = 0; j < 480; j++) //Rows
            for (int k = 0; k < 640; k++) //Cols
            {
                if (k >= extractBegX && k < extractEndX
                 && j >= extractBegY && j < extractEndY) {
                    context->rgbInBoxXY.push_back(*itRgb++);
                    context->rgbInBoxXY.push_back(*itRgb++);
                    context->rgbInBoxXY.push_back(*itRgb++);

                    context->depthImageInBoxXY.push_back((*itDepth) & 0xff);
                    context->depthImageInBoxXY.push_back((*itDepth) >> 8);
                    context->depthImageInBoxXY.push_back(0);

                    context->depthInBoxXY.push_back(*itDepth);

                    if ((*itDepth) > (context->boxPos->getZ() - context->boxDim->getZ())
                            && (*itDepth) < (context->boxPos->getZ() + context->boxDim->getZ()))
                    {
                        context->depthImageInBoxXYZ.push_back((*itDepth) & 0xff);
                        context->depthImageInBoxXYZ.push_back((*itDepth) >> 8);
                        context->depthImageInBoxXYZ.push_back(0);

                        context->depthInBoxXYZ.push_back(*itDepth);
                    } else {
                        context->depthImageInBoxXYZ.push_back(0);
                        context->depthImageInBoxXYZ.push_back(0);
                        context->depthImageInBoxXYZ.push_back(0);
                        context->depthInBoxXYZ.push_back(0);
                    }
                } else {
                    itRgb+=3;
                }
                itDepth++;

//                checkRegions()
            }
    }
};

class FrontCamViewPort: public ContextViewPort
{
public:
    void update(std::vector<uint8_t> &rgb, std::vector<uint16_t> &depth) {

        glViewport(context->width/3, context->height/2, context->width/3, context->height/3);

        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
        {
            glRotatef(180, 1.0f, 0.0f, 0.0f); // Rotate our camera on the x-axis (looking up and down)
            glTranslatef( 0, -0, -500 );

            glPointSize(1.0f);
            glBegin(GL_POINTS);

            int w = extractEndX - extractBegX;
            int h = extractEndY - extractBegY;
            for (int i = 0; i < w*h; ++i)
            {
                if (!context->depthInBoxXYZ[i]) continue;

                glColor4f(1/(((context->depthInBoxXYZ[i]/2) & 0xff) / 100.f),
                          1/((context->depthInBoxXYZ[i] & 0xff) / 100.f),
                          1/(((context->depthInBoxXYZ[i]/4) & 0xff) / 100.f),
                          0.7f);

                glVertex3f( (i%w- (w-1)/2.f) * context->depthInBoxXYZ[i] / Context::instance()->f,  // X = (x - cx) * d / fx
                            (i/w- (h-1)/2.f) * context->depthInBoxXYZ[i] / Context::instance()->f,  // Y = (y - cy) * d / fy
                            context->depthInBoxXYZ[i] );

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
};


class LeftCamViewPort: public ContextViewPort
{
public:
    void update(std::vector<uint8_t> &rgb, std::vector<uint16_t> &depth) {

        glViewport(0, context->height/2, context->width/3, context->height/3);

        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
        {
            glRotatef(180, 1.0f, 0.0f, 0.0f); // Rotate our camera on the x-axis (looking up and down)
            glRotatef(-90, 0.0f, 1.0f, 0.0f); // Rotate our camera on the  y-axis (looking left and right)
            glTranslatef( 500, 0, -1000 );

            glPointSize(1.0f);
            glBegin(GL_POINTS);

            int w = extractEndX - extractBegX;
            int h = extractEndY - extractBegY;
            for (int i = 0; i < w*h; ++i)
            {
                if (!context->depthInBoxXYZ[i]) continue;

                glColor4f(1/((context->depthInBoxXYZ[i] & 0xff) / 100.f),
                          1/((context->depthInBoxXYZ[i] & 0xff) / 100.f),
                          1/((context->depthInBoxXYZ[i] >> 8) / 50.f), 0.7f);

                glVertex3f( (i%w- (w-1)/2.f) * context->depthInBoxXYZ[i] / Context::instance()->f,  // X = (x - cx) * d / fx
                            (i/w- (h-1)/2.f) * context->depthInBoxXYZ[i] / Context::instance()->f,  // Y = (y - cy) * d / fy
                            context->depthInBoxXYZ[i] );

            }

            glEnd();

            glBegin(GL_LINES);
                glColor3f(1, 0,  0);
                glVertex3f(context->boxPos->getX(), context->boxPos->getY() + context->boxDim->getY(), context->boxPos->getZ() - (0.95f * context->boxDim->getZ()));
                glVertex3f(context->boxPos->getX(), context->boxPos->getY() - context->boxDim->getY(), context->boxPos->getZ() - (0.95f * context->boxDim->getZ()));
            glEnd();
        }
    }
};

class RightCamViewPort: public ContextViewPort
{
public:
    void update(std::vector<uint8_t> &rgb, std::vector<uint16_t> &depth) {

        glViewport(context->width/3*2, context->height/2, context->width/3, context->height/3);

        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
        {
            glRotatef(180, 1.0f, 0.0f, 0.0f); // Rotate our camera on the x-axis (looking up and down)
            glRotatef(90, 0.0f, 1.0f, 0.0f); // Rotate our camera on the  y-axis (looking left and right)
            glTranslatef( -500, 0, -1000 );

            glPointSize(1.0f);
            glBegin(GL_POINTS);

            int w = extractEndX - extractBegX;
            int h = extractEndY - extractBegY;
            for (int i = 0; i < w*h; ++i)
            {
                if (!context->depthInBoxXYZ[i]) continue;

                glColor4f(1/((context->depthInBoxXYZ[i] & 0xff) / 100.f),
                          1/((context->depthInBoxXYZ[i] & 0xff) / 100.f),
                          1/((context->depthInBoxXYZ[i] >> 8) / 50.f), 0.7f);

                glVertex3f( (i%w- (w-1)/2.f) * context->depthInBoxXYZ[i] / Context::instance()->f,  // X = (x - cx) * d / fx
                            (i/w- (h-1)/2.f) * context->depthInBoxXYZ[i] / Context::instance()->f,  // Y = (y - cy) * d / fy
                            context->depthInBoxXYZ[i] );

            }

            glEnd();

            glBegin(GL_LINES);
                glColor3f(1, 0,  0);
                glVertex3f(context->boxPos->getX(), context->boxPos->getY() + context->boxDim->getY(), context->boxPos->getZ() - (0.95f * context->boxDim->getZ()));
                glVertex3f(context->boxPos->getX(), context->boxPos->getY() - context->boxDim->getY(), context->boxPos->getZ() - (0.95f * context->boxDim->getZ()));
            glEnd();
        }
    }
};

std::string UUID() {
    std::string uuid;
    uuid.reserve(7);

    srand (time(NULL));

    const char* charmap = "ABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890";
    const size_t charmapLength = 36;
    auto generator = [&](){ return charmap[rand()%charmapLength]; };

    std::generate_n(std::back_inserter(uuid), 7, generator);

    return uuid;
}

class GenerateUUIDAction: public ContextAction
{
    void exec() {
        uuidFolderName = UUID();
    }
};

class SaveImagesAction: public ContextAction
{
    void exec() {
        int w = extractEndX - extractBegX;
        int h = extractEndY - extractBegY;

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
          std::string info;

          info += uuidFolderName + ";";
          info += std::to_string(w)+ ";";
          info += std::to_string(h)+ ";";
          info += "\r\n";

          std::fputs(info.c_str(),csvFile);
          std::fclose (csvFile);
        }

        //
        // KINECT 1
        //
        //Save original rgb image
        path.clear();
        path.append(IMAGES_DIR).append("/").append(uuidFolderName).append("/").append(KINECT_1_XY_FILE);
        WriteBMPFile(context->rgbInBoxXY, path, w, h);

        //Save original bitmap deth image for view.
        path.clear();
        path.append(IMAGES_DIR).append("/").append(uuidFolderName).append("/").append(KINECT_1_XY_DEPTH_VIEW_FILE);
        WriteBMPFile(context->depthImageInBoxXY, path, w, h);

        //Save original bitmap deth image for view.
        path.clear();
        path.append(IMAGES_DIR).append("/").append(uuidFolderName).append("/").append(KINECT_1_XY_DATA_FILE);
        WriteFile(context->depthInBoxXY, path, w, h);

        //Save original bitmap deth image for view.
        path.clear();
        path.append(IMAGES_DIR).append("/").append(uuidFolderName).append("/").append(KINECT_1_XYZ_DEPTH_VIEW_FILE);
        WriteBMPFile(context->depthImageInBoxXYZ, path, w, h);

        //Save original bitmap deth image for view.
        path.clear();
        path.append(IMAGES_DIR).append("/").append(uuidFolderName).append("/").append(KINECT_1_XYZ_DATA_FILE);
        WriteFile(context->depthInBoxXYZ, path, w, h);
    }
};

class SaveTestImagesAction: public ContextAction
{
    void exec() {
        int w = extractEndX - extractBegX;
        int h = extractEndY - extractBegY;

        std::string path;
        path.append("mkdir ").append(IMAGES_DIR);
        system(path.c_str());

        path.clear();
        path.append("mkdir ").append(IMAGES_DIR).append("/").append(uuidFolderName);
        system(path.c_str());

        path.clear();
        path.append("mkdir ").append(IMAGES_DIR).append("/").append(uuidFolderName).append("/").append(TEST_DIR);
        system(path.c_str());


        std::string prefixFilesName = UUID();
        std::string folderTest;
        folderTest.append(IMAGES_DIR).append("/").append(uuidFolderName).append("/").append(TEST_DIR).append("/");

        std::string csvFilePath;
        csvFilePath.append(folderTest).append(CSV_IMAGES_INFO);


        //Save csv database info
        std::FILE * csvFile;
        csvFile = std::fopen(csvFilePath.c_str(),"a");
        if (csvFile)
        {
          std::string info;

          info += prefixFilesName + ";";
          info += std::to_string(w)+ ";";
          info += std::to_string(h)+ ";";
          info += "\r\n";

          std::fputs(info.c_str(),csvFile);
          std::fclose (csvFile);
        }

        //
        // KINECT 1
        //
        //Save original rgb image
        path.clear(); path.append(folderTest).append(prefixFilesName).append(KINECT_1_XY_FILE);
        WriteBMPFile(context->rgbInBoxXY, path, w, h);

        //Save original bitmap deth image for view.
        path.clear(); path.append(folderTest).append(prefixFilesName).append(KINECT_1_XY_DEPTH_VIEW_FILE);
        WriteBMPFile(context->depthImageInBoxXY, path, w, h);

        //Save original bitmap deth image for view.
        path.clear(); path.append(folderTest).append(prefixFilesName).append(KINECT_1_XY_DATA_FILE);
        WriteFile(context->depthInBoxXY, path, w, h);

        //Save original bitmap deth image for view.
        path.clear(); path.append(folderTest).append(prefixFilesName).append(KINECT_1_XYZ_DEPTH_VIEW_FILE);
        WriteBMPFile(context->depthImageInBoxXYZ, path, w, h);

        //Save original bitmap deth image for view.
        path.clear(); path.append(folderTest).append(prefixFilesName).append(KINECT_1_XYZ_DATA_FILE);
        WriteFile(context->depthInBoxXYZ, path, w, h);
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
    context->addAction('5', new SaveTestImagesAction);

    context->initGlLoop(argc, argv);

    return 0;
}
