#include "World.hpp"
#include "Tools/Tools.hpp"
#include "Graphics.hpp"
#include "Shapes.hpp"

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
		nite::print("failed to remove object id '"+nite::toStr(objectId)+"': it doesn't exist.");
	}
}

void nite::World::remove(Shared<nite::PhysicsObject> obj){
	this->remove(obj->id);
}

void nite::World::step(){
  for (auto& it : objects){
		auto current = it.second;
		current->step();
		current->onStep();
	}
}

void nite::World::updateObjectPhysics(Shared<nite::PhysicsObject> obj, float x, float y){
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

void nite::World::update(){
  	for (auto& it : objects){
		auto current = it.second;
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
		current->draw();
		current->onDraw();
	}
}
