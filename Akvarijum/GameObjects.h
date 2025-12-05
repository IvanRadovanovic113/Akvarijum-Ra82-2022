#pragma once
#include <vector>

namespace GameObjects
{
    struct Bubble {
        float x, y;
        float speed = 0.004f;
    };

    struct Food {
        float x, y;
        unsigned int texture;
        float speed = 0.003f;
        bool atBottom = false;
    };

    void spawnBubbles(std::vector<Bubble>& bubbles, float fishX, float fishY, float scaleX);

    // spawnFood now accepts the foods vector and the two textures it may use
    void spawnFood(std::vector<Food>& foods, unsigned int wormTexture, unsigned int bugTexture);
}