#version 330 core

layout(location = 0) in vec2 inPos;   // pozicija verteksa
layout(location = 1) in vec3 inCol;   // boja verteksa

out vec4 chCol;  // izlaz ka fragment shaderu

uniform float uX;
uniform float uY;

void main()
{
    gl_Position = vec4(inPos.x + uX, inPos.y + uY, 0.0, 1.0);
    chCol = vec4(inCol, 1.0);
}
