#ifndef TEXT_RENDERER_H
#define TEXT_RENDERER_H

#include <map>

#include "glad.h"
#include <glm/glm.hpp>

#include "texture.h"
#include "shader.h"

// Holds all state information relevant to a character as loaded using FreeType
struct Character{
    unsigned int TextureID; // ID handle of the glyph texture
    glm::ivec2   Size;      // size of the glyph
    glm::ivec2   Bearing;   // offset form baseline to left/top of glyph
    unsigned int Advance;   // horizontal offset to advance to next glyph
};

class TextRenderer{
public:
    // holds a list of pre-compiled Characters
std::map<char, Character> Characters;

    Shader TextShader;

    TextRenderer(unsigned int width, unsigned int height);

    // pre-compiles a list of characters form the given font
    void Load(std::string font, unsigned int fontSize);

    // renders a string of text using the precompiled list of characters
    void RenderText(std::string text, float x, float y, float scale, glm::vec3 color = glm::vec3(1.0f));

private:

    unsigned int VAO;
    unsigned int VBO;
};

#endif // !TEXT_RENDERER_H
