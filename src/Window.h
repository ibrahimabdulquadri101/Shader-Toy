#ifndef WINDOW_H
#define WINDOW_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <string>

class Window
{
private:
    GLFWwindow* handle;
    int width;
    int height;
    std::string title;

public:
    bool init(int width, int height, const std::string& title);
    bool shouldClose();
    void swapBuffers();
    void pollEvents();
    void getMousePosition(double& x, double& y);
    bool isKeyPressed(int key);
    bool isMouseButtonPressed(int button);
    int getWidth();
    int getHeight();
    void terminate();
};

#endif // WINDOW_H
