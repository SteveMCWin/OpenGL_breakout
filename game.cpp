#include "game.h"
#include <algorithm>
#include <tuple>


SpriteRenderer  *Renderer;
GameObject      *Player;
BallObject      *Ball;

// Initial size of the player paddle
const glm::vec2 PLAYER_SIZE(100.0f, 20.0f);
// Initial velocity of the player paddle
const float PLAYER_VELOCITY(500.0f);
// Initial velocity of the Ball
const glm::vec2 INITIAL_BALL_VELOCITY(100.0f, -350.0f);
// Radius of the ball object
const float BALL_RADIUS = 12.5f;



Game::Game(unsigned int width, unsigned int height) 
    : State(GAME_ACTIVE), Keys(), Width(width), Height(height), Lives(2){ 

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

    glm::vec2 ballPos = playerPos + glm::vec2(PLAYER_SIZE.x / 2.0f - BALL_RADIUS, -BALL_RADIUS * 2.0f);

    Ball = new BallObject(ballPos, BALL_RADIUS, INITIAL_BALL_VELOCITY, ResourceManager::GetTexture("face"));

}

void Game::Update(float dt){

    // update objects
    Ball->Move(dt, this->Width);

    // check for collisions
    this->DoCollisions();

    if(Ball->Position.y >= this->Height){
        if(this->Lives <= 0){
            this->Lives = 2;
            this->ResetLevel();
            this->ResetPlayer();
        }
        else{
            this->Lives -= 1;
            this->ResetPlayer();
        }
    }
    
}

void Game::ProcessInput(float dt){
   if(this->State == GAME_ACTIVE){
        float velocity = PLAYER_VELOCITY * dt;

        // move player
        if(this->Keys[GLFW_KEY_A]){
            if(Player ->Position.x >= 0.0f){
                Player->Position.x -= velocity;
                if(Ball->Stuck)
                    Ball->Position.x -= velocity;
            }
        }
        if(this->Keys[GLFW_KEY_D]){
            if(Player->Position.x <= this->Width - Player->Size.x){
                Player->Position.x += velocity;
                if(Ball->Stuck)
                    Ball->Position.x += velocity;
            }
        }

        // release the ball
        if(this->Keys[GLFW_KEY_SPACE])
            Ball->Stuck = false;
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

        // draw the ball
        Ball->Draw(*Renderer);
    }
}

Direction VectorDirection(glm::vec2 target){
    glm::vec2 compass[] = {
        glm::vec2(0.0f, 1.0f),  // up
        glm::vec2(1.0f, 0.0f),  // right
        glm::vec2(0.0f, -1.0f), // down
        glm::vec2(-1.0f, 0.0f)  // left
    };

    float max = 0.0f;
    unsigned int best_match = -1;
    for(unsigned int i = 0; i < 4; i++){
        float dot_product = glm::dot(glm::normalize(target), compass[i]);
        if(dot_product > max){
            max = dot_product;
            best_match = i;
        }
    }

    return (Direction)best_match;
}

Collision CheckCollision(BallObject &one, GameObject &two){

    // get center point circle first 
    glm::vec2 center(one.Position + one.Radius);

    // calculate AABB info (center, half-extents)
    glm::vec2 aabb_half_extents(two.Size.x / 2.0f, two.Size.y / 2.0f);
    glm::vec2 aabb_center(two.Position.x + aabb_half_extents.x, 
                          two.Position.y + aabb_half_extents.y);

    // get difference vector between both centers
    glm::vec2 difference = center - aabb_center;
    glm::vec2 clamped = glm::clamp(difference, -aabb_half_extents, aabb_half_extents);

    // add clamped value to AABB_center and we get the value of box closest to circle
    glm::vec2 closest = aabb_center + clamped;

    // retrieve vector between center circle and closest point AABB and check if length <= radius
    difference = closest - center;

    if(glm::length(difference) <= one.Radius)
        return std::make_tuple(true, VectorDirection(difference), difference);
    else
        return std::make_tuple(false, UP, glm::vec2(0.0f, 0.0f));
}

void Game::DoCollisions(){
    for(GameObject &box : this->Levels[this->Level].Bricks){
        if(!box.Destroyed){
            Collision collision = CheckCollision(*Ball, box);
            if(std::get<0>(collision)){

                // destroy block if not solid
                if(!box.IsSolid){
                    box.Destroyed = true;
                }
                
                Direction dir = std::get<1>(collision);
                glm::vec2 diff_vector = std::get<2>(collision);
                if(dir == LEFT || dir == RIGHT){
                    Ball->Velocity.x = -Ball->Velocity.x;

                    // relocate
                    float penetration = Ball->Radius - std::abs(diff_vector.x);
                    if(dir == LEFT)
                        Ball->Position.x += penetration;
                    else
                        Ball->Position.x -= penetration;
                }
                else{
                    Ball->Velocity.y = -Ball->Velocity.y;

                    //relocate
                    float penetration = Ball->Radius - std::abs(diff_vector.y);
                    if(dir == UP)
                        Ball->Position.y -= penetration;
                    else 
                        Ball->Position.y += penetration;
                }

                if(this->Levels[Level].IsCompleted()){
                    this->Level = (this->Level + 1) % static_cast<int>(this->Levels.size());
                    this->ResetLevel();
                    this->ResetPlayer();
                }
            }
        }
    }

    Collision result = CheckCollision(*Ball, *Player);
    if(!Ball->Stuck && std::get<0>(result)){
        
        // check where it hit the board, and change velocity based on where it hit
        float centerBoard = Player->Position.x + Player->Size.x / 2.0f;
        float distance = (Ball->Position.x + Ball->Radius) - centerBoard;
        float percentage = distance / (Player->Size.x / 2.0f);

        // then move accordingly
        float strength = 2.0f;
        glm::vec2 oldVelocity = Ball->Velocity;
        Ball->Velocity.x = INITIAL_BALL_VELOCITY.x * percentage * strength;
        Ball->Velocity.y = -1.0f * abs(Ball->Velocity.y);
        Ball->Velocity = glm::normalize(Ball->Velocity) * glm::length(oldVelocity);
    }
}

void Game::ResetLevel(){
    if(this->Level == 0)
        this->Levels[0].Load("/home/stevica/openGL_projects/breakout/levels/one.lvl", this->Width, this->Height / 2);
    else if(this->Level == 1)
        this->Levels[1].Load("/home/stevica/openGL_projects/breakout/levels/two.lvl", this->Width, this->Height / 2);
    else if(this->Level == 2)
        this->Levels[2].Load("/home/stevica/openGL_projects/breakout/levels/three.lvl", this->Width, this->Height / 2);
    else if(this->Level == 3)
        this->Levels[3].Load("/home/stevica/openGL_projects/breakout/levels/four.lvl", this->Width, this->Height / 2);
}

void Game::ResetPlayer(){
    Player->Size = PLAYER_SIZE;
    Player->Position = glm::vec2(this->Width / 2.0f - PLAYER_SIZE.x / 2.0f,
                                 this->Height       - PLAYER_SIZE.y * 1.5f);
    Ball->Reset(Player->Position + glm::vec2(PLAYER_SIZE.x / 2.0f - BALL_RADIUS, -(BALL_RADIUS * 2.0f)), INITIAL_BALL_VELOCITY);
}





