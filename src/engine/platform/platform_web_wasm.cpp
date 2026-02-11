#include "platform_api.hpp"
#include "core_data.hpp"

#include "graphics_api.hpp"



using namespace Mln;

namespace Mln
{
    extern CoreData gCore;
}

struct PlatformData
{
    int dummy;
} gPlatform;


// These will be imported from js-land
extern "C" {
    bool JsIsKeyDown(int glfw_key);
    bool JsIsMouseButtonDown(int glfw_button);
    void JsClearInput();
    float JsGetCanvasWidth();
    float JsGetCanvasHeight();
}


void PlatformInit()
{
}

void PlatformBeginFrame()
{
    float new_width = JsGetCanvasWidth();
    if (new_width != gCore.viewport.width)
    {
        gCore.viewport.width = new_width;
        gCore.windowResized = true;
    }
    float new_height = JsGetCanvasHeight();
    if (new_height != gCore.viewport.height)
    {
        gCore.viewport.height = new_height;
        gCore.windowResized = true;
    }

}

void PlatformEndFrame()
{

}

void PlatformInitTimer()
{

}

void PlatformPollInput()
{
    for (int key = 0; key < KEY__COUNT; key++) 
    {
        gCore.input.current.keys[key] = JsIsKeyDown(key);
    }

    for (int button = 0; button < MOUSE_BUTTON__COUNT; button++) 
    {
        gCore.input.current.mouse_buttons[button] = JsIsMouseButtonDown(button);
    }

    JsClearInput();
}

void BeginDrawing()
{

}

void EndDrawing()
{

}