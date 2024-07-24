#ifndef POST_PROCESSOR_H
#define POST_PROCESSOR_H

#include "glad.h"
#include <glm/glm.hpp>

#include "texture.h"
#include "sprite_renderer.h"
#include "shader.h"

class PostProcessor{
public:
    Shader PostProcessingShader;
    Texture2D Texture;
    unsigned int Width;
    unsigned int Height;

    bool Confuse;
    bool Chaos;
    bool Shake;

    PostProcessor(Shader shader, unsigned int width, unsigned int height);

    // prepares the postprocessor's framebuffer operations before rendering the game
    void BeginRender();

    // should be called after rendering the game, so it stores all the rendered data into a texture object
    void EndRender();

    // renders the PostProcessor texture quad (as a screen-encompassing large sprite)
    void Render(float time);

private:
    unsigned int MSFBO; // Multisampled FBO
    unsigned int FBO;
    unsigned int RBO;
    unsigned int VAO;

    // initialize quad for rendering postprocessing texture
    void InitRenderData();
};

#endif // !POST_PROCESSOR_H
