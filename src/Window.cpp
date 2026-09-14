#include "Window.h"

#include <iostream>
#include <string>

bool Window::init(int width, int height, const std::string& title)
{
    this->width = width;
    this->height = height;
    this->title = title;

    if (!glfwInit())
    {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return false;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

    handle = glfwCreateWindow(width, height, this->title.c_str(), nullptr, nullptr);
    if (!handle)
    {
        std::cerr << "Error occurred opening your window" << std::endl;
        glfwTerminate();
        return false;
    }

    glfwMakeContextCurrent(handle);
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cerr << "Failed to initialize GLAD" << std::endl;
        glfwDestroyWindow(handle);
        glfwTerminate();
        return false;
    }

    glfwGetFramebufferSize(handle, &this->width, &this->height);
    glViewport(0, 0, this->width, this->height);
    return true;
}

bool Window::shouldClose()
{
    return handle && glfwWindowShouldClose(handle);
}

void Window::swapBuffers()
{
    if (handle)
    {
        glfwSwapBuffers(handle);
    }
}

void Window::pollEvents()
{
    glfwPollEvents();

    if (handle)
    {
        glfwGetFramebufferSize(handle, &width, &height);
        glViewport(0, 0, width, height);
    }
}

void Window::getMousePosition(double& x, double& y)
{
    if (!handle)
    {
        x = 0.0;
        y = 0.0;
        return;
    }

    double xpos = 0.0;
    double ypos = 0.0;
    glfwGetCursorPos(handle, &xpos, &ypos);
    x = xpos;
    y = ypos;
}

bool Window::isKeyPressed(int key)
{
    return handle && glfwGetKey(handle, key) == GLFW_PRESS;
}

int Window::getWidth()
{
    return width;
}

int Window::getHeight()
{
    return height;
}

void Window::terminate()
{
    if (handle)
    {
        glfwDestroyWindow(handle);
        handle = nullptr;
    }
    glfwTerminate();
}

bool Window::isMouseButtonPressed(int button)
{
    return handle && glfwGetMouseButton(handle, button) == GLFW_PRESS;
}
