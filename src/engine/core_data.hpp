#pragma once

#include <cstddef>
#ifndef CORE_DATA_HPP
#define CORE_DATA_HPP

#include "melon_types.hpp"
#include "keys.h"

#ifndef TEXT_BUFFER_SIZE
    #define TEXT_BUFFER_SIZE 1024
#endif

namespace Mln
{

    constexpr int deltaCacheSize = 16;
    static_assert((deltaCacheSize & (deltaCacheSize - 1)) == 0, "deltaCacheSize must be a power of 2");
    
    struct InputState{
        bool keys[KEY__COUNT];
        bool mouse_buttons[MOUSE_BUTTON__COUNT];
    };
    
    struct CoreData {
        bool shouldClose = false;
        bool windowResized = false;
        const char* windowTitle = nullptr;
        struct{
            int width;
            int height;
        } viewport;
        
        double time;
        double delta;
        double deltaCache[deltaCacheSize];
        int deltaCacheIndex = 0;
        int deltaCacheFrameCounter = 0;
        
        struct{
            InputState current;
            InputState previous;
        } input;

        char textBuffer[TEXT_BUFFER_SIZE];
    
    };
}

#endif // CORE_DATA_HPP