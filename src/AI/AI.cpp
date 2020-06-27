#include "AI.hpp"
#include "../Entity/Base.hpp"
#include "../Engine/Object.hpp"
#include "../Engine/World.hpp"

void Game::AI::WanderBehavior::init(){
    behaviorTimeout = 100;
    priority = 10;
    type = Game::AI::Type::Wanderer;
    name = "Stare";
}

void Game::AI::WanderBehavior::think(EntityBase *subject){

}


