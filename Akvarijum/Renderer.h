#pragma once
#include <cstddef>

namespace Renderer
{
    void formVAOs(float* verticesRect, size_t rectSize, unsigned int& VAOrect);
    void formVAOTexture(float* vertices, size_t size, unsigned int& VAO);
    void drawRect(unsigned int shader, unsigned int VAO, float x, float y);
    void drawLine(unsigned int shader, unsigned int VAO);
    void drawTexturedRect(unsigned int shader, unsigned int VAO, unsigned int texture,
        float x, float y, float alpha, float scaleX, float scaleY);
}