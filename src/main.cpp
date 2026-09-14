#include "FileWatcher.h"
#include "Quad.h"
#include "ShaderProgram.h"
#include "TextureGenerator.h"
#include "Uniforms.h"
#include "Window.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <iostream>
#include <filesystem>
#include <string>

namespace
{
    // Sampler declarations injected into each shader stage. Buffer 1 samples a
    // 2D skin/noise/stone texture plus a cubemap environment for reflections.
    const char* BUFFER_CHANNEL_DECLS =
        "uniform sampler2D iChannel0;\n"
        "uniform sampler2D iChannel1;\n"
        "uniform sampler2D iChannel2;\n"
        "uniform samplerCube iChannel3;\n";

    const char* IMAGE_CHANNEL_DECLS =
        "uniform sampler2D iChannel0;\n"
        "uniform sampler2D iChannel1;\n"
        "uniform sampler2D iChannel2;\n"
        "uniform samplerCube iChannel3;\n";

    struct FrameBuffer
    {
        GLuint fbo = 0;
        GLuint colorTex = 0;
        int width = 0;
        int height = 0;

        bool create(int w, int h)
        {
            destroy();

            width = w;
            height = h;

            glGenFramebuffers(1, &fbo);
            glGenTextures(1, &colorTex);

            glBindTexture(GL_TEXTURE_2D, colorTex);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, w, h, 0,
                         GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

            glBindFramebuffer(GL_FRAMEBUFFER, fbo);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                                   GL_TEXTURE_2D, colorTex, 0);

            GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
            glBindFramebuffer(GL_FRAMEBUFFER, 0);

            if (status != GL_FRAMEBUFFER_COMPLETE)
            {
                std::cerr << "Framebuffer is incomplete" << std::endl;
                destroy();
                return false;
            }
            return true;
        }

        void destroy()
        {
            if (fbo) glDeleteFramebuffers(1, &fbo);
            if (colorTex) glDeleteTextures(1, &colorTex);
            fbo = 0;
            colorTex = 0;
            width = 0;
            height = 0;
        }

        void bind()
        {
            glBindFramebuffer(GL_FRAMEBUFFER, fbo);
            glViewport(0, 0, width, height);
        }

        static void unbind(int w, int h)
        {
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
            glViewport(0, 0, w, h);
        }
    };

    void bindChannel(int unit, GLenum target, GLuint texture)
    {
        glActiveTexture(GL_TEXTURE0 + unit);
        glBindTexture(target, texture);
    }

    void setShaderToyUniforms(ShaderProgram& program, const Uniforms& uniforms)
    {
        program.setVec2("iResolution", uniforms.iResolution[0], uniforms.iResolution[1]);
        program.setFloat("iTime", uniforms.iTime);
        program.setVec2("iMouse", uniforms.iMouse[0], uniforms.iMouse[1]);
        for (int i = 0; i < 4; ++i)
        {
            program.setInt("iChannel" + std::to_string(i), i);
        }
    }
}

int main()
{
    Window window;
    if (!window.init(800, 600, "Shader Toy"))
    {
        return 1;
    }

    Quad quad;
    quad.init();

    ShaderProgram bufferProgram;
    ShaderProgram imageProgram;
    if (!imageProgram.load("../shaders/default.vert", "../shaders/toy.frag", IMAGE_CHANNEL_DECLS))
    {
        quad.cleanup();
        window.terminate();
        return 1;
    }

    // The buffer shader (shaders/1.frag) is optional. If it is missing it is
    // skipped and toy.frag runs as a single pass directly to the screen.
    const std::string bufferPath = "../shaders/1.frag";
    bool bufferLoaded = false;
    if (std::filesystem::exists(bufferPath))
    {
        bufferLoaded = bufferProgram.load("../shaders/default.vert", bufferPath, BUFFER_CHANNEL_DECLS);
    }
    if (!bufferLoaded)
    {
        std::cout << "No buffer shader (shaders/1.frag); running single-pass." << std::endl;
    }

    FrameBuffer buffer;
    if (bufferLoaded && !buffer.create(window.getWidth(), window.getHeight()))
    {
        imageProgram.cleanup();
        bufferProgram.cleanup();
        quad.cleanup();
        window.terminate();
        return 1;
    }

    FileWatcher bufferWatcher(bufferPath);
    FileWatcher imageWatcher("../shaders/toy.frag");

    GLuint skinTex = TextureGenerator::createSkin2D(128, 7);
    GLuint noiseTex = TextureGenerator::createNoise2D(256, 13);
    GLuint stoneTex = TextureGenerator::createStone2D(256, 29);
    GLuint envCube = TextureGenerator::createEnvCube(256, 41);

    Uniforms uniforms{};
    auto startTime = glfwGetTime();

    while (!window.shouldClose())
    {
        window.pollEvents();

        // Load the buffer shader if it has just appeared on disk.
        if (!bufferLoaded && std::filesystem::exists(bufferPath))
        {
            bufferLoaded = bufferProgram.load("../shaders/default.vert", bufferPath, BUFFER_CHANNEL_DECLS);
            if (bufferLoaded)
            {
                std::cout << "[1.frag] loaded" << std::endl;
                buffer.create(window.getWidth(), window.getHeight());
            }
        }

        if (bufferLoaded && bufferWatcher.checkModified())
        {
            if (bufferProgram.reload())
            {
                std::cout << "[1.frag] reloaded" << std::endl;
            }
            else
            {
                std::cerr << "[1.frag] reload failed; keeping the previous shader" << std::endl;
            }
        }
        if (imageWatcher.checkModified())
        {
            if (imageProgram.reload())
            {
                std::cout << "[toy.frag] reloaded" << std::endl;
            }
            else
            {
                std::cerr << "[toy.frag] reload failed; keeping the previous shader" << std::endl;
            }
        }

        int width = window.getWidth();
        int height = window.getHeight();
        if (bufferLoaded && (buffer.width != width || buffer.height != height))
        {
            buffer.create(width, height);
        }

        uniforms.iTime = static_cast<float>(glfwGetTime() - startTime);
        uniforms.iResolution[0] = static_cast<float>(width);
        uniforms.iResolution[1] = static_cast<float>(height);

        double mouseX = 0.0;
        double mouseY = 0.0;
        window.getMousePosition(mouseX, mouseY);
        uniforms.iMouse[0] = static_cast<float>(mouseX);
        uniforms.iMouse[1] = static_cast<float>(mouseY);

        // ---- Pass 1 (optional): buffer scene -> offscreen texture ----
        if (bufferLoaded)
        {
            buffer.bind();
            glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
            glClear(GL_COLOR_BUFFER_BIT);

            bufferProgram.use();
            bindChannel(0, GL_TEXTURE_2D, skinTex);
            bindChannel(1, GL_TEXTURE_2D, noiseTex);
            bindChannel(2, GL_TEXTURE_2D, stoneTex);
            bindChannel(3, GL_TEXTURE_CUBE_MAP, envCube);
            setShaderToyUniforms(bufferProgram, uniforms);
            quad.draw();
        }

        // ---- Pass 2: image shader -> screen ----
        FrameBuffer::unbind(width, height);
        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        imageProgram.use();
        if (bufferLoaded)
        {
            bindChannel(0, GL_TEXTURE_2D, buffer.colorTex);
        }
        bindChannel(1, GL_TEXTURE_2D, noiseTex);
        bindChannel(2, GL_TEXTURE_2D, stoneTex);
        bindChannel(3, GL_TEXTURE_CUBE_MAP, envCube);
        setShaderToyUniforms(imageProgram, uniforms);
        quad.draw();

        window.swapBuffers();
    }

    glDeleteTextures(1, &envCube);
    glDeleteTextures(1, &stoneTex);
    glDeleteTextures(1, &noiseTex);
    glDeleteTextures(1, &skinTex);

    imageProgram.cleanup();
    bufferProgram.cleanup();
    buffer.destroy();
    quad.cleanup();
    window.terminate();
    return 0;
}