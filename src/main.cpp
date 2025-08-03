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
    glfwSetTime(0);

    Render::InitRenderer();
    Render::SetClearColor({0.2f, 0.2f, 0.6f, 1.f});

    Vector2 viewportSize = Mln::GetViewportSize();
    Render::SetProjection(HMM_Orthographic_LH_NO(-SCR_WIDTH / 2, SCR_WIDTH / 2, SCR_HEIGHT / 2, -SCR_HEIGHT / 2, -1.f, 1.f));
    
    float horizontal_scale = viewportSize.X / SCR_WIDTH;
    float vertical_scale = viewportSize.Y / SCR_HEIGHT;
    float scale = HMM_MIN(horizontal_scale, vertical_scale);
    Render::SetView(HMM_Scale({scale, scale, 1.f}));

    double time = glfwGetTime();
    double timer = 0;
    constexpr int deltaCacheSize = 10;
    double deltaCache[deltaCacheSize];


    Game::Init();

    // render loop
    // -----------
    while (!Mln::WindowShouldClose())
    {
        double newTime = glfwGetTime();
        double delta = newTime - time;
        time = newTime;

        for (size_t i = 1; i < deltaCacheSize; i++)
        {
            deltaCache[i - 1] = deltaCache[i];
        }
        deltaCache[deltaCacheSize - 1] = delta;

        timer += delta;
        if (timer > 1)
        {
            timer -= 1;
            double average = 0;
            for (size_t i = 0; i < deltaCacheSize; i++)
            {
                average += delta;
            }
            average /= deltaCacheSize;            

            std::cout << 1/average << '\n';
        }

        Mln::BeginFrame();
        Game::Update(delta);

        Game::Draw();

        Render::DrawFrame();
        Mln::EndFrame();
    }

    // optional: de-allocate all resources once they've outlived their purpose:
    // ------------------------------------------------------------------------

    Render::UnloadRenderer();


    // glfw: terminate, clearing all previously allocated GLFW resources.
    // ------------------------------------------------------------------
    glfwTerminate();
    return 0;
}
