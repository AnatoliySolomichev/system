//#include <iostream>
//#include "config.h"
//
//int main(int argc, char **argv) {
//	std::cout << "Hello World" << std::endl;
//	std::cout << "Version " << System_1_VERSION_MAJOR << "." << System_1_VERSION_MINOR << std::endl;
//	return 0; 
//}

#include <GL/glut.h>
#include <GLFW/glfw3.h>
#include <vector>

// Координаты точек с текстами
struct Point {
    float x, y, z;
};
std::vector<Point> points = {
    { -0.5f,  0.5f,  0.0f },
    {  0.5f,  0.5f,  0.0f },
    {  0.0f, -0.5f,  0.0f }
};

int selected = -1; // Выбранная точка

void drawText(const char *text, float x, float y, float z) {
    glRasterPos3f(x, y, z);
    while (*text) {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *text++);
    }
}

void display() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();
    
    glTranslatef(0, 0, -3); // Отдаляем камеру

    // Рисуем линии между точками
    glColor3f(1.0, 1.0, 1.0);
    glBegin(GL_LINES);
    for (int i = 0; i < points.size(); i++) {
        for (int j = i + 1; j < points.size(); j++) {
            glVertex3f(points[i].x, points[i].y, points[i].z);
            glVertex3f(points[j].x, points[j].y, points[j].z);
        }
    }
    glEnd();

    // Рисуем текст
    glColor3f(1.0, 0.0, 0.0);
    drawText("Point 1", points[0].x, points[0].y, points[0].z);
    drawText("Point 2", points[1].x, points[1].y, points[1].z);
    drawText("Point 3", points[2].x, points[2].y, points[2].z);

    glutSwapBuffers();
}

// Обработчик мыши
void mouse(int button, int state, int x, int y) {
    if (button == GLUT_LEFT_BUTTON) {
        if (state == GLUT_DOWN) {
            float xf = (x / 400.0f) - 1.0f;
            float yf = 1.0f - (y / 300.0f);

            for (int i = 0; i < points.size(); i++) {
                if ((points[i].x - xf) * (points[i].x - xf) +
                    (points[i].y - yf) * (points[i].y - yf) < 0.02f) {
                    selected = i;
                    break;
                }
            }
        } else if (state == GLUT_UP) {
            selected = -1;
        }
    }
}

// Перемещение точки
void motion(int x, int y) {
    if (selected != -1) {
        points[selected].x = (x / 400.0f) - 1.0f;
        points[selected].y = 1.0f - (y / 300.0f);
        glutPostRedisplay();
    }
}

void reshape(int w, int h) {
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45, (float)w / h, 1, 10);
    glMatrixMode(GL_MODELVIEW);
}

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(800, 600);
    glutCreateWindow("3D Text with OpenGL");

    glEnable(GL_DEPTH_TEST);

    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutMouseFunc(mouse);
    glutMotionFunc(motion);

    glutMainLoop();
    return 0;
}