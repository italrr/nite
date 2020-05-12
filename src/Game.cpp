#include "Engine/View.hpp"
#include "Engine/Console.hpp"

#include "Game.hpp"
#include "Entity/Skill.hpp"
#include "Entity/Effect.hpp"
#include "Entity/Inventory.hpp"

static Game::GameCore *instance = NULL;
static nite::Vec2 mvSpeedDef = nite::Vec2(32.0f);	
static nite::Vec2 mvSpeedTop = nite::Vec2(600.0f);	
static nite::Vec2 mvSpeed = mvSpeedDef;	

static nite::Console::Result cfMapSave(Vector<String> params){
	if(params.size() < 1){
		return nite::Console::Result("not enough parameters(1)", nite::Color(0.80f, 0.15f, 0.22f, 1.0f));
	}
	String path = params[0];
	instance->client.map->exportToJson(path, true);
	return nite::Console::Result();
}
static auto cfMapSaveIns = nite::Console::CreateFunction("map_save", &cfMapSave); 

static nite::Console::Result cfMapLoad(Vector<String> params){
	if(params.size() < 1){
		return nite::Console::Result("not enough parameters(1)", nite::Color(0.80f, 0.15f, 0.22f, 1.0f));
	}
	String path = params[0];
	if(instance->client.connected){
		return nite::Console::Result("you cannot load up a map while on a game. disconnect first", nite::Color(0.80f, 0.15f, 0.22f, 1.0f));
	}
	auto map = Shared<nite::Map>(new nite::Map());
	map->load(path);
	instance->client.clear();
	instance->client.map = map;
	return nite::Console::Result();
}
static auto cfMapLoadIns = nite::Console::CreateFunction("map_load", &cfMapLoad); 


static nite::Console::Result cfRINGGenerateMap(Vector<String> params){
	if(params.size() < 2){
		return nite::Console::Result("not enough parameters(1)", nite::Color(0.80f, 0.15f, 0.22f, 1.0f));
	}

	if(!nite::isNumber(params[0])){
		return nite::Console::Result("invalid parameter '"+params[0]+"'", nite::Color(0.80f, 0.15f, 0.22f, 1.0f));
	}

	if(!nite::isNumber(params[1])){
		return nite::Console::Result("invalid parameter '"+params[1]+"'", nite::Color(0.80f, 0.15f, 0.22f, 1.0f));
	}	
	int x = nite::toInt(params[0]);
	int y = nite::toInt(params[1]);
	if(instance->client.connected){
		return nite::Console::Result("you cannot generate a map while on a game. disconnect first", nite::Color(0.80f, 0.15f, 0.22f, 1.0f));
	}	
	instance->client.clear();
	// TODO: allow to provide blueprint specs, tilesource and scale
	// may require to create other commands
	Game::RING::TileSource ts;
	ts.load("data/tileset/dungeon.json");
	auto bp = Shared<Game::RING::Blueprint>(new Game::RING::Blueprint());
	bp->generate(x, y);
	instance->client.map = Game::RING::generateMap(bp, ts, 3);
	return nite::Console::Result();

}
static auto cfRINGGenerateMapIns = nite::Console::CreateFunction("ring_generate_map", &cfRINGGenerateMap); 


static nite::Console::Result cfClSwitchMode(Vector<String> params){
	// static auto game = Game::getGameCoreInstance();
	if(params.size() < 1){
		return nite::Console::Result("not enough parameters(1)", nite::Color(0.80f, 0.15f, 0.22f, 1.0f));
	}

	return nite::Console::Result();
}
static auto cfClSwitchModeIns = nite::Console::CreateFunction("cl_mode_switch", &cfClSwitchMode); 

void Game::GameCore::start(){
	instance = this;
	nite::graphicsInit();
	nite::inputInit();
	isRunning = true;
	stepGeneralTimer.name = "STEP GENERAL";
	drawGeneralTimer.name = "DRAW GENERAL";
	// load dbs
	Game::DBLoadSkill("data/db/skills.json");
	Game::DBLoadEffect("data/db/effects.json");
	Game::DBLoadInventory("data/db/items.json");
	nite::setView(true, nite::RenderTargetGame);	
}

void Game::GameCore::end(){
	isRunning = false;
}

void Game::GameCore::onEnd(){
	nite::graphicsEnd();
	this->localSv.close();	
	this->client.disconnect();
	nite::print("bye!");
}

void Game::GameCore::update(){
	nite::setZoom(nite::RenderTargetGame, 0.75f);
	nite::viewUpdate();
	nite::inputUpdate();
	this->client.update();
	this->localSv.update();
}

void Game::GameCore::render(){	
	this->client.render();
	nite::graphicsRender();
}

Game::GameCore *Game::getGameCoreInstance(){
	return instance;
}