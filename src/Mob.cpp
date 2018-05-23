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
  addBaseStat(BaseStatType::Agility, 0);
  addBaseStat(BaseStatType::Dexterity, 0);
  addBaseStat(BaseStatType::Intelligence, 0);
  addBaseStat(BaseStatType::Endurance, 100);
  addBaseStat(BaseStatType::Luck, 0);
//   printEntity();

  add(std::shared_ptr<Game::OverworldBehavior>(new Game::TestStareOB()));
}

void Game::BasicMob::add(std::shared_ptr<Game::OverworldBehavior> behavior){
  behavior->init();
  overworldAI.push_back(behavior);
}

void Game::BasicMob::step(){
  for(int i = 0; i < overworldAI.size(); ++i){
    if(overworldAI[i]->isReady()){
      overworldAI[i]->think(*this);
    }
  }
}