#ifndef GAME_OBJECT_H
#define GAME_OBJECT_H

#include "glad.h"
#include <glm/glm.hpp>

#include "texture.h"
#include "sprite_renderer.h"

class GameObject{
public:
    // object states
    glm::vec2 Position;
    glm::vec2 Size;
    glm::vec2 Velocity;
    glm::vec3 Color;
    float Rotation;
    bool IsSolid;
    bool Destroyed;
    // render state
    Texture2D Sprite;
    // constructor(s)
    GameObject();
    GameObject(glm::vec2 pos, glm::vec2 size, Texture2D sprite, glm::vec3 color = glm::vec3(1.0f), glm::vec2 velocity = glm::vec2(0.0f, 0.0f));
    // draw sprite
    virtual void Draw(SpriteRenderer &renderer);
};

#endif // !GAME_OBJECT_H
