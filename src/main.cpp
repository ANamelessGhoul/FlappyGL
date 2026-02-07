#include "core.hpp"
#include "game/game.hpp"

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

    Mln::EndFrame();
}

int main()
{
    Mln::InitWindow(SCR_WIDTH, SCR_HEIGHT, "Flappy Bird");
    
    
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

    Mln::UnloadWindow();

    return 0;
}
