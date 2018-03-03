#include "Entity.hpp"
#include "../Game.hpp"
#include "World.hpp"
#include "../Tools/Tools.hpp"

using namespace nite;

float entityT::getWalkLerp(){
  return walkTime/320.0f;
}

void entityT::create(){
  walkTime   = 40; //A step every 80 milliseconds
  walkT      = 0;
}

void entityT::destroy(){

}

void entityT::update(){

}

void entityT::move(int hSpaces, int vSpaces){
  if(getTicks()-walkT < walkTime) return;
  World->move(this, position.x+hSpaces, position.y+vSpaces);
  walkT = getTicks();
}
