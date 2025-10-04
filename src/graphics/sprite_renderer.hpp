#pragma once
#ifndef SPRITE_RENDERER_HPP
#define SPRITE_RENDERER_HPP

#include "draw_commands.hpp"
#include "melon_types.hpp"

namespace Render
{
    namespace Sprite
    {
        void Load(Mln::Image atlas_image);
        void Unload();
        void DrawCommands(DrawCommand* commands, size_t count);
    }
}




#endif // SPRITE_RENDERER_HPP