#include "core.hpp"
#include "HandmadeMath.h"
#include "keys.h"
#include "core_data.hpp"
#include "melon_types.hpp"
#include "platform_api.hpp"

#include <cstring>

#define STBI_NO_STDIO
#define STBI_ASSERT ASSERT_C
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define STBIW_ASSERT ASSERT_C
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#define STB_SPRINTF_IMPLEMENTATION
#include "stb_sprintf.h"

#define STBRP_ASSERT ASSERT_C
#define STB_RECT_PACK_IMPLEMENTATION
#include "stb_rect_pack.h"

#define STBTT_assert ASSERT_C
#define STB_TRUETYPE_IMPLEMENTATION
#include "stb_truetype.h"



#include "graphics_api.hpp"

#include "audio.hpp"

namespace Mln{
    CoreData gCore;

    Error InitWindow(int width, int height, const char *title)
    {
        gCore.viewport.width = width;
        gCore.viewport.height = height;
        gCore.windowTitle = title;

        PlatformInit();
        PlatformInitTimer();

        InitGraphics(width, height);

        InitAudio();

        gCore.time = 0;
        gCore.deltaCacheIndex = 0;
        gCore.deltaCacheFrameCounter = 0;

        return OK;
    }

    void UnloadWindow()
    {
        ShutdownGraphics();
        PlatformShutdown();
    }

    void SetWindowTitle(const char* title)
    {
        gCore.windowTitle = title;
        PlatformSetWindowTitle(title);
    }

    bool WindowShouldClose()
    {
        return PlatformWindowShouldClose() || gCore.shouldClose;
    }

    bool DidWindowResize()
    {
        return gCore.windowResized;
    }

    Vector2 GetViewportSize()
    {
        return {(float)gCore.viewport.width, (float)gCore.viewport.height};
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
        gCore.input.previous = gCore.input.current;
        PlatformBeginFrame();
        PlatformPollInput();
        
        BeginDrawing();
    }

    void EndFrame()
    {
        EndDrawing();

        PlatformEndFrame();
        gCore.windowResized = false;

        
        double newTime = PlatformGetTime();
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
        PrintLog(LOG_DEBUG, "loading image: %s\n", path);
#endif
        size_t file_len = 0;
        unsigned char* file_data = LoadFileBinary(path, &file_len);
        image.data = stbi_load_from_memory(file_data, (int)file_len, &image.width, &image.height, &image.components, 4);
        UnloadFileBinary(file_data);

        if (!image.data)
        {
            PrintLog(LOG_ERROR, "Failed to load image: %s\n", path);
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
        ASSERT(src.components == dst.components, "Cannot draw if components aren't the same");

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

    Vector2 GetMousePosition()
    {
        return Vector2{(float)gCore.input.current.mouse_x, (float)gCore.input.current.mouse_y};
    }

    Vector2 GetMouseMotion()
    {
        return Vector2{(float)gCore.input.current.mouse_x, (float)gCore.input.current.mouse_y} - Vector2{(float)gCore.input.previous.mouse_x, (float)gCore.input.previous.mouse_y};
    }

    Color ColorFromBytes(unsigned char r, unsigned char g, unsigned char b, unsigned char a)
    {
        return {r / 255.f, g / 255.f, b / 255.f, a / 255.f};
    }


    Vector2 TransformVector(Matrix transform, Vector2 vector)
    {
        return (transform * HMM_Vec4{vector.X, vector.Y, 0, 1}).XY;
    }

    Vector2 InvTransformVector(Matrix transform, Vector2 vector)
    {
        Matrix inv_transform = HMM_InvGeneralM4(transform);
        return (inv_transform * HMM_Vec4{vector.X, vector.Y, 0, 1}).XY;
    }

    Matrix GetMatrix(Transform2D transform)
    {
        return HMM_Translate({transform.position.X, transform.position.Y, 0.f}) * HMM_Rotate_LH(transform.rotation, {0.f, 0.f, 1.f}) * HMM_Scale({transform.scale.X, transform.scale.Y, 1.f});
    }

    const char* TextFormat(const char* format, ...)
    {
        va_list args;
        va_start(args, format);
        int requiredByteCount = stbsp_vsnprintf(gCore.textBuffer, TEXT_BUFFER_SIZE, format, args);
        va_end(args);

        return gCore.textBuffer;
    }

    void PrintLog(int logLevel, const char* format, ...)
    {
        const char* prefix = NULL;
        switch (logLevel) {
            case LOG_TRACE: {
                prefix = "TRACE: ";
            } break;
            case LOG_DEBUG: {
                prefix = "DEBUG: ";
            } break;
            case LOG_INFO: {
                prefix = "INFO : ";
            } break;
            case LOG_WARNING: {
                prefix = "WARN : ";
            } break;
            case LOG_ERROR: {
                prefix = "ERROR: ";
            } break;
            case LOG_FATAL: {
                prefix = "FATAL: ";
            } break;
        }

        char* body = stpcpy(gCore.textBuffer, prefix); // returns null terminator of dest

        va_list args;
        va_start(args, format);
        int requiredByteCount = stbsp_vsnprintf(body, TEXT_BUFFER_SIZE, format, args);
        va_end(args);

        PlatformPrint(gCore.textBuffer);

    }


    void* GetProcAddressPtr()
    {
        return PlatformGetProcAddressPtr();
    }


    unsigned char *LoadFileBinary(const char *fileName, size_t *dataSize)
    {
        size_t size = 0;
        unsigned char* bytes = PlatformLoadFileBinary(fileName, &size);
        if (dataSize)
        {
            *dataSize = size;
        }
        return bytes;
    }

    void UnloadFileBinary(unsigned char *data)
    {
        PlatformUnloadFileBinary(data);
    }

    bool SaveFileBinary(const char *fileName, void *data, size_t dataSize)
    {
        return PlatformSaveFileBinary(fileName, data, dataSize);
    }

    char *LoadFileText(const char *fileName)
    {
        return PlatformLoadFileText(fileName);
    }

    void UnloadFileText(char *text)
    {
        PlatformUnloadFileText(text);
    }

    bool SaveFileText(const char *fileName, char *text)
    {
        return PlatformSaveFileText(fileName, text);
    }

}