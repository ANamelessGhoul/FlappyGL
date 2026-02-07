#include "core.hpp"
#include "keys.h"

#include <iostream>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#include "graphics_api.hpp"

#include <cassert>

#include "audio.hpp"

namespace Mln{
    constexpr int deltaCacheSize = 16;
    static_assert((deltaCacheSize & (deltaCacheSize - 1)) == 0, "deltaCacheSize must be a power of 2");

    struct {
        GLFWwindow* window = nullptr;
        bool shouldClose = false;
        bool windowResized = false;
        Vector2 viewportSize = {0, 0};

        double time;
        double delta;
        double deltaCache[deltaCacheSize];
        int deltaCacheIndex = 0;
        int deltaCacheFrameCounter = 0;

        struct{
            InputState current;
            InputState previous;
        } input;



    } gCore;



    void framebuffer_size_callback(GLFWwindow* window, int width, int height);
    void processInput(GLFWwindow *window);


    Error InitWindow(int width, int height, const char *name)
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
        gCore.window = glfwCreateWindow(width, height, name, NULL, NULL);
        if (gCore.window == NULL)
        {
            std::cout << "Failed to create GLFW window" << std::endl;
            glfwTerminate();
            return ERR_GENERIC;
        }
        glfwMakeContextCurrent(gCore.window);
        glfwSetFramebufferSizeCallback(gCore.window, framebuffer_size_callback);

        InitGraphics(width, height);

        #if !defined(PLATFORM_WEB)
        glfwSwapInterval(1);
        #endif

        gCore.viewportSize = {(float)width, (float)height};

        InitAudio();

        glfwSetTime(0);
        gCore.time = 0;
        gCore.deltaCacheIndex = 0;
        gCore.deltaCacheFrameCounter = 0;

        return OK;
    }

    void UnloadWindow()
    {
        ShutdownGraphics();

        glfwTerminate();
    }

    bool WindowShouldClose()
    {
        return glfwWindowShouldClose(gCore.window) || gCore.shouldClose;
    }

    bool DidWindowResize()
    {
        return gCore.windowResized;
    }

    Vector2 GetViewportSize()
    {
        return gCore.viewportSize;
    }

    double GetFrameTime()
    {
        return gCore.delta;
    }

    double GetFPS()
    {
        double sum = 0;
        for (size_t i = 0; i < deltaCacheSize; i++)
        {
            sum += gCore.deltaCache[i];
        }
        return deltaCacheSize / sum;
    }

    void BeginFrame()
    {
        glfwPollEvents();
        gCore.input.previous = gCore.input.current;

        for (int key = 0; key < KEY__COUNT; key++) 
        {
            gCore.input.current.keys[key] = glfwGetKey(gCore.window, key) == GLFW_PRESS;
        }

        for (int button = 0; button < MOUSE_BUTTON__COUNT; button++) 
        {
            gCore.input.current.mouse_buttons[button] = glfwGetMouseButton(gCore.window, button) == GLFW_PRESS;
        }

        BeginDrawing();
    }

    void EndFrame()
    {
        EndDrawing();

        glfwSwapBuffers(gCore.window);
        gCore.windowResized = false;

        double newTime = glfwGetTime();
        gCore.delta = newTime - gCore.time;
        gCore.time = newTime;

        gCore.deltaCacheFrameCounter++;
        if (gCore.deltaCacheFrameCounter > 10)
        {
            gCore.deltaCacheFrameCounter -= 10;
            gCore.deltaCache[gCore.deltaCacheIndex] = gCore.delta;
            gCore.deltaCacheIndex = (gCore.deltaCacheIndex + 1) & (deltaCacheSize - 1);
        }
    }


    Image LoadImage(const char *path)
    {
        Image image{0};

#if defined(_DEBUG)
        std::cout << "INFO: loading image: " << path << std::endl;
#endif

        image.data = stbi_load(path, &image.width, &image.height, &image.components, 4);
        if (!image.data)
        {
            std::cout << "ERROR: Failed to load image: " << path << std::endl;
        }

        return image;
    }

    Image CreateImage(int width, int height, int components)
    {
        Image image;
        image.data = (unsigned char*)STBI_MALLOC(width * height * sizeof(unsigned char) * components);
        image.components = components;
        image.width = width;
        image.height = height;

        memset(image.data, 0, width * height * sizeof(unsigned char) * components);

        return image;
    }

    void UnloadImage(Image image)
    {
        stbi_image_free(image.data);
    }

    
    void ImageDrawImage(Image dst, RectI dst_rect, Image src, RectI src_rect)
    {
        assert(src.components == dst.components && "Cannot draw if components aren't the same");

        if (src_rect.width == dst_rect.width && src_rect.height == dst_rect.height)
        {
            // Same type we can memcpy
            for (int row = 0; row < dst_rect.height; row++)
            {
                int components = src.components;
                memcpy(
                    dst.data + ((row + dst_rect.y) * dst.width + dst_rect.x) * components,
                    src.data + ((row + src_rect.y) * src.width + src_rect.x) * components,
                    src_rect.width * components
                );
            }
        }
    }

    void WriteImage(Image image, const char* path)
    {
        stbi_write_png(path, image.width, image.height, image.components, image.data, 0);
    }

    bool IsKeyDown(Key key)
    {
        return gCore.input.current.keys[key];
    }

    bool IsKeyUp(Key key)
    {
        return !gCore.input.current.keys[key];
    }

    bool IsKeyJustPressed(Key key)
    {
        return gCore.input.current.keys[key] && !gCore.input.previous.keys[key];
    }

    bool IsMouseButtonDown(MouseButton button)
    {
        return gCore.input.current.mouse_buttons[button];
    }

    bool IsMouseButtonUp(MouseButton button)
    {
        return !gCore.input.current.mouse_buttons[button];
    }

    bool IsMouseButtonJustPressed(MouseButton button)
    {
        return gCore.input.current.mouse_buttons[button] && !gCore.input.previous.mouse_buttons[button];
    }

    Vector2 TransformVector(Transform transform, Vector2 vector)
    {
        return (transform * HMM_Vec4{vector.X, vector.Y, 0, 1}).XY;
    }

    Transform GetMatrix(Transform2D transform)
    {
        return HMM_Translate({transform.position.X, transform.position.Y, 0.f}) * HMM_Rotate_LH(transform.rotation, {0.f, 0.f, 1.f}) * HMM_Scale({transform.scale.X, transform.scale.Y, 1.f});
    }

    void* GetProcAddressPtr()
    {
        return (void*)glfwGetProcAddress;
    }

    // glfw: whenever the window size changed (by OS or user resize) this callback function executes
    // ---------------------------------------------------------------------------------------------
    void framebuffer_size_callback(GLFWwindow* window, int width, int height)
    {
        // make sure the viewport matches the new window dimensions; note that width and 
        // height will be significantly larger than specified on retina displays.
        glViewport(0, 0, width, height);
        gCore.windowResized = true;
        gCore.viewportSize = {(float)width, (float)height};
    }


}