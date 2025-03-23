#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <vector>

// Координаты точек (имитация текста)
struct Point {
    float x, y, z;
};
std::vector<Point> points = {
    { -0.5f,  0.5f,  0.0f },
    {  0.5f,  0.5f,  0.0f },
    {  0.0f, -0.5f,  0.0f }
};

int selected = -1;  // Выбранная точка
float angleX = 0.0f, angleY = 0.0f;  // Углы поворота
float zoom = -2.0f;  // Зум камеры
bool rotating = false;
double lastX, lastY;

// Имитация "текста" — рисуем квадраты
void drawTextPlaceholder(float x, float y, float z) {
    float size = 0.05f;  // Размер квадрата
    glBegin(GL_QUADS);
    glVertex3f(x - size, y - size, z);
    glVertex3f(x + size, y - size, z);
    glVertex3f(x + size, y + size, z);
    glVertex3f(x - size, y + size, z);
    glEnd();
}

// Рендеринг сцены
void display() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();

    glTranslatef(0.0f, 0.0f, zoom);
    glRotatef(angleX, 1, 0, 0);
    glRotatef(angleY, 0, 1, 0);

    // Линии между точками
    glColor3f(1.0, 1.0, 1.0);
    glBegin(GL_LINES);
    for (int i = 0; i < points.size(); i++) {
        for (int j = i + 1; j < points.size(); j++) {
            glVertex3f(points[i].x, points[i].y, points[i].z);
            glVertex3f(points[j].x, points[j].y, points[j].z);
        }
    }
    glEnd();

    // "Текст" (имитация точками)
    glColor3f(1.0, 0.0, 0.0);
    for (const auto& p : points) {
        drawTextPlaceholder(p.x, p.y, p.z);
    }

    glFlush();  // ВАЖНО!
}

// Обработчик клика мыши
void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
        selected = 0; // Пример: всегда выбираем 1 точку
    } else if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE) {
        selected = -1;
    }

    if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS) {
        rotating = true;
        glfwGetCursorPos(window, &lastX, &lastY);
    } else if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_RELEASE) {
        rotating = false;
    }
}

// Обработчик движения мыши
void cursorPosCallback(GLFWwindow* window, double xpos, double ypos) {
    if (selected != -1) {
        points[selected].x = (xpos / 400.0f) - 1.0f;
        points[selected].y = 1.0f - (ypos / 300.0f);
    }

    if (rotating) {
        double dx = xpos - lastX;
        double dy = ypos - lastY;
        angleY += dx * 0.5f;
        angleX -= dy * 0.5f;
        lastX = xpos;
        lastY = ypos;
    }
}

// Обработчик зума колесиком мыши
void scrollCallback(GLFWwindow* window, double xoffset, double yoffset) {
    zoom += yoffset * 0.1f;
}

int main() {
    if (!glfwInit()) {
        std::cerr << "Ошибка инициализации GLFW\n";
        return -1;
    }

    GLFWwindow* window = glfwCreateWindow(800, 600, "3D Points with Rotation & Zoom", NULL, NULL);
    if (!window) {
        std::cerr << "Ошибка создания окна\n";
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glewExperimental = true;
    if (glewInit() != GLEW_OK) {
        std::cerr << "Ошибка инициализации GLEW\n";
        return -1;
    }

    glEnable(GL_DEPTH_TEST);
    glfwSetMouseButtonCallback(window, mouseButtonCallback);
    glfwSetCursorPosCallback(window, cursorPosCallback);
    glfwSetScrollCallback(window, scrollCallback);

    while (!glfwWindowShouldClose(window)) {
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		gluPerspective(45.0, 800.0 / 600.0, 0.1, 10.0); // Настройка перспективы
		glMatrixMode(GL_MODELVIEW);

		display();
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}