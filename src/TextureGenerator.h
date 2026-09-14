#ifndef TEXTURE_GENERATOR_H
#define TEXTURE_GENERATOR_H

#include <glad/glad.h>

namespace TextureGenerator
{
    GLuint createSkin2D(int size, int seed);
    GLuint createNoise2D(int size, int seed);
    GLuint createStone2D(int size, int seed);
    GLuint createEnvCube(int size, int seed);
}

#endif // TEXTURE_GENERATOR_H