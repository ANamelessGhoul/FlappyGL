#pragma once
#ifndef CONFIG_HPP
#define CONFIG_HPP


// TODO: Use these
#if defined(_DEBUG)
    #define GENERATE_SPRITE_ATLAS
    #define GENERATE_FONT_ATLAS

    
#endif // DEBUG_MODE

#if defined (PLATFORM_DESKTOP)
    #define FEATHER_SPRITE_ATLAS
#endif 

#ifndef ASSERT
    #if defined(_DEBUG)
        #include <assert.h>
        #define ASSERT_C(cond) assert(cond)
        #define ASSERT(cond, msg) ASSERT_C(cond && msg)
    #else
        #define ASSERT_C(cond)
        #define ASSERT(cond, msg)
    #endif
#endif

#endif // CONFIG_HPP