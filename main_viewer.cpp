#include <vector>
#include <string>
#include <iostream>
#include <thread>

#if defined(__APPLE__)
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif

#include "utils.h"

void foo();

int width = 640;
int height = 480;
int window = 0;

GLfloat xv = 0.f;
int xvOrient = 1;

float f = 595.f;
int filePos = 0;

std::vector< std::vector<uint16_t> >files;
std::vector< std::vector<uint16_t> >colorFiles;
std::vector<std::string> widths;
std::vector<std::string> heights;

int main(int argc, char **argv)
{
    std::string csvFilePath;
    //csvFilePath.append(IMAGES_DIR).append("/").append(JSON_IMAGES_INFO);
    csvFilePath.append("/home/piemontez/Projects/piemontez/rap3df-database/rap3df_data").append("/").append("database.csv");

    std::cout << csvFilePath << "\n";
    //Carrega arquivo com arquivos
    std::vector<std::string> folders;

    {//load csv file
        std::FILE* csvFile = std::fopen(csvFilePath.c_str(),"r");
        if (csvFile) {
            std::vector<char> foldername;
            char buf[2];
            int pos = 0;
            while (std::fgets(buf, sizeof buf, csvFile) != NULL)
            {
                if (buf[0] == '\n') {
                    foldername.clear();
                    pos = 0;
                } else if(buf[0] != ';') {
                    foldername.push_back(buf[0]);
                } else if(buf[0] == ';') {
                    switch (pos) {
                    case 0:
                        folders.push_back(std::string(foldername.begin(), foldername.end()));
                        foldername.clear();
                        break;
                    case 1:
                        widths.push_back(std::string(foldername.begin(), foldername.end()));
                        foldername.clear();
                        break;
                    case 2:
                        heights.push_back(std::string(foldername.begin(), foldername.end()));
                        foldername.clear();
                        break;
                    }
                    pos++;
                }
            }
        }
    }

    if (!folders.size()) {
        std::cout << "No images files found." << std::endl;
        exit(0);
    }

    {//Load depth data
//        for (std::vector<std::string>::iterator i = folders.end(); i != folders.begin(); i--)
        for (std::vector<std::string>::iterator i = folders.begin(); i != folders.end(); i++)
        {
            csvFilePath.clear();
            csvFilePath.append(IMAGES_DIR).append("/").append(*i).append("/").append(FILE_DATA_DEPTH_BG_REM);

            std::vector<uint16_t> data;
            std::FILE * dataFile = std::fopen(csvFilePath.c_str(),"r");
            if (dataFile!=NULL)
            {

                int i = 0;
                uint16_t info;
                while (std::fread(&info, 1, sizeof(uint16_t), dataFile) != 0)
                {
                    data.push_back(info);
                }
                std::cout << std::endl;
                files.push_back(data);
            }

            /*csvFilePath.clear();
            csvFilePath.append(IMAGES_DIR).append("/").append(*i).append("/").append(KINECT_1_XYZ_DATA_FILE);

            std::vector<uint16_t> data;
            std::FILE * dataFile = std::fopen(csvFilePath.c_str(),"r");
            if (dataFile!=NULL)
            {

                int i = 0;
                uint16_t info;
                while (std::fread(&info, 1, sizeof(uint16_t), dataFile) != 0)
                {
                    data.push_back(info);
                }
                std::cout << std::endl;
                colorFiles.push_back(data);
            }*/
        }
    }

    //Exibe os arquivos

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
        int w = atoi(widths[filePos].c_str());
        int h = atoi(heights[filePos].c_str());

        std::vector<uint16_t> file = files[filePos];

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
            glTranslatef(xv*3, 0, abs((int)xv * 2));
            glRotatef(-xv, 0.f, 1.f, 0.f);

            glPointSize(1.0f);
//            glBegin(GL_POINTS);
            glBegin(GL_TRIANGLES);

            int j;
            uint16_t min = 0, max = 0;
            float range;
            for (int y = 0; y < (h -1); ++y)
                for (int x = 0; x < (w -1); ++x)
                {
                    int i = ((y * w) + x);
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
                    int i = ((y * w) + x);

                    glColor3f(1- range * (file[i] - min) * 3,
                               (range * (file[i] - min)),
                              (range * (file[i] - min) * 2));

                    if (file[i] >= min && file[i+4] >= min && file[i+(w*4)] >= min
                     && file[i] < max && file[i+4] < max && file[i+(w*4)] < max)
                    {
                        j = i;
                        glVertex3f(x-(w/2), y-(h/2), file[j] * 2);
                        j = i+4;
                        glVertex3f(x+1-(w/2), y-(h/2), file[j] * 2);
                        j = i+(w * 4);
                        glVertex3f(x-(w/2), y+1-(h/2), file[j] * 2);
                    }
                    if (file[i+4] > min && file[i+(w*4)] > min && file[i+(w*4)+4] > min
                     && file[i+4] < max && file[i+(w*4)] < max && file[i+(w*4)+4] < max)
                    {
                        j = i+4;
                        glVertex3f(x+1-(w/2), y-(h/2), file[j] * 2);
                        j = i+(w*4);
                        glVertex3f(x-(w/2), y+1-(h/2), file[j] * 2);
                        j = i+(w*4)+4;
                        glVertex3f(x+1-(w/2), y+1-(h/2), file[j] * 2);
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
        gluPerspective(50.0, (float)width / height, 1.0, 12000.0);
    });


    glutKeyboardFunc([] (unsigned char key, int x, int y) {
        if (key == 0x1B) {// ESC
            glutDestroyWindow(window);
            exit(0);
        }
        switch (key)
        {
            case  'Q':
            case  'q':
                xv = 0.f;
                filePos--;
                if (filePos < 0)
                    filePos = 0;

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
