#include "World.hpp"
#include "../Engine/Tools/Tools.hpp"

using namespace nite;

static int generateId(){
	static UInt32 seed = nite::randomInt(25, 50);
	return ++seed;
}

Game::NetWorld::NetWorld(){
	timescale = 1.0f;
	debugPhysics = false;
	objects.reserve(1000);
}

bool Game::NetWorld::exists(UInt32 id){
	return id != -1 ? objects.find(id) != objects.end() : false;
}

UInt32 Game::NetWorld::add(Shared<Game::NetObject> obj){
	UInt32 id = generateId();
	objects[id] = obj;
	obj->id = id;
	obj->container = this;
	obj->onCreate();
	return id;
}

void Game::NetWorld::clear(){
	objects.clear();
}

void Game::NetWorld::remove(UInt32 objectId){
	auto search = objects.find(objectId);
	if(search != objects.end()) {
		for(int i = 0; i < removeQueue.size(); ++i){
			if(removeQueue[i] == objectId) return;
		}
		removeQueue.push_back(objectId);
	}else{
		nite::print("unable to remove object id '"+nite::toStr(objectId)+"': it doesn't exist.");
	}
}

void Game::NetWorld::remove(Shared<Game::NetObject> obj){
	this->remove(obj->id);
}

void Game::NetWorld::step(){
  for (auto& it : objects){
		auto current = it.second;
		current->entityStep();
		current->step();
	}
}

void Game::NetWorld::update(){
  for (auto& it : objects){
		auto current = it.second;
		current->update();
		current->updatePhysics();
	}
	// clean up
	for(UInt32 i = 0; i < removeQueue.size(); ++i){
		objects[removeQueue[i]]->onDestroy();
		objects[removeQueue[i]]->id = -1;
		objects.erase(removeQueue[i]);
	}
	removeQueue.clear();
}

void Game::NetWorld::render(){
  for (auto& it : objects){
		auto current = it.second;
		current->entityDraw();
		current->draw();
	}
}
