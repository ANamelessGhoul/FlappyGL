#pragma once
#ifndef TEXT_RENDERER_HPP
#define TEXT_RENDERER_HPP

#include "draw_commands.hpp"
#include "melon_types.hpp"

namespace Render
{
    namespace Text
    {
        void Load(const char* font_file);
        void Unload();
        void DrawCommands(const TextCommandBuffer& commands);

        float MeasureText(const char* text);
    }
}




#endif // SPRITE_RENDERER_HPP