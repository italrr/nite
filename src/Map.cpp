#include "Map.hpp"
#include "Engine/Console.hpp"
#include "Game.hpp"

/////////////
// COMMAND: map_reload
////////////
static void cfMapReload(Vector<String> params){
	static auto game = Game::getInstance();
	auto &map = game->map;
	map.reload();
	
}
static auto cfMapReloadIns = nite::Console::CreateFunction("map_reload", &cfMapReload); 


void Game::Map::load(const String &path, const nite::Color &transparency){
	static auto game = Game::getInstance();
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
				locals.push_back(mask.get());
				game->world.add(mask);
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
  nite::setDepth(0.0);
  nite::setRenderTarget(nite::RenderTargetGame);
  nite::setColor(1.0f, 1.0f, 1.0f, 1.0f);
  tiles.draw(0, 0);
}

void Game::Map::clear(){
  static auto game = Game::getInstance();
  nite::dropFrame();

  tiles.clear();
  tiles = nite::Tileset();
  for(int i = 0; i < locals.size(); ++i){
    game->world.remove(locals[i]->id);
    //delete locals[i];
  }
  locals.clear();
}