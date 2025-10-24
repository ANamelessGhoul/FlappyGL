cmake_minimum_required(VERSION 3.13)

set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Release>:Release>")


project(game)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(EXECUTABLE_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/bin)

file(GLOB_RECURSE GAME_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp" "${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/glad/*.c")

add_executable("${CMAKE_PROJECT_NAME}" "${GAME_SOURCES}")
if ("${CMAKE_BUILD_TYPE}" STREQUAL "Debug")
    target_compile_definitions("${CMAKE_PROJECT_NAME}" PUBLIC "_DEBUG")
    target_compile_definitions("${CMAKE_PROJECT_NAME}" PUBLIC RESOURCES_PATH="${CMAKE_CURRENT_SOURCE_DIR}/resources/") # This is useful to get an ASSETS_PATH in your IDE during development but you should comment this if you compile a release version and uncomment the next line
else()
    target_compile_definitions("${CMAKE_PROJECT_NAME}" PUBLIC RESOURCES_PATH="./resources/")
    
    if (WIN32)
        target_link_options("${CMAKE_PROJECT_NAME}" PRIVATE -mwindows)
    endif()
endif()

if (EMSCRIPTEN)
    target_compile_definitions("${CMAKE_PROJECT_NAME}" PUBLIC "PLATFORM_WEB") 
    target_compile_definitions("${CMAKE_PROJECT_NAME}" PUBLIC "OPENGL_ES") 

    if ("${CMAKE_BUILD_TYPE}" STREQUAL "Debug")
        set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -sSTACK_SIZE=1048576 -s INITIAL_MEMORY=536870912 -s ASSERTIONS=2 -s GL_ASSERTIONS=1 -s GL_DEBUG=1 -s USE_GLFW=3 -s WASM=1 --embed-file ${CMAKE_SOURCE_DIR}/resources")
    else ()
        set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -sSTACK_SIZE=1048576 -s INITIAL_MEMORY=536870912 -s USE_GLFW=3 -s WASM=1 --embed-file ${CMAKE_SOURCE_DIR}/resources@/resources --shell-file ${CMAKE_SOURCE_DIR}/src/shell.html")
    endif()

    set(CMAKE_EXECUTABLE_SUFFIX ".html") # This line is used to set your executable to build with the emscripten html template so that you can directly open it.
else()
    add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/glfw")
endif()

target_include_directories("${CMAKE_PROJECT_NAME}" PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/glfw/include")
target_include_directories("${CMAKE_PROJECT_NAME}" PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/glad/include")
target_include_directories("${CMAKE_PROJECT_NAME}" PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/thirdparty")
target_include_directories("${CMAKE_PROJECT_NAME}" PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/src")
target_include_directories("${CMAKE_PROJECT_NAME}" PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/src/engine")


target_link_libraries("${CMAKE_PROJECT_NAME}" PUBLIC glfw)

