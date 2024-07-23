#ifndef GAME_LEVEL_H
#define GAME_LEVEL_H

#include "glad.h"
#include "game_object.h"
#include "sprite_renderer.h"
#include "resource_manager.h"
#include <vector>
#include <fstream>
#include <sstream>
#include <glm/detail/type_vec.hpp>

class GameLevel{
public:
    // level state
    std::vector<GameObject> Bricks;
    // constructor
    GameLevel() {}
    // loads level rom file
    void Load(const char *file, unsigned int levelWidth, unsigned int levelHeight);
    // render level
    void Draw(SpriteRenderer &renderer);
    // check if the level is completed (all non solid tiles are destroyed)
    bool IsCompleted();

private:
    // initialize level from tile data
    void Init(std::vector<std::vector<unsigned int>> tileData, unsigned int levelWidth, unsigned int levelHeight);
};

#endif // !GAME_LEVEL_H
