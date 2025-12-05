#include "GameObjects.h"
#include <cstdlib>

namespace GameObjects
{
    void spawnBubbles(std::vector<Bubble>& bubbles, float fishX, float fishY, float scaleX) {
        float mouthOffsetX = 0.15f * scaleX;  // pomeri do usta, zavisi od orijentacije
        float mouthOffsetY = 0.02f;          // iznad centra

        bubbles.push_back({ fishX + mouthOffsetX,     fishY + mouthOffsetY });
        bubbles.push_back({ fishX + mouthOffsetX + 0.03f, fishY + mouthOffsetY + 0.03f });
        bubbles.push_back({ fishX + mouthOffsetX - 0.03f, fishY + mouthOffsetY + 0.05f });
    }

    void spawnFood(std::vector<Food>& foods, unsigned int wormTexture, unsigned int bugTexture) {
        bool chooseWorm = (rand() % 2 == 0);
        int numFoodItems = 2 + (rand() % 3); // između 2 i 4 komada
        for (int i = 0; i < numFoodItems; i++) {
            float randomX = ((rand() % 200) / 100.0f) - 1.0f; // od -1 do 1
            float randomY = 1.1f + (rand() % 50) / 100.0f;    // od 1.10 do 1.19

            unsigned int chosenTexture = chooseWorm ? wormTexture : bugTexture;
            chooseWorm = !chooseWorm;
            foods.push_back({ randomX, randomY, chosenTexture });
        }
    }
}