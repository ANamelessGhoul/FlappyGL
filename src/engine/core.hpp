#pragma once

#ifndef MELON_CORE_HPP
#define MELON_CORE_HPP

#include "keys.h"
#include "melon_types.hpp"

#ifndef RESOURCES_PATH
#define RESOURCES_PATH "./resources/"
#endif


namespace Mln
{
    Error InitWindow(int width, int height, const char* name);
    void UnloadWindow();
    bool WindowShouldClose();
    bool DidWindowResize();
    Vector2 GetViewportSize();

    double GetFrameTime();
    double GetFPS();

    void BeginFrame();
    void EndFrame();

    Shader LoadShader(const char* vertexText, const char* fragmentText);
    Texture LoadTexture(const char* path, bool filter, bool mipmaps);
    Texture LoadTextureFromImage(Image image, bool filter, bool mipmaps);
    void UnloadTexture(Texture& texture);
    Image LoadImage(const char* path);
    Image CreateImage(int width, int height, int components);
    void UnloadImage(Image image);
    void ImageDrawImage(Image dst, RectI dst_rect, Image src, RectI src_rect);
    void WriteImage(Image image, const char* path);
    

    bool IsKeyDown(Key key);

    Vector2 TransformVector(Transform transform, Vector2 vector);
    Transform GetMatrix(Transform2D transform2D);
} // namespace Mln


#endif // MELON_CORE_HPP