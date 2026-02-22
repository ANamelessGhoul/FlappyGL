#ifndef GAME_HPP
#define GAME_HPP

#include "core.hpp"
#include "audio.hpp"
#include "melon_types.hpp"

namespace Game{
    constexpr int GAME_HEIGHT = 600;
    constexpr int GAME_WIDTH = 800;

    constexpr float GAP_HEIGHT = 128;
    constexpr float WALL_TILE_HEIGHT = 64 - 9;
    constexpr int WALL_TILE_COUNT = 10;
    constexpr int WALL_COUNT = 10;
    constexpr float WALL_SPEED_INITIAL = 120;
    constexpr float WALL_SEPARATION_INITIAL = 500;

    constexpr float PLAYER_ACCELERATION = 100;
    constexpr float PLAYER_JUMP_SPEED = 100;

    constexpr float WING_UP_ROTATION = 0.f * HMM_DegToRad; 
    constexpr float WING_DOWN_ROTATION = 60.f * HMM_DegToRad;
    constexpr float FLAP_TIME = 0.2f;

    constexpr Mln::Color WHITE = {1.f, 1.f, 1.f, 1.f};
    constexpr Mln::Color YELLOW = {253.f/255.f, 249.f/255.f, 0.f/255.f, 1.f};
    constexpr Mln::Color NO_COLOR = {0, 0, 0, 0};

    constexpr Mln::Color TEXT_COLOR = WHITE;

    struct Scene
    {
        void (*Init)(void);
        void (*Update)(float);
        void (*Draw)(void);
        void (*Unload)(void);
    };

    struct State{
        float game_time = 0;
        float game_scale = 1.f;

        const Scene* current_scene;

        Mln::Sound coin_sound;
        Mln::Sound hurt_sound;
        Mln::Sound jump_sound;

        Mln::Font font;
        Mln::Font pixel_font;

        Mln::Matrix view_matrix;

        //// GAME SCENE

        // Walls
        Mln::Vector2 walls[WALL_COUNT];
        int active_walls = 0;
        float wall_separation = WALL_SEPARATION_INITIAL;

        float wall_speed = 128;

        // Player
        Mln::Vector2 player_position;
        float player_speed;
        float player_rotation;

        float wing_rotation;
        float flap_timer;

        Mln::Vector2 wing_position;
        Mln::Vector2 wing_velocity;

        // Game State
        bool is_game_over;
        int score;
        int high_score;
        bool new_high_score;

        float death_time;

        // Background
        float background_scroll;

    };

    void Init();
    void Update(float delta);
    void Draw();
    void Unload();

    

    void DrawGap(Mln::Vector2 position);
}

#endif // GAME_HPP