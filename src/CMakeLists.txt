cmake_policy(SET CMP0072 NEW)  # Предпочитаем GLVND

set(OpenGL_GL_PREFERENCE "GLVND")
set(CMAKE_CXX_FLAGS_DEBUG "-g -O0")

set(SOURCE_FILES main.cpp)

find_package(OpenGL REQUIRED)
find_package(glfw3 REQUIRED)
find_package(Freetype REQUIRED)
find_package(OpenSSL REQUIRED)

include_directories(${FREETYPE_INCLUDE_DIRS})

# This project will output an executable file
add_executable(${PROJECT_NAME} ${SOURCE_FILES})

# Include the configuration header in the build
target_include_directories(${PROJECT_NAME} PUBLIC "${PROJECT_BINARY_DIR}" ${FREETYPE_INCLUDE_DIRS} ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(${PROJECT_NAME} OpenGL::GL GLU glfw GLEW ${FREETYPE_LIBRARIES} OpenSSL::SSL OpenSSL::Crypto)
