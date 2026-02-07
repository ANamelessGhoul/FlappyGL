#include "game.hpp"
#include "audio.hpp"
#include "core.hpp"
#include "keys.h"
#include <cmath>
#include <cstdio>

#include "graphics_api.hpp"
#include "flappy_drawing.hpp"

using namespace Mln;

Game::State state;

// TODO: Organize all rand functions and don't use rand()

// [-1, 1]
float rand_flt()
{
    return (rand() / (float)(RAND_MAX)) * 2 - 1;
}

float rand_flt01()
{
    return (rand() / (float)(RAND_MAX));
}

float Lerp(float a, float b, float t)
{
    return a + (b - a) * t;
}

float Damp(float a, float b, float smoothing, float delta)
{
    return Lerp(a, b, 1 - powf(smoothing, delta));
}

float LerpAngle(float a, float b, float t) 
{
    float TAU = HMM_PI32 * 2;
    float difference = fmodf(b - a, TAU);
    float shortest = fmodf(2.0f * difference, TAU) - difference;
    return a + shortest * t;
}

float DampAngle(float a, float b, float smoothing, float delta)
{
    return LerpAngle(a, b, 1 - powf(smoothing, delta));
}


namespace Game
{
    void TriggerGameOver();

    void ChangeSceneTo(const Scene* scene);

    void InitSceneMainMenu();
    void UpdateSceneMainMenu(float delta);
    void DrawSceneMainMenu();
    void UnloadSceneMainMenu();

    void InitSceneGame();
    void UpdateSceneGame(float delta);
    void DrawSceneGame();
    void UnloadSceneGame();

    constexpr Scene MainMenuScene = {
        InitSceneMainMenu,
        UpdateSceneMainMenu,
        DrawSceneMainMenu,
        UnloadSceneMainMenu
    };
    constexpr Scene GameScene = {
        InitSceneGame,
        UpdateSceneGame,
        DrawSceneGame,
        UnloadSceneGame
    };
} // namespace Game


void Game::Init()
{
    state.game_time = 0;
    state.game_scale = 1.f;

    Vector2 viewportSize = Mln::GetViewportSize();
    SetProjection(HMM_Orthographic_LH_NO(-viewportSize.X / 2, viewportSize.X / 2, viewportSize.Y / 2, -viewportSize.Y / 2, -1.f, 1.f));
    
    float horizontal_scale = viewportSize.X / GAME_WIDTH;
    float vertical_scale = viewportSize.Y / GAME_HEIGHT;
    float scale = HMM_MIN(horizontal_scale, vertical_scale);
    SetView(HMM_Scale({scale, scale, 1.f}));

    state.current_scene = &MainMenuScene;
    state.current_scene->Init();




    state.coin_sound = Mln::LoadSoundFromFileWave(RESOURCES_PATH "coin.wav");
    state.hurt_sound = Mln::LoadSoundFromFileWave(RESOURCES_PATH "hurt.wav");
    state.jump_sound = Mln::LoadSoundFromFileWave(RESOURCES_PATH "jump.wav");


    state.pixel_font = LoadFont(RESOURCES_PATH "Kenney Pixel.ttf");
    UnloadFont(state.pixel_font);
    state.font = LoadFont(RESOURCES_PATH "Kenney Future Narrow.ttf");
    state.pixel_font = LoadFont(RESOURCES_PATH "Kenney Pixel.ttf");
    
    LoadSpriteAtlas();
}

void Game::Update(float delta)
{
    // Screen scaling
    if(Mln::DidWindowResize())
    {
        Vector2 viewportSize = Mln::GetViewportSize();
        SetProjection(HMM_Orthographic_LH_NO(-viewportSize.X / 2, viewportSize.X / 2, viewportSize.Y / 2, -viewportSize.Y / 2, -1.f, 1.f));
        float horizontal_scale = viewportSize.X / GAME_WIDTH;
        float vertical_scale = viewportSize.Y / GAME_HEIGHT;
        state.game_scale = HMM_MIN(horizontal_scale, vertical_scale);
        SetView(HMM_Scale({state.game_scale, state.game_scale, 1.f}));
    }

    state.current_scene->Update(delta);

    state.game_time += delta;
}

void Game::Draw()
{
    ClearBackground({0.2f, 0.2f, 0.6f, 1.f});
    state.current_scene->Draw();
}

void Game::Unload()
{
    state.current_scene->Unload();

    UnloadFont(state.font);
    UnloadFont(state.pixel_font);
    UnloadSpriteAtlas();
}

void Game::DrawGap(Vector2 position)
{
    float bottom_border = position.Y + GAP_HEIGHT * 0.5f;
    float top_border = position.Y - GAP_HEIGHT * 0.5f;
    
    Vector2 spriteSize = GetSpriteSize(SpriteAtlas::WALL);
    
    DrawSprite(Mln::Transform2D{{position.X, bottom_border + spriteSize.Y / 2.f}, {1.f, 1.f}, 0}, {0, 0, 0, 0}, SpriteAtlas::WALL);
    DrawSprite(Mln::Transform2D{{position.X, top_border - spriteSize.Y / 2.f}, {1.f, 1.f}, HMM_PI32}, {0, 0, 0, 0}, SpriteAtlas::WALL);
}



void Game::TriggerGameOver()
{
    Mln::PlaySound(state.hurt_sound);

    state.is_game_over = true;
    state.player_speed = -PLAYER_JUMP_SPEED;
    state.wing_rotation = 0;
    state.wing_position = state.player_position;
    state.wing_velocity = {-rand_flt01() * 100.f, -rand_flt01() * 100.f};
    state.death_time = state.game_time;
}

void Game::ChangeSceneTo(const Scene *scene)
{
    state.current_scene->Unload();
    state.current_scene = scene;
    state.current_scene->Init();
}

void Game::InitSceneMainMenu()
{
    state.active_walls = 0;
    state.wall_speed = WALL_SPEED_INITIAL;
    state.wall_separation = WALL_SEPARATION_INITIAL;

    state.background_scroll = 0.f;

}

void Game::UpdateSceneMainMenu(float delta)
{
    // Menu logic
    if (Mln::IsKeyJustPressed(KEY_SPACE) || Mln::IsMouseButtonJustPressed(MOUSE_BUTTON_LEFT))
    {
        ChangeSceneTo(&GameScene);
    }

    // Move walls
    int rightmost_wall = -1;
    float rightmost_wall_pos = (Mln::GetViewportSize().X / state.game_scale) * 0.5f;
    for (int i = 0; i < state.active_walls; i++)
    {
        state.walls[i].X -= delta * state.wall_speed;

        // Find rightmost wall for spawning new walls
        if (rightmost_wall == -1 || state.walls[i].X > rightmost_wall_pos)
        {
            rightmost_wall = i;
            rightmost_wall_pos = state.walls[i].X;
        }

        // Remove walls past screen
        if (state.walls[i].X < - (Mln::GetViewportSize().X / state.game_scale) * 0.5f - 100)
        {
            state.walls[i] = state.walls[state.active_walls - 1];
            state.active_walls -= 1;
            i -= 1;
        }
    }

    if (rightmost_wall_pos < (Mln::GetViewportSize().X / state.game_scale) * 0.5f + state.wall_separation && state.active_walls < WALL_COUNT)
    {
        state.walls[state.active_walls].X = rightmost_wall_pos + state.wall_separation;
        state.walls[state.active_walls].Y = rand_flt() * (GAME_HEIGHT - GAP_HEIGHT) * 0.5f;
        state.active_walls += 1;
    }


}

void Game::DrawSceneMainMenu()
{
    float player_ratio = sinf(state.game_time * 0.1f);
    float background_size = floorf(GAME_HEIGHT * 1.2f);

    DrawSprite({-background_size + 1 + state.background_scroll, -player_ratio * GAME_HEIGHT * 0.1f}, {background_size, background_size}, {0, 0, 0, 0}, static_cast<SpriteAtlas::Sprite>(SpriteAtlas::BACKGROUND_1));
    DrawSprite({state.background_scroll, -player_ratio * GAME_HEIGHT * 0.1f}, {background_size, background_size}, {0, 0, 0, 0}, static_cast<SpriteAtlas::Sprite>(SpriteAtlas::BACKGROUND_1));
    DrawSprite({background_size - 1 + state.background_scroll, -player_ratio * GAME_HEIGHT * 0.1f}, {background_size, background_size}, {0, 0, 0, 0}, static_cast<SpriteAtlas::Sprite>(SpriteAtlas::BACKGROUND_1));

    // for (int i = 0; i < state.active_walls; i++)
    // {
    //     DrawGap(state.walls[i]);
    // }


    DrawText(state.font, "FLAPPY ALIEN", {0, GAME_HEIGHT * -0.5f + 100}, 1.2f, TEXT_COLOR, TEXT_ALIGN_CENTER);
    DrawText(state.font, "Press SPACE to start", {0, 0}, 1.0f, {TEXT_COLOR.RGB, 0.75f}, TEXT_ALIGN_CENTER);
}

void Game::UnloadSceneMainMenu()
{
}

void Game::InitSceneGame()
{
    state.active_walls = 0;
    state.wall_speed = WALL_SPEED_INITIAL;
    state.wall_separation = WALL_SEPARATION_INITIAL;
    
    state.player_position = {0, 0};
    state.player_speed = 0;
    state.player_rotation = 0;
    
    state.wing_rotation = WING_UP_ROTATION;
    state.flap_timer = 0;
    
    state.is_game_over = false;
    state.score = 0;
    state.death_time = 0;

    state.background_scroll = 0.f;
}

void Game::UpdateSceneGame(float delta)
{
    
    // Walls
    if (!state.is_game_over)
    {
        int rightmost_wall = -1;
        float rightmost_wall_pos = (Mln::GetViewportSize().X / state.game_scale) * 0.5f;
        for (int i = 0; i < state.active_walls; i++)
        {
            // Check for passing gap
            bool wasAheadOfPlayer = state.walls[i].X > state.player_position.X;
            state.walls[i].X -= delta * state.wall_speed;
            bool isBehindPlayer = state.walls[i].X <= state.player_position.X;
            if (wasAheadOfPlayer && isBehindPlayer)
            {
                state.score += 1;
                Mln::PlaySound(state.coin_sound);
            }

            // Find rightmost wall for spawning new walls
            if (rightmost_wall == -1 || state.walls[i].X > rightmost_wall_pos)
            {
                rightmost_wall = i;
                rightmost_wall_pos = state.walls[i].X;
            }

            // Remove walls past screen
            if (state.walls[i].X < - (Mln::GetViewportSize().X / state.game_scale) * 0.5f - 100)
            {
                state.walls[i] = state.walls[state.active_walls - 1];
                state.active_walls -= 1;
                i -= 1;
            }
        }

        if (rightmost_wall_pos < (Mln::GetViewportSize().X / state.game_scale) * 0.5f + state.wall_separation && state.active_walls < WALL_COUNT)
        {
            state.walls[state.active_walls].X = rightmost_wall_pos + state.wall_separation;
            state.walls[state.active_walls].Y = rand_flt() * (GAME_HEIGHT - GAP_HEIGHT) * 0.5f;
            state.active_walls += 1;
        }
        
    }


    // Player
    if (!state.is_game_over)
    {
        if (state.player_position.Y < -GAME_HEIGHT / 2.f || state.player_position.Y > GAME_HEIGHT / 2.f)
        {
            TriggerGameOver();
        }

        for (int i = 0; i < state.active_walls; i++)
        {
            Vector2 wallPosition = state.walls[i];
            Vector2 spriteSize = GetSpriteSize(SpriteAtlas::WALL);
            float left_border = wallPosition.X - spriteSize.X * 0.5f;
            float right_border = wallPosition.X + spriteSize.X * 0.5f;
            float bottom_border = wallPosition.Y + GAP_HEIGHT * 0.5f;
            float top_border = wallPosition.Y - GAP_HEIGHT * 0.5f;
            
            if (state.player_position.X > left_border && state.player_position.X < right_border && (state.player_position.Y > bottom_border || state.player_position.Y < top_border))
            {
                TriggerGameOver();
            }
        }
        

        // Jump
        if (!state.is_game_over && (Mln::IsKeyJustPressed(KEY_SPACE) || Mln::IsMouseButtonJustPressed(MOUSE_BUTTON_LEFT)))
        {
            state.player_speed -= PLAYER_JUMP_SPEED; 
            state.flap_timer = Game::FLAP_TIME;
            Mln::PlaySound(state.jump_sound);
        }
        
    }
    
    // Movement
    state.player_speed += delta * 0.5f * PLAYER_ACCELERATION;
    state.player_position.Y += delta * state.player_speed;
    state.player_speed += delta * 0.5f * PLAYER_ACCELERATION;


    
    // Animation
    Vector2 velocity = {state.player_speed, state.wall_speed};
    float desired_rotation = atan2f(velocity.X, -velocity.Y) + HMM_PI32;
    state.player_rotation = DampAngle(state.player_rotation, desired_rotation, 0.05f, delta);

    state.flap_timer -= delta;
    if (!state.is_game_over)
    {
        state.wing_rotation = DampAngle(state.wing_rotation, state.flap_timer <= 0 ? WING_UP_ROTATION : WING_DOWN_ROTATION, 0.005f, delta);
    }
    else
    {
        state.wing_velocity.X = Damp(state.wing_velocity.X, 0, 0.05f, delta);

        state.wing_velocity.Y += delta * 0.5f * PLAYER_ACCELERATION;
        state.wing_position += state.wing_velocity * delta;
        state.wing_velocity.Y += delta * 0.5f * PLAYER_ACCELERATION;

        state.wing_rotation += HMM_PI32 * delta * 0.1f;
    }


    // Background
    if (!state.is_game_over)
    {
        float background_size = floorf(GAME_HEIGHT * 1.2f);
        state.background_scroll -= delta * state.wall_speed * 0.1f;
        if (state.background_scroll < -background_size * 0.5f)
        {
            state.background_scroll += background_size;
        }
    }


    if (state.is_game_over && state.game_time > state.death_time + 5.f)
    {
        ChangeSceneTo(&GameScene);
    }
}

void Game::DrawSceneGame()
{
    
    float player_ratio = HMM_Clamp(-1, state.player_position.Y / (GAME_HEIGHT * 0.5f), 1);
    float background_size = floorf(GAME_HEIGHT * 1.2f);

    DrawSprite({-background_size + 1 + state.background_scroll, -player_ratio * GAME_HEIGHT * 0.1f}, {background_size, background_size}, {0, 0, 0, 0}, static_cast<SpriteAtlas::Sprite>(SpriteAtlas::BACKGROUND_1));
    DrawSprite({state.background_scroll, -player_ratio * GAME_HEIGHT * 0.1f}, {background_size, background_size}, {0, 0, 0, 0}, static_cast<SpriteAtlas::Sprite>(SpriteAtlas::BACKGROUND_1));
    DrawSprite({background_size - 1 + state.background_scroll, -player_ratio * GAME_HEIGHT * 0.1f}, {background_size, background_size}, {0, 0, 0, 0}, static_cast<SpriteAtlas::Sprite>(SpriteAtlas::BACKGROUND_1));



    for (int i = 0; i < state.active_walls; i++)
    {
        DrawGap(state.walls[i]);
    }
    
    
    Transform playerMatrix = Mln::GetMatrix(Mln::Transform2D{state.player_position, {.5f, .5f}, state.player_rotation});
    Transform wingMatrix = Mln::GetMatrix(Mln::Transform2D{state.player_position, {.5f, .5f}, state.player_rotation + state.wing_rotation}) * Mln::GetMatrix(Mln::Transform2D{Vector2{-50.f , 2.f}, {1.2f, 1.2f}, 0});
    if (state.is_game_over)
    {
        DrawSprite(Mln::Transform2D{state.wing_position, {.5f, .5f}, state.wing_rotation}, {0, 0, 0, 0}, static_cast<SpriteAtlas::Sprite>(SpriteAtlas::WINGS));
    }
    else
    {
        DrawSprite(wingMatrix, {0, 0, 0, 0}, static_cast<SpriteAtlas::Sprite>(SpriteAtlas::WINGS));
    }
    int frame = static_cast<int>(state.game_time * 2) % 2;
    SpriteAtlas::Sprite playerSprite = static_cast<SpriteAtlas::Sprite>(SpriteAtlas::PLAYER_FLOAT_1 + frame);
    if (state.is_game_over)
    {
        playerSprite = SpriteAtlas::PLAYER_HIT;
    }
    DrawSprite(playerMatrix, {0, 0, 0, 0}, static_cast<SpriteAtlas::Sprite>(playerSprite));


    
    char buffer[64];
    std::snprintf(buffer, 64, "%d", state.score);
    DrawText(state.font, buffer, {0, -GAME_HEIGHT / 2.0f + 48}, 1.f, TEXT_COLOR, TEXT_ALIGN_CENTER);

    // std::snprintf(buffer, 64, "%02.f", floorf(state.game_time / 60.f));
    // DrawText(state.font, buffer, {-10, -GAME_HEIGHT / 2.0f + 48}, 1.f, TEXT_COLOR, TEXT_ALIGN_RIGHT);
    // DrawText(state.font, ":", {0, -GAME_HEIGHT / 2.0f + 48}, 1.f, TEXT_COLOR, TEXT_ALIGN_CENTER);
    // std::snprintf(buffer, 64, "%02.f", floorf(fmodf(state.game_time, 60.f)));
    // DrawText(state.font, buffer, {10, -GAME_HEIGHT / 2.0f + 48}, 1.f, TEXT_COLOR, TEXT_ALIGN_LEFT);


    if (state.is_game_over)
    {
        DrawText(state.font, "Game Over!", {0, 0}, 1.f, TEXT_COLOR, TEXT_ALIGN_CENTER);
    }
}

void Game::UnloadSceneGame()
{
}

