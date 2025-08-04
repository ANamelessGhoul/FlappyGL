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


    Game::Unload();
    Render::UnloadRenderer();

    Mln::UnloadWindow();

    return 0;
}
