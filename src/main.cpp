#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <cstring>


#include "stb_image.h"

#include "core.hpp"

#include "graphics/renderer.hpp"

#include "game.hpp"

// settings
const int SCR_WIDTH = 800;
const int SCR_HEIGHT = 600;

int main()
{
    Mln::InitWindow(SCR_WIDTH, SCR_HEIGHT, "Flappy Bird");

    Render::InitRenderer();
    Render::SetClearColor({0.2f, 0.2f, 0.6f, 1.f});

    Vector2 viewportSize = Mln::GetViewportSize();
    Render::SetProjection(HMM_Orthographic_LH_NO(-SCR_WIDTH / 2, SCR_WIDTH / 2, SCR_HEIGHT / 2, -SCR_HEIGHT / 2, -1.f, 1.f));
    
    float horizontal_scale = viewportSize.X / SCR_WIDTH;
    float vertical_scale = viewportSize.Y / SCR_HEIGHT;
    float scale = HMM_MIN(horizontal_scale, vertical_scale);
    Render::SetView(HMM_Scale({scale, scale, 1.f}));

    Game::Init();

    // render loop
    // -----------
    while (!Mln::WindowShouldClose())
    {
        Mln::BeginFrame();
        Game::Update(Mln::GetFrameTime());

        Game::Draw();

        Render::DrawFrame();
        Mln::EndFrame();
    }

    // optional: de-allocate all resources once they've outlived their purpose:
    // ------------------------------------------------------------------------

    Game::Unload();
    Render::UnloadRenderer();


    // glfw: terminate, clearing all previously allocated GLFW resources.
    // ------------------------------------------------------------------
    glfwTerminate();
    return 0;
}
