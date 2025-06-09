#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <vector>

#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_OUTLINE_H

#include <openssl/ssl.h>
#include <openssl/evp.h>
#include <openssl/rsa.h>
#include <openssl/pem.h>
#include <openssl/err.h>
#include <cstring>

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

FT_Face face;
FT_Library ft;

void handleErrors() {
    ERR_print_errors_fp(stderr);
    abort();
}

int test_openssl() {
    OpenSSL_add_all_algorithms();
    ERR_load_crypto_strings();

    // 1. Генерация ключей RSA
    EVP_PKEY* keypair = EVP_PKEY_new();
    RSA* rsa = RSA_generate_key(2048, RSA_F4, nullptr, nullptr);
    if (!rsa) handleErrors();
    EVP_PKEY_assign_RSA(keypair, rsa);

    // 2. Сообщение для подписи
    const std::string message = "Hello, OpenSSL!";
    const unsigned char* data = reinterpret_cast<const unsigned char*>(message.c_str());
    size_t data_len = message.size();

    // 3. Подпись
    EVP_MD_CTX* mdctx = EVP_MD_CTX_new();
    if (!mdctx) handleErrors();

    if (1 != EVP_DigestSignInit(mdctx, nullptr, EVP_sha256(), nullptr, keypair))
        handleErrors();

    if (1 != EVP_DigestSignUpdate(mdctx, data, data_len))
        handleErrors();

    size_t sig_len = 0;
    EVP_DigestSignFinal(mdctx, nullptr, &sig_len);  // Получить размер подписи

    std::vector<unsigned char> signature(sig_len);
    if (1 != EVP_DigestSignFinal(mdctx, signature.data(), &sig_len))
        handleErrors();

    signature.resize(sig_len);
    EVP_MD_CTX_free(mdctx);

    std::cout << "✔ Подпись создана (" << sig_len << " байт)\n";

    // 4. Проверка подписи
    EVP_MD_CTX* verify_ctx = EVP_MD_CTX_new();
    if (!verify_ctx) handleErrors();

    if (1 != EVP_DigestVerifyInit(verify_ctx, nullptr, EVP_sha256(), nullptr, keypair))
        handleErrors();

    if (1 != EVP_DigestVerifyUpdate(verify_ctx, data, data_len))
        handleErrors();

    int result = EVP_DigestVerifyFinal(verify_ctx, signature.data(), sig_len);

    if (result == 1)
        std::cout << "✅ Подпись подтверждена\n";
    else if (result == 0)
        std::cout << "❌ Подпись недействительна\n";
    else
        handleErrors();

    EVP_MD_CTX_free(verify_ctx);
    EVP_PKEY_free(keypair);
    EVP_cleanup();
    ERR_free_strings();

    return 0;
}

void drawOutlinedText(float x, float y, float z, const char* text) {
	std::cout << "drawOutlinedText();\n";
    glPushMatrix();
    glTranslatef(x, y, z);
    
    glColor3f(1.0, 1.0, 1.0);  // Белый цвет обводки
    
    while (*text) {
//        if (FT_Load_Char(face, *text, FT_LOAD_NO_BITMAP | FT_LOAD_OUTLINE)) {
	    if (FT_Load_Char(face, *text, FT_LOAD_NO_BITMAP)) {
            text++;
            continue;
        }

        FT_Outline* outline = &face->glyph->outline;
        glBegin(GL_LINE_LOOP);
        for (int i = 0; i < outline->n_points; i++) {
            glVertex3f(outline->points[i].x / 64.0f / 100.0f, 
                       outline->points[i].y / 64.0f / 100.0f, 0);
        }
        glEnd();

        glTranslatef(face->glyph->advance.x / 64.0f / 100.0f, 0, 0);
        text++;
    }

    glPopMatrix();
}

// Имитация "текста" — рисуем квадраты
void drawTextPlaceholder(float x, float y, float z) {
	std::cout << "drawTextPlaceholder();\n";
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
	std::cout << "display();\n";
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
//    for (const auto& p : points) {
//        drawTextPlaceholder(p.x, p.y, p.z);
//    }
	for (int i = 0; i < points.size(); i++) {
		drawOutlinedText(points[i].x, points[i].y, points[i].z,
						 ("P" + std::to_string(i)).c_str());
	}

	glFlush();  // ВАЖНО!
}

// Обработчик клика мыши
void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
	std::cout << "mouseButtonCallback();\n";
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
	std::cout << "cursorPosCallback();\n";
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
	std::cout << "scrollCallback();\n";
    zoom += yoffset * 0.1f;
}

int main() {

    test_openssl();
    
	std::cout << "main();\n";
    if (!glfwInit()) {
        std::cerr << "Ошибка инициализации GLFW\n";
        return -1;
    }

	if (FT_Init_FreeType(&ft)) {
		std::cerr << "Ошибка инициализации FreeType\n";
		return -1;
	}

	if (FT_New_Face(ft, "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf", 0,
					&face)) {
		std::cerr << "Ошибка загрузки шрифта!\n";
		return -1;
	}

	FT_Set_Pixel_Sizes(face, 0, 48); // Размер шрифта

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
        glfwWaitEvents();
    }

    glfwTerminate();
    return 0;
}
