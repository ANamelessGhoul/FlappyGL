#ifndef SPRITE_ATLAS_HPP
#define SPRITE_ATLAS_HPP

namespace SpriteAtlas
{
    enum Sprite: int {
        BACKGROUND_1,
        PLAYER_FLOAT_1,
        PLAYER_FLOAT_2,
        PLAYER_HIT,
        PLAYER_BOOST,
        WINGS,
        WALL,
        BRONZE,
        SILVER,
        GOLD,
        _LENGTH
    };
	
	constexpr const char* file_list[] = {
		"orange_grass.png",
        "player_float1.png",
        "player_float2.png",
        "player_hit.png",
        "player_jump.png",
        "wings.png",
        "wall.png",
        "bronze.png",
        "silver.png",
        "gold.png",
	};
}

#endif // SPRITE_ATLAS_HPP
