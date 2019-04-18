#include "Game.hpp"
#include "Sword.hpp"
#include "Engine/Shapes.hpp"
#include "Engine/nScript.hpp"

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
	map.load("data/map/romdou/romdou.json");

	
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
	// TODO: make debugPhysics toggle be assigned from nScript files instead of hardcoded
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
	nite::nScript initDebug("debug_init.ns");
	initDebug.execute();
	while(game.isRunning){
		game.update();
		game.render();
	}
	game.onEnd();

	return 0;
}
