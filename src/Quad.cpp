#include "Quad.h"
#include <glad/glad.h>

void Quad::init()
{
    float vertices[] =
    {
        -1.0f, -1.0f,  // bottom-left
        1.0f, -1.0f,   // bottom-right
        1.0f,  1.0f,  // top-right
        -1.0f, -1.0f,  // bottom-left
        1.0f,  1.0f,  // top-right
        -1.0f,  1.0f  // top-left
    };
    glGenBuffers(1,&vbo);
    glGenVertexArrays(1,&vao);
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER,vbo);
    glBufferData(GL_ARRAY_BUFFER,sizeof(vertices), vertices , GL_STATIC_DRAW);
    glVertexAttribPointer(0,2,GL_FLOAT,GL_FALSE,2*sizeof(float),(void*)0);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER,0);
    glBindVertexArray(0);
}

void Quad::draw()
{
    glBindVertexArray(vao);
    glDrawArrays(GL_TRIANGLES,0,6);
    glBindVertexArray(0);
}

void Quad::cleanup()
{
    glDeleteVertexArrays(1, &vao);
    glDeleteBuffers(1, &vbo);
}