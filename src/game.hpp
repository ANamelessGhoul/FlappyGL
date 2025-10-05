#ifndef GAME_HPP
#define GAME_HPP

#include "core.hpp"

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

    constexpr Color WHITE = {1.f, 1.f, 1.f, 1.f};

    constexpr Color TEXT_COLOR = WHITE;

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

        //// GAME SCENE

        // Walls
        Vector2 walls[WALL_COUNT];
        int active_walls = 0;
        float wall_separation = WALL_SEPARATION_INITIAL;

        float wall_speed = 128;

        // Player
        Vector2 player_position = {0, 0};
        float player_speed = 0;
        float player_rotation = 0;

        float wing_rotation = 0;
        float flap_timer = 0;

        Vector2 wing_position = {0, 0};
        Vector2 wing_velocity = {0, 0};

        // Game State
        bool is_game_over = false;
        int score = 0;
        float death_time;

        // Background
        float background_scroll = 0.f;

    };

    void Init();
    void Update(float delta);
    void Draw();
    void Unload();

    

    void DrawGap(Vector2 position);
}

#endif // GAME_HPP