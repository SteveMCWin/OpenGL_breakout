#include "game.h"
#include "text_renderer.h"
#include <GLFW/glfw3.h>

using namespace irrklang;

SpriteRenderer      *Renderer;
GameObject          *Player;
BallObject          *Ball;
ParticleGenerator   *Particles;
PostProcessor       *Effects;
ISoundEngine        *SoundEngine;
TextRenderer        *Text;

float ShakeTime = 0.0f;

// Initial size of the player paddle
const glm::vec2 PLAYER_SIZE(100.0f, 20.0f);
// Initial velocity of the player paddle
const float PLAYER_VELOCITY(500.0f);
// Initial velocity of the Ball
const glm::vec2 INITIAL_BALL_VELOCITY(100.0f, -350.0f);
// Radius of the ball object
const float BALL_RADIUS = 12.5f;

unsigned int nr_particles = 500;
std::vector<Particle> particles;

Game::Game(unsigned int width, unsigned int height) 
    : State(GAME_MENU), Keys(), Width(width), Height(height), Lives(3){ 

}

Game::~Game(){
    delete Renderer;   
}

void Game::Init(){

    // load shaders
    ResourceManager::LoadShader("/home/stevica/openGL_projects/breakout/shaders/v_sprite.glsl",
                                "/home/stevica/openGL_projects/breakout/shaders/f_sprite.glsl",
                                nullptr, "sprite");
    ResourceManager::LoadShader("/home/stevica/openGL_projects/breakout/shaders/v_particle.glsl",
                                "/home/stevica/openGL_projects/breakout/shaders/f_particle.glsl",
                                nullptr, "particle");
    ResourceManager::LoadShader("/home/stevica/openGL_projects/breakout/shaders/v_post_processing.glsl",
                                "/home/stevica/openGL_projects/breakout/shaders/f_post_processing.glsl",
                                nullptr, "post_processing");

    // configure shaders  
    glm::mat4 projection = glm::ortho(0.0f, static_cast<float>(this->Width), static_cast<float>(this->Height), 0.0f, -1.0f, 1.0f);
    ResourceManager::GetShader("sprite").Use().SetInteger("image", 0);
    ResourceManager::GetShader("sprite").Use().SetMatrix4("projection", projection);
    ResourceManager::GetShader("particle").Use().SetInteger("image", 0);
    ResourceManager::GetShader("particle").Use().SetMatrix4("projection", projection);

    // set render specific controls
    Renderer = new SpriteRenderer(ResourceManager::GetShader("sprite"));

    // load textures
    ResourceManager::LoadTexture("/home/stevica/openGL_projects/breakout/sprites/awesomeface.png", true, "face");
    ResourceManager::LoadTexture("/home/stevica/openGL_projects/breakout/sprites/background.jpg", false, "background");
    ResourceManager::LoadTexture("/home/stevica/openGL_projects/breakout/sprites/block.png", false, "block");
    ResourceManager::LoadTexture("/home/stevica/openGL_projects/breakout/sprites/block_solid.png", false, "block_solid");
    ResourceManager::LoadTexture("/home/stevica/openGL_projects/breakout/sprites/paddle.png", true, "paddle");
    ResourceManager::LoadTexture("/home/stevica/openGL_projects/breakout/sprites/particle.png", true, "particle");
    ResourceManager::LoadTexture("/home/stevica/openGL_projects/breakout/sprites/powerup_chaos.png", true, "chaos");
    ResourceManager::LoadTexture("/home/stevica/openGL_projects/breakout/sprites/powerup_confuse.png", true, "confuse");
    ResourceManager::LoadTexture("/home/stevica/openGL_projects/breakout/sprites/powerup_increase.png", true, "pad-size-increase");
    ResourceManager::LoadTexture("/home/stevica/openGL_projects/breakout/sprites/powerup_passthrough.png", true, "pass-through");
    ResourceManager::LoadTexture("/home/stevica/openGL_projects/breakout/sprites/powerup_speed.png", true, "speed");
    ResourceManager::LoadTexture("/home/stevica/openGL_projects/breakout/sprites/powerup_sticky.png", true, "sticky");

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

    Particles = new ParticleGenerator(ResourceManager::GetShader("particle"), ResourceManager::GetTexture("particle"), 500);

    Effects = new PostProcessor(ResourceManager::GetShader("post_processing"), this->Width, this->Height);

    Text = new TextRenderer(this->Width, this->Height);
    Text->Load("/home/stevica/openGL_projects/breakout/fonts/ocraext.TTF", 24);

    SoundEngine = createIrrKlangDevice();
    SoundEngine -> play2D("/home/stevica/openGL_projects/breakout/audio/breakout.ogg", true);
}

void Game::Update(float dt){

    // update objects
    Ball->Move(dt, this->Width);

    // check for collisions
    this->DoCollisions();

    if(Ball->Position.y >= this->Height){

        this->Lives -= 1;

        if(this->Lives <= 0){
            this->ResetLevel();
            this->State = GAME_MENU;
        }

        this->ResetPlayer();
    }

    this->UpdatePowerUps(dt);
    
    Particles->Update(dt, *Ball, 2, glm::vec2(Ball->Radius / 2.0f));

    if(ShakeTime >= 0.0f){
        ShakeTime -= dt;
        if(ShakeTime <= 0.0f)
            Effects->Shake = false;
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
    else if(this->State == GAME_MENU){
        if(this->Keys[GLFW_KEY_ENTER] && !this->KeysProcessed[GLFW_KEY_ENTER]){
            this->State = GAME_ACTIVE;
            this->KeysProcessed[GLFW_KEY_ENTER] = true;
        }
        if(this->Keys[GLFW_KEY_W] && !this->KeysProcessed[GLFW_KEY_W]){
            this->Level = (this->Level + 1) % 4;
            this->KeysProcessed[GLFW_KEY_W] = true;
        }
        if(this->Keys[GLFW_KEY_S] && !this->KeysProcessed[GLFW_KEY_S]){
            if(this->Level > 0){
                --this->Level;
            }
            else{
                this->Level = 3;
            }
            this->KeysProcessed[GLFW_KEY_S] = true;
        }
    }
    else if(this->State == GAME_WIN){
        if(this->Keys[GLFW_KEY_ENTER]){
            this->KeysProcessed[GLFW_KEY_ENTER] = true;
            Effects->Chaos = false;
            this->State = GAME_MENU;
        }
    }
}

void Game::Render(){
    if(this->State == GAME_ACTIVE || this->State == GAME_MENU){

        Effects->BeginRender();

        // draw background
        Renderer->DrawSprite(ResourceManager::GetTexture("background"), glm::vec2(0.0f, 0.0f), glm::vec2(this->Width, this->Height), 0.0f);

        // draw level
        this->Levels[this->Level].Draw(*Renderer);

        // draw player
        Player->Draw(*Renderer);

        for(PowerUp &powerUp : this->PowerUps){
            if(!powerUp.Destroyed){
                powerUp.Draw(*Renderer);
            }
        }

        // draw the particles
        Particles->Draw();

        // draw the ball
        Ball->Draw(*Renderer);

        Effects->EndRender();
        Effects->Render(glfwGetTime());

        std::stringstream ss;
        ss << this->Lives;
        Text->RenderText("Lives: " + ss.str(), 5.0f, 5.0f, 1.0f);
    }
    if(this->State == GAME_MENU){
        Text->RenderText("Press ENTER to start", 250.0f, Height / 2.0f, 1.0f);
        Text->RenderText("Press W or S to select level", 245.0f, Height / 2.0f + 20.0f, 0.75f);
    }
    if(this->State == GAME_WIN){
        Text->RenderText("You WON!!!", 320.0f, Height / 2.0f - 20.0f, 1.0f, glm::vec3(0.0, 1.0, 0.0));
        Text->RenderText("Press ENTER to retry or ESC to quit", 130.0, Height / 2.0, 1.0, glm::vec3(1.0, 1.0, 1.0));
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

void ActivatePowerUp(PowerUp powerUp){
    if(powerUp.Type == "speed"){
        Ball->Velocity *= 1.2;
    }
    else if(powerUp.Type == "sticky"){
        Ball->Sticky = true;
        Player->Color = glm::vec3(1.0f, 0.5f, 1.0f);
    }
    else if(powerUp.Type == "pass-through"){
        Ball->PassThrough = true;
        Ball->Color = glm::vec3(1.0f, 0.5f, 0.5f);
    }
    else if(powerUp.Type == "pad-size-increase"){
        Player->Size.x += 50;
    }
    else if(powerUp.Type == "confuse"){
        if(!Effects->Chaos){
            Effects->Confuse = true;
        }
    }
    else if(powerUp.Type == "chaos"){
        if(!Effects->Confuse){
            Effects->Chaos = true;
        }
    }
    
}

Collision CheckCollision(GameObject &one, GameObject &two){

    // get center point circle first 
    glm::vec2 center(one.Position + one.Size.x / 2.0f);

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

    if(glm::length(difference) <= one.Size.x / 2.0f)
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
                    this->SpawnPowerUps(box);
                    SoundEngine->play2D("/home/stevica/openGL_projects/breakout/audio/bleep.ogg", false);
                }
                else{
                    ShakeTime = 0.05f;
                    Effects->Shake = true;
                    SoundEngine->play2D("/home/stevica/openGL_projects/breakout/audio/solid.wav", false);
                }
                
                Direction dir = std::get<1>(collision);
                glm::vec2 diff_vector = std::get<2>(collision);
                if(!(Ball->PassThrough && !box.IsSolid)){
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
                }

                if(this->Levels[this->Level].IsCompleted() && this->State == GAME_ACTIVE){
                    // this->Level = (this->Level + 1) % static_cast<int>(this->Levels.size());
                    this->ResetLevel();
                    this->ResetPlayer();
                    Effects->Chaos = true;
                    this->State = GAME_WIN;
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

        Ball->Stuck = Ball->Sticky;

        SoundEngine->play2D("/home/stevica/openGL_projects/breakout/audio/bleep.wav", false);
    }

    for(PowerUp &powerUp : this->PowerUps){
        if(!powerUp.Destroyed){
            if(powerUp.Position.y >= this->Height)
                powerUp.Destroyed = true;

            Collision powerUpCollision = CheckCollision(*Player, powerUp);
            if(std::get<0>(powerUpCollision)){
                SoundEngine->play2D("/home/stevica/openGL_projects/breakout/audio/powerup.wav", false);
                ActivatePowerUp(powerUp);
                powerUp.Destroyed = true;
                powerUp.Activated = true;
            }
        }
    }
}

void Game::ResetLevel(){
    this->Lives = 3;
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

bool ShouldSpawn(unsigned int chance){
    unsigned int random = rand() % chance;
    return random == 0;
}

void Game::SpawnPowerUps(GameObject &block){
    
    if(ShouldSpawn(75)){
        this->PowerUps.push_back(PowerUp("speed", glm::vec3(0.5f, 0.5f, 1.0f), 0.0f, block.Position, ResourceManager::GetTexture("speed")));
    }
    if(ShouldSpawn(75)){
        this->PowerUps.push_back(PowerUp("sticky", glm::vec3(1.0f, 0.5f, 1.0f), 20.0f, block.Position, ResourceManager::GetTexture("sticky")));
    }
    if(ShouldSpawn(75)){
        this->PowerUps.push_back(PowerUp("pass-through", glm::vec3(0.5f, 1.0f, 0.5f), 10.0f, block.Position, ResourceManager::GetTexture("pass-through")));
    }
    if(ShouldSpawn(75)){
        this->PowerUps.push_back(PowerUp("pad-size-increase", glm::vec3(1.0f, 0.6f, 0.4f), 0.0f, block.Position, ResourceManager::GetTexture("pad-size-increase")));
    }
    if(ShouldSpawn(15)){
        this->PowerUps.push_back(PowerUp("confuse", glm::vec3(1.0f, 0.3f, 0.3f), 15.0f, block.Position, ResourceManager::GetTexture("confuse")));
    }
    if(ShouldSpawn(15)){
        this->PowerUps.push_back(PowerUp("chaos", glm::vec3(0.9f, 0.25f, 0.25f), 15.0f, block.Position, ResourceManager::GetTexture("chaos")));
    }
}

bool isOtherPowerUpActive(std::vector<PowerUp> &powerUps, std::string type){
    for(const PowerUp &powerUp : powerUps){
        if(powerUp.Activated){
            if(powerUp.Type == type){
                return true;
            }
        }
    }
    return false;
}

void Game::UpdatePowerUps(float dt){
    for(PowerUp &powerUp : this->PowerUps){
        powerUp.Position += powerUp.Velocity * dt;
        if(powerUp.Activated){
            powerUp.Duration -= dt;

            if(powerUp.Duration <= 0.0f){
                powerUp.Activated = false;

                if(powerUp.Type == "sticky"){
                    if(!isOtherPowerUpActive(this->PowerUps, "sticky")){
                        Ball->Sticky = false;
                        Player->Color = glm::vec3(1.0f);
                    }
                }
                else if(powerUp.Type == "pass-through"){
                    if(!isOtherPowerUpActive(this->PowerUps, "pass-through")){
                        Ball->PassThrough = false;
                        Ball->Color = glm::vec3(1.0f);
                    }
                }
                else if(powerUp.Type == "confuse"){
                    if(!isOtherPowerUpActive(this->PowerUps, "confuse")){
                        Effects->Confuse = false;
                    }
                }
                else if(powerUp.Type == "chaos"){
                    if(!isOtherPowerUpActive(this->PowerUps, "chaos")){
                        Effects->Chaos = false;
                    }
                }
            }
        }
    }
    this->PowerUps.erase(std::remove_if(this->PowerUps.begin(), this->PowerUps.end(),
        [](const PowerUp &powerUp) { return powerUp.Destroyed && !powerUp.Activated; }), this->PowerUps.end());
}


