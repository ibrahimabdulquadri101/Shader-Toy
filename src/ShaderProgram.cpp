#include <iostream>
#include "ShaderProgram.h"
#include <fstream>
#include <sstream>
#include <string>
#include <cctype>
#include <vector>
#include <cstdlib>
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

// Strip a handful of declarations that the ShaderToy wrapper injects itself,
// so shaders can be pasted in almost verbatim.
std::string ShaderProgram::sanitizeSource(std::string source)
{
    std::istringstream in(source);
    std::ostringstream out;
    std::string line;
    while (std::getline(in, line))
    {
        std::string trimmed = line;
        size_t first = trimmed.find_first_not_of(" \t");
        if (first != std::string::npos)
        {
            trimmed = trimmed.substr(first);
        }
        if (trimmed.rfind("#version", 0) == 0) continue;
        if (trimmed == "out vec4 fragColor;") continue;
        if (trimmed == "precision highp float;") continue;
        if (trimmed == "precision mediump float;") continue;
        if (trimmed == "precision lowp float;") continue;
        out << line << "\n";
    }
    return out.str();
}

namespace
{
    // true if the source declares a uniform with the given name
    bool declaresUniform(const std::string& source, const std::string& name)
    {
        std::istringstream in(source);
        std::string line;
        while (std::getline(in, line))
        {
            if (line.find("uniform") != std::string::npos &&
                line.find(name) != std::string::npos)
            {
                return true;
            }
        }
        return false;
    }

    // extract a sampler's identifier from a line like
    // "uniform sampler2D iChannel0;"
    std::string samplerName(const std::string& line)
    {
        size_t semi = line.rfind(';');
        if (semi == std::string::npos) return "";
        size_t p = line.find_last_of(" \t", semi);
        size_t start = (p == std::string::npos) ? 0 : p + 1;
        auto isIdentChar = [](char c)
        {
            return std::isalnum(static_cast<unsigned char>(c)) || c == '_';
        };
        size_t end = start;
        while (end < semi && isIdentChar(line[end])) ++end;
        return line.substr(start, end - start);
    }
}

// Wrap a ShaderToy-style shader into a full GLSL fragment shader, injecting
// the common uniforms, iChannel samplers and the entry point. Anything the
// user shader already declares (e.g. "uniform vec2 iResolution;") is left
// untouched so raw shaders with their own main() compile too.
std::string ShaderProgram::wrapShadertoySource(std::string source)
{
    std::string cleaned = sanitizeSource(source);

    std::ostringstream out;
    out << "#version 410 core\n";
    if (!declaresUniform(cleaned, "iResolution"))
    {
        out << "uniform vec2 iResolution;\n";
    }
    if (!declaresUniform(cleaned, "iTime"))
    {
        out << "uniform float iTime;\n";
    }
    if (!declaresUniform(cleaned, "iMouse"))
    {
        out << "uniform vec2 iMouse;\n";
    }
    if (!channelDeclarations.empty())
    {
        std::istringstream chans(channelDeclarations);
        std::string line;
        while (std::getline(chans, line))
        {
            std::string name = samplerName(line);
            if (!declaresUniform(cleaned, name))
            {
                out << line << "\n";
            }
        }
        out << "\n";
    }
    out << "out vec4 fragColor;\n\n";
    out << cleaned << "\n";
    if (cleaned.find("mainImage") != std::string::npos)
    {
        out << "void main()\n"
            << "{\n"
            << "    mainImage(fragColor, gl_FragCoord.xy);\n"
            << "}\n";
    }
    return out.str();
}

bool ShaderProgram::load(std::string vertexPath, std::string fragmentPath, std::string channelDeclarations)
{
    this->vertexPath = vertexPath;
    this->fragmentPath = fragmentPath;
    this->channelDeclarations = channelDeclarations;
    std::string vertex = readFile(vertexPath);
    std::string rawFragment = readFile(fragmentPath);
    
    if (vertex.empty() || rawFragment.empty()) {
        std::cerr << "Shader files are empty or missing.\n";
        return false;
    }

    std::string fragment = wrapShadertoySource(rawFragment);
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
    unsigned int oldProgram = programID;
    unsigned int newProgram = 0;

    std::string vertex = readFile(vertexPath);
    std::string rawFragment = readFile(fragmentPath);
    if (vertex.empty() || rawFragment.empty())
    {
        return false;
    }

    std::string fragment = wrapShadertoySource(rawFragment);
    unsigned int vertexShader = compileShader(vertex, GL_VERTEX_SHADER);
    unsigned int fragmentShader = compileShader(fragment, GL_FRAGMENT_SHADER);
    if (vertexShader == 0 || fragmentShader == 0)
    {
        if (vertexShader != 0) glDeleteShader(vertexShader);
        if (fragmentShader != 0) glDeleteShader(fragmentShader);
        return false;
    }

    newProgram = glCreateProgram();
    glAttachShader(newProgram, vertexShader);
    glAttachShader(newProgram, fragmentShader);
    glLinkProgram(newProgram);

    GLint success = GL_FALSE;
    glGetProgramiv(newProgram, GL_LINK_STATUS, &success);
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    if (success == GL_FALSE)
    {
        glDeleteProgram(newProgram);
        return false;
    }

    programID = newProgram;
    if (oldProgram != 0)
    {
        glDeleteProgram(oldProgram);
    }
    return true;
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