#include "Mob.hpp"
#include "AI/AI.hpp"

void Game::BasicMob::onCreate(){

  unmovable = false;
  solid = true;
  friction = 0.25f; 
  mass = 2.8f;
  dead = false;
  walkPushRate = 5.0f;
  name = "Mob Type";
  setupEntity(50, 1.0f);
  entityInit();
  addBaseStat(BaseStatType::Strength, 0);
  addBaseStat(BaseStatType::Agility, 100);
  addBaseStat(BaseStatType::Dexterity, 0);
  addBaseStat(BaseStatType::Intelligence, 0);
  addBaseStat(BaseStatType::Endurance, 0);
  addBaseStat(BaseStatType::Luck, 0);
//   printEntity();

  // addBehavior(std::shared_ptr<Game::AI::BaseBehavior>(new Game::AI::StareBehavior()));
  addBehavior(std::shared_ptr<Game::AI::BaseBehavior>(new Game::AI::DumbFollowBehavior()));
}

void Game::BasicMob::addBehavior(std::shared_ptr<Game::AI::BaseBehavior> behavior){
  behavior->init();
  behaviors.push_back(behavior);
}

void Game::BasicMob::step(){
  for(int i = 0; i < behaviors.size(); ++i){
    if(behaviors[i]->isReady()){
      behaviors[i]->think(*this);
    }
  }
}