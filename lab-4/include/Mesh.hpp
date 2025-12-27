#ifndef MESH_HPP
#define MESH_HPP

#include <glad/glad.h>

#include "stb_image.h"

#include <vector>

class Mesh {
public:
    void setup();
    void draw(GLenum mode = GL_TRIANGLES);
    void destroy();

public:
    unsigned int VAO = 0, VBO = 0;
    std::vector<float> vertices;
    size_t vertexCount;
};

#endif // MESH_HPP