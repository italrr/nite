#include "Object.hpp"

Game::Object::Object(){
    friction = 0.0f;
    setMass(0.0f);
    setPosition(nite::Vec2(0.0f));  
    reshape(nite::Vec2(64.0f));
}

void Game::Object::setMass(float mass){
    this->mass = mass;
    this->invMass = mass <= 0 ? 0.0f : 1.0f / mass;
}


void Game::Object::reshape(const nite::Vec2 &size){
    this->size = size;
}

void Game::Object::push(float force, float angle){
    this->accel.x += (nite::cos(angle) * (force / 100.0f)) / this->mass;
    this->accel.y += (nite::sin(angle) * (force / 100.0f)) / this->mass;
}	

void Game::Object::move(float x, float y){
    // TODO: handle diagonal speed
    if(x != 0.0f){
        float diffx = ((x / 100.0f) / this->mass);
        if(nite::abs(this->accel.x) < nite::abs(diffx)){
            this->accel.x += diffx;
        }
    }
    if(y != 0.0f){
        float diffy = ((y / 100.0f) / this->mass);
        if(nite::abs(this->accel.y) < nite::abs(diffy)){
            this->accel.y += diffy;
        }
    }
}

void Game::Object::setPosition(const nite::Vec2 &p){
    this->position = p;
    this->lerpPos = p;
}