#include "AI.hpp"
#include "../Core/Object.hpp"
#include "../Core/World.hpp"
#include "../Entity/Base.hpp"

void Game::AI::WanderBehavior::init(){
    tickrate = 100;
    priority = 10;
    type = Game::AI::BehaviorType::Wanderer;
    name = "Stare";
}

void Game::AI::WanderBehavior::think(EntityBase *subject){

}


void Game::AI::DumbassBehavior::init(){
    lastRand = nite::getTicks();
    nextRand = nite::randomInt(500, 1500);
}

void Game::AI::DumbassBehavior::think(Game::EntityBase *subject){
    if(nite::getTicks()-lastRand > nextRand){
        rand = nite::randomInt(0, 3);
        nextRand = nite::randomInt(100, 500);
        lastRand = nite::getTicks();
        subject->input.setAll(Game::PressType::Released);
    }
    switch(rand){
        case 0: {
            subject->input.set(Game::Key::RIGHT, Game::PressType::Pressed);
        } break;
        case 1: {
            subject->input.set(Game::Key::DOWN, Game::PressType::Pressed);
        } break;
        case 2: {
            subject->input.set(Game::Key::LEFT, Game::PressType::Pressed);
        } break;
        case 3: {
            subject->input.set(Game::Key::UP, Game::PressType::Pressed);
        } break;
        default: {
            subject->input.setAll(Game::PressType::Released);
        } break;                     
    }
}