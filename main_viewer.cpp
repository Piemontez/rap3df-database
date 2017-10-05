#include <vector>
#include <string>
#include <iostream>

#if defined(__APPLE__)
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif

#include "utils.h"

int width = 640;
int height = 480;
int window = 0;

float f = 595.f;
int filePos = 0;
std::vector< std::vector<uint16_t> >files;

int main(int argc, char **argv)
{
    std::string csvFilePath;
    csvFilePath.append(IMAGES_DIR).append("/").append(CSV_IMAGES_INFO);

    //Carrega arquivo com arquivos
    std::vector<std::string> folders;
    std::vector<std::string> widths;
    std::vector<std::string> heights;

    std::FILE * csvFile = std::fopen(csvFilePath.c_str(),"r");

    std::vector<char> foldername;
    char buf[2];
    while (std::fgets(buf, sizeof buf, csvFile) != NULL)
    {
        if (buf[0] == '\n') {
            foldername.clear();
        } else if(buf[0] != ';') {
            foldername.push_back(buf[0]);
        } else if(buf[0] == ';') {
            if (folders.size() == heights.size())
                folders.push_back(std::string(foldername.begin(), foldername.end()));
            else if (widths.size() == heights.size())
                widths.push_back(std::string(foldername.begin(), foldername.end()));
            else
                heights.push_back(std::string(foldername.begin(), foldername.end()));
        }
    }

    //Carrega depth data
    for (std::vector<std::string>::iterator i = folders.begin(); i != folders.end(); ++i)
    {
        csvFilePath.clear();
        csvFilePath.append(IMAGES_DIR).append("/").append(*i).append("/").append(KINECT_1_XY_DATA_FILE);

        std::vector<uint16_t> data;
        std::FILE * dataFile = std::fopen(csvFilePath.c_str(),"r");
        if (dataFile!=NULL)
        {

            int i = 0;
            uint16_t info;
            while (std::fread(&info, 1, sizeof(uint16_t), dataFile) != NULL)
            {
                data.push_back(info);
            }
            std::cout << std::endl;
            files.push_back(data);
        }
    }

    //Exibe os arquivos

    glutInit(&argc, argv);

    glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);
    glutInitWindowSize(width, height);
    glutInitWindowPosition(0, 0);

    window = glutCreateWindow("RAP3DF");
    glClearColor(0.4f, 0.4f, 0.4f, 0.0f);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_ALPHA_TEST);
    glAlphaFunc(GL_GREATER, 0.0f);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glMatrixMode(GL_PROJECTION);
    gluPerspective(50.0, 1.0, 900.0, 11000.0);

    glutDisplayFunc([] () {
        std::vector<uint16_t> file = files[filePos];

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glViewport(0, 0, width, height);

        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
        {
            glRotatef(180, 1.0f, 0.0f, 0.0f); // Rotate our camera on the x-axis (looking up and down)
            glTranslatef( 0, -0, -500 );

            glPointSize(1.0f);
            glBegin(GL_POINTS);

//            int w = widths[filePos];
//            int h = heights[filePos];
            int w = 209;
            int h = 279;

            for (int i = 0; i < w*h; ++i)
            {

                if (!file[i]) continue;

                glColor4f(1/(((file[i]/2) & 0xff) / 100.f),
                          1/((file[i] & 0xff) / 100.f),
                          1/(((file[i]/4) & 0xff) / 100.f),
                          1.f);

                glVertex3f( (i%w- (w-1)/2.f) * file[i] / f,  // X = (x - cx) * d / fx
                            (i/w- (h-1)/2.f) * file[i] / f,  // Y = (y - cy) * d / fy
                            file[i] );

            }

            glEnd();
        }

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
                filePos--;
                if (filePos < 0)
                    filePos = 0;

                glutPostRedisplay();
                break;
            case  'e':
            case  'E':
                filePos++;
                if (filePos >= files.size())
                    filePos = files.size() - 1;

                glutPostRedisplay();
                break;
        }
    });

    glutMainLoop();
}
