#include "World.hpp"
#include "../Engine/Shapes.hpp"
#include "../Engine/Tools/Tools.hpp"
#include "../Engine/Graphics.hpp"

using namespace nite;

static UInt16 generateId(){
	static UInt16 seed = nite::randomInt(25, 50);
	return ++seed; // networld ids are never to be 0. so a response with 0 means error
}

Game::NetWorld::NetWorld(){
	timescale = 1.0f;
	debugPhysics = false;
	objects.reserve(5000);
}

bool Game::NetWorld::exists(UInt16 id){
	return id != -1 ? objects.find(id) != objects.end() : false;
}

UInt16 Game::NetWorld::add(Shared<Game::NetObject> obj){
	UInt16 id = generateId();
	objects[id] = obj;
	obj->id = id;
	obj->container = this;
	obj->onCreate();
	return id;
}

void Game::NetWorld::clear(){
	objects.clear();
}

void Game::NetWorld::remove(UInt16 objectId){
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
		current->step();
		current->onStep();
	}
}

void Game::NetWorld::updateObjectPhysics(Shared<nite::PhysicsObject> obj, float x, float y){
	if(obj->unmovable) return;
	if(x != 0){
		obj->position.x += x * this->timescale * obj->relativeTimescale * nite::getDelta() * nite::getTimescale() * 0.067f;
		if(obj->solid){
			for (auto& it : this->objects){
				auto obj = it.second;
				if(obj->id == obj->id) continue;				
				if(!obj->solid) continue;
				if(obj->isCollidingWith(obj)){
					obj->onCollision(obj);
					obj->collided = true;
					if(x > 0.0f){
						obj->position.x = obj->position.x - obj->size.x * 0.5f - obj->size.x * 0.5f - 1.0f;
					}
					if(x < 0.0f){
						obj->position.x = obj->position.x + obj->size.x * 0.5f + obj->size.x * 0.5f + 1.0f;
					}
					x = 0.0f;
					break;
				}
			}
		}
	}
	if(y != 0){
		obj->position.y += y * this->timescale * obj->relativeTimescale * nite::getDelta() * nite::getTimescale() * 0.067f;
		if(obj->solid){
			for (auto& it : this->objects){	
				auto obj = it.second;		
				if(obj->id == obj->id) continue;
				if(!obj->solid) continue;
				if(obj->isCollidingWith(obj)){
					obj->onCollision(obj);
					obj->collided = true;
					if(y > 0.0f){
						obj->position.y = obj->position.y - obj->size.y/2.0f - obj->size.y/2.0f - 1.0f;
					}
					if(y < 0.0f){
						obj->position.y = obj->position.y + obj->size.y/2.0f + obj->size.y/2.0f + 1.0f;
					}
					y = 0.0f;
					break;
				}
			}
		}
	}	
}

void Game::NetWorld::update(){
  	for (auto it : objects){
		auto current = it.second;
		nite::Vec2 p = current->position;
		if(this->debugPhysics){
			nite::setDepth(nite::DepthTop);
			nite::setRenderTarget(nite::RenderTargetGame);
			nite::setColor(current->collided ? nite::Color(1.0f, 0, 0) : nite::Color(0, 1.0f, 0));
			Draw::Rectangle(current->position, current->size, false, Vec2(0.5, 0.5), 0);
			setDepth(0);
			resetColor();
		}
		current->collided = false;
		updateObjectPhysics(it.second, current->speed.x, current->speed.y);
		current->speed.lerp(Vec2(0.0f), nite::getDelta() * 0.067f * current->friction * current->relativeTimescale * this->timescale);
		if(current->position != p){
			this->updateQueue.push_back(current->id);
		}		
	}
	// clean up
	for(int i = 0; i < removeQueue.size(); ++i){
		objects[removeQueue[i]]->onDestroy();
		objects[removeQueue[i]]->id = 0;
		objects.erase(removeQueue[i]);
	}
	removeQueue.clear();
}

void Game::NetWorld::render(){
  for (auto& it : objects){
		auto current = it.second;
		current->draw();
		current->onDraw();
	}
}
