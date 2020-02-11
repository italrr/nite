#include "Player.hpp"
#include "Engine/Input.hpp"
#include "Engine/Console.hpp"
#include "Game.hpp"

/////////////
// COMMAND: player_reloadanims
////////////
static void cfReloadPlayerAnims(Vector<String> params){
	static auto game = Game::getGameCoreInstance();
	// game->player->entityReloadAnimation();
}
static auto cfReloadPlayerAnimsIns = nite::Console::CreateFunction("player_reloadanims", &cfReloadPlayerAnims);  

/////////////
// COMMAND: player_info
////////////
static void cfPrintPlayerInfo(Vector<String> params){
	static auto game = Game::getGameCoreInstance();
	// game->player->printEntity();
}
static auto cfPrintPlayerInfoIns = nite::Console::CreateFunction("player_info", &cfPrintPlayerInfo);  

/////////////
// COMMAND: player_statadd
////////////
static void cfPlayerStatUp(Vector<String> params){
	// static auto game = Game::getGameCoreInstance();
	// if(params.size() < 2){
	// 	nite::Console::add("Not enough parameters(2)", nite::Color(0.80f, 0.15f, 0.22f, 1.0f));
	// 	return;
	// }
	// String &stat = params[0];
	// String &amount = params[1];
	// if(!nite::isNumber(amount)){
	// 	nite::Console::add("'"+amount+"' is not a valid parameter", nite::Color(0.80f, 0.15f, 0.22f, 1.0f));
	// 	return;
	// }	
	// int amnt = nite::toInt(amount);
	// int added = 0;
	// bool success = true;
	// if(stat == "str"){
	// 	added = game->player->addBaseStat(Game::BaseStatType::Strength, amnt);
	// }else
	// if(stat == "agi"){
	// 	added = game->player->addBaseStat(Game::BaseStatType::Agility, amnt);
	// }else
	// if(stat == "dex"){
	// 	added = game->player->addBaseStat(Game::BaseStatType::Dexterity, amnt);
	// }else	
	// if(stat == "end"){
	// 	added = game->player->addBaseStat(Game::BaseStatType::Endurance, amnt);
	// }else	
	// if(stat == "luk"){
	// 	added = game->player->addBaseStat(Game::BaseStatType::Luck, amnt);
	// }else	
	// if(stat == "cha"){
	// 	added = game->player->addBaseStat(Game::BaseStatType::Charisma, amnt);
	// }else		
	// if(stat == "int"){
	// 	added = game->player->addBaseStat(Game::BaseStatType::Intelligence, amnt);
	// }else{
	// 	success = false;
	// 	nite::print("Invalid stat "+stat);
	// }
	// if(success){
	// 	nite::print("Increase player's "+stat+" by "+nite::toStr(added));
	// }
	
}
static auto cfPlayerStatUpIns = nite::Console::CreateFunction("player_statadd", &cfPlayerStatUp);  

/////////////
// COMMAND: player_statreset
////////////
static void cfPlayerStatReset(Vector<String> params){
	// static auto game = Game::getGameCoreInstance();
	// if(params.size() < 1){
	// 	nite::Console::add("Not enough parameters(1)", nite::Color(0.80f, 0.15f, 0.22f, 1.0f));
	// 	return;
	// }
	// String &stat = params[0];
	// bool success = true;
	// if(stat == "str"){
	// 	game->player->resetStat(Game::BaseStatType::Strength);
	// }else
	// if(stat == "agi"){
	// 	game->player->resetStat(Game::BaseStatType::Agility);
	// }else
	// if(stat == "dex"){
	// 	game->player->resetStat(Game::BaseStatType::Dexterity);
	// }else	
	// if(stat == "end"){
	// 	game->player->resetStat(Game::BaseStatType::Endurance);
	// }else	
	// if(stat == "luk"){
	// 	game->player->resetStat(Game::BaseStatType::Luck);
	// }else
	// if(stat == "cha"){
	// 	game->player->resetStat(Game::BaseStatType::Charisma);
	// }else	
	// if(stat == "int"){
	// 	game->player->resetStat(Game::BaseStatType::Intelligence);
	// }else{
	// 	success = false;
	// 	nite::print("Invalid stat "+stat);
	// }
	// if(success){
	// 	nite::print("Reset player's "+stat+" to 0");
	// }
	
}
static auto cfPlayerStatResetIns = nite::Console::CreateFunction("player_statreset", &cfPlayerStatReset); 


void Game::Player::onCreate(){
  unmovable = false;
  solid = true;
  friction = 0.25f; 
  mass = 2.8f;
  dead = false;
  walkPushRate = 5.0f;
  name = "Player Type";
  objType = Game::ObjectType::Entity;
  sigId = Game::ObjectSig::Player;
  setupEntity(startLv, Game::EntityType::Humanoid, 1.0f);
  entityInit();
}

void Game::Player::step(){
	bool isSpace = nite::keyboardCheck(nite::keySPACE);
	if(nite::keyboardCheck(nite::keyUP) && nite::keyboardCheck(nite::keyRIGHT)){
		entityMove(-0.7854f, walkPushRate, isSpace);
	}else
	if(nite::keyboardCheck(nite::keyDOWN) && nite::keyboardCheck(nite::keyRIGHT)){
		entityMove(-5.498f, walkPushRate, isSpace);
	}else
	if(nite::keyboardCheck(nite::keyUP) && nite::keyboardCheck(nite::keyLEFT)){
		entityMove(-2.356f, walkPushRate, isSpace);
	}else		
	if(nite::keyboardCheck(nite::keyDOWN) && nite::keyboardCheck(nite::keyLEFT)){
		entityMove(-3.927f, walkPushRate, isSpace);
	}else				
	if(nite::keyboardCheck(nite::keyUP)){
		entityMove(-1.571f, walkPushRate, isSpace);
	}else
	if(nite::keyboardCheck(nite::keyRIGHT)){
		entityMove(0.0f, walkPushRate, isSpace);
	}else
	if(nite::keyboardCheck(nite::keyDOWN)){
		entityMove(-4.712f, walkPushRate, isSpace);
	}else
	if(nite::keyboardCheck(nite::keyLEFT)){
		entityMove(3.142f, walkPushRate, isSpace);
	}		

	if(nite::keyboardPressed(nite::keyZ)){
		entityUseActiveSlot(Game::InventoryActiveSlot::Main);
	}
}