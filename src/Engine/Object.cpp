#include <cmath>

#include "Object.hpp"
#include "World.hpp"
#include "Types.hpp"
#include "Graphics.hpp"
#include "Tools/Tools.hpp"

using namespace nite;

nite::Hitbox::Hitbox(const Jzon::Node &node){
	load(node);
}

nite::Hitbox::Hitbox(){

}

void nite::Hitbox::load(const Jzon::Node &node){
	position.set(node.get("position").get("x").toFloat(), node.get("position").get("y").toFloat());
	size.set(node.get("size").get("width").toFloat(), node.get("size").get("height").toFloat());
	type = node.get("type").toInt();
	name = node.get("name").toString();
}

bool nite::Hitbox::collision(const Hitbox &box){
	return ((position.x + size.x * 0.5 > box.position.x-box.size.x * 0.5 &&
		position.x - size.x * 0.5 < box.position.x+box.size.x * 0.5) &&
		(position.y + size.y * 0.5 > box.position.y-box.size.y * 0.5 &&
		position.y - size.y * 0.5 < box.position.y+box.size.y * 0.5));
}

void PhysicsObject::destroy(){
	if(container == NULL) return;
	container->remove(this->id);
}

nite::PhysicsObject::PhysicsObject(){
	relativeTimescale = 1.0f;
	friction = 0.10f;
	mass = 1.0f;
	container = NULL;
	position.set(0.0f);
}


bool nite::PhysicsObject::move(const nite::Vec2 &dir){
	speed = dir;
	return true;
}

bool nite::PhysicsObject::push(const nite::Vec2 &dir){
	speed = speed + dir;
	return true;
}

bool nite::PhysicsObject::isCollidingWith(nite::PhysicsObject *other){
	return ((position.x + size.x > other->position.x &&
	position.x  < other->position.x + other->size.x) &&
	(position.y + size.y > other->position.y &&
	position.y < other->position.y+other->size.y));
}

float nite::PhysicsObject::getDistance(nite::PhysicsObject *other){
	return nite::sqrt((other->position.x - position.x) * (other->position.x - position.x) + (other->position.y - position.y) * (other->position.y - position.y));
}

static inline float veryFastDistance(float x, float y){
	return x * x + y * y;
}

bool nite::PhysicsObject::isCollidingWithSomething(){
	return isCollidingWithExcept(Vector<nite::PhysicsObject*>());
}

bool nite::PhysicsObject::isCollidingWithExcept(const Vector<nite::PhysicsObject*> &ignores){
	auto isInIgnores = [&](nite::PhysicsObject *ref){
		for(int i = 0; i < ignores.size(); ++i){
			if(ignores[i] == ref){
				return true;
			}
		}	
		return false;
	};
	if(container == NULL) return false;
	for (auto& it : container->objects){		
		auto obj = it.second.get();
		if(obj->id == this->id) continue;
		if(!obj->solid) continue;
		if(isInIgnores(obj)) continue;
		if(isCollidingWith(obj)){
			return true;
		}
	}
	collided = true;
	return false;
}