cmake_minimum_required(VERSION 3.10)

# Set some basic project attributes
project (System_1
	VERSION 0.1
	DESCRIPTION "A Hello World Project")

cmake_policy(SET CMP0072 NEW)  # Предпочитаем GLVND
set(OpenGL_GL_PREFERENCE "GLVND")  # Используем GLVND

set(CMAKE_CXX_FLAGS_DEBUG "-g -O0")


find_package(OpenGL REQUIRED)
#find_package(GLUT REQUIRED)

#set(GLFW_DIR "/usr/local/lib/cmake/glfw3/glfw3Config.cmake")  # Путь может отличаться в зависимости от системы
#find_package(GLFW REQUIRED)
find_package(glfw3 REQUIRED)

#Freetype
#find_package(Freetype REQUIRED)
#find_package(Freetype REQUIRED PATHS /usr/include/freetype2 NO_DEFAULT_PATH)
#include_directories(${FREETYPE_INCLUDE_DIRS})
#include_directories(/usr/include/freetype2)
#find_library(FREETYPE_LIBRARIES freetype REQUIRED)
#include_directories(/usr/include/freetype2)
#set(FREETYPE_INCLUDE_DIRS /usr/include/freetype2)
#include_directories(${FREETYPE_INCLUDE_DIRS})
#set(FREETYPE_LIBRARIES /usr/lib/x86_64-linux-gnu/libfreetype.so)

#set(CMAKE_PREFIX_PATH "/home/user-1/my/packets/freetype/freetype-2.13.3/install;${CMAKE_PREFIX_PATH}")
#find_package(Freetype REQUIRED)
##include_directories(${FREETYPE_INCLUDE_DIRS})
#include_directories(/home/user-1/my/packets/freetype/freetype-2.13.3/install/include/freetype2)

find_package(Freetype REQUIRED)
include_directories(${FREETYPE_INCLUDE_DIRS})

find_package(OpenSSL REQUIRED)

# This project will output an executable file
add_executable(${PROJECT_NAME} System_1.cpp)

# Create a simple configuration header
#configure_file(config.h.in config.h)

# Include the configuration header in the build
target_include_directories(${PROJECT_NAME} PUBLIC "${PROJECT_BINARY_DIR}" ${FREETYPE_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME} OpenGL::GL GLU glfw GLEW ${FREETYPE_LIBRARIES} OpenSSL::SSL OpenSSL::Crypto)
