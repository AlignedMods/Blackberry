#include "blackberry/renderer/shader.hpp"
#include "blackberry/core/log.hpp"
#include "blackberry/core/util.hpp"

#include "glad/gl.h"

namespace Blackberry {

    Shader Shader::Create(const std::string& vert, const std::string& frag) {
        Shader shader;

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
    
        shader.ID = glCreateProgram();
        glAttachShader(shader.ID, vertexShader);
        glAttachShader(shader.ID, fragmentShader);
        glLinkProgram(shader.ID);
    
        glGetProgramiv(shader.ID, GL_LINK_STATUS, &errorCode);
    
        if (!errorCode) {
            glGetProgramInfoLog(shader.ID, 512, nullptr, buf);
            BL_CORE_ERROR("Failed to link shader program! Error: {}", buf);
        }
        glDeleteShader(fragmentShader);
        glDeleteShader(vertexShader);

        return shader;
    }

    Shader Shader::Create(const FS::Path& vert, const FS::Path& frag) {
        std::string vertSrc = Util::ReadEntireFile(vert);
        std::string fragSrc = Util::ReadEntireFile(frag);

        return Create(vertSrc, fragSrc);
    }
    
    void Shader::Delete() {
        glDeleteProgram(ID);
    }
    
    void Shader::SetFloat(const std::string& uniform, f32 val) {
        glUniform1f(glGetUniformLocation(ID, uniform.c_str()), val);
    }

    void Shader::SetInt(const std::string& uniform, int val) {
        glUniform1i(glGetUniformLocation(ID, uniform.c_str()), val);
    }

    void Shader::SetUInt(const std::string& uniform, u32 val) {
        glUniform1ui(glGetUniformLocation(ID, uniform.c_str()), val);
    }

    void Shader::SetUInt64(const std::string& uniform, u64 val) {
        glUniform1ui64ARB(glGetUniformLocation(ID, uniform.c_str()), val);
    }
    
    void Shader::SetIntArray(const std::string& uniform, u32 count, int* array) {
        glUniform1iv(glGetUniformLocation(ID, uniform.c_str()), count, array);
    }
    
    void Shader::SetVec2(const std::string& uniform, BlVec2<f32> val) {
        glUniform2f(glGetUniformLocation(ID, uniform.c_str()), val.x, val.y);
    }
    
    void Shader::SetVec3(const std::string& uniform, BlVec3<f32> val) {
        glUniform3f(glGetUniformLocation(ID, uniform.c_str()), val.x, val.y, val.z);
    }

    void Shader::SetVec4(const std::string& uniform, BlVec4<f32> val) {
        glUniform4f(glGetUniformLocation(ID, uniform.c_str()), val.x, val.y, val.z, val.w);
    }
    
    void Shader::SetMatrix(const std::string& uniform, f32* mat) {
        glUniformMatrix4fv(glGetUniformLocation(ID, uniform.c_str()), 1, GL_FALSE, mat);
    }

} // namespace Blackberry