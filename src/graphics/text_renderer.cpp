#include "text_renderer.hpp"

#include "stb_rect_pack.h"
#define STB_TRUETYPE_IMPLEMENTATION
#include "stb_truetype.h"

#include "core.hpp"
#include <cstdio>
#include <cstddef>


#include <glad/glad.h>

#if defined(OPENGL_ES)
    #include "gen/gles/text.fs.h"
    #include "gen/gles/text.vs.h"
#else
    #include "gen/gl/text.fs.h"
    #include "gen/gl/text.vs.h"
#endif

#define GET_ATTRIBUTE_LOCATION(var) state.var = glGetAttribLocation(state.shader, #var)
#define GET_UNIFORM_LOCATION(var) state.var = glGetUniformLocation(state.shader, #var)

unsigned char ttf_buffer[1<<20];
stbtt_packedchar packed_chars[256];
unsigned char image[512*512];

namespace Render::Text
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


    void Load(const char *font_file)
    {
        stbtt_pack_context ctx;

        Mln::Image font_atlas_image = Mln::CreateImage(512, 512, 1);

        stbtt_PackBegin(&ctx, image, 512, 512, 0, 2, nullptr);
        
        // stbtt_PackSetOversampling(&ctx, 0, 0);
        
        FILE* file = fopen(font_file, "rb");
        fread(ttf_buffer, 1, 1<<20, file);
        fclose(file);
        
        stbtt_PackFontRange(&ctx, ttf_buffer, 0, 48, 0, 256, packed_chars);
        stbtt_PackEnd(&ctx);
        
        for (size_t i = 0; i < 512*512; i++)
        {
            font_atlas_image.data[i] = image[i];
        }

#if defined(GENERATE_FONT_ATLAS)
        Mln::WriteImage(font_atlas_image, "font.png");
#endif
        
        state.shader = Mln::LoadShader(text_vs, text_fs).id;
        
        state.atlas_texture = Mln::LoadTextureFromImage(font_atlas_image, true, true);
        assert(state.atlas_texture.id != -1 && "Could not load font texture!");

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
            const TextCommand& textCommand = commands[i].text;
            int length = strlen(textCommand.text);

            float x = 0;
            float y = 0;
            for (size_t i = 0; i < length; i++)
            {
                char character = textCommand.text[i];
                stbtt_aligned_quad quad;
                stbtt_GetPackedQuad(packed_chars, 512, 512, character, &x, &y, &quad, 0);
                
                Vertex* vertices = state.vertices + state.vertex_count;
                vertices[0].position = (textCommand.transform * HMM_Vec4{quad.x1, quad.y0, 0, 1}).XY;
                vertices[1].position = (textCommand.transform * HMM_Vec4{quad.x1, quad.y1, 0, 1}).XY;
                vertices[2].position = (textCommand.transform * HMM_Vec4{quad.x0, quad.y1, 0, 1}).XY;
                vertices[3].position = (textCommand.transform * HMM_Vec4{quad.x0, quad.y0, 0, 1}).XY;
                
                vertices[0].uv = {quad.s1, quad.t0};
                vertices[1].uv = {quad.s1, quad.t1};
                vertices[2].uv = {quad.s0, quad.t1};
                vertices[3].uv = {quad.s0, quad.t0};
                
                vertices[0].color = textCommand.color;
                vertices[1].color = textCommand.color;
                vertices[2].color = textCommand.color;
                vertices[3].color = textCommand.color;
                
                state.vertex_count += 4;
            }
        }


        if (state.vertex_count == 0)
        {
            return;
        }

        glUseProgram(state.shader);
        glUniform1i(state.uTexture, 2);

        glActiveTexture(GL_TEXTURE2);
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
        glUseProgram(0);

        state.vertex_count = 0;
    }

    float MeasureText(const char* text)
    {
        int length = strlen(text);

        float x = 0;
        float y = 0;
        for (size_t i = 0; i < length; i++)
        {
            float x_advance = 0, y_advance = 0;
            char character = text[i];
            stbtt_aligned_quad quad;
            stbtt_GetPackedQuad(packed_chars, 512, 512, character, &x_advance, &y_advance, &quad, 0);

            if (i == length - 1)
            {
                x += quad.x1;
            }
            else
            {
                x += x_advance;
            }
        }

        return x;
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



}