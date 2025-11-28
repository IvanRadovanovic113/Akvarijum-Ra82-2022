#include <GL/glew.h>
#include <GLFW/glfw3.h>

#define _USE_MATH_DEFINES
#include <cmath>
#include <algorithm>
#include <iostream>
#include "Util.h"

#define NUM_SLICES 40

int endProgram(std::string message) {
    std::cout << message << std::endl;
    glfwTerminate();
    return -1;
}

float goldX = 0.0f, goldY = -0.5f;   // goldFish pozicija
float blowX = 0.5f, blowY = 0.0f;    // blowFish pozicija
float moveSpeed = 0.005f;

// Dimenzije ekrana
int screenWidth = 1000;
int screenHeight = 800;

// texture
unsigned int sandTexture;
unsigned int blowFishTexture;

void preprocessTexture(unsigned& texture, const char* filepath) {
    texture = loadImageToTexture(filepath);
    glBindTexture(GL_TEXTURE_2D, texture);

    glGenerateMipmap(GL_TEXTURE_2D);

    // mirror da se pesak lepo nastavlja
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
}

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

void drawTexturedRect(unsigned int shader, unsigned int VAO, unsigned int texture, float x, float y, float alpha)
{
    glUseProgram(shader);

    glUniform1f(glGetUniformLocation(shader, "uX"), x);
    glUniform1f(glGetUniformLocation(shader, "uY"), y);
    glUniform1f(glGetUniformLocation(shader, "uAlpha"), alpha);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);
    glUniform1i(glGetUniformLocation(shader, "uTexture"), 0);

    glBindVertexArray(VAO);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

    glBindVertexArray(0);
}

int main()
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(screenWidth, screenHeight, "Vezba 3", NULL, NULL);
    if (window == NULL) return endProgram("Prozor nije uspeo da se kreira.");
    glfwMakeContextCurrent(window);

    if (glewInit() != GLEW_OK) return endProgram("GLEW nije uspeo da se inicijalizuje.");

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // 2D scena -> depth nam ne treba, gasimo ga ako je bio upaljen
    glDisable(GL_DEPTH_TEST);

    unsigned int rectShader = createShader("rect.vert", "rect.frag");
    unsigned int colorShader = createShader("color.vert", "color.frag");
    unsigned int textureShader = createShader("texture.vert", "texture.frag");

    float goldFish[] = {
        -0.2f,  0.2f,  0.0f, 0.0f, 1.0f,
        -0.2f, -0.2f,  0.0f, 1.0f, 0.0f,
         0.2f, -0.2f,  1.0f, 0.0f, 0.0f,
         0.2f,  0.2f,  0.0f, 1.0f, 1.0f
    };

    float blowFish[] = {
        -0.1f,  0.1f,   0.0f, 1.0f,  // gornje levo
        -0.1f, -0.1f,   0.0f, 0.0f,  // donje levo
         0.1f, -0.1f,   1.0f, 0.0f,  // donje desno
         0.1f,  0.1f,   1.0f, 1.0f   // gornje desno
    };

    // Pravougaonik vode (unutrasnjost akvarijuma)
    float aquariumRect[] = {
        -1.0f, -1.0f,  0.5f, 0.6f, 1.0f,
         1.0f, -1.0f,  0.5f, 0.6f, 1.0f,
         1.0f,  0.4f,  0.5f, 0.7f, 1.0f,
        -1.0f,  0.4f,  0.5f, 0.7f, 1.0f
    };

    // Donja linija
    float bottomLine[] = {
        -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
         1.0f, -1.0f, 0.0f, 0.0f, 0.0f
    };

    // Leva linija
    float leftLine[] = {
        -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
        -1.0f,  0.4f, 0.0f, 0.0f, 0.0f
    };

    // Desna linija
    float rightLine[] = {
         1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
         1.0f,  0.4f, 0.0f, 0.0f, 0.0f
    };

    // Pesak (teksturisano dno)
    float sandVertices[] = {
        -1.0f, -1.0f,   0.0f, 0.0f,
         1.0f, -1.0f,   6.0f, 0.0f,
         1.0f, -0.6f,   6.0f, 1.0f,
        -1.0f, -0.6f,   0.0f, 1.0f
    };

    unsigned int VAOGoldFish, VAOBlowFish;
    formVAOs(goldFish, sizeof(goldFish), VAOGoldFish);
    formVAOTexture(blowFish, sizeof(blowFish), VAOBlowFish);

    unsigned int VAOAquarium, VAOBottom, VAOLeft, VAORight;
    formVAOs(aquariumRect, sizeof(aquariumRect), VAOAquarium);
    formVAOs(bottomLine, sizeof(bottomLine), VAOBottom);
    formVAOs(leftLine, sizeof(leftLine), VAOLeft);
    formVAOs(rightLine, sizeof(rightLine), VAORight);

    unsigned int VAOSand;
    formVAOTexture(sandVertices, sizeof(sandVertices), VAOSand);

    glClearColor(0.5f, 0.6f, 1.0f, 1.0f);

    preprocessTexture(sandTexture, "res/sand.png");
    if (!sandTexture) {
        std::cout << "Neuspesno ucitavanje teksture peska!" << std::endl;
    }
    else {
        std::cout << "Tekstura peska uspesno ucitana!" << std::endl;
    }

    preprocessTexture(blowFishTexture, "res/blowFish.png");
    if (!blowFishTexture) {
        std::cout << "Neuspesno ucitavanje teksture blowFishTexture!" << std::endl;
    }
    else {
        std::cout << "Tekstura peska uspesno blowFishTexture!" << std::endl;
    }



    while (!glfwWindowShouldClose(window))
    {
        // INPUT

        glClear(GL_COLOR_BUFFER_BIT);
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) goldX -= moveSpeed;
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) goldX += moveSpeed;
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) goldY += moveSpeed;
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) goldY -= moveSpeed;

        if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS) blowX -= moveSpeed;
        if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) blowX += moveSpeed;
        if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) blowY += moveSpeed;
        if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) blowY -= moveSpeed;

        //  Pesak
        drawTexturedRect(textureShader, VAOSand, sandTexture, 0.0f, 0.0f, 1.0f);

        // Ribe (uvek pune, bez providnosti)
        drawRect(rectShader, VAOGoldFish, goldX, goldY);
        drawTexturedRect(textureShader, VAOBlowFish, blowFishTexture, blowX, blowY, 1.0f);

        // Providno staklo (lagana plava providna površina)
        glUseProgram(rectShader);
        glUniform1f(glGetUniformLocation(rectShader, "uAlpha"), 0.3f);
        glUniform1f(glGetUniformLocation(rectShader, "uX"), 0.0f);
        glUniform1f(glGetUniformLocation(rectShader, "uY"), 0.0f);
        glBindVertexArray(VAOAquarium);
        glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

        // Crne ivice akvarijuma (iznad svega)
        glUseProgram(colorShader);
        glUniform4f(glGetUniformLocation(colorShader, "uColor"), 0.0f, 0.0f, 0.0f, 1.0f);
        glLineWidth(8.0f);
        drawLine(colorShader, VAOBottom);
        drawLine(colorShader, VAOLeft);
        drawLine(colorShader, VAORight);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteProgram(rectShader);
    glDeleteProgram(colorShader);
    glDeleteProgram(textureShader);
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
