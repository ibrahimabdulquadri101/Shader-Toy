#include <iostream>
#include "ShaderProgram.h"
#include <fstream>
#include <sstream>
#include <string>
#include <glad/glad.h>
std::string ShaderProgram::readFile(std::string filePath)
{
    std::ifstream file(filePath);
    if(!file.is_open())
    {
        std::cerr << "Error opening the file: " << filePath << std::endl;
        return "";
    }
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

bool ShaderProgram::load(std::string vertexPath, std::string fragmentPath)
{
    this->vertexPath = vertexPath;
    this->fragmentPath = fragmentPath;
    std::string vertex = readFile(vertexPath);
    std::string fragment = readFile(fragmentPath);
    
    if (vertex.empty() || fragment.empty()) {
        std::cerr << "Shader files are empty or missing.\n";
        return false;
    }

    auto vertexShader = compileShader(vertex,GL_VERTEX_SHADER);
    auto fragmentShader = compileShader(fragment,GL_FRAGMENT_SHADER);
    auto program = glCreateProgram();
    glAttachShader(program,vertexShader);
    glAttachShader(program,fragmentShader);
    glLinkProgram(program);
    GLint success;
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (success == GL_FALSE) 
    {
        GLint logLength;
        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &logLength);
        if (logLength > 0) 
        {
            char* infoLog = (char*)malloc(logLength * sizeof(char));
            glGetProgramInfoLog(program, logLength, NULL, infoLog);
            fprintf(stderr, "OpenGL Program Link Error:\n%s\n", infoLog);
            free(infoLog);
        } else 
        {
            fprintf(stderr, "OpenGL Program Link Error: Unknown error.\n");
        }
        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);
        glDeleteProgram(program);
        return false;
    } else 
    {
        printf("OpenGL Program linked successfully!\n");
    }

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    this->programID = program;
    return true;
}

unsigned int ShaderProgram::compileShader(std::string source , unsigned int type)
{
    GLuint shaderID = glCreateShader(type);
    const char* sourceString = source.c_str();
    glShaderSource(shaderID,1,&sourceString,nullptr);
    glCompileShader(shaderID);
    std::string typeStr = (type == GL_VERTEX_SHADER) ? "VERTEX" : "FRAGMENT";
    checkCompileErrors(shaderID, typeStr);
    return shaderID;
}

bool ShaderProgram::checkCompileErrors(unsigned int shader, std::string type)
{
    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (success == GL_FALSE)
    {
        GLint logLength;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logLength);
        std::vector<char> infoLog(logLength);
        glGetShaderInfoLog(shader, logLength, NULL, infoLog.data());
        std::cerr << "ERROR::SHADER_COMPILATION_ERROR of type: " << type << "\n" 
                  << infoLog.data() << "\n";
        return false;
    }
    return true;
}

bool ShaderProgram::reload()
{
    // Clean up the old program before loading the new one
    cleanup();
    return load(this->vertexPath, this->fragmentPath);
}

void ShaderProgram::use()
{
    glUseProgram(this->programID);
}

void ShaderProgram::cleanup()
{
    glDeleteProgram(this->programID);
}

// ==========================================
// Uniform Setters
// ==========================================

void ShaderProgram::setFloat(std::string name, float value)
{
    glUniform1f(glGetUniformLocation(programID, name.c_str()), value);
}

void ShaderProgram::setVec2(std::string name, float x, float y)
{
    glUniform2f(glGetUniformLocation(programID, name.c_str()), x, y);
}

void ShaderProgram::setVec3(std::string name, float x, float y, float z)
{
    glUniform3f(glGetUniformLocation(programID, name.c_str()), x, y, z);
}

void ShaderProgram::setInt(std::string name, int value)
{
    glUniform1i(glGetUniformLocation(programID, name.c_str()), value);
}