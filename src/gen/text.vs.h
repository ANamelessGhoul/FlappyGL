#pragma once
/*
AUTO GENERATED DO NOT EDIT
*/

/*
#version 330 core
layout (location = 0) in vec2 aPos;
layout (location = 1) in vec4 aColor;
layout (location = 2) in vec2 aTexCoord;

out vec2 uv;
out vec4 color;

void main()
{
    gl_Position = vec4(aPos.x, aPos.y, 0.0, 1.0);
    color = aColor;
    uv = aTexCoord;
}
*/
static const char* text_vs = 
	"#version 330 core\n"
	"layout (location = 0) in vec2 aPos;\n"
	"layout (location = 1) in vec4 aColor;\n"
	"layout (location = 2) in vec2 aTexCoord;\n"
	"\n"
	"out vec2 uv;\n"
	"out vec4 color;\n"
	"\n"
	"void main()\n"
	"{\n"
	"    gl_Position = vec4(aPos.x, aPos.y, 0.0, 1.0);\n"
	"    color = aColor;\n"
	"    uv = aTexCoord;\n"
	"}\n"
;