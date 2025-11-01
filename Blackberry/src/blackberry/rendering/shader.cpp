#include "blackberry/rendering/shader.hpp"
#include "blackberry/core/log.hpp"

#include "glad/glad.h"

BlShader::BlShader() {}

void BlShader::Create(const std::string& vert, const std::string& frag) {
    int errorCode = 0;
    char buf[512]{};

    const GLchar* const vertSource = vert.c_str();
    const GLchar* const fragSource = frag.c_str();

    u32 vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertSource, nullptr);
    glCompileShader(vertexShader);

    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &errorCode);

    if (!errorCode) {
        glGetShaderInfoLog(vertexShader, 512, nullptr, buf);
        BL_CORE_ERROR("Failed to compile vertex shader! Error: {}", buf);
    }

    u32 fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragSource, nullptr);
    glCompileShader(fragmentShader);

    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &errorCode);

    if (!errorCode) {
        glGetShaderInfoLog(fragmentShader, 512, nullptr, buf);
        BL_CORE_ERROR("Failed to compile fragment shader! Error: {}", buf);
    }

    ID = glCreateProgram();
    glAttachShader(ID, vertexShader);
    glAttachShader(ID, fragmentShader);
    glLinkProgram(ID);

    glGetProgramiv(ID, GL_LINK_STATUS, &errorCode);

    if (!errorCode) {
        glGetProgramInfoLog(ID, 512, nullptr, buf);
        BL_CORE_ERROR("Failed to link shader program! Error: {}", buf);
    }
    glDeleteShader(fragmentShader);
    glDeleteShader(vertexShader);
}

void BlShader::Delete() {
    glDeleteProgram(ID);
}

void BlShader::SetFloat(const std::string& uniform, f32 val) {
    glUniform1f(glGetUniformLocation(ID, uniform.c_str()), val);
}

void BlShader::SetIntArray(const std::string& uniform, u32 count, int* array) {
    glUniform1iv(glGetUniformLocation(ID, uniform.c_str()), count, array);
}

void BlShader::SetVec2(const std::string& uniform, BlVec2 val) {
    glUniform2f(glGetUniformLocation(ID, uniform.c_str()), val.x, val.y);
}

void BlShader::SetVec3(const std::string& uniform, BlVec3 val) {
    glUniform3f(glGetUniformLocation(ID, uniform.c_str()), val.x, val.y, val.z);
}

void BlShader::SetMatrix(const std::string& uniform, f32* mat) {
    glUniformMatrix4fv(glGetUniformLocation(ID, uniform.c_str()), 1, GL_FALSE, mat);
}