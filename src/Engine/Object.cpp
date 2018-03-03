#include "Object.hpp"
#include "World.hpp"
#include "Types.hpp"
#include "Graphics.hpp"
#include "Shapes.hpp"
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
	position.set(0.0f);
}

void nite::PhysicsObject::move(float an, float mod){
	float x = nite::cos(an) * mod;
	float y = nite::sin(an) * mod;
	speed.x += x;
	speed.y += y;
	if(x < 0 && speed.x < x || x > 0 && speed.x > x) speed.x = x;
	if(y < 0 && speed.y < y || y > 0 && speed.y > y) speed.y = y;
}

void nite::PhysicsObject::push(float angle, float force){
	float _f = (force / mass);
	float x = nite::cos(angle) * _f;
	float y = nite::sin(angle) * _f;
	speed.x += x;
	speed.y += y;
}

bool nite::PhysicsObject::isCollidingWith(PhysicsObject *other){
	return ((position.x + size.x * 0.5 > other->position.x-other->size.x * 0.5 &&
	position.x - size.x * 0.5 < other->position.x+other->size.x * 0.5) &&
	(position.y + size.y * 0.5 > other->position.y-other->size.y * 0.5 &&
	position.y - size.y * 0.5 < other->position.y+other->size.y * 0.5));
}

float nite::PhysicsObject::getDistance(PhysicsObject *other){
	//return (other->position.x - position.x) * (other->position.x - position.x) + (other->position.y - position.y) * (other->position.y - position.y);
	return nite::sqrt((other->position.x - position.x) * (other->position.x - position.x) + (other->position.y - position.y) * (other->position.y - position.y));
}

static inline float veryFastDistance(float x, float y){
	return x * x + y * y;
}

bool nite::PhysicsObject::isCollidingWithSomething(){
	isCollidingWithExcept(Vector<nite::PhysicsObject*>());
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
		auto obj = it.second;
		if(obj->id == this->id) continue;
		if(!obj->solid) continue;
		if(isInIgnores(obj.get())) continue;
		if(isCollidingWith(obj.get())){
			return true;
		}
	}
	collided = true;
	return false;
}

void nite::PhysicsObject::physicsObjectUpdate(float x, float y){
	if(unmovable) return;
	if(x != 0){
		position.x += x * container->timescale * relativeTimescale * nite::getDelta() * nite::getTimescale() * 0.067f;
		if(solid){
			for (auto& it : container->objects){
				auto obj = it.second;
				if(obj->id == this->id) continue;				
				if(!obj->solid) continue;
				if(isCollidingWith(obj.get())){
					onCollision(obj.get());
					collided = true;
					if(x > 0){
						position.x = obj->position.x - obj->size.x * 0.5 - size.x * 0.5 - 1;
					}
					if(x < 0){
						position.x = obj->position.x + obj->size.x * 0.5 + size.x * 0.5 + 1;
					}
					x = 0;
					break;
				}
			}
		}
	}
	if(y != 0){
		position.y += y * container->timescale * relativeTimescale * nite::getDelta() * nite::getTimescale() * 0.067f;
		if(solid){
			for (auto& it : container->objects){	
				auto obj = it.second;		
				if(obj->id == this->id) continue;
				if(!obj->solid) continue;
				if(isCollidingWith(obj.get())){
					onCollision(obj.get());
					collided = true;
					if(y > 0){
						position.y = obj->position.y - obj->size.y/2.0 - size.y/2.0 - 1;
					}
					if(y < 0){
						position.y = obj->position.y + obj->size.y/2.0 + size.y/2.0 + 1;
					}
					y = 0;
					break;
				}
			}
		}
	}
}

void nite::PhysicsObject::updatePhysics(){
	if(container != NULL && container->debugPhysics){
		nite::setDepth(nite::DepthTop);
        nite::setRenderTarget(nite::RenderTargetGame);
		nite::setColor(collided ? nite::Color(1.0f, 0, 0) : nite::Color(0, 1.0f, 0));
		Draw::Rectangle(position, size, false, Vec2(0.5, 0.5), 0);
		setDepth(0);
		resetColor();
	}
	collided = false;
	physicsObjectUpdate(speed.x, speed.y);
	speed.lerp(Vec2(0.0f), nite::getDelta() * 0.067f * friction * relativeTimescale * container->timescale);
}
