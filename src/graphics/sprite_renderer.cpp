#include "sprite_renderer.hpp"
#include <GLES/gl.h>
#include <glad/glad.h>
#include <cstddef>
#include <cassert>
#include <cstdio>
#include <iostream>

#include "core.hpp"

#if defined (OPENGL_ES)
    #include "gen/gles/default.fs.h"
    #include "gen/gles/default.vs.h"
#else
    #include "gen/gl/default.fs.h"
    #include "gen/gl/default.vs.h"
#endif
#include "graphics/draw_commands.hpp"

#define GET_ATTRIBUTE_LOCATION(var) state.var = glGetAttribLocation(state.shader, #var)
#define GET_UNIFORM_LOCATION(var) state.var = glGetUniformLocation(state.shader, #var)

namespace Render::Sprite
{
    void GetLocations();
    void CreateBuffers();

    constexpr unsigned int MaxVertices = 4 * MaxDrawCommands;
    constexpr unsigned int MaxIndices = 6 * MaxDrawCommands;

    #pragma pack(push, 1)
    struct Vertex{
        Vector2 position;
        Color color;
        Vector2 uv;
    };
    #pragma pack(pop)

    struct {
        Vertex vertices[MaxVertices];
        unsigned int indices[MaxIndices];
        size_t vertex_count;

        GLuint vao;
        GLuint vbo;
        GLuint ebo;

        GLuint shader;
        Mln::Texture atlas_texture;

        GLuint aPos;
        GLuint aColor;
        GLuint aTexCoord;

        GLuint uTexture;
    } state;


    void Load(Mln::Image atlas_image)
    {
        state.shader = Mln::LoadShader(default_vs, default_fs).id;

        state.atlas_texture = Mln::LoadTextureFromImage(atlas_image, true, true);
        assert(state.atlas_texture.id != -1 && "Cannot load sprite renderer with invalid texture");

        GetLocations();
        CreateBuffers();
    }

    void Unload()
    {
        if (glDeleteVertexArrays)
        {
            glDeleteVertexArrays(1, &state.vao);
        }
        glDeleteBuffers(1, &state.vbo);
        glDeleteBuffers(1, &state.ebo);

        glDeleteProgram(state.shader);

        Mln::UnloadTexture(state.atlas_texture);
    }

    void DrawCommands(DrawCommand* commands, size_t count)
    {
        for (size_t i = 0; i < count; i++)
        {
            const SpriteCommand& spriteCommand = commands[i].sprite;
            Vertex* vertices = state.vertices + state.vertex_count;
            vertices[0].position = spriteCommand.vertices[0];
            vertices[1].position = spriteCommand.vertices[1];
            vertices[2].position = spriteCommand.vertices[2];
            vertices[3].position = spriteCommand.vertices[3];

            vertices[0].uv = spriteCommand.uvs[0];
            vertices[1].uv = spriteCommand.uvs[1];
            vertices[2].uv = spriteCommand.uvs[2];
            vertices[3].uv = spriteCommand.uvs[3];

            vertices[0].color = spriteCommand.color;
            vertices[1].color = spriteCommand.color;
            vertices[2].color = spriteCommand.color;
            vertices[3].color = spriteCommand.color;

            state.vertex_count += 4;
        }


        if (state.vertex_count == 0)
        {
            return;
        }

        glUseProgram(state.shader);
        glUniform1i(state.uTexture, 1);

        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, state.atlas_texture.id);


        if (glBindVertexArray)
        {
            glBindVertexArray(state.vao);
        }

        glBindBuffer(GL_ARRAY_BUFFER, state.vbo);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(Vertex) * state.vertex_count, state.vertices);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, state.ebo);

        glVertexAttribPointer(state.aPos, sizeof(Vertex::position) / sizeof(float), GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, position));
        glEnableVertexAttribArray(state.aPos);

        glVertexAttribPointer(state.aColor, sizeof(Vertex::color) / sizeof(float), GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, color));
        glEnableVertexAttribArray(state.aColor);

        glVertexAttribPointer(state.aTexCoord, sizeof(Vertex::uv) / sizeof(float), GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, uv));
        glEnableVertexAttribArray(state.aTexCoord);

        glDrawElements(GL_TRIANGLES, 6 * (state.vertex_count / 4), GL_UNSIGNED_INT, 0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindTexture(GL_TEXTURE_2D, 0);


        state.vertex_count = 0;
    }

    void GetLocations()
    {
        GET_ATTRIBUTE_LOCATION(aPos);
        GET_ATTRIBUTE_LOCATION(aColor);
        GET_ATTRIBUTE_LOCATION(aTexCoord);

        GET_UNIFORM_LOCATION(uTexture);
    }

    void CreateBuffers()
    {
        for (size_t i = 0; i < MaxDrawCommands; i++)
        {
            state.indices[i * 6 + 0] = i * 4 + 0;
            state.indices[i * 6 + 1] = i * 4 + 1;
            state.indices[i * 6 + 2] = i * 4 + 3;
            state.indices[i * 6 + 3] = i * 4 + 1;
            state.indices[i * 6 + 4] = i * 4 + 2;
            state.indices[i * 6 + 5] = i * 4 + 3;
        }

        if (glGenVertexArrays)
        {
            glGenVertexArrays(1, &state.vao);
        }
        glGenBuffers(1, &state.vbo);
        glGenBuffers(1, &state.ebo);
        // bind the Vertex Array Object first, then bind and set vertex buffer(s), and then configure vertex attributes(s).
        if (glBindVertexArray)
        {
            glBindVertexArray(state.vao);
        }

        glBindBuffer(GL_ARRAY_BUFFER, state.vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * MaxVertices, state.vertices, GL_DYNAMIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, state.ebo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(*state.indices) * MaxIndices, state.indices, GL_STATIC_DRAW);



        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    }

} // namespace Render::Sprite
