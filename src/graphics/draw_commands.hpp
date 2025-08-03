#pragma once
#ifndef DRAW_COMMANDS_HPP
#define DRAW_COMMANDS_HPP

#include "melon_types.hpp"

namespace Render
{
    struct SpriteCommand{
        Vector2 vertices[4];
        Vector2 uvs[4];
        Color color;
        Color color_override;
    };
    constexpr size_t MaxSpriteCommands = 64 * 1024;
    
    struct SpriteCommandBuffer{
        SpriteCommand items[MaxSpriteCommands];
        size_t count;
    };

    enum class TextAlignment;

    struct TextCommand{
        Transform transform;
        TextAlignment alignment;
        Color color;
        char* text;
    };
    
    constexpr size_t MaxTextCommands = 1024;
    constexpr size_t MaxTextBufferSize = 8 * 1024;

    struct TextCommandBuffer
    {   
        TextCommand items[MaxTextCommands];
        size_t count;
        char text_buffer[MaxTextBufferSize];
        char* text_next;
    };
} // namespace Render

#endif // DRAW_COMMANDS_HPP