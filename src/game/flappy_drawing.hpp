#pragma once

#ifndef FLAPPY_DRAWING_HPP
#define FLAPPY_DRAWING_HPP

#include "melon_types.hpp"
#include "sprite_atlas.hpp"

void LoadSpriteAtlas();
void UnloadSpriteAtlas();
Mln::Vector2 GetSpriteSize(SpriteAtlas::Sprite sprite);

void DrawSprite(Mln::Vector2 position, Mln::Vector2 size, Mln::Color color, SpriteAtlas::Sprite sprite);
void DrawSprite(Mln::Transform2D transform, Mln::Color color, SpriteAtlas::Sprite sprite);
void DrawSprite(Mln::Transform transform, Mln::Color color, SpriteAtlas::Sprite sprite);

#endif // FLAPPY_DRAWING_HPP