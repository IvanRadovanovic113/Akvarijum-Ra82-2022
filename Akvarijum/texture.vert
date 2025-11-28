#version 330 core

layout (location = 0) in vec2 aPos;
layout (location = 1) in vec2 aTexCoord;

uniform float uX;
uniform float uY;

out vec2 TexCoord;

void main()
{
    gl_Position = vec4(aPos.x + uX, aPos.y + uY, 0.0, 1.0);
    TexCoord = aTexCoord;
}
