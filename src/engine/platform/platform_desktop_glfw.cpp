#include "platform_api.hpp"
#include "core_data.hpp"

#include <GLFW/glfw3.h>

#include <cstdlib>
#include <cstring>
#include <iostream>
#include "stdio.h"
#include "graphics_api.hpp"

#include "core.hpp"

void _FramebufferSizeCallback(GLFWwindow* window, int width, int height);

using namespace Mln;

namespace Mln
{
    extern CoreData gCore;
}

struct PlatformData
{
    GLFWwindow* window = nullptr;

} gPlatform;


void PlatformInit()
{
    // glfw: initialize and configure
    // ------------------------------
    glfwInit();
    
#if defined (OPENGL_ES)
    glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
#else
    glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_API);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
#endif

    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // glfw window creation
    // --------------------
    gPlatform.window = glfwCreateWindow(gCore.viewport.width, gCore.viewport.height, (gCore.windowTitle != 0) ? gCore.windowTitle : " ", NULL, NULL);
    if (gPlatform.window == NULL)
    {
        PrintLog(LOG_ERROR, "Failed to create GLFW window\n");
        glfwTerminate();
        return;
    }
    glfwMakeContextCurrent(gPlatform.window);
    
    glfwSetFramebufferSizeCallback(gPlatform.window, _FramebufferSizeCallback);

    
    #if !defined(PLATFORM_WEB)
    glfwSwapInterval(1);
    #endif
}

void PlatformShutdown()
{
    glfwTerminate();
}

void PlatformSetWindowTitle(const char* title)
{
    glfwSetWindowTitle(gPlatform.window, title);
}

void PlatformPollInput()
{
    glfwPollEvents();

    for (int key = 0; key < KEY__COUNT; key++) 
    {
        gCore.input.current.keys[key] = glfwGetKey(gPlatform.window, key) == GLFW_PRESS;
    }

    for (int button = 0; button < MOUSE_BUTTON__COUNT; button++) 
    {
        gCore.input.current.mouse_buttons[button] = glfwGetMouseButton(gPlatform.window, button) == GLFW_PRESS;
    }
}

bool PlatformWindowShouldClose()
{
    return glfwWindowShouldClose(gPlatform.window);
}

void PlatformPrint(const char* text)
{
    printf("%s", text);
}

void PlatformBeginFrame()
{

}

void PlatformEndFrame()
{
    glfwSwapBuffers(gPlatform.window);
}

void PlatformInitTimer()
{
    glfwSetTime(0);
}

double PlatformGetTime()
{
    return glfwGetTime();
}


void* PlatformGetProcAddressPtr()
{
    return (void*)glfwGetProcAddress;
}


unsigned char *PlatformLoadFileBinary(const char *fileName, size_t *dataSize)
{
    unsigned char *data = NULL;
    *dataSize = 0;

    if (!fileName)
    {
        PrintLog(LOG_ERROR, "File name not valid\n");
        return nullptr;
    }

    FILE *file = fopen(fileName, "rb");

    if (file != NULL)
    {
        // NOTE: using fseek() and ftell() may not work in some (rare) cases
        fseek(file, 0, SEEK_END);
        size_t size = ftell(file);
        fseek(file, 0, SEEK_SET);

        if (size > 0)
        {
            data = (unsigned char *)malloc(size*sizeof(unsigned char));

            if (data != NULL)
            {
                size_t count = fread(data, sizeof(unsigned char), size, file);

                *dataSize = count;
            }
        }

        fclose(file);
    }

    return data;
}

void PlatformUnloadFileBinary(unsigned char *data)
{
    free(data);
}

bool PlatformSaveFileBinary(const char *fileName, void *data, size_t dataSize)
{
    bool success = false;

    if (fileName != NULL)
    {
        PrintLog(LOG_ERROR, "File name not valid\n");
        return false;
    }

    FILE *file = fopen(fileName, "wb");

    if (file != NULL)
    {
        size_t count = fwrite(data, sizeof(unsigned char), dataSize, file);

        // TODO: Meaningful errors 
        // if (count == 0) 
        //     error = FILE_WRITE_FAILED;
        // else if (count != dataSize) 
        //     error = FILE_WRITE_PARTIAL;
        // else 
        //     error = OK;

        int result = fclose(file);
        success = result == 0;
    }

    return success;
}

char *PlatformLoadFileText(const char *fileName)
{
    char *data = NULL;

    if (!fileName)
    {
        PrintLog(LOG_ERROR, "File name not valid\n");
        return nullptr;
    }

    FILE *file = fopen(fileName, "rt");

    if (file != NULL)
    {
        // NOTE: using fseek() and ftell() may not work in some (rare) cases
        fseek(file, 0, SEEK_END);
        size_t size = ftell(file);
        fseek(file, 0, SEEK_SET);

        if (size > 0)
        {
            data = (char *)malloc(size*sizeof(char));

            if (data != NULL)
            {
                size_t count = fread(data, sizeof(unsigned char), size, file);
            }
        }

        fclose(file);
    }

    return data;
}

void PlatformUnloadFileText(char *text)
{
    free(text);
}

bool PlatformSaveFileText(const char *fileName, char *text)
{
    bool success = false;

    if (fileName != NULL)
    {
        PrintLog(LOG_ERROR, "File name not valid\n");
        return false;
    }

    size_t textSize = strlen(text);

    FILE *file = fopen(fileName, "wt");

    if (file != NULL)
    {
        size_t count = fwrite(text, sizeof(unsigned char), textSize, file);

        // TODO: Meaningful errors 
        // if (count == 0) 
        //     error = FILE_WRITE_FAILED;
        // else if (count != textSize) 
        //     error = FILE_WRITE_PARTIAL;
        // else 
        //     error = OK;

        int result = fclose(file);
        success = result == 0;
    }

    return success;
}


void _FramebufferSizeCallback(GLFWwindow* window, int width, int height)
{
    gCore.viewport.width = width;
    gCore.viewport.height = height;
    gCore.windowResized = true;

    ResizeViewport(width, height);
}