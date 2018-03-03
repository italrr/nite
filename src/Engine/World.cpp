#include "World.hpp"
#include "Tools/Tools.hpp"

using namespace nite;

static int getUniqueId(){
	static int seed = 1 + nite::randomInt(300, 350);
	return ++seed;
}

nite::World::World(){
	timescale = 1.0f;
	debugPhysics = false;
}

bool nite::World::exists(int id){
	return id != -1 ? objects.find(id) != objects.end() : false;
}

int nite::World::add(Shared<nite::PhysicsObject> obj){
	int id = getUniqueId();
	objects[id] = obj;
	obj->id = id;
	obj->container = this;
	obj->onCreate();
}

void nite::World::clear(){
	objects.clear();
}

void nite::World::remove(int objectId){
	auto search = objects.find(objectId);
	if(search != objects.end()) {
		for(int i = 0; i < removeQueue.size(); ++i){
			if(removeQueue[i] == objectId) return;
		}
		removeQueue.push_back(objectId);
	}else{
		nite::print("Unable to remove object id '"+nite::toStr(objectId)+"': It doesn't exist.");
	}
}

void nite::World::remove(Shared<nite::PhysicsObject> obj){
	this->remove(obj->id);
}

void nite::World::step(){
  for (auto& it : objects){
		auto current = it.second;
		current->entityStep();
		current->step();
	}
}

void nite::World::update(){
  for (auto& it : objects){
		auto current = it.second;
		current->update();
		current->updatePhysics();
	}
	// clean up
	for(int i = 0; i < removeQueue.size(); ++i){
		objects[removeQueue[i]]->onDestroy();
		objects[removeQueue[i]]->id = -1;
		objects.erase(removeQueue[i]);
	}
	removeQueue.clear();
}

void nite::World::render(){
  for (auto& it : objects){
		auto current = it.second;
		current->entityDraw();
		current->draw();
	}
}
