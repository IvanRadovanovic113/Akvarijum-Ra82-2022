#include <GL/glew.h>
#include <GLFW/glfw3.h>

#define _USE_MATH_DEFINES
#include <cmath>
#include <algorithm>
#include <iostream>
#include "Util.h"
#include <vector>

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
unsigned int goldFishTexture;
unsigned int seaweedTexture;
unsigned int bubbleTexture;

// mehurići vreme
double lastGoldBubbleTime = 0.0;
double lastBlowBubbleTime = 0.0;
double bubbleCooldown = 0.4; // 0.4 sekunde pauze

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

void drawTexturedRect(unsigned int shader, unsigned int VAO, unsigned int texture,
    float x, float y, float alpha, float scaleX)
{
    glUseProgram(shader);

    glUniform1f(glGetUniformLocation(shader, "uX"), x);
    glUniform1f(glGetUniformLocation(shader, "uY"), y);
    glUniform1f(glGetUniformLocation(shader, "uAlpha"), alpha);
    glUniform1f(glGetUniformLocation(shader, "uScaleX"), scaleX);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);
    glUniform1i(glGetUniformLocation(shader, "uTexture"), 0);

    glBindVertexArray(VAO);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

    glBindVertexArray(0);
}

struct Bubble {
    float x, y;
    float speed = 0.004f;
};

std::vector<Bubble> goldBubbles;
std::vector<Bubble> blowBubbles;

void spawnBubbles(std::vector<Bubble>& bubbles, float fishX, float fishY, float scaleX) {
    float mouthOffsetX = 0.15f * scaleX;  // pomeri do usta, zavisi od orijentacije
    float mouthOffsetY = 0.02f;          // iznad centra

    bubbles.push_back({ fishX + mouthOffsetX,     fishY + mouthOffsetY });
    bubbles.push_back({ fishX + mouthOffsetX + 0.03f, fishY + mouthOffsetY + 0.03f });
    bubbles.push_back({ fishX + mouthOffsetX - 0.03f, fishY + mouthOffsetY + 0.05f });
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
        -0.2f,  0.2f,  0.0f, 1.0f,
        -0.2f, -0.2f,  0.0f, 0.0f,
         0.2f, -0.2f,  1.0f, 0.0f,
         0.2f,  0.2f,  1.0f, 1.0f,
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

    // Pesak
    float sandVertices[] = {
        -1.0f, -1.0f,   0.0f, 0.0f,
         1.0f, -1.0f,   6.0f, 0.0f,
         1.0f, -0.4f,   6.0f, 1.0f,
        -1.0f, -0.4f,   0.0f, 1.0f
    };

	// morska trava
    float seaweedVertices[] = {
        -0.95f, -0.8f,  0.0f, 0.0f,   // donje levo - malo dublje u pesak
        -0.73f, -0.8f,  1.0f, 0.0f,   // donje desno (širimo je)
        -0.73f,  -0.4f, 1.0f, 1.0f,   // gornje desno (malo viša)
        -0.95f,  -0.4f, 0.0f, 1.0f    // gornje levo
    };
	//mehurići
    float bubbleVertices[] = {
        -0.02f, 0.02f,   0.0f, 1.0f,   // gornje levo
        -0.02f,-0.02f,   0.0f, 0.0f,   // donje levo
         0.02f,-0.02f,   1.0f, 0.0f,   // donje desno
         0.02f, 0.02f,   1.0f, 1.0f    // gornje desno
    };
    unsigned int VAOBubble;
    formVAOTexture(bubbleVertices, sizeof(bubbleVertices), VAOBubble);
    unsigned int VAOGoldFish, VAOBlowFish;
    formVAOTexture(goldFish, sizeof(goldFish), VAOGoldFish);
    formVAOTexture(blowFish, sizeof(blowFish), VAOBlowFish);

    unsigned int VAOAquarium, VAOBottom, VAOLeft, VAORight;
    formVAOs(aquariumRect, sizeof(aquariumRect), VAOAquarium);
    formVAOs(bottomLine, sizeof(bottomLine), VAOBottom);
    formVAOs(leftLine, sizeof(leftLine), VAOLeft);
    formVAOs(rightLine, sizeof(rightLine), VAORight);

    unsigned int VAOSand;
    formVAOTexture(sandVertices, sizeof(sandVertices), VAOSand);

    unsigned int VAOSeaweed;
    formVAOTexture(seaweedVertices, sizeof(seaweedVertices), VAOSeaweed);

    glClearColor(0.7f, 0.8f, 1.0f, 1.0f);

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

    preprocessTexture(goldFishTexture, "res/goldFish.png");
    if (!goldFishTexture) {
        std::cout << "Neuspesno ucitavanje teksture goldFishTexture!" << std::endl;
    }
    else {
        std::cout << "Tekstura peska uspesno goldFishTexture!" << std::endl;
    }

    preprocessTexture(seaweedTexture, "res/seaweed.png");
    if (!seaweedTexture) {
        std::cout << "Neuspesno ucitavanje teksture seaweed!" << std::endl;
    }
    else {
        std::cout << "Tekstura seaweed uspesno ucitana!" << std::endl;
    }
    preprocessTexture(bubbleTexture, "res/bubble.png");
    if (!bubbleTexture) {
        std::cout << "Neuspesno ucitavanje teksture bubble!" << std::endl;
    }
    else {
        std::cout << "Tekstura seaweed uspesno bubble!" << std::endl;
    }

    float goldScaleX = 1.0f;
    float blowScaleX = 1.0f;

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

        double now = glfwGetTime();

        // GOLD (Z)
        if (glfwGetKey(window, GLFW_KEY_Z) == GLFW_PRESS) {
            if (now - lastGoldBubbleTime > bubbleCooldown) {
                spawnBubbles(goldBubbles, goldX, goldY - 0.05f, -goldScaleX);
                lastGoldBubbleTime = now;
            }
        }

        // BLOW (K)
        if (glfwGetKey(window, GLFW_KEY_K) == GLFW_PRESS) {
            if (now - lastBlowBubbleTime > bubbleCooldown) {
                spawnBubbles(blowBubbles, blowX - (0.05f * blowScaleX), blowY, blowScaleX);
                lastBlowBubbleTime = now;
            }
        }

        //  Pesak
        drawTexturedRect(textureShader, VAOSand, sandTexture, 0.0f, 0.0f, 1.0f, 1.0f);

        // Seaweed 1 (leva)
        drawTexturedRect(textureShader, VAOSeaweed, seaweedTexture, 0.0f, 0.0f, 1.0f, 1.0f);


        // Ribe (uvek pune, bez providnosti)
        // Goldfish – orijentacija
        goldScaleX = (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) ? 1.0f :
            (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) ? -1.0f : goldScaleX;

        drawTexturedRect(textureShader, VAOGoldFish, goldFishTexture, goldX, goldY, 1.0f, goldScaleX);

        // Blowfish – orijentacija
        blowScaleX = (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS) ? -1.0f :
            (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) ? 1.0f : blowScaleX;

        drawTexturedRect(textureShader, VAOBlowFish, blowFishTexture, blowX, blowY, 1.0f, blowScaleX);

        // Seaweed 2 (desna)
        drawTexturedRect(textureShader, VAOSeaweed, seaweedTexture, 0.7f, -0.1f, 1.0f, 1.0f);

        // Providno staklo (lagana plava providna površina)
        glUseProgram(rectShader);
        glUniform1f(glGetUniformLocation(rectShader, "uAlpha"), 0.2f);
        glUniform1f(glGetUniformLocation(rectShader, "uX"), 0.0f);
        glUniform1f(glGetUniformLocation(rectShader, "uY"), 0.0f);
        glBindVertexArray(VAOAquarium);
        glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

        // GOLD mehurići
        for (int i = 0; i < goldBubbles.size(); i++) {
            goldBubbles[i].y += goldBubbles[i].speed;
            goldBubbles[i].x += sin(glfwGetTime() * 2.0f + i) * 0.0005f;

            drawTexturedRect(textureShader, VAOBubble, bubbleTexture,
                goldBubbles[i].x, goldBubbles[i].y, 1.0f, 1.0f);

            if (goldBubbles[i].y > 0.4f)
            {
                goldBubbles.erase(goldBubbles.begin() + i); i--;
            }
        }

        // BLOW mehurići
        for (int i = 0; i < blowBubbles.size(); i++) {
            blowBubbles[i].y += blowBubbles[i].speed;
            blowBubbles[i].x += sin(glfwGetTime() * 2.0f + i) * 0.0005f;

            drawTexturedRect(textureShader, VAOBubble, bubbleTexture,
                blowBubbles[i].x, blowBubbles[i].y, 1.0f, 1.0f);

            if (blowBubbles[i].y > 0.4f)
            {
                blowBubbles.erase(blowBubbles.begin() + i); i--;
            }
        }

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
