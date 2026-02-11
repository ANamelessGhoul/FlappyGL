#include "flappy_drawing.hpp"
#include "graphics_api.hpp"
#include "melon_types.hpp"
#include "stb_rect_pack.h"

#include <cstring>
#include <cassert>
#include "core.hpp"

#include "graphics_api.hpp"

constexpr unsigned int MaxSpriteSheetSize = 1024 * 2;

struct {
    Mln::Texture sprite_atlas_texture;
    int sprite_coords[SpriteAtlas::Sprite::_LENGTH][4];
} state;


void FeatherEdges(Mln::Image image, int feather_amount)
{
    ASSERT(image.data, "Feathering only supported for valid images");
    ASSERT(image.components == 4, "Feathering only supported for images with 4 components");

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

void LoadSpriteAtlas()
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
    ASSERT(success, "Could not pack textures");

    

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

    // TODO: Use premultiplied alpha instead
#if defined(FEATHER_SPRITE_ATLAS)
    FeatherEdges(image, 5);
#endif

#if defined(GENERATE_SPRITE_ATLAS)
    Mln::WriteImage(image, "demo.png");
#endif

    state.sprite_atlas_texture = ::LoadTextureFromImage(image, true, true);
    Mln::UnloadImage(image);
}

void UnloadSpriteAtlas()
{
    ::UnloadTexture(state.sprite_atlas_texture);
}

Mln::Vector2 GetSpriteSize(SpriteAtlas::Sprite sprite)
{
    return Mln::Vector2{static_cast<float>(state.sprite_coords[sprite][2]), static_cast<float>(state.sprite_coords[sprite][3])};
}

void DrawSprite(Mln::Vector2 position, Mln::Vector2 size, Mln::Color color, SpriteAtlas::Sprite sprite)
{
    Mln::Vector2 spriteSize = GetSpriteSize(sprite);
    DrawSprite({position, size / spriteSize, 0.f}, color, sprite);
}

void DrawSprite(Mln::Transform2D transform, Mln::Color color, SpriteAtlas::Sprite sprite)
{
    Mln::Transform matrix = HMM_Translate({transform.position.X, transform.position.Y, 0.f}) * HMM_Rotate_LH(transform.rotation, {0.f, 0.f, 1.f}) * HMM_Scale({transform.scale.X, transform.scale.Y, 1.f});
    DrawSprite(matrix, color, sprite);
}

void DrawSprite(Mln::Transform transform, Mln::Color color, SpriteAtlas::Sprite sprite)
{
    int sprite_x = state.sprite_coords[sprite][0];
    int sprite_y = state.sprite_coords[sprite][1];
    int sprite_w = state.sprite_coords[sprite][2];
    int sprite_h = state.sprite_coords[sprite][3];
    DrawRectTextured(transform, state.sprite_atlas_texture, Mln::RectI{sprite_x, sprite_y, sprite_w, sprite_h}, color);
}