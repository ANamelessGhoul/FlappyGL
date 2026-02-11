#include "core.hpp"
#include "game/game.hpp"

// settings
const int SCR_WIDTH = 800;
const int SCR_HEIGHT = 600;

#if defined(EMSCRIPTEN)
#include <emscripten.h>
#elif defined(PLATFORM_WEB_WASM)
extern "C" void WasmSetMainLoop(void (*func)(void));
extern "C" void __wasm_call_ctors();
#endif

extern "C"
{
    void MainLoop();
}

void MainLoop()
{
    Mln::BeginFrame();

    Game::Update(Mln::GetFrameTime());

    Game::Draw();

    Mln::EndFrame();
}

int main()
{
#if defined (PLATFORM_WEB_WASM)
    // NOTE: If we don't call this function the linker adds it to every exported function and calls constructors on all static objects
    __wasm_call_ctors();
#endif 

    Mln::InitWindow(SCR_WIDTH, SCR_HEIGHT, "Flappy Bird");
    
    Game::Init();
    
    
#if defined(PLATFORM_WEB) && defined(EMSCRIPTEN)
    emscripten_set_main_loop(MainLoop, 0, true);
#elif defined(PLATFORM_WEB_WASM)
    WasmSetMainLoop(MainLoop);
    return 0;
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
