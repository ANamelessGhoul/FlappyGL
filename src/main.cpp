#include "core.hpp"
#include "graphics/renderer.hpp"
#include "game.hpp"

// settings
const int SCR_WIDTH = 800;
const int SCR_HEIGHT = 600;

#if defined (PLATFORM_WEB)
#include <emscripten.h>
#endif

void MainLoop()
{
    Mln::BeginFrame();
    Game::Update(Mln::GetFrameTime());

    Game::Draw();

    Render::DrawFrame();
    Mln::EndFrame();
}

int main()
{
    Mln::InitWindow(SCR_WIDTH, SCR_HEIGHT, "Flappy Bird");
    
    Render::InitRenderer();
    Render::SetClearColor({0.2f, 0.2f, 0.6f, 1.f});
    
    
    Game::Init();
    
#if defined(PLATFORM_WEB)
    emscripten_set_main_loop(MainLoop, 0, true);
#else
    // render loop
    // -----------
    while (!Mln::WindowShouldClose())
    {
        MainLoop();
    }
#endif
    

    Game::Unload();
    Render::UnloadRenderer();

    Mln::UnloadWindow();

    return 0;
}
