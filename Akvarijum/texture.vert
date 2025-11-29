#version 330 core

layout (location = 0) in vec2 aPos;       // Pozicija verteksa
layout (location = 1) in vec2 aTexCoord;  // Tex koordinate

out vec2 TexCoord;

uniform float uX;
uniform float uY;
uniform float uScaleX;
uniform float uScaleY;

void main()
{
    // Skaliranje po X i Y
    vec2 pos = aPos;
    pos.x *= uScaleX;
    pos.y *= uScaleY;

    // Translacija (pomeranje)
    pos.x += uX;
    pos.y += uY;

    gl_Position = vec4(pos, 0.0, 1.0);
    TexCoord = aTexCoord;
}
