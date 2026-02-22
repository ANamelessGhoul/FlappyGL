#pragma once

#ifndef MELON_CORE_HPP
#define MELON_CORE_HPP

#include "keys.h"
#include "melon_types.hpp"

#include "config.hpp"

#ifndef RESOURCES_PATH
#define RESOURCES_PATH "./resources/"
#endif



namespace Mln
{
    Error InitWindow(int width, int height, const char* title); // title must have the same lifetime as the window or until SetWindowTitle is called
    void UnloadWindow();

    void SetWindowTitle(const char* title); // title must have the same lifetime as the window or until SetWindowTitle is called

    bool WindowShouldClose();
    bool DidWindowResize();
    Vector2 GetViewportSize();

    double GetFrameTime();
    double GetFPS();

    void BeginFrame();
    void EndFrame();

    Image LoadImage(const char* path);
    Image CreateImage(int width, int height, int components);
    void UnloadImage(Image image);
    void ImageDrawImage(Image dst, RectI dst_rect, Image src, RectI src_rect);
    void WriteImage(Image image, const char* path);
    

    bool IsKeyDown(Key key);
    bool IsKeyUp(Key key);
    bool IsKeyJustPressed(Key key);

    bool IsMouseButtonDown(MouseButton button);
    bool IsMouseButtonUp(MouseButton button);
    bool IsMouseButtonJustPressed(MouseButton button);

    Vector2 GetMousePosition();
    Vector2 GetMouseMotion();

    Color ColorFromBytes(unsigned char r, unsigned char g, unsigned char b, unsigned char a);

    Vector2 TransformVector(Matrix transform, Vector2 vector);
    Vector2 InvTransformVector(Matrix transform, Vector2 vector);
    Matrix GetMatrix(Transform2D transform2D);

    const char* TextFormat(const char* format, ...) ATTRIBUTE_FORMAT(1, 2);
    void PrintLog(int logLevel, const char* format, ...) ATTRIBUTE_FORMAT(2, 3);

    void* GetProcAddressPtr();

    unsigned char *LoadFileBinary(const char *fileName, size_t *dataSize);
    void UnloadFileBinary(unsigned char *data);
    bool SaveFileBinary(const char *fileName, void *data, size_t dataSize);

    char *LoadFileText(const char *fileName);
    void UnloadFileText(char *text);
    bool SaveFileText(const char *fileName, char *text);

} // namespace Mln


#endif // MELON_CORE_HPP