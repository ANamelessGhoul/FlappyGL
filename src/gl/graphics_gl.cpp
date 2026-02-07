#include "graphics_api.hpp"
#include "core.hpp"
#include "melon_types.hpp"
#include "quad_renderer.hpp"

#include <cassert>
#include <cstdint>
#include <glad/glad.h>

#if defined (OPENGL_ES)
    #include "gen/gles/default.fs.h"
    #include "gen/gles/default.vs.h"
    #include "gen/gles/text.fs.h"
    #include "gen/gles/text.vs.h"
#else
    #include "gen/gl/default.fs.h"
    #include "gen/gl/default.vs.h"
    #include "gen/gl/text.fs.h"
    #include "gen/gl/text.vs.h"
#endif

#include <cstring>
#include <cstdio>
#include <iostream>

#define STB_RECT_PACK_IMPLEMENTATION
#include "stb_rect_pack.h"

#define STB_TRUETYPE_IMPLEMENTATION
#include "stb_truetype.h"

#ifndef MAX_FONTS
    #define MAX_FONTS 16
#endif

struct AtlasFont{
    stbtt_packedchar packed_chars[256];
    Mln::Texture texture;
};

struct {
    Mln::Transform view;
    Mln::Transform projection;

    Mln::Shader sprite_shader;
    Mln::Shader text_shader;

    AtlasFont fonts[MAX_FONTS];
    int font_count;
    uintptr_t font_ids[MAX_FONTS];
    uintptr_t last_font_id;

} state = {0};

Mln::Shader _LoadShader(const char *vertexText, const char *fragmentText);
AtlasFont* _FindFont(Mln::Font font, int* font_index);

void InitGraphics(int width, int height)
{
    // glad: load all OpenGL function pointers
    // ---------------------------------------
    #if defined (OPENGL_ES)
    if (!gladLoadGLES2Loader((GLADloadproc)Mln::GetProcAddressPtr()))
    #else
    if (!gladLoadGLLoader((GLADloadproc)Mln::GetProcAddressPtr()))
    #endif
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return;
    }


    glViewport(0, 0, width, height);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); // TODO: premultiplied alpha

    state.sprite_shader = _LoadShader(default_vs, default_fs);
    state.text_shader = _LoadShader(text_vs, text_fs);
    InitQuadRenderer();


    state.view = HMM_M4D(1.0);
    state.projection = HMM_M4D(1.0);
}

void ShutdownGraphics()
{
    ShutdownQuadRenderer();
}

void SetView(Mln::Transform view)
{
    state.view = view;
}

void SetProjection(Mln::Transform proj)
{
    state.projection = proj;
}

void ClearBackground(Mln::Color color)
{
    glClearColor(color.R, color.G, color.B, color.A);
    glClear(GL_COLOR_BUFFER_BIT);
}

void BeginDrawing()
{
}

void EndDrawing()
{
    DrawBatch();
}

Mln::Texture LoadTexture(const char* path, bool filter, bool mipmaps)
{
    Mln::Image image = Mln::LoadImage(path);
    Mln::Texture texture = LoadTextureFromImage(image, filter, mipmaps);
    Mln::UnloadImage(image);
    return texture;
}

Mln::Texture LoadTextureFromImage(Mln::Image image, bool filter, bool mipmaps)
{
    Mln::Texture result {Mln::InvalidID, 0, 0};

    if (!image.data)
    {
        return result;
    }

    // load and create a texture 
    // -------------------------
    unsigned int texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture); // all upcoming GL_TEXTURE_2D operations now have effect on this texture object
    // set the texture wrapping parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	// set texture wrapping to GL_REPEAT (default wrapping method)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    // set texture filtering parameters
    if (filter)
    {
        unsigned int minFilter = mipmaps ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR;
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, minFilter);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    }
    else
    {
        unsigned int minFilter = mipmaps ? GL_NEAREST_MIPMAP_NEAREST : GL_NEAREST;
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, minFilter);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    }
    // load image, create texture and generate mipmaps
    GLenum componentTypes[] = {
        0,
#if !defined(OPENGL_ES)
        GL_RED,
        GL_RG,
        GL_RGB,
        GL_RGBA
#else
        GL_LUMINANCE,
        GL_LUMINANCE_ALPHA,
        GL_RGB,
        GL_RGBA
#endif
    };

    int internalFormat = GL_RGBA;
    #if defined(OPENGL_ES)
    internalFormat = componentTypes[image.components];
    #endif

    glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, image.width, image.height, 0, componentTypes[image.components], GL_UNSIGNED_BYTE, image.data);
    if (mipmaps)
    {
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    
    result.width = image.width;
    result.height = image.height;
    result.id = texture;

    return result;

}

void UnloadTexture(Mln::Texture texture)
{
    glDeleteTextures(1, &texture.id);
    texture.id = -1;
    texture.width = 0;
    texture.height = 0;
}


void DrawRectTextured(Mln::Transform transform, Mln::Texture texture, Mln::RectI coords, Mln::Color color)
{
    SetTexture(texture);
    SetShader(state.sprite_shader);

    Quad quad = {0};
    
    float sprite_w = coords.width;
    float sprite_h = coords.height;

    Mln::Vector2 top_right    = Mln::Vector2{ sprite_w / 2.f, -sprite_h / 2.f};
    Mln::Vector2 bottom_right = Mln::Vector2{ sprite_w / 2.f,  sprite_h / 2.f};
    Mln::Vector2 bottom_left  = Mln::Vector2{-sprite_w / 2.f,  sprite_h / 2.f};
    Mln::Vector2 top_left     = Mln::Vector2{-sprite_w / 2.f, -sprite_h / 2.f};
    
    Mln::Transform mvp = state.projection * state.view * transform;
    
    quad.vertices[0] = (mvp * HMM_Vec4{top_right.X   , top_right.Y   , 0, 1}).XY;
    quad.vertices[1] = (mvp * HMM_Vec4{bottom_right.X, bottom_right.Y, 0, 1}).XY;
    quad.vertices[2] = (mvp * HMM_Vec4{bottom_left.X , bottom_left.Y , 0, 1}).XY;
    quad.vertices[3] = (mvp * HMM_Vec4{top_left.X    , top_left.Y    , 0, 1}).XY;
    
    float texture_w = texture.width;
    float texture_h = texture.height;

    float sprite_uv_x = coords.x / texture_w;
    float sprite_uv_y = coords.y / texture_h;
    float sprite_uv_w = coords.width / texture_w;
    float sprite_uv_h = coords.height / texture_h;

    quad.uvs[0] = Mln::Vector2{sprite_uv_x + sprite_uv_w, sprite_uv_y};
    quad.uvs[1] = Mln::Vector2{sprite_uv_x + sprite_uv_w, sprite_uv_y + sprite_uv_h};
    quad.uvs[2] = Mln::Vector2{sprite_uv_x, sprite_uv_y + sprite_uv_h};
    quad.uvs[3] = Mln::Vector2{sprite_uv_x, sprite_uv_y};

    quad.color = color;


    PushQuad(quad);
}

Mln::Font LoadFont(const char* path)
{
    assert(state.font_count + 1 < MAX_FONTS && "Maximum font limit reached");
    if (state.font_count + 1 >= MAX_FONTS)
    {
        return nullptr;
    }


    // TODO: Handle errors
    // TODO: Make a global scratch buffer for images?
    
    // Get the next font slot
    Mln::id_t font_id = ++state.last_font_id;
    AtlasFont* font = &state.fonts[state.font_count];
    assert(state.font_ids[state.font_count] == 0 && "The next unused font has a valid id!");
    state.font_ids[state.font_count] = font_id;
    state.font_count++;

    stbtt_pack_context ctx;


    Mln::Image font_atlas_image = Mln::CreateImage(512, 512, 1);

    stbtt_PackBegin(&ctx, font_atlas_image.data, 512, 512, 0, 2, nullptr);

    FILE* file = fopen(path, "rb");
    
    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    fseek(file, 0, SEEK_SET);

    unsigned char* ttf_buffer = (unsigned char*)malloc(size + 1);

    fread(ttf_buffer, 1, size, file);
    fclose(file);
    
    stbtt_PackFontRange(&ctx, ttf_buffer, 0, 48, 0, 256, font->packed_chars);
    stbtt_PackEnd(&ctx);
    
    free(ttf_buffer);

    font->texture = LoadTextureFromImage(font_atlas_image, true, true);
    
#if defined(GENERATE_FONT_ATLAS)
    Mln::WriteImage(font_atlas_image, "font.png");
#endif
    
    Mln::UnloadImage(font_atlas_image);

    return (void*)(uintptr_t)font_id;
}

void UnloadFont(Mln::Font font)
{
    int font_index = 0;
    AtlasFont* atlas_font =_FindFont(font, &font_index);

    assert(atlas_font != nullptr && "Font could not be found for unloading.");
    if (atlas_font == nullptr)
    {
        return;
    }

    UnloadTexture(atlas_font->texture);


    state.font_ids[font_index] = state.font_ids[state.font_count - 1];
    state.fonts[font_index] = state.fonts[state.font_count - 1];
    state.font_ids[state.font_count - 1] = 0;
    state.font_count--;
}

float MeasureText(Mln::Font font, const char* str)
{
    AtlasFont* atlas_font = _FindFont(font, NULL);
    assert(atlas_font && "Font not found");
    int length = strlen(str);

    float x = 0;
    float y = 0;
    for (size_t i = 0; i < length; i++)
    {
        float x_advance = 0, y_advance = 0;
        char character = str[i];
        stbtt_aligned_quad quad;
        stbtt_GetPackedQuad(atlas_font->packed_chars, 512, 512, character, &x_advance, &y_advance, &quad, 0);

        if (i == length - 1)
        {
            x += quad.x1;
        }
        else
        {
            x += x_advance;
        }
    }

    return x;
}

void DrawText(Mln::Font font, const char *str, Mln::Vector2 position, float scale, Mln::Color color, TextAlign alignment)
{
    AtlasFont* atlas_font = _FindFont(font, NULL);
    assert(atlas_font && "Font not found");


    SetTexture(atlas_font->texture);
    SetShader(state.text_shader);
    
    size_t str_length = strlen(str);

    float text_width = MeasureText(font, str);
    Mln::Transform alignment_offset = HMM_M4D(1.0f);
    switch (alignment)
    {
    case TEXT_ALIGN_LEFT:
        alignment_offset = HMM_M4D(1.0f);
        break;
    case TEXT_ALIGN_CENTER:
        alignment_offset = HMM_Translate({-text_width * 0.5f, 0.f, 0.f});
        break;
    case TEXT_ALIGN_RIGHT:
        alignment_offset = HMM_Translate({-text_width, 0.f, 0.f});
        break;
    }

    Mln::Transform model = HMM_Translate({position.X, position.Y, 0.f}) * HMM_Scale({scale, scale, 1.f}) * alignment_offset;
    Mln::Transform mvp = state.projection * state.view * model;


    float x = 0;
    float y = 0;
    for (size_t i = 0; i < str_length; i++)
    {
        char character = str[i];
        stbtt_aligned_quad font_quad;
        stbtt_GetPackedQuad(atlas_font->packed_chars, 512, 512, character, &x, &y, &font_quad, 0);
        
        Quad quad;

        // This positions the text so the baseline is at the target position
        quad.vertices[0] = (mvp * HMM_Vec4{font_quad.x1, font_quad.y0, 0, 1}).XY;
        quad.vertices[1] = (mvp * HMM_Vec4{font_quad.x1, font_quad.y1, 0, 1}).XY;
        quad.vertices[2] = (mvp * HMM_Vec4{font_quad.x0, font_quad.y1, 0, 1}).XY;
        quad.vertices[3] = (mvp * HMM_Vec4{font_quad.x0, font_quad.y0, 0, 1}).XY;
        
        quad.uvs[0] = {font_quad.s1, font_quad.t0};
        quad.uvs[1] = {font_quad.s1, font_quad.t1};
        quad.uvs[2] = {font_quad.s0, font_quad.t1};
        quad.uvs[3] = {font_quad.s0, font_quad.t0};
        
        quad.color = color;

        PushQuad(quad);
    }
}

Mln::Shader _LoadShader(const char *vertexText, const char *fragmentText)
{
    bool hasError = false;

    // build and compile our shader program
    // ------------------------------------
    // vertex shader
    unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexText, NULL);
    glCompileShader(vertexShader);
    // check for shader compile errors
    GLint success;
    char infoLog[512];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
        hasError = true;
    }
    // fragment shader
    unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentText, NULL);
    glCompileShader(fragmentShader);
    // check for shader compile errors
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
        hasError = true;
    }
    // link shaders
    unsigned int shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
    // check for linking errors
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
        hasError = true;
    }
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);


    if (hasError)
    {
        return Mln::Shader{Mln::InvalidID};
    }

    return Mln::Shader{shaderProgram};
}


AtlasFont* _FindFont(Mln::Font font, int* font_index)
{
    uintptr_t font_id = (uintptr_t)font;
    AtlasFont* atlas_font = nullptr;
    for (int i = 0; i < MAX_FONTS; i++)
    {
        if (state.font_ids[i] == font_id)
        {
            atlas_font = &state.fonts[i];
            if (font_index)
            {
                *font_index = i;
            }
            break;
        }
    }

    return atlas_font;
}