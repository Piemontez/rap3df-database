#include <vector>
#include <string>
#include <iostream>
#include <thread>
#include <locale>

#if defined(__APPLE__)
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif

#include "json/value.h"
#include "json/reader.h"
#include "utils.h"

void foo();

static int imageType = 0;

static int zoom = -300;
static int width = 640;
static int height = 480;
static int window = 0;

static GLfloat xv = 0.f;
static int xvOrient = 1;

static std::vector< std::vector<uint16_t>>::size_type filePos = 0;

static std::vector< std::vector<uint16_t> >files;
static std::vector< std::vector<uint16_t> >irFiles;
static std::vector< std::vector<uint16_t> >colorFiles;

int main(int argc, char **argv)
{
    std::string uuidFilter;
    std::string jsonFilePath;
    jsonFilePath.append(IMAGES_DIR).append("/").append(JSON_IMAGES_INFO);

    if (argc >= 2) {
        uuidFilter = argv[1];

        std::locale loc;
        for (std::string::size_type i =0; i < uuidFilter.length(); i++) {
          uuidFilter[i] = std::toupper(uuidFilter[i], loc);
        }
    }

    std::cout << "Openning " << jsonFilePath << std::endl;
    std::cout << "Filter: " << uuidFilter << std::endl;
    //Carrega arquivo com arquivos
    Json::Value root;
    Json::Value facesUUIDs;

    {//load json file
        std::FILE* jsonFile = std::fopen(jsonFilePath.c_str(),"r");
        if (jsonFile) {
            std::cout << "json opened" << std::endl;

            Json::Reader reader;
            std::string json;
            char buf[2];
            while (std::fgets(buf, sizeof buf, jsonFile) != nullptr) {
                json.push_back(buf[0]);
            }
            std::fclose(jsonFile);

            reader.parse(json, root);
        }
    }

    if (root["_faces"].empty()) {
        std::cout << "No images files found." << std::endl;
        exit(0);
    } else {
        facesUUIDs = root["_faces"];
        std::cout << facesUUIDs.size() << " faces in json file." << std::endl;
    }

    {//Load depth data
        for (auto && i = facesUUIDs.begin(); i != facesUUIDs.end(); i++)
        {
            std::string uuid = (*i).asString();
            if (uuidFilter.length() && uuidFilter.compare(uuid) != 0) continue;


            Json::Value voluntary = root[uuid];
            if (voluntary.empty() && !voluntary["front"].begin()->isObject()) continue;

            Json::Value front = *voluntary["front"].begin();

            std::string dethPath = front["depth_data_with_bg"].asString();
            jsonFilePath.clear();
            jsonFilePath.append(dethPath);
            std::cout << jsonFilePath << std::endl;

            std::vector<uint16_t> data;
            std::FILE * dataFile = std::fopen(jsonFilePath.c_str(),"r");
            if (dataFile!=nullptr)
            {
                uint16_t info;
                while (std::fread(&info, 1, sizeof(uint16_t), dataFile) != 0)
                {
                    data.push_back(info);
                }
                files.push_back(data);
            }

            std::string irPath = front["ir_data_with_bg"].asString();
            jsonFilePath.clear();
            jsonFilePath.append(irPath);
            std::cout << jsonFilePath << std::endl;

            std::vector< uint16_t > dataIr;
            dataFile = std::fopen(jsonFilePath.c_str(),"r");
            if (dataFile!=nullptr)
            {
                uint16_t info;
                while (std::fread(&info, 1, sizeof(uint16_t), dataFile) != 0) {
                    dataIr.push_back(info);
                }
                irFiles.push_back(dataIr);
            }
        }
    }

    glutInit(&argc, argv);

    glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);
    glutInitWindowSize(width, height);
    glutInitWindowPosition(0, 0);

    window = glutCreateWindow("RAP3DF");
    glClearColor(1.f, 1.0f, 1.0f, 0.0f);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_ALPHA_TEST);
    glAlphaFunc(GL_GREATER, 0.0f);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glMatrixMode(GL_PROJECTION);
    gluPerspective(50.0, 1.0, 900.0, 11000.0);

    glutDisplayFunc([] () {
        int w = 119;
        int h = 149;

        std::vector<uint16_t>::size_type i, j;
        std::vector<uint16_t> file = files[filePos];
        std::vector<uint16_t> irFile = irFiles[filePos];

        if (xv > (w/2)) {
            xvOrient = -1;
        }
        if (xv < -(w/2)) {
            xvOrient = 1;
        }
        xv += xvOrient;

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glViewport(0, 0, width, height);

        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
        {
            glRotatef(180, 1.f, 0.f, 0.f); // Rotate our camera on the x-axis (looking up and down)
            glTranslatef(xv*3, 0, abs(xv * 2));
            glRotatef(-xv, 0.f, 1.f, 0.f);

            glPointSize(1.0f);
//            glBegin(GL_POINTS);
            glBegin(GL_TRIANGLES);

            uint16_t min = 0, max = 0, z;
            float range;
            for (int y = 0; y < (h -1); ++y)
                for (int x = 0; x < (w -1); ++x)
                {
                    i = static_cast<unsigned long>(y * w) + static_cast<unsigned long>(x);
                    if (!file[i]) continue;

                    if (max < file[i]) {
                        if (max == 0)
                            min = file[i];
                        max = file[i];
                    }
                    if (min > file[i])
                        min = file[i];
                }
//            max *= 0.9f;
            range = 1.f/(max - min);
//            max *= 0.9f;

            for (int y = 0; y < (h -1); ++y)
                for (int x = 0; x < (w -1); ++x)
                {
                    i = static_cast< std::vector<uint16_t>::size_type >((y * w) + x);

                    if (file[i] == 0) continue;

                    if (imageType == 0) {
                        glColor3f(1- range * (file[i] - min) * 3,
                                   (range * (file[i] - min)),
                                  (range * (file[i] - min) * 2));
                    } else {
                        glColor3ub(static_cast<unsigned char>(irFile[i]), 0, 0);
                    }

//                    if (file[i] >= min && file[i+4] >= min && file[i+static_cast<unsigned long>(w*4)] >= min
//                     && file[i] < max && file[i+4] < max && file[i+static_cast<unsigned long>(w*4)] < max)
                    {
                        j = i; z = !file[j] ? file[i] : file[j];
                        glVertex3f(x-(w/2), y-(h/2), z * 3+ zoom);
                        j = i+1; z = !file[j] ? file[i] : file[j];
                        glVertex3f(x+1-(w/2), y-(h/2), z * 3+ zoom);
                        j = i+static_cast<unsigned long>(w); z = !file[j] ? file[i] : file[j];
                        glVertex3f(x-(w/2), y+1-(h/2), z * 3+ zoom);
                    }
//                    if (file[i+4] > min && file[i+static_cast<unsigned long>(w*4)] > min && file[i+static_cast<unsigned long>(w*4)+4] > min
//                     && file[i+4] < max && file[i+static_cast<unsigned long>(w*4)] < max && file[i+static_cast<unsigned long>(w*4)+4] < max)
                    {
                        j = i+1; z = !file[j] ? file[i] : file[j];
                        glVertex3f(x+1-(w/2), y-(h/2), z * 3+ zoom);
                        j = i+static_cast<unsigned long>(w); z = !file[j] ? file[i] : file[j];
                        glVertex3f(x-(w/2), y+1-(h/2), z * 3+ zoom);
                        j = i+static_cast<unsigned long>(w)+1; z = !file[j] ? file[i] : file[j];
                        glVertex3f(x+1-(w/2), y+1-(h/2), z * 3 + zoom);
                    }

                }
            }

            glEnd();

        glFlush();
        glutSwapBuffers();
    });

//    glutIdleFunc([] () {
//        glutPostRedisplay();
//    });

    glutReshapeFunc([] (int w, int h) {
        width = w;
        height = h;

        glViewport(0, 0, width, height);
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        gluPerspective(50.0, static_cast<double>(width) / height, 1.0, 12000.0);
    });


    glutKeyboardFunc([] (unsigned char key, int UNUSED(x), int UNUSED(y)) {
        if (key == 0x1B) {// ESC
            glutDestroyWindow(window);
            exit(0);
        }
        switch (key)
        {
            case  'Q':
            case  'q':
                xv = 0.f;
                if (filePos != 0)
                    filePos--;

                glutPostRedisplay();
                break;
            case  'e':
            case  'E':
                xv = 0.f;
                filePos++;
                if (filePos >= files.size())
                    filePos = files.size() - 1;

                glutPostRedisplay();
                break;
            case  'w':
            case  'W':
                glutPostRedisplay();
                break;
            case  'p':
            case  'P':
                imageType = (++imageType) % 3;
                glutPostRedisplay();
                break;
        }
    });

    std::thread first(foo);
    first.join();

    glutMainLoop();
}

void foo()
{
  // do stuff...
}
