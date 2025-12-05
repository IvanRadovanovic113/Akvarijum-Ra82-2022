#include "TextureUtils.h"
#include "Util.h"
#include <GL/glew.h>

namespace TextureUtils
{
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
}