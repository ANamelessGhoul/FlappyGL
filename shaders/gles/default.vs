//#version 330 core
attribute vec2 aPos;
attribute vec4 aColor;
attribute vec2 aTexCoord;

varying vec2 uv;
varying vec4 color;

void main()
{
    gl_Position = vec4(aPos.x, aPos.y, 0.0, 1.0);
    color = aColor;
    uv = aTexCoord;
}