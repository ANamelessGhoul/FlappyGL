#pragma once
/*
AUTO GENERATED DO NOT EDIT
*/

/*
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
*/
static const char* text_fs = 
	"#version 330 core\n"
	"out vec4 FragColor;\n"
	"\n"
	"in vec4 color;\n"
	"in vec2 uv;\n"
	"\n"
	"uniform sampler2D uTexture;\n"
	"\n"
	"void main()\n"
	"{\n"
	"    vec4 uvColor = vec4(uv.x, uv.y, 0, 1.0);\n"
	"    vec4 textureColor = texture(uTexture, uv);\n"
	"    FragColor = vec4(color.rgb, textureColor.r * color.a);\n"
	"} \n"
;