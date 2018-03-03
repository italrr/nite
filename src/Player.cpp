#include "Player.hpp"
#include "Engine/Input.hpp"

void Game::Player::onCreate(){
  unmovable = false;
  solid = true;
  friction = 0.25f; 
  mass = 2.8f;
  dead = false;
  walkPushRate = 5.0f;
  name = "Player Type";
  setupEntity(50, 1.0f);
  entityInit();
  addBaseStat(BaseStatType::Strength, 100);
  addBaseStat(BaseStatType::Agility, 0);
  addBaseStat(BaseStatType::Dexterity, 0);
  addBaseStat(BaseStatType::Intelligence, 0);
  addBaseStat(BaseStatType::Endurance, 0);
  addBaseStat(BaseStatType::Luck, 0);
  printEntity();
}

void Game::Player::step(){

  if(nite::keyboardCheck(nite::keyUP) && nite::keyboardCheck(nite::keyRIGHT)){
    entityMove(45.0f, walkPushRate);
  }else
  if(nite::keyboardCheck(nite::keyDOWN) && nite::keyboardCheck(nite::keyRIGHT)){
    entityMove(315.0f, walkPushRate);
  }else
  if(nite::keyboardCheck(nite::keyUP) && nite::keyboardCheck(nite::keyLEFT)){
    entityMove(135.0f, walkPushRate);
  }else		
  if(nite::keyboardCheck(nite::keyDOWN) && nite::keyboardCheck(nite::keyLEFT)){
    entityMove(225.0f, walkPushRate);
  }else				
  if(nite::keyboardCheck(nite::keyUP)){
    entityMove(90.0f, walkPushRate);
  }else
  if(nite::keyboardCheck(nite::keyRIGHT)){
    entityMove(0.0f, walkPushRate);
  }else
  if(nite::keyboardCheck(nite::keyDOWN)){
    entityMove(270.0f, walkPushRate);
  }else
  if(nite::keyboardCheck(nite::keyLEFT)){
    entityMove(180.0f, walkPushRate);
  }		

  if(nite::keyboardPressed(nite::keyZ)){
	entityUseActiveSlot(Game::InventoryActiveSlot::Main);
  }
}