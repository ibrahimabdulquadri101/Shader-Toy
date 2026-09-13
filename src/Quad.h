#ifndef QUAD_H
#define QUAD_H
#include <glad/glad.h>
class Quad
{
    private:
        GLuint vbo;
        GLuint vao;
    public:
        void init();
        void draw();
        void cleanup();
};
#endif // QUAD_H
