#include "renderer.hpp"
#include <cstdio>
#include <glad/glad.h>
#include <cstddef>
#include <cassert>

#include "core.hpp"

#include "draw_commands.hpp"

#include "sprite_renderer.hpp"
#include "text_renderer.hpp"

#include <cstring>

#define STB_RECT_PACK_IMPLEMENTATION
#include "stb_rect_pack.h"

namespace Render {
    void DrawCommands();

    #pragma pack(push, 1)
    struct Vertex{
        Vector2 position;
        Color color;
        Vector2 uv;
    };
    #pragma pack(pop)

    constexpr unsigned int MaxSpriteSheetSize = 1024 * 2;

    struct {
        struct {
            DrawCommand items[MaxDrawCommands];
            size_t count;
        } commands;

        char text_buffer[MaxTextBufferSize];
        char* text_next;

        Transform view;
        Transform projection;
        Color clear_color;

        int sprite_atlas_width;
        int sprite_atlas_height;

        int sprite_coords[SpriteAtlas::Sprite::_LENGTH][4];
    } state = {0};

    void FeatherEdges(Mln::Image image, int feather_amount)
    {
        assert(image.data && "Feathering only supported for valid images");
        assert(image.components == 4 && "Feathering only supported for images with 4 components");

        Mln::Image temp_image = Mln::CreateImage(image.width, image.height, image.components);

        struct Pixel4
        {
            unsigned char r;
            unsigned char g;
            unsigned char b;
            unsigned char a;
        };

        Pixel4* pixels = (Pixel4*)image.data;
        Pixel4* out_pixels = (Pixel4*)temp_image.data;

        for (int j = 0; j < image.height; j++)
        {
            for (int i = 0; i < image.width; i++)
            {
                int index = i + j * image.width;
                if (pixels[index].a != 0)
                {
                    out_pixels[index] = pixels[index];
                    continue;
                }

                bool has_neighbour = false;
                int close_offset_x = 0;
                int close_offset_y = 0;

                for (int off_y = -feather_amount; off_y < feather_amount; off_y++)
                {
                    int neighbour_y = j + off_y;
                    if (neighbour_y < 0 || neighbour_y >= image.height)
                    {
                        continue;
                    }

                    for (int off_x = -feather_amount; off_x < feather_amount; off_x++)
                    {
                        int neighbour_x = i + off_x;
                        if (neighbour_x < 0 || neighbour_x >= image.width)
                        {
                            continue;
                        }

                        int neighbour_index = neighbour_x + neighbour_y * image.width;
                        if (pixels[neighbour_index].a == 0)
                        {
                            continue;
                        }

                        if (!has_neighbour || off_x * off_x + off_y * off_y < close_offset_x * close_offset_x + close_offset_y * close_offset_y)
                        {
                            has_neighbour = true;
                            close_offset_x = off_x;
                            close_offset_y = off_y;
                        }
                    }
                }
                
                if (has_neighbour)
                {
                    int neighbour_index = (i + close_offset_x) + (j + close_offset_y) * image.width;
                    out_pixels[index] = pixels[neighbour_index];
                    out_pixels[index].a = 0;
                }
            }
        }
        memcpy(image.data, temp_image.data, image.width * image.height * image.components * sizeof(unsigned char));
        Mln::UnloadImage(temp_image);
    }

    void InitRenderer()
    {
        constexpr int padding = 2;
        Mln::Image images[SpriteAtlas::Sprite::_LENGTH];
        stbrp_rect pack_rects[SpriteAtlas::Sprite::_LENGTH];
        for (int i = 0; i < SpriteAtlas::Sprite::_LENGTH; i++)
        {
            char buffer[256];
            buffer[0] = 0;
            strncat(buffer, RESOURCES_PATH, 255);
            strncat(buffer, SpriteAtlas::file_list[i], 255);
            buffer[255] = 0;
            images[i] = Mln::LoadImage(buffer);
            pack_rects[i].w = images[i].width + padding * 2;
            pack_rects[i].h = images[i].height + padding * 2;
        }

        stbrp_context pack_ctx;
        stbrp_node pack_nodes[MaxSpriteSheetSize];

        stbrp_init_target(&pack_ctx, MaxSpriteSheetSize, MaxSpriteSheetSize, pack_nodes, MaxSpriteSheetSize);
        bool success = stbrp_pack_rects(&pack_ctx, pack_rects, SpriteAtlas::Sprite::_LENGTH);
        assert(success && "Could not pack textures");

        

        Mln::Image image = Mln::CreateImage(MaxSpriteSheetSize, MaxSpriteSheetSize, 4);

        for (int i = 0; i < SpriteAtlas::Sprite::_LENGTH; i++)
        {
            Mln::RectI src_rect = {0, 0, images[i].width, images[i].height};
            Mln::RectI dst_rect = {pack_rects[i].x + padding, pack_rects[i].y + padding, images[i].width, images[i].height};
            
            state.sprite_coords[i][0] = dst_rect.x;
            state.sprite_coords[i][1] = dst_rect.y;
            state.sprite_coords[i][2] = dst_rect.width;
            state.sprite_coords[i][3] = dst_rect.height;

            Mln::ImageDrawImage(image, dst_rect, images[i], src_rect);

            Mln::UnloadImage(images[i]);
        }
        
        FeatherEdges(image, 5);

#if defined(GENERATE_SPRITE_ATLAS)
        Mln::WriteImage(image, "demo.png");
#endif
        // Mln::Image i = Mln::LoadImage("demo1.png");

        state.sprite_atlas_width = image.width;
        state.sprite_atlas_height = image.height;

        Render::Text::Load(RESOURCES_PATH "Kenney Future Narrow.ttf");
        Render::Sprite::Load(image);

        
        state.text_next = state.text_buffer;

        state.clear_color = Color{0.2f, 0.3f, 0.3f, 1.0f};

        state.view = HMM_M4D(1.0);
        state.projection = HMM_M4D(1.0);


        state.commands.count = 0;
    }

    void UnloadRenderer()
    {
        Render::Sprite::Unload();
    }

    void SetView(Transform view)
    {
        state.view = view;
    }

    void SetProjection(Transform proj)
    {
        state.projection = proj;
    }

    void SetClearColor(Color color)
    {
        state.clear_color = color;
    }

    Color GetClearColor()
    {
        return state.clear_color;
    }

    void DrawFrame()
    {
        glClearColor(state.clear_color.R, state.clear_color.G, state.clear_color.B, state.clear_color.A);
        glClear(GL_COLOR_BUFFER_BIT);


        DrawCommands();
    }

    void DrawBatch(DrawCommandType commandType, DrawCommand* batch, size_t count)
    {
        switch (commandType) 
        {
            case DRAW_COMMAND_SPRITE:
            {
                Render::Sprite::DrawCommands(batch, count);
            } break;
            case DRAW_COMMAND_TEXT:
            {
                Render::Text::DrawCommands(batch, count);
            } break;
            case DRAW_COMMAND_INVALID:
            {

            }break;
            default:
            {
                assert(0 && "Incorrect draw command type");
            }
        }
    }

    void DrawCommands()
    {
        state.commands.items[state.commands.count++].type = DRAW_COMMAND_INVALID;

        int batchType = DRAW_COMMAND_INVALID;
        DrawCommand* batchStart = state.commands.items;
        size_t batchCount = 0;


        for (size_t i = 0; i < state.commands.count; i++) 
        {
            DrawCommand* next = state.commands.items + i;
            if (next->type != batchType)
            {
                DrawBatch(batchType, batchStart, batchCount);

                batchStart = next;
                batchCount = 1;
                batchType = next->type;
            }
            else
            {
                batchCount++;
            }
        }

        // NOTE(takintug): render the last batch in the list
        if (batchCount > 0)
        {
            DrawBatch(batchType, batchStart, batchCount);
        }

        state.text_next = state.text_buffer;
        state.commands.count = 0;
    }
    
    void DrawSprite(Vector2 position, Vector2 size, Color color, SpriteAtlas::Sprite sprite)
    {
        Vector2 spriteSize = GetSpriteSize(sprite);
        DrawSprite({position, size / spriteSize, 0.f}, color, sprite);
    }

    void DrawSprite(Mln::Transform2D transform, Color color, SpriteAtlas::Sprite sprite)
    {
        Transform matrix = HMM_Translate({transform.position.X, transform.position.Y, 0.f}) * HMM_Rotate_LH(transform.rotation, {0.f, 0.f, 1.f}) * HMM_Scale({transform.scale.X, transform.scale.Y, 1.f});
        DrawSprite(matrix, color, sprite);
    }

    void DrawSprite(Transform transform, Color color, SpriteAtlas::Sprite sprite)
    {
        assert(state.commands.count != Render::MaxDrawCommands && "Draw Command limit reached");

        SpriteCommand command;
        
        float sprite_w = state.sprite_coords[sprite][2];
        float sprite_h = state.sprite_coords[sprite][3];

        Vector2 top_right    = Vector2{ sprite_w / 2.f, -sprite_h / 2.f};
        Vector2 bottom_right = Vector2{ sprite_w / 2.f,  sprite_h / 2.f};
        Vector2 bottom_left  = Vector2{-sprite_w / 2.f,  sprite_h / 2.f};
        Vector2 top_left     = Vector2{-sprite_w / 2.f, -sprite_h / 2.f};
        
        Transform mvp = state.projection * state.view * transform;
        
        command.vertices[0] = (mvp * HMM_Vec4{top_right.X   , top_right.Y   , 0, 1}).XY;
        command.vertices[1] = (mvp * HMM_Vec4{bottom_right.X, bottom_right.Y, 0, 1}).XY;
        command.vertices[2] = (mvp * HMM_Vec4{bottom_left.X , bottom_left.Y , 0, 1}).XY;
        command.vertices[3] = (mvp * HMM_Vec4{top_left.X    , top_left.Y    , 0, 1}).XY;
        
        float texture_w = state.sprite_atlas_width;
        float texture_h = state.sprite_atlas_height;

        float sprite_uv_x = state.sprite_coords[sprite][0] / texture_w;
        float sprite_uv_y = state.sprite_coords[sprite][1] / texture_h;
        float sprite_uv_w = state.sprite_coords[sprite][2] / texture_w;
        float sprite_uv_h = state.sprite_coords[sprite][3] / texture_h;

        command.uvs[0] = Vector2{sprite_uv_x + sprite_uv_w, sprite_uv_y};
        command.uvs[1] = Vector2{sprite_uv_x + sprite_uv_w, sprite_uv_y + sprite_uv_h};
        command.uvs[2] = Vector2{sprite_uv_x, sprite_uv_y + sprite_uv_h};
        command.uvs[3] = Vector2{sprite_uv_x, sprite_uv_y};

        command.color = color;
        command.color_override = {0, 0, 0, 0};

        command.type = DRAW_COMMAND_SPRITE;
        state.commands.items[state.commands.count++].sprite = command;
    }

    Vector2 GetSpriteSize(SpriteAtlas::Sprite sprite)
    {
        return Vector2{static_cast<float>(state.sprite_coords[sprite][2]), static_cast<float>(state.sprite_coords[sprite][3])};
    }

    float MeasureText(const char *str)
    {
        return Text::MeasureText(str);
    }

    void DrawText(const char *str, Vector2 position, float scale, Color color, TextAlignment alignment /* = TextAlignment::LEFT */)
    {
        assert(state.commands.count != Render::MaxDrawCommands && "Draw Command limit reached");

        size_t str_length = strlen(str);
        assert((state.text_buffer + MaxTextBufferSize - state.text_next) > str_length && "Text buffer limit reached");

        TextCommand command;
        command.alignment = alignment;
        command.color = color;
        command.text = state.text_next;

        memcpy(state.text_next, str, str_length * sizeof(char));
        state.text_next += str_length * sizeof(char);
        *state.text_next = 0;
        state.text_next += 1;

        float text_width = MeasureText(command.text);
        Transform alignment_offset = HMM_M4D(1.0f);
        switch (alignment)
        {
        case TextAlignment::LEFT:
            alignment_offset = HMM_M4D(1.0f);
            break;
        case TextAlignment::CENTER:
            alignment_offset = HMM_Translate({-text_width * 0.5f, 0.f, 0.f});
            break;
        case TextAlignment::RIGHT:
            alignment_offset = HMM_Translate({-text_width, 0.f, 0.f});
            break;
        }

        Transform model = HMM_Translate({position.X, position.Y, 0.f}) * HMM_Scale({scale, scale, 1.f}) * alignment_offset;
        Transform mvp = state.projection * state.view * model;
        command.transform = mvp;
        
        command.type = DRAW_COMMAND_TEXT;
        state.commands.items[state.commands.count++].text = command;
    }

    Mln::Texture LoadTexture(Mln::Image image, bool filter, bool mipmaps)
    {
        using namespace Mln;
        Texture result {InvalidID, 0, 0};

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


    void UnloadTexture(Mln::Texture& texture)
    {
        glDeleteTextures(1, &texture.id);
        texture.id = -1;
        texture.width = 0;
        texture.height = 0;
    }

}