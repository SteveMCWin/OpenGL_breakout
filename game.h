#ifndef GAME_H
#define GAME_H

#include "game_level.h"
#include "game_object.h"
#include "ball_object.h"
#include "shader.h"
#include "resource_manager.h"
#include "sprite_renderer.h"
#include "particle_generator.h"
#include "post_processor.h"
#include "glad.h"
#include <GLFW/glfw3.h>
#include <cfenv>
#include <tuple>
#include <vector>
#include <algorithm>

// Represents the current state of the game
enum GameState {
    GAME_ACTIVE,
    GAME_MENU,
    GAME_WIN
};

enum Direction {
    UP,
    RIGHT,
    DOWN,
    LEFT
};



typedef std::tuple<bool, Direction, glm::vec2> Collision;

// Game holds all game-related state and functionality.
// Combines all game-related data into a single class for
// easy access to each of the components and manageability.
class Game {
public:
    // game state
    GameState               State;	
    bool                    Keys[1024];
    unsigned int            Width;
    unsigned int            Height;
    std::vector<GameLevel>  Levels;
    unsigned int            Level;
    int                     Lives;

    // constructor/destructor
    Game(unsigned int width, unsigned int height);
    ~Game();

    // initialize game state (load all shaders/textures/levels)
    void Init();

    // game loop
    void ProcessInput(float dt);
    void Update(float dt);
    void Render();

    void DoCollisions();

    void ResetLevel();
    void ResetPlayer();
};

#endif


