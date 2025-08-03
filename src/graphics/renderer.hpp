#pragma once
#ifndef RENDERER_HPP
#define RENDERER_HPP

#include "melon_types.hpp"
#include "sprite_atlas.hpp"

namespace Render {
    enum class TextAlignment
    {
        LEFT,
        CENTER,
        RIGHT
    };

    void InitRenderer();
    void UnloadRenderer();

    void SetView(Transform view);
    void SetProjection(Transform proj);
    void SetClearColor(Color color);
    Color GetClearColor();

    void DrawFrame();
    void DrawSprite(Vector2 position, Vector2 size, Color color, SpriteAtlas::Sprite sprite);
    void DrawSprite(Mln::Transform2D transform, Color color, SpriteAtlas::Sprite sprite);
    void DrawSprite(Transform transform, Color color, SpriteAtlas::Sprite sprite);
    Vector2 GetSpriteSize(SpriteAtlas::Sprite sprite);

    float MeasureText(const char* str);
    void DrawText(const char *str, Vector2 position, float scale, Color color, TextAlignment alignment = TextAlignment::LEFT);

    Mln::Texture LoadTexture(Mln::Image image, bool filter, bool mipmaps);
    void UnloadTexture(Mln::Texture& texture);
}

#endif // RENDERER_HPP