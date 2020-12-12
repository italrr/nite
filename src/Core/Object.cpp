#include "Object.hpp"
#include "Network.hpp"
#include "World.hpp"
#include "Server.hpp"

#include "../Entity/Base.hpp"
#include "../Entity/Mobs/Humanoid.hpp"
#include "../Entity/Projectile.hpp"

/*
    All the different types of objects must be created from here
*/
Shared<Game::NetObject> Game::createNetObject(UInt16 id, UInt16 sig, float x, float y){
    switch(sig){
        case Game::ObjectSig::Player: {
            auto player = Shared<Game::NetObject>(new EntityBase());
            player->setPosition(x, y);
            player->id = id;
            player->sigId = Game::ObjectSig::Player;
            return player;
        } break;
        case Game::ObjectSig::MobHumanoid: {
            auto mob = Shared<Game::NetObject>(new MobHumanoid());
            mob->setPosition(x, y);
            mob->id = id;
            mob->sigId = Game::ObjectSig::MobHumanoid;
            return mob;
        } break;
        case Game::ObjectSig::Projectile: {
            auto proj = Shared<Game::NetObject>(new Projectile());
            proj->setPosition(x, y);
            proj->id = id;
            proj->sigId = Game::ObjectSig::Projectile;
            return proj;
        } break;        
        default:
            nite::print("[netplay] failed to created object of sig '"+nite::toStr(sig)+"': it's undefined");
            return Shared<Game::NetObject>(NULL);
    }
}

Game::NetObject::NetObject(){
    localId = 0;
    container = NULL;
    sv = NULL;
    quadrant = -1;
    frictionRate = 30;
    net = NULL;
    solid = true;
    objType = Game::ObjectType::Base;
    sigId = Game::ObjectSig::Base;
	relativeTimescale = 1.0f;
	friction = 0.10f;
	mass = 1.0f;
	localId = 0;
    speed = 0.0f;
    direction = 0.0f;
	container = NULL;
	position.set(0.0f);
    destroyed = false;
    clearDeltaUpdates();
}
#include <cmath>
static float lerp(float x1, float x2, float f){
	return  f * x2 + (1.0f - f) * x1;
}
static float damp(float src, float target, float smoothing, float dt){
    return lerp(src, target, 1.0f - nite::pow(smoothing, dt));
}
static float _lerp(float x1, float x2, float rate, float delta){
	float f = (1.0f - nite::pow(1.0f - rate, delta * 0.077f));
	return f * x2 + (1.0f - f) * x1;    
}
void Game::NetObject::runState(){
    currentState.delta = nextState.delta;
    // physics
    if(DeltaUpdateType::hasIssuedDeltaStateUpdate(DeltaUpdateType::PHYSICS, nextState.states)){
        float dt = (nextState.delta - prevState.delta) * net->gameTickRate;
        float step = 1.0f - ((float)prevState.delta) / ((float)nextState.delta);
        currentState.x = _lerp(prevState.x, nextState.x, step, dt);
        currentState.y = _lerp(prevState.y, nextState.y, step, dt);
        currentState.speed = _lerp(prevState.speed, nextState.speed, step, dt);
        currentState.direction = _lerp(prevState.direction, nextState.direction, step, dt);
        this->speed =  currentState.speed;
        this->position = nite::Vec2(currentState.x, currentState.y);
        this->direction = currentState.direction;
    }
    // anim
    if(DeltaUpdateType::hasIssuedDeltaStateUpdate(DeltaUpdateType::ANIMATION, nextState.states) && this->objType == ObjectType::Entity){
        float dt = (nextState.delta - prevState.delta) * net->gameTickRate;
        float step = (float)prevState.delta / (float)nextState.delta;
        currentState.faceDir = nextState.faceDir;
        currentState.xLookingAt = nextState.xLookingAt;
        currentState.yLookingAt = nextState.yLookingAt;
        for(int j = 0; j < AnimPart::total; ++j){
            currentState.animSt[j] = nextState.animSt[j];
            currentState.animNum[j] = nextState.animNum[j];
            currentState.animExtime[j] = nextState.animExtime[j];
        }        
        auto ent = static_cast<EntityBase*>(this);
        ent->faceDirection = currentState.faceDir;
        ent->pointingAt = nite::Vec2(currentState.xLookingAt, currentState.yLookingAt);
        for(int j = 0; j < AnimPart::total; ++j){
            ent->state[j] = currentState.animSt[j];
            ent->stNum[j] = currentState.animNum[j];
            ent->lastExpectedTime[j] = currentState.animExtime[j];
        }
    }
}

void Game::NetObject::destroy(){
	if(container == NULL || destroyed) return; 
    UInt16 _id = this->id;
    if(id == 0 && localId < 0){
        container->removeWallMask(this);
    }else{
	    container->remove(this->id);
        if(sv != NULL){
            nite::Packet desUpt;
            desUpt.setHeader(Game::PacketType::SV_DESTROY_OBJECT);
            desUpt.write(&_id, sizeof(_id));	
            sv->sendPersPacketForMany(sv->getAllClientsIps(), desUpt, sv->getAllClientsAcks());
        }          
    }   
    destroyed = true;  
}

void Game::NetObject::setPosition(const nite::Vec2 &p){
    this->position.set(p);
    // snapPosition();
    // this->lerpPosition.set(this->position);
    updateQuadrant();
    if(sv != NULL){
		nite::Packet posUpt;
		posUpt.setHeader(Game::PacketType::SV_SET_OBJECT_POSITION);
		posUpt.write(&id, sizeof(id));
		posUpt.write(&position.x, sizeof(position.x));		
		posUpt.write(&position.y, sizeof(position.y));		
		sv->sendPacketForMany(sv->getAllClientsIps(), posUpt);
    }
}


void Game::NetObject::snapPosition(){
    if(container == NULL){
        return;
    }
    position.x = nite::round(position.x / container->gridSpec.x) * container->gridSpec.x;
    position.y = nite::round(position.y / container->gridSpec.y) * container->gridSpec.y;
}

void Game::NetObject::updateQuadrant(){
    if(container != NULL && container->cells != NULL && container->ghosts != NULL){
        int _x = nite::floor((position.x / container->size.x) * container->cwidth);
        int _y = nite::floor((position.y / container->size.y) * container->cheight);
        if(_x < 0 || _x >= container->cwidth){
            _x = lqPos.x;
        }
        if(_y < 0 || _y >= container->cheight){
            _y = lqPos.y;
        }
        UInt32 nq = _x + container->cwidth * _y;
        if(nq == quadrant || (objType == ObjectType::Ghost ? (container->ghosts[nq] != NULL) : (container->cells[nq] != NULL))) {
            return;
        }
        if(quadrant != -1 && nq != quadrant && (objType == ObjectType::Ghost ? (container->ghosts[quadrant] == this) : (container->cells[quadrant] == this))){
            if(objType == ObjectType::Ghost){
                container->ghosts[quadrant] = NULL;
            }else{
                container->cells[quadrant] = NULL;
            }
        }
        quadrant = nq;
        if(nq < container->ctotal){
            if(objType == ObjectType::Ghost){
                container->ghosts[nq] = this;
            }else{
                container->cells[nq] = this;
            }
            lqPos.set(_x, _y); // last valid in-map quadrant position
        }
    }
}

void Game::NetObject::clearQuadrant(){
    if(container != NULL && quadrant != -1  && (objType == ObjectType::Ghost ? container->ghosts[quadrant] == this : container->cells[quadrant] == this)){
        if(objType == ObjectType::Ghost){
            container->ghosts[quadrant] = NULL;
        }else{
            container->cells[quadrant] = NULL;
        }
    }
    quadrant = -1;
}

bool Game::NetObject::push(float dir, float force){
    // auto add = nite::Vec2(nite::cos(dir) * force, nite::sin(dir) * force);
    // auto init = nite::Vec2(nite::cos(direction) * speed + add.x, nite::sin(direction) * speed + add.y);
    this->direction = dir;
    this->speed = force;
	return true;
}

bool Game::NetObject::isCollidingWith(Game::NetObject *b){
    return position.x < b->position.x + b->size.x &&
   position.x + size.x > b->position.x &&
   position.y < b->position.y + b->size.y &&
   position.y + size.y > b->position.y;
}

float Game::NetObject::getDistance(Game::NetObject *other){
	return nite::sqrt((other->position.x - position.x) * (other->position.x - position.x) + (other->position.y - position.y) * (other->position.y - position.y));
}

static inline float veryFastDistance(float x, float y){
	return x * x + y * y;
}

bool Game::NetObject::isCollidingWithSomething(Game::NetObject **who){
    *who = NULL;
	if(container == NULL) return false;

	Vector<Game::NetObject*> nearObjs;

	container->getQuadrant(position.x - 1500 * 0.5f,  position.y - 1500 * 0.5f, 3000, 3000, nearObjs);

	for (int i = 0; i < nearObjs.size(); ++i){
		auto obj = nearObjs[i];
        if(obj->objType == ObjectType::Ghost){
            continue;
        }
		if(obj->id == this->id) continue;
		if(!obj->solid) continue;
		if(isCollidingWith(obj)){
            *who = obj;
			return true;
		}
	}
	collided = true;
	return false;
}

void Game::NetObject::clearDeltaUpdates(){
    deltaUpdates = 0;
}

void Game::NetObject::issueDeltaUpdate(UInt8 type){
    deltaUpdates |= (1 << type);
}

bool Game::NetObject::isCollidingWithExcept(const Vector<Game::NetObject*> &ignores){
	auto isInIgnores = [&](Game::NetObject *ref){
		for(int i = 0; i < ignores.size(); ++i){
			if(ignores[i] == ref){
				return true;
			}
		}
		return false;
	};
	if(container == NULL) return false;

	Vector<Game::NetObject*> nearObjs;

	container->getQuadrant(position.x - 1500 * 0.5f,  position.y - 1500 * 0.5f, 3000, 3000, nearObjs);

	for (int i = 0; i < nearObjs.size(); ++i){
		auto obj = nearObjs[i];
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
