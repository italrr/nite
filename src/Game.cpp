#include "Game.hpp"
#include "Sword.hpp"
#include "Engine/Shapes.hpp"

using namespace nite;

static Game::GameMaster *instance = NULL;

static bool cameraFreeroam = false;
static nite::Console::CreateProxy cpAnDatTo("cl_camera_freeroam", nite::Console::ProxyType::Bool, sizeof(bool), &cameraFreeroam);

Game::Camera::Camera(){
	followId = -1;
}

void Game::Camera::update(nite::Vec2 &v, float mu){
	static auto *inst = Game::getInstance();		
	nite::setView(true, nite::RenderTargetGame);
	nite::Vec2 P = nite::getView(nite::RenderTargetGame);
	nite::Vec2 K = v - nite::getSize() * 0.5f;
	P.lerp(K, mu);
	setViewPosition(P, nite::RenderTargetGame);
}

void Game::Camera::update(nite::Vec2 &v){
	update(v, 0.15f);
}

void Game::Camera::update(){
	static auto *inst = Game::getInstance();	
	if(cameraFreeroam){
		nite::Vec2 np = nite::getView(nite::RenderTargetGame);
		if(nite::keyboardCheck(nite::keyLEFT)){
			np.set(nite::getView(nite::RenderTargetGame) - nite::Vec2(32.0f, 0.0f));
		}
		if(nite::keyboardCheck(nite::keyRIGHT)){
			np.set(nite::getView(nite::RenderTargetGame) + nite::Vec2(32.0f, 0.0f));
		}		
		if(nite::keyboardCheck(nite::keyUP)){
			np.set(nite::getView(nite::RenderTargetGame) - nite::Vec2(0.0f, 32.0f));			
		}
		if(nite::keyboardCheck(nite::keyDOWN)){
			np.set(nite::getView(nite::RenderTargetGame) + nite::Vec2(0.0f, 32.0f));						
		}		
		nite::setView(true, nite::RenderTargetGame);
		nite::setViewPosition(np, nite::RenderTargetGame);
		return;
	}
	if(followId == -1 || !inst->world.exists(followId)) return;
	update(inst->world.objects[followId]->position);
}

void Game::Camera::follow(int id){
	this->followId = id;
	cameraFreeroam = false;
	nite::print("Following entity id "+nite::toStr(id));
	// setView(id != -1, nite::RenderTargetGame);
}

/////////////
// COMMAND: player_reloadanims
////////////
static void cfReloadPlayerAnims(Vector<String> params){
	static auto game = Game::getInstance();
	game->player->entityReloadAnimation();
}
static auto cfReloadPlayerAnimsIns = nite::Console::CreateFunction("player_reloadanims", &cfReloadPlayerAnims);  

/////////////
// COMMAND: player_info
////////////
static void cfPrintPlayerInfo(Vector<String> params){
	static auto game = Game::getInstance();
	game->player->printEntity();
}
static auto cfPrintPlayerInfoIns = nite::Console::CreateFunction("player_info", &cfPrintPlayerInfo);  

/////////////
// COMMAND: player_statadd
////////////
static void cfPlayerStatUp(Vector<String> params){
	static auto game = Game::getInstance();
	if(params.size() < 2){
		nite::Console::add("Not enough parameters(2)", nite::Color(0.80f, 0.15f, 0.22f, 1.0f));
		return;
	}
	String &stat = params[0];
	String &amount = params[1];
	if(!nite::isNumber(amount)){
		nite::Console::add("'"+amount+"' is not a valid parameter", nite::Color(0.80f, 0.15f, 0.22f, 1.0f));
		return;
	}	
	int amnt = nite::toInt(amount);
	int added = 0;
	bool success = true;
	if(stat == "str"){
		added = game->player->addBaseStat(Game::BaseStatType::Strength, amnt);
	}else
	if(stat == "agi"){
		added = game->player->addBaseStat(Game::BaseStatType::Agility, amnt);
	}else
	if(stat == "dex"){
		added = game->player->addBaseStat(Game::BaseStatType::Dexterity, amnt);
	}else	
	if(stat == "end"){
		added = game->player->addBaseStat(Game::BaseStatType::Endurance, amnt);
	}else	
	if(stat == "luk"){
		added = game->player->addBaseStat(Game::BaseStatType::Luck, amnt);
	}else	
	if(stat == "cha"){
		added = game->player->addBaseStat(Game::BaseStatType::Charisma, amnt);
	}else		
	if(stat == "int"){
		added = game->player->addBaseStat(Game::BaseStatType::Intelligence, amnt);
	}else{
		success = false;
		nite::print("Invalid stat "+stat);
	}
	if(success){
		nite::print("Increase player's "+stat+" by "+nite::toStr(added));
	}
	
}
static auto cfPlayerStatUpIns = nite::Console::CreateFunction("player_statadd", &cfPlayerStatUp);  

/////////////
// COMMAND: player_statreset
////////////
static void cfPlayerStatReset(Vector<String> params){
	static auto game = Game::getInstance();
	if(params.size() < 1){
		nite::Console::add("Not enough parameters(1)", nite::Color(0.80f, 0.15f, 0.22f, 1.0f));
		return;
	}
	String &stat = params[0];
	bool success = true;
	if(stat == "str"){
		game->player->resetStat(Game::BaseStatType::Strength);
	}else
	if(stat == "agi"){
		game->player->resetStat(Game::BaseStatType::Agility);
	}else
	if(stat == "dex"){
		game->player->resetStat(Game::BaseStatType::Dexterity);
	}else	
	if(stat == "end"){
		game->player->resetStat(Game::BaseStatType::Endurance);
	}else	
	if(stat == "luk"){
		game->player->resetStat(Game::BaseStatType::Luck);
	}else
	if(stat == "cha"){
		game->player->resetStat(Game::BaseStatType::Charisma);
	}else	
	if(stat == "int"){
		game->player->resetStat(Game::BaseStatType::Intelligence);
	}else{
		success = false;
		nite::print("Invalid stat "+stat);
	}
	if(success){
		nite::print("Reset player's "+stat+" to 0");
	}
	
}
static auto cfPlayerStatResetIns = nite::Console::CreateFunction("player_statreset", &cfPlayerStatReset); 

/////////////
// COMMAND: show_entities
////////////
static void cfShowEntities(Vector<String> params){
	static auto game = Game::getInstance();
	String output;
	for (auto& it : game->world.objects){
		auto current = it.second;
		if(auto ent = dynamic_cast<Game::Entity*>(current.get())){
			output += "'"+ent->name+"' id: "+nite::toStr(ent->id)+", ";
		}
	}	
	nite::print("Active entities: "+output);
	
}
static auto cfShowEntitiesIns = nite::Console::CreateFunction("show_entities", &cfShowEntities); 

/////////////
// COMMAND: cl_camera_follow
////////////
static void cfCameraFollow(Vector<String> params){
	static auto game = Game::getInstance();
	if(params.size() < 1){
		nite::Console::add("Not enough parameters(1)", nite::Color(0.80f, 0.15f, 0.22f, 1.0f));
		return;
	}
	auto &_id = params[0];
	if(!nite::isNumber(_id)){
		nite::Console::add("'"+_id+"' is not a valid parameter", nite::Color(0.80f, 0.15f, 0.22f, 1.0f));
		return;
	}
	auto id = nite::toInt(_id);
	if(!game->world.exists(id)){
		nite::Console::add("Entity id '"+_id+"' does not exist", nite::Color(0.80f, 0.15f, 0.22f, 1.0f));
		return;		
	}
	cameraFreeroam = false;
	game->camera.follow(id);	
}
static auto cfCameraFollowIns = nite::Console::CreateFunction("cl_camera_follow", &cfCameraFollow); 



void Game::GameMaster::start(){
	instance = this;
	// nite Engine init
	nite::graphicsInit();
	nite::inputInit();
	renderEntities = true;
	isRunning = true;
	
	
	// Load a placeholder map
	map.load("data/map/test.json", nite::Color(1.0f, 1.0f, 1.0f, 1.0f));

	
	auto player = Shared<nite::PhysicsObject>(new Game::Player());
	world.add(player);

	this->player = static_cast<Entity*>(player.get());
	this->player->fullHeal();
	this->player->position.set(map.playerSpawn);
	
	auto mob = Shared<nite::PhysicsObject>(new Game::BasicMob());
	world.add(mob);
	mob->position.set(1100, 400);
	auto mobEntity = static_cast<Entity*>(mob.get());
	mobEntity->fullHeal();	
	

	auto sword = Shared<Game::BaseItem>(new Sword());
	sword.get()->load("data/weap/base_sword.json");
	auto playerEntity = static_cast<Entity*>(player.get());
	playerEntity->addItem(sword);	
	playerEntity->setActiveItem(Game::InventoryActiveSlot::Main, sword);	
	 camera.follow(player->id);
}

void Game::GameMaster::update(){
	// nite::setZoom(nite::RenderTargetGame, 0.80f);
	// Basic Shortcuts
	if(nite::keyboardPressed(nite::keyF1)){
		world.debugPhysics = !world.debugPhysics;
	}
	if(nite::keyboardPressed(nite::keyTILDE)){
		if(nite::Console::isOpen()){
			nite::Console::close();
		}else{
			nite::Console::open();
		}
	}

	// Update World
	nite::viewUpdate();
	nite::inputUpdate();

	nite::Console::update();
	ui.update();
	map.update();
	camera.update();
	world.update();
	world.step();
}

void Game::GameMaster::render(){	
	nite::graphicsUpdate();
	nite::Console::render();
	ui.render();
	map.render();
	world.render();		
}

void Game::GameMaster::end(){
	isRunning = false;
}

void Game::GameMaster::onEnd(){
	nite::Console::end();

}

Game::GameMaster *Game::getInstance(){
	return instance;
}

int main(int argc, char* argv[]){
	Vector<String> params;
	for(int i = 0; i < argc; ++i){
		params.push_back(String(argv[i]));
	}
	nite::setParameters(params);

	Game::GameMaster game;
	game.start();


	while(game.isRunning){
		game.update();
		game.render();
	}
	game.onEnd();

	return 0;
}
