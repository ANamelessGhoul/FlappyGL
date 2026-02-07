
#include "quad_renderer.hpp"
#include <GLES/gl.h>
#include <glad/glad.h>
#include <cstddef>


using namespace Mln;
constexpr size_t MaxQuads = 2 * 1024;

constexpr unsigned int MaxVertices = 4 * MaxQuads;
constexpr unsigned int MaxIndices = 6 * MaxQuads;

#define GET_ATTRIBUTE_LOCATION(var) state.var = glGetAttribLocation(state.active_shader.id, #var)
#define GET_UNIFORM_LOCATION(var) state.var = glGetUniformLocation(state.active_shader.id, #var)


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

    Mln::Shader active_shader;
    Mln::Texture active_texture;

    GLuint aPos;
    GLuint aColor;
    GLuint aTexCoord;

    GLuint uTexture;
} state = {0};


void _CreateBuffers();
void _GetLocations();

void InitQuadRenderer()
{
    _CreateBuffers();
}

void ShutdownQuadRenderer()
{
    if (glDeleteVertexArrays)
    {
        glDeleteVertexArrays(1, &state.vao);
    }
    glDeleteBuffers(1, &state.vbo);
    glDeleteBuffers(1, &state.ebo);
}

void SetShader(Mln::Shader shader)
{
    if (state.active_shader.id == shader.id)
    {
        return;
    }

    DrawBatch();
    state.active_shader = shader;
    _GetLocations();
}

void SetTexture(Mln::Texture texture)
{
    if (state.active_texture.id == texture.id)
    {
        return;
    }

    DrawBatch();
    state.active_texture = texture;
}

void PushQuad(Quad quad)
{
    if (state.vertex_count + 4 >= MaxQuads)
    {
        DrawBatch();
    }

    Vertex* vertices = state.vertices + state.vertex_count;
    vertices[0].position = quad.vertices[0];
    vertices[1].position = quad.vertices[1];
    vertices[2].position = quad.vertices[2];
    vertices[3].position = quad.vertices[3];

    vertices[0].uv = quad.uvs[0];
    vertices[1].uv = quad.uvs[1];
    vertices[2].uv = quad.uvs[2];
    vertices[3].uv = quad.uvs[3];

    vertices[0].color = quad.color;
    vertices[1].color = quad.color;
    vertices[2].color = quad.color;
    vertices[3].color = quad.color;

    state.vertex_count += 4;
}


void DrawBatch()
{
    if (state.vertex_count == 0)
    {
        return;
    }

    glUseProgram(state.active_shader.id);
    glUniform1i(state.uTexture, 0);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, state.active_texture.id);


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



void _CreateBuffers()
{
    for (size_t i = 0; i < MaxQuads; i++)
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

void _GetLocations()
{
    GET_ATTRIBUTE_LOCATION(aPos);
    GET_ATTRIBUTE_LOCATION(aColor);
    GET_ATTRIBUTE_LOCATION(aTexCoord);

    GET_UNIFORM_LOCATION(uTexture);
}