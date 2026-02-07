#pragma once

#ifndef GRAPHICS_API_H
#define GRAPHICS_API_H

#include "melon_types.hpp"

extern "C"{

enum TextAlign
{
    TEXT_ALIGN_LEFT,
    TEXT_ALIGN_CENTER,
    TEXT_ALIGN_RIGHT
};

void InitGraphics(int width, int height);
void ShutdownGraphics();


void SetView(Mln::Transform view);
void SetProjection(Mln::Transform proj);
void ClearBackground(Mln::Color color);

void BeginDrawing();
void EndDrawing();

Mln::Texture LoadTexture(const char* path, bool filter, bool mipmaps);
Mln::Texture LoadTextureFromImage(Mln::Image image, bool filter, bool mipmaps);
void UnloadTexture(Mln::Texture texture);


void DrawRectTextured(Mln::Transform transform, Mln::Texture texture, Mln::RectI texture_source, Mln::Color color);

Mln::Font LoadFont(const char* path);
void UnloadFont(Mln::Font font);

float MeasureText(Mln::Font font, const char* str);
void DrawText(Mln::Font font, const char *str, Mln::Vector2 position, float scale, Mln::Color color, TextAlign alignment = TEXT_ALIGN_LEFT);

}
#endif // GRAPHICS_API_H