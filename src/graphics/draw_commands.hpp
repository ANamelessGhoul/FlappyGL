#pragma once
#ifndef DRAW_COMMANDS_HPP
#define DRAW_COMMANDS_HPP

#include "melon_types.hpp"

namespace Render
{
    constexpr size_t MaxDrawCommands = 2 * 1024;

    typedef int DrawCommandType;
    typedef enum{
        DRAW_COMMAND_INVALID = 0,
        DRAW_COMMAND_SPRITE,
        DRAW_COMMAND_TEXT
    } DrawCommandKind;

    struct SpriteCommand{
        DrawCommandType type;
        Vector2 vertices[4];
        Vector2 uvs[4];
        Color color;
        Color color_override;
    };
    
    enum class TextAlignment;

    struct TextCommand{
        DrawCommandType type;
        Transform transform;
        TextAlignment alignment;
        Color color;
        char* text;
    };
    
    constexpr size_t MaxTextBufferSize = 8 * 1024;

    union DrawCommand
    {
        DrawCommandType type;
        SpriteCommand sprite;
        TextCommand text;
    };
} // namespace Render

#endif // DRAW_COMMANDS_HPP