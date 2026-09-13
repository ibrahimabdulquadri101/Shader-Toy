#ifndef SHADER_PROGRAM_H
#define SHADER_PROGRAM_H
#include <string>
class ShaderProgram
{
    private:
        unsigned int programID;
        std::string vertexPath;
        std::string fragmentPath;
        //helper functions
        unsigned int compileShader(std::string source, unsigned int type);
        std::string readFile(std::string filePath);
        bool checkCompileErrors(unsigned int shader, std::string type);

    public:
        bool load(std::string vertexPath, std::string fragmentPath);
        bool reload();
        void use();
        void cleanup();
        //uniform-setters
        void setFloat(std::string name, float value);
        void setVec2(std::string name, float x , float y);
        void setVec3(std::string name, float x, float y, float z);
        void setInt(std::string name, int value);
};

#endif // SHADER_PROGRAM_H
