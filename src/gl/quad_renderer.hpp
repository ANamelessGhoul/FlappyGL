

#include "melon_types.hpp"

struct Quad{
    Mln::Vector2 vertices[4];
    Mln::Vector2 uvs[4];
    Mln::Color color;
};

void InitQuadRenderer();
void ShutdownQuadRenderer();

void SetShader(Mln::Shader shader);
void SetTexture(Mln::Texture texture);

void PushQuad(Quad quad);

void DrawBatch();



