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
		instance->client.disconnect("disconnected by user");
	}
	auto map = Shared<nite::Map>(new nite::Map());
	map->load(path);
	instance->client.clear();
	instance->client.map = map;
	return nite::Console::Result();
}
static auto cfMapLoadIns = nite::Console::CreateFunction("map_load", &cfMapLoad);


static nite::Console::Result cfMapReadInfo(Vector<String> params){
	if(params.size() < 1){
		return nite::Console::Result("not enough parameters(1)", nite::Color(0.80f, 0.15f, 0.22f, 1.0f));
	}
	String path = params[0];
	if(!nite::fileExists(path)){
		return nite::Console::Result("failed to read map file '"+path+"': it doesn't exist", nite::Color(0.80f, 0.15f, 0.22f, 1.0f));
	}

	Jzon::Parser parser;
	auto obj = parser.parseFile(path);
	if(!obj.isValid()){
		return nite::Console::Result("failed to read map file '"+path+"': "+parser.getError(), nite::Color(0.80f, 0.15f, 0.22f, 1.0f));
	}

	String title = obj.get("title").toString("undefined");
	String author = obj.get("author").toString("undefined");
	String version = obj.get("version").toString("undefined");
	String hash = nite::hashFile(path);
	nite::print("map file '"+path+"':");
	nite::print("title: '"+title+"' | author: '"+author+"' | version: '"+version+"'");
	nite::print("md5: '"+hash+"' | size: "+nite::toStr(nite::fileSize(path))+" bytes");
	return nite::Console::Result();
}
static auto cfMapReadInfoIns = nite::Console::CreateFunction("map_read_info", &cfMapReadInfo);



static nite::Console::Result cfRINGGenerateMap(Vector<String> params){
	if(params.size() < 3){
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
	String tilesource = params[2];
	if(instance->client.connected){
		return nite::Console::Result("you cannot generate a map while in a game. disconnect first", nite::Color(0.80f, 0.15f, 0.22f, 1.0f));
	}
	instance->client.clear();
	// TODO: allow user to provide blueprint specs, tilesource and scale
	// may require to create other commands
	auto indexer = nite::getIndexer();
	auto *file = indexer->getByName(tilesource);
	if(file == NULL){
		return nite::Console::Result("'"+tilesource+"' doesn't exist", nite::Color(0.80f, 0.15f, 0.22f, 1.0f));
	}
	if(!file->isIt("tilesource")){
		return nite::Console::Result("'"+tilesource+"' is not a tilesource", nite::Color(0.80f, 0.15f, 0.22f, 1.0f));
	}
	nite::Console::interpret("cl_camera_forcezoom false", false, false, false, false);
	Game::RING::TileSource ts;
	ts.load(file->path);
	auto bp = Shared<Game::RING::Blueprint>(new Game::RING::Blueprint());
	bp->generate(x, y);
	instance->client.map = Game::RING::generateMap(bp, ts, 4);
	return nite::Console::Result();

}
static auto cfRINGGenerateMapIns = nite::Console::CreateFunction("ring_generate_map", &cfRINGGenerateMap);

void Game::GameCore::start(){
	instance = this;
	nite::graphicsInit();
	nite::inputInit();
	svTickRate = 1000 / 60;
	isRunning = true;
	stepGeneralTimer.name = "STEP GENERAL";
	drawGeneralTimer.name = "DRAW GENERAL";
	// load dbs
	Game::DBLoadSkill("data/db/skills.json");
	Game::DBLoadEffect("data/db/effects.json");
	Game::DBLoadInventory("data/db/items.json");
	nite::setView(true, nite::RenderTargetGame);
	spawnServerThread();
}

void Game::GameCore::end(){
	isRunning = false;
}

void Game::GameCore::onEnd(){
	nite::graphicsEnd();
	killServerThread();
	this->client.disconnect();
	nite::print("bye!");
}

void Game::GameCore::update(){
	nite::viewUpdate();
	nite::inputUpdate();
	this->client.update();
	this->client.camera.update();
}

static void *localSvThread(void *vargp){
	auto *core = static_cast<Game::GameCore*>(vargp);
	auto &sv = core->localSv;
    while(true){
		if(nite::getTicks()-core->svLastTick > core->svTickRate){
			sv.update();
			core->svLastTick = nite::getTicks();
		}
	}
    return NULL;
}

void Game::GameCore::spawnServerThread(){
	localSv.preinit();
	localSv.setupGame("Pacifier's corner", 4, 1);
	pthread_create(&svThread, NULL, localSvThread, this);
	nite::print("spawned local server thread. tickrate "+nite::toStr(this->svTickRate)+" msecs");
}

void Game::GameCore::killServerThread(){
	pthread_cancel(svThread);
	// pthread_join(svThread, NULL); // causes it to hang on windows
	nite::print("killed local server thread");
	this->localSv.close();
}


void Game::GameCore::render(){
	this->client.render();
	nite::graphicsRender();
}

Game::GameCore *Game::getGameCoreInstance(){
	return instance;
}
