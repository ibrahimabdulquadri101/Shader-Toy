#include "TextureGenerator.h"

#include <algorithm>
#include <cmath>
#include <vector>

namespace
{
    unsigned int hashNoise(int x, int y, int seed)
    {
        unsigned int h = static_cast<unsigned int>(x) * 374761393u
                       + static_cast<unsigned int>(y) * 668265263u
                       + static_cast<unsigned int>(seed) * 1440445633u;
        h = (h ^ (h >> 13)) * 1274126177u;
        h = h ^ (h >> 16);
        return h;
    }

    float rand01(int x, int y, int seed)
    {
        return float(hashNoise(x, y, seed) & 0x00FFFFFFu) / float(0x00FFFFFFu);
    }

    float smoothValue(float x, float y, int period, int seed)
    {
        int xi = int(std::floor(x));
        int yi = int(std::floor(y));
        float fx = x - float(xi);
        float fy = y - float(yi);
        fx = fx * fx * (3.0f - 2.0f * fx);
        fy = fy * fy * (3.0f - 2.0f * fy);

        auto cell = [&](int dx, int dy) -> float
        {
            int px = (xi + dx) % period;
            int py = (yi + dy) % period;
            if (px < 0) px += period;
            if (py < 0) py += period;
            return rand01(px, py, seed);
        };

        float a = cell(0, 0);
        float b = cell(1, 0);
        float c = cell(0, 1);
        float d = cell(1, 1);
        return a + (b - a) * fx + (c - a) * fy + (a - b - c + d) * fx * fy;
    }

    float fbm(float x, float y, int octaves, int seed)
    {
        const int period = 1 << 15;
        float sum = 0.0f;
        float amp = 0.5f;
        float freq = 1.0f;
        float norm = 0.0f;
        for (int i = 0; i < octaves; ++i)
        {
            sum += amp * smoothValue(x * freq, y * freq, period, seed + i * 37);
            norm += amp;
            amp *= 0.5f;
            freq *= 2.0f;
        }
        return norm > 0.0f ? sum / norm : 0.0f;
    }

    GLuint make2D(int width, int height, const std::vector<unsigned char>& rgba)
    {
        GLuint tex = 0;
        glGenTextures(1, &tex);
        glBindTexture(GL_TEXTURE_2D, tex);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0,
                     GL_RGBA, GL_UNSIGNED_BYTE, rgba.data());
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        return tex;
    }
}

namespace TextureGenerator
{
    GLuint createNoise2D(int size, int seed)
    {
        std::vector<unsigned char> data(size * size * 4);
        for (int y = 0; y < size; ++y)
        {
            for (int x = 0; x < size; ++x)
            {
                float v = rand01(x, y, seed);
                unsigned char c = static_cast<unsigned char>(v * 255.0f);
                size_t i = (size_t(y) * size + size_t(x)) * 4;
                data[i + 0] = c;
                data[i + 1] = c;
                data[i + 2] = c;
                data[i + 3] = 255;
            }
        }
        return make2D(size, size, data);
    }

    GLuint createSkin2D(int size, int seed)
    {
        std::vector<unsigned char> data(size * size * 4);
        for (int y = 0; y < size; ++y)
        {
            for (int x = 0; x < size; ++x)
            {
                float n = fbm(float(x) * 0.1f, float(y) * 0.1f, 4, seed);
                float v = n * n * (3.0f - 2.0f * n);
                unsigned char c = static_cast<unsigned char>(
                    std::clamp(v, 0.0f, 1.0f) * 255.0f);
                size_t i = (size_t(y) * size + size_t(x)) * 4;
                data[i + 0] = c;
                data[i + 1] = c;
                data[i + 2] = c;
                data[i + 3] = 255;
            }
        }
        return make2D(size, size, data);
    }

    GLuint createStone2D(int size, int seed)
    {
        std::vector<unsigned char> data(size * size * 4);
        for (int y = 0; y < size; ++y)
        {
            for (int x = 0; x < size; ++x)
            {
                float coarse = fbm(float(x) * 0.12f + 17.0f, float(y) * 0.12f + 3.0f, 6, seed);
                float cracks = fbm(float(x) * 0.5f - 5.0f, float(y) * 0.5f - 9.0f, 3, seed + 997);
                float v = coarse * 0.6f + cracks * 0.4f;
                v = std::pow(std::clamp(v, 0.0f, 1.0f), 1.5f);
                unsigned char c = static_cast<unsigned char>(std::clamp(v, 0.0f, 1.0f) * 255.0f);
                size_t i = (size_t(y) * size + size_t(x)) * 4;
                data[i + 0] = c;
                data[i + 1] = c;
                data[i + 2] = c;
                data[i + 3] = 255;
            }
        }
        return make2D(size, size, data);
    }

    GLuint createEnvCube(int size, int seed)
    {
        GLuint tex = 0;
        glGenTextures(1, &tex);
        glBindTexture(GL_TEXTURE_CUBE_MAP, tex);

        const GLenum faces[6] =
        {
            GL_TEXTURE_CUBE_MAP_POSITIVE_X, GL_TEXTURE_CUBE_MAP_NEGATIVE_X,
            GL_TEXTURE_CUBE_MAP_POSITIVE_Y, GL_TEXTURE_CUBE_MAP_NEGATIVE_Y,
            GL_TEXTURE_CUBE_MAP_POSITIVE_Z, GL_TEXTURE_CUBE_MAP_NEGATIVE_Z
        };

        for (int face = 0; face < 6; ++face)
        {
            std::vector<unsigned char> data(size * size * 4);
            for (int y = 0; y < size; ++y)
            {
                for (int x = 0; x < size; ++x)
                {
                    float u = (float(x) / float(size)) * 2.0f - 1.0f;
                    float v = (float(y) / float(size)) * 2.0f - 1.0f;

                    float up = 0.5f + 0.5f * v;
                    float n1 = fbm(u * 3.0f + float(face) * 37.0f, v * 3.0f, 4, seed + face * 73);
                    float n2 = fbm(u * 8.0f - float(face) * 11.0f, v * 8.0f + 5.0f, 2, seed + 123 + face * 31);
                    float clouds = 0.5f + 0.5f * (n1 * 0.7f + n2 * 0.3f);
                    float light = 0.35f + 0.9f * clouds;

                    float r = (0.10f + 0.25f * up) * light;
                    float g = (0.15f + 0.40f * up) * light;
                    float b = (0.35f + 0.45f * up) * light;

                    auto toByte = [](float c)
                    {
                        return static_cast<unsigned char>(std::clamp(int(c * 255.0f), 0, 255));
                    };
                    size_t i = (size_t(y) * size + size_t(x)) * 4;
                    data[i + 0] = toByte(r);
                    data[i + 1] = toByte(g);
                    data[i + 2] = toByte(b);
                    data[i + 3] = 255;
                }
            }
            glTexImage2D(faces[face], 0, GL_RGBA8, size, size, 0,
                         GL_RGBA, GL_UNSIGNED_BYTE, data.data());
        }

        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
        return tex;
    }
}