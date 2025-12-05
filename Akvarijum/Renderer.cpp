#include "Renderer.h"
#include <GL/glew.h>
#include <GLFW/glfw3.h>

namespace Renderer
{
    void formVAOs(float* verticesRect, size_t rectSize, unsigned int& VAOrect) {
        unsigned int VBOrect;
        glGenVertexArrays(1, &VAOrect);
        glGenBuffers(1, &VBOrect);

        glBindVertexArray(VAOrect);
        glBindBuffer(GL_ARRAY_BUFFER, VBOrect);
        glBufferData(GL_ARRAY_BUFFER, rectSize, verticesRect, GL_STATIC_DRAW);

        // pozicija
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);

        // boja
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(2 * sizeof(float)));
        glEnableVertexAttribArray(1);
    }

    void drawRect(unsigned int shader, unsigned int VAO, float x, float y)
    {
        glUseProgram(shader);
        glUniform1f(glGetUniformLocation(shader, "uAlpha"), 1.0f);
        glUniform1f(glGetUniformLocation(shader, "uX"), x);
        glUniform1f(glGetUniformLocation(shader, "uY"), y);

        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    }

    void drawLine(unsigned int shader, unsigned int VAO)
    {
        glUseProgram(shader);
        glBindVertexArray(VAO);
        glLineWidth(10.0f);
        glDrawArrays(GL_LINES, 0, 2);
    }

    void formVAOTexture(float* vertices, size_t size, unsigned int& VAO)
    {
        unsigned int VBO;
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);

        glBindVertexArray(VAO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, size, vertices, GL_STATIC_DRAW);

        // pozicija (x, y)
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);

        // tex koordinate (u, v)
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
        glEnableVertexAttribArray(1);

        glBindVertexArray(0);
    }

    void drawTexturedRect(unsigned int shader, unsigned int VAO, unsigned int texture,
        float x, float y, float alpha, float scaleX, float scaleY)
    {
        glUseProgram(shader);

        glUniform1f(glGetUniformLocation(shader, "uX"), x);
        glUniform1f(glGetUniformLocation(shader, "uY"), y);
        glUniform1f(glGetUniformLocation(shader, "uAlpha"), alpha);
        glUniform1f(glGetUniformLocation(shader, "uScaleX"), scaleX);
        glUniform1f(glGetUniformLocation(shader, "uScaleY"), scaleY);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture);
        glUniform1i(glGetUniformLocation(shader, "uTexture"), 0);

        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

        glBindVertexArray(0);
    }
}