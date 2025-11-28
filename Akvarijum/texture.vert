#version 330 core

layout (location = 0) in vec2 aPos;
layout (location = 1) in vec2 aTexCoord;

out vec2 TexCoord;

uniform float uX;
uniform float uY;
uniform float uScaleX;

void main()
{
    // Skaliranje po X osovini za flip levo/desno
    float scaledX = aPos.x * uScaleX;

    gl_Position = vec4(scaledX + uX, aPos.y + uY, 0.0, 1.0);
    TexCoord = aTexCoord;
}
