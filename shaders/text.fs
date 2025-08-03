#version 330 core
out vec4 FragColor;

in vec4 color;
in vec2 uv;

uniform sampler2D uTexture;

void main()
{
    vec4 uvColor = vec4(uv.x, uv.y, 0, 1.0);
    vec4 textureColor = texture(uTexture, uv);
    FragColor = vec4(color.rgb, textureColor.r * color.a);
} 