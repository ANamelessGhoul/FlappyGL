#pragma once
#ifndef MELON_TYPES_HPP
#define MELON_TYPES_HPP

#include "HandmadeMath.h"


namespace Mln
{
    using Vector2 = HMM_Vec2;
    using Vector3 = HMM_Vec3;
    using Vector4 = HMM_Vec4;
    using Color = HMM_Vec4;
    using Transform = HMM_Mat4;

    typedef void* Font;

    enum Error{
        OK,
        ERR_GENERIC
    };

    typedef unsigned int id_t;
    static constexpr id_t InvalidID = -1;

    struct Image
    {
        unsigned char* data;
        int width;
        int height;
        int components;
    };

    struct Rect
    {
        float x, y;
        float width, height;
    };
    
    struct RectI
    {
        int x, y;
        int width, height;
    };

    struct Vector2I
    {
        int x, y;
    };

    struct Texture
    {
        id_t id;
        int width;
        int height;
    };

    struct Shader
    {
        id_t id; 
    };
    
    struct Transform2D
    {
        Vector2 position;
        Vector2 scale;
        float rotation;
    };

}

#endif // MELON_TYPES_HPP