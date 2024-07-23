#include "game.h"
#include "game_level.h"
#include "game_object.h"
#include "resource_manager.h"
#include "sprite_renderer.h"
#include "texture.h"
#include <GLFW/glfw3.h>

SpriteRenderer  *Renderer;
GameObject      *Player;
// Initialize size of the player paddle
const glm::vec2 PLAYER_SIZE(100.0f, 20.0f);
// Initialize velocity of the player paddle
const float PLAYER_VELOCITY(500.0f);


Game::Game(unsigned int width, unsigned int height) 
    : State(GAME_ACTIVE), Keys(), Width(width), Height(height){ 

}

Game::~Game(){
    delete Renderer;   
}

void Game::Init(){

    // load shaders
    ResourceManager::LoadShader("/home/stevica/openGL_projects/breakout/shaders/v_sprite.glsl", "/home/stevica/openGL_projects/breakout/shaders/f_sprite.glsl", nullptr, "sprite");

    // configure shaders  
    glm::mat4 projection = glm::ortho(0.0f, static_cast<float>(this->Width), static_cast<float>(this->Height), 0.0f, -1.0f, 1.0f);
    ResourceManager::GetShader("sprite").Use().SetInteger("image", 0);
    ResourceManager::GetShader("sprite").Use().SetMatrix4("projection", projection);

    // set render specific controls
    Renderer = new SpriteRenderer(ResourceManager::GetShader("sprite"));

    // load textures
    ResourceManager::LoadTexture("/home/stevica/openGL_projects/breakout/sprites/awesomeface.png", true, "face");
    ResourceManager::LoadTexture("/home/stevica/openGL_projects/breakout/sprites/background.jpg", false, "background");
    ResourceManager::LoadTexture("/home/stevica/openGL_projects/breakout/sprites/block.png", false, "block");
    ResourceManager::LoadTexture("/home/stevica/openGL_projects/breakout/sprites/block_solid.png", false, "block_solid");
    ResourceManager::LoadTexture("/home/stevica/openGL_projects/breakout/sprites/paddle.png", true, "paddle");

    // load levels
    GameLevel one;
    one.Load("/home/stevica/openGL_projects/breakout/levels/one.lvl", this->Width, this->Height / 2);
    GameLevel two;
    two.Load("/home/stevica/openGL_projects/breakout/levels/two.lvl", this->Width, this->Height / 2);
    GameLevel three;
    three.Load("/home/stevica/openGL_projects/breakout/levels/three.lvl", this->Width, this->Height / 2);
    GameLevel four;
    four.Load("/home/stevica/openGL_projects/breakout/levels/four.lvl", this->Width, this->Height / 2);

    this->Levels.push_back(one);
    this->Levels.push_back(two);
    this->Levels.push_back(three);
    this->Levels.push_back(four);
    this->Level = 0;

    glm::vec2 playerPos = glm::vec2(this->Width / 2.0f - PLAYER_SIZE.x / 2.0f,
                                    this->Height       - PLAYER_SIZE.y * 1.5f);

    Player = new GameObject(playerPos, PLAYER_SIZE, ResourceManager::GetTexture("paddle"));

}

void Game::Update(float dt){
    
}

void Game::ProcessInput(float dt){
   if(this->State == GAME_ACTIVE){
        float velocity = PLAYER_VELOCITY * dt;

        // move player
        if(this->Keys[GLFW_KEY_A]){
            if(Player ->Position.x >= 0.0f)
                Player->Position.x -= velocity;
        }
        if(this->Keys[GLFW_KEY_D]){
            if(Player->Position.x <= this->Width - Player->Size.x)
                Player->Position.x += velocity;
        }
    }
}

void Game::Render(){
    if(this->State == GAME_ACTIVE){

        // draw background
        Renderer->DrawSprite(ResourceManager::GetTexture("background"), glm::vec2(0.0f, 0.0f), glm::vec2(this->Width, this->Height), 0.0f);

        // draw level
        this->Levels[this->Level].Draw(*Renderer);

        // draw player
        Player->Draw(*Renderer);
    }
}


