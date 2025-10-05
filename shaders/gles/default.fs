//#version 330 core
//out vec4 FragColor;
precision mediump float; 
varying vec4 color;
varying vec2 uv;

uniform sampler2D uTexture;

void main()
{
    vec4 uvColor = vec4(uv.x, uv.y, 0, 1.0);
    vec4 textureColor = texture2D(uTexture, uv);
    gl_FragColor = vec4(mix(textureColor.rgb, color.rgb, color.a), textureColor.a);
} 