#include "Engine/Console.hpp"
#include "Engine/Shapes.hpp"
#include "Map.hpp"
#include "Game.hpp"

/////////////
// COMMAND: map_reload
////////////
static nite::Console::Result cfMapReload(Vector<String> params){
	static auto game = Game::getGameCoreInstance();
	// auto &map = game->map;
	// map.reload();
	return nite::Console::Result();
}
static auto cfMapReloadIns = nite::Console::CreateFunction("map_reload", &cfMapReload); 

static bool showNvMap = false;
static nite::Console::CreateProxy cpRenAutoReso("map_shownvmap", nite::Console::ProxyType::Bool, sizeof(bool), &showNvMap);


Game::NavMap::NavMap(Vector<Shared<nite::PhysicsObject>> &locals, nite::Vec2 nvmBs, nite::Vec2 mBs, nite::Vec2 mapSizeInBlock){
	build(locals, nvmBs, mBs, mapSizeInBlock);
}

Game::NavMap::NavMap(){

}

void Game::NavMap::build(Vector<Shared<nite::PhysicsObject>> &locals, nite::Vec2 nvmBs, nite::Vec2 mBs, nite::Vec2 mapSizeInBlock){
	this->mapBlockSize = mBs;
	this->nvBlockSize = nvmBs;
	this->mapSize = mapSizeInBlock * mBs;
	this->nvSize = this->mapSize / this->nvBlockSize;
	this->blockNumber = nvSize.x * nvSize.y;
	nite::print("Building NavMap ("+nite::toStr(this->blockNumber)+" blocks): mapBlockSize "+mBs.str()+" | mapSize "+mapSize.str()+" |  navMapSize "+nvSize.str());	
	this->grid = Shared<Game::NavMapBlock>(new Game::NavMapBlock[blockNumber]);

	for(int i = 0; i < this->blockNumber; ++i){
		nite::Vec2 p = nite::Vec2(i % (int)(nvSize.x), (int)(i / nvSize.x)) * this->nvBlockSize; 
		auto &block = this->grid.get()[i];
		block.value = Game::NavMapType::Empty;
		block.position = p;
		block.size = this->nvBlockSize;
		block.index = i;
		for(int j = 0; j < locals.size(); ++j){
			nite::PhysicsObject dummy;
			dummy.solid = true;
			dummy.unmovable = false;
			dummy.size = this->nvBlockSize;			
			dummy.position.set(p);
			if(dummy.isCollidingWith(locals[j])){
				block.value = locals[j]->unmovable ? Game::NavMapType::SolidStatic : Game::NavMapType::SolidMovable;
			}
		}
	}
}

void Game::NavMap::draw(float x, float y){
	// for debugging only
	nite::setDepth(nite::DepthTop);
	static nite::Texture tex("data/sprite/empty.png");
	nite::setRenderTarget(nite::RenderTargetGame);
	float ga = 0.85f;
	nite::setColor(0.0f, 0.0f, 0.0f, 1.0f);
	tex.draw(x, y, 16.0f, 16.0f, 0.5f, 0.5f, 0.0f);
	for(int i = 0; i < this->blockNumber; ++i){
		auto &block = this->grid.get()[i];
		nite::setColor(block.value == Game::NavMapType::Empty ? nite::Color(0.0f, 1.0f, 0.0f, ga) : nite::Color(1.0f, 0.0f, 0.0f, ga));	
		tex.draw(x + block.position.x, y + block.position.y, block.size.x, block.size.y, 0.0f, 0.0f, 0.0f);
	}
}

void Game::Map::load(const String &path, const nite::Color &transparency){
	static auto game = Game::getGameCoreInstance();
	this->path = path;
	this->transparency = transparency;

	clear();
	nite::print("loading map '"+path+"'...");

	UInt64 startTime = nite::getTicks();
	Jzon::Parser parser;
	Jzon::Node node = parser.parseFile(path);
	if (!node.isValid()){
		nite::print("failed to read map "+path+": '"+parser.getError()+"'");
		return;
	}

	// load tilesets
	tiles.load(path);
	tiles.loadSources(transparency);

	// load map metadata
	mapSize.set(node.get("width").toInt(), node.get("height").toInt());
    tileSize.set(node.get("tilewidth").toInt(), node.get("tileheight").toInt());

	
	// load objects
	Jzon::Node layers = node.get("layers");
	for(int i = 0; i < layers.getCount(); ++i){
		if(layers.get(i).get("type").toString() != "objectgroup") continue;
		Jzon::Node objects = layers.get(i).get("objects");
		for(int c = 0; c < objects.getCount(); ++c){
			// COLLISIONS
			if(objects.get(c).get("type").toString() == "collision"){
				nite::Vec2 size(objects.get(c).get("width").toFloat(), objects.get(c).get("height").toFloat());
				nite::Vec2 position(objects.get(c).get("x").toFloat(), objects.get(c).get("y").toFloat());
				auto mask = Shared<nite::PhysicsObject>(new nite::PhysicsObject());
				mask->solid = true;
				mask->unmovable = true;
				mask->size.set(size);
				locals.push_back(mask);
				// game->world.add(mask);
				mask->position.set(position + size * 0.5f);
			}    
			// PLAYER SPAWN
			if(objects.get(c).get("type").toString() == "player_spawn"){
				nite::Vec2 size(objects.get(c).get("width").toFloat(), objects.get(c).get("height").toFloat());
				nite::Vec2 position(objects.get(c).get("x").toFloat(), objects.get(c).get("y").toFloat());
				playerSpawn.set(position + size * 0.5f);
			}
		}
	}
	// build nav map
	navMap.build(locals, nite::Vec2(32.0f), tileSize, mapSize); //we'll be use 32x32 for nvm block size 
	
	// this->mapFile = map;
	// loaded = true;
	// loadTimeout = nite::getTicks();
	nite::print("done: objects: "+nite::toStr(locals.size())+". Time "+nite::toStr(nite::getTicks()-startTime)+" msecs.");
}

void Game::Map::load(const String &path){
	load(path, nite::Color(-1.0f, -1.0f, -1.0f, -1.0f));
}

void Game::Map::update(){

}

void Game::Map::reload(){
    nite::Console::add("map '"+path+"' reload requested...", nite::Color(0.80f, 0.80f, 0.22f, 1.0f));
	if(!nite::fileExists(path)){
		nite::Console::add("cannot loaded map '"+path+"'. It doesn't exist.", nite::Color(0.80f, 0.15f, 0.22f, 1.0f));
		return;
	}
	clear();
	this->load(path, transparency);
}

void Game::Map::render(){
  nite::setDepth(0.0f);
  nite::setRenderTarget(nite::RenderTargetGame);
  nite::setColor(1.0f, 1.0f, 1.0f, 1.0f);
  tiles.draw(0.0f, 0.0f);
  if(showNvMap){
  	this->navMap.draw(0.0f, 0.0f);
  }
}

void Game::Map::clear(){
  static auto game = Game::getGameCoreInstance();
  nite::dropFrame();
  tiles.clear();
  tiles = nite::Tileset();
  for(int i = 0; i < locals.size(); ++i){
    // game->world.remove(locals[i]->id);
    //delete locals[i];
  }
  locals.clear();
}