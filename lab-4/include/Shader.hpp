#ifndef SHADER_H
#define SHADER_H

#include <glad/glad.h> 
#include <glm/glm.hpp>

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>

class Shader {
public:
    Shader(const char* vertexPath, const char* fragmentPath);
    ~Shader();

    void checkShaderCompile(GLuint shader);
    void checkProgramLink(GLuint program);

    // Use the shader
    void use();

    // Utility uniform functions
    void setBool(const std::string& name, bool value) const;
    void setInt(const std::string& name, int value) const;
    void setFloat(const std::string& name, float value) const;
    void setMat4(const std::string& name, const glm::mat4& mat) const;
    void setVec3(const std::string& name, const glm::vec3& v) const;

    public:
    unsigned int m_programID; // Shader program ID
};

#endif // SHADER_H