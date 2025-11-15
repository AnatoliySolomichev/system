#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <vector>
#include <stdio.h> // Добавлено для определения FILE

#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_OUTLINE_H

#include <openssl/ssl.h>
#include <openssl/evp.h>
#include <openssl/rsa.h>
#include <openssl/pem.h>
#include <openssl/err.h>
#include <cstring>

#include "blocks.h"
#include "crypto_utils.h"
#include "blockchain.h"
#include <ctime>
#include <string>
#include <sstream>
#include <iomanip>
#include <fstream>
#include <memory>

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

int ensure_keys() {
    const std::string priv = "private_key.pem";
    const std::string pub = "public_key.pem";
    if (crypto::keys_exist(priv, pub)) {
        std::cout << "Keys already exist\n";
        return 0;
    }
    if (!crypto::generate_rsa_keys(priv, pub, 2048)) {
        std::cerr << "Failed to generate keys\n";
        return -1;
    }
    std::cout << "Generated keys: " << priv << ", " << pub << "\n";
    return 0;
}

int demo_blockchain(const std::string &chainfile = "chain.dat") {
    // create a small chain and write it to disk, then load and verify
    const std::string priv = "private_key.pem";
    const std::string pub = "public_key.pem";
    if (!crypto::keys_exist(priv, pub)) {
        std::cerr << "Keys missing, generate them first\n";
        return -1;
    }
    EVP_PKEY *privk = crypto::load_private_key(priv);
    if (!privk) { std::cerr << "failed to load private key\n"; return -1; }

    std::vector<std::string> payloads = {"Demo A","Demo B","Demo C"};
    std::vector<sign_block_t> created;
    // remove existing chain file
    std::remove(chainfile.c_str());
    for (size_t i = 0; i < payloads.size(); ++i) {
        sign_block_t sb{};
        sb.block_data.number = (int)i;
        sb.block_data.time = (int)time(nullptr);
        if (i == 0) memset(sb.block_data.previous_hash, 0, HASH_LEN);
        else {
            std::string prev_hex = bc::compute_block_hash_hex(created.back().block_data, created.back().sign);
            memcpy(sb.block_data.previous_hash, prev_hex.c_str(), HASH_LEN);
        }
        sb.block_data.data_len = (int)payloads[i].size();
        sb.block_data.data = (char*)malloc(sb.block_data.data_len);
        memcpy(sb.block_data.data, payloads[i].data(), sb.block_data.data_len);

        std::vector<unsigned char> ser;
        bc::serialize_blockdata_be(sb.block_data, ser);
        std::vector<unsigned char> sig;
        if (!crypto::sign_data(privk, ser, sig)) { std::cerr << "sign failed\n"; return -1; }
        sb.sign.sign_len = sig.size();
        sb.sign.sign = (char*)malloc(sb.sign.sign_len);
        memcpy(sb.sign.sign, sig.data(), sb.sign.sign_len);

        if (!bc::append_block_file(sb, chainfile)) { std::cerr << "append failed\n"; return -1; }
        created.push_back(sb);
    }

    EVP_PKEY_free(privk);

    // load and verify
    std::vector<sign_block_t> loaded;
    if (!bc::load_chain_file(chainfile, loaded)) { std::cerr << "load_chain_file failed\n"; return -1; }
    EVP_PKEY *pubk = crypto::load_public_key(pub);
    if (!pubk) { std::cerr << "load public failed\n"; return -1; }
    bool ok = bc::verify_chain(loaded, pubk);
    std::cout << (ok ? "Chain OK" : "Chain FAILED") << "\n";

    bc::free_chain(loaded);
    for (auto &s: created) { if (s.block_data.data) free(s.block_data.data); if (s.sign.sign) free(s.sign.sign); }
    EVP_PKEY_free(pubk);
    return ok ? 0 : -1;
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

    ensure_keys();
    demo_blockchain();
    
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
