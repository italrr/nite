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
Shared<Game::NetObject> Game::createNetObject(UInt16 sig, float x, float y){
    switch(sig){
        case Game::ObjectSig::Player: {
            auto player = Shared<Game::NetObject>(new EntityBase());
            player->id = 0;
            player->position = nite::Vec2(x, y);
            player->sigId = Game::ObjectSig::Player;
            return player;
        } break;
        case Game::ObjectSig::MobHumanoid: {
            auto mob = Shared<Game::NetObject>(new MobHumanoid());
            mob->id = 0;
            mob->position = nite::Vec2(x, y);
            mob->sigId = Game::ObjectSig::MobHumanoid;
            return mob;
        } break;
        case Game::ObjectSig::Projectile: {
            auto proj = Shared<Game::NetObject>(new Projectile());
            proj->id = 0;
            proj->position = nite::Vec2(x, y);
            proj->sigId = Game::ObjectSig::Projectile;
            return proj;
        } break;        
        default:
            nite::print("[netplay] failed to created object of sig '"+nite::toStr(sig)+"': it's undefined");
            return Shared<Game::NetObject>(NULL);
    }
}

Game::NetObject::NetObject(){
    container = NULL;
    sv = NULL;
    net = NULL;
    objType = Game::ObjectType::Base;
    sigId = Game::ObjectSig::Base;
	relativeTimescale = 1.0f;
	container = NULL;
	position.set(0.0f);
    speed = 0;
    destroyed = false;
    steprate = 300;
    lastRouteMove = 0;
    lastMove = nite::getTicks();
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
    // currentState.delta = nextState.delta;
    // // physics
    // if(DeltaUpdateType::hasIssuedDeltaStateUpdate(DeltaUpdateType::PHYSICS, nextState.states)){
    //     float dt = (nextState.delta - prevState.delta) * net->gameTickRate;
    //     float step = 1.0f - ((float)prevState.delta) / ((float)nextState.delta);
    //     currentState.x = nextState.x;
    //     currentState.y = nextState.y;
    //     currentState.speed = nextState.speed;
    //     currentState.direction = nextState.direction;
    //     currentState.orientation = nextState.orientation;
    //     this->speed = currentState.speed;
    //     this->position.x = lerp(this->position.x, currentState.x, 0.1f);
    //     this->position.y = lerp(this->position.y, currentState.y, 0.1f);
    //     this->direction = currentState.direction;
    //     this->orientation = currentState.orientation;
    // }
    // // anim
    // if(DeltaUpdateType::hasIssuedDeltaStateUpdate(DeltaUpdateType::ANIMATION, nextState.states) && this->objType == ObjectType::Entity){
    //     float dt = (nextState.delta - prevState.delta) * net->gameTickRate;
    //     float step = (float)prevState.delta / (float)nextState.delta;
    //     currentState.faceDir = nextState.faceDir;
    //     currentState.xLookingAt = nextState.xLookingAt;
    //     currentState.yLookingAt = nextState.yLookingAt;
    //     for(int j = 0; j < AnimPart::total; ++j){
    //         currentState.animSt[j] = nextState.animSt[j];
    //         currentState.animNum[j] = nextState.animNum[j];
    //         currentState.animExtime[j] = nextState.animExtime[j];
    //     }        
    //     auto ent = static_cast<EntityBase*>(this);
    //     ent->faceDirection = currentState.faceDir;
    //     ent->pointingAt = nite::Vec2(currentState.xLookingAt, currentState.yLookingAt);
    //     for(int j = 0; j < AnimPart::total; ++j){
    //         ent->state[j] = currentState.animSt[j];
    //         ent->stNum[j] = currentState.animNum[j];
    //         ent->lastExpectedTime[j] = currentState.animExtime[j];
    //     }
    // }
}

void Game::NetObject::setPosition(const nite::Vec2 &p){
    this->position = p;
    this->rPosition = nite::Vec2(container->cellsize) * p;
}

bool Game::NetObject::setMoveRoute(const nite::MapRoute &route, UInt32 total){
    this->route = route;
    this->lastRouteMove = route.route.size();
    this->totalMove = total;
    this->speed = total / route.route.size();
    issueDeltaUpdate(DeltaUpdateType::PHYSICS);
}

bool Game::NetObject::move(int x, int y){
    int j = container->toIndex(position.x, position.y);
    int i = container->toIndex(position + nite::Vec2(x, y));
    if(container->cells[i] == 0){
        container->cells[i] = this->id;
        container->cells[j] = 0;
        position = position + nite::Vec2(x, y);
        rPosition = position * nite::Vec2(container->cellsize);
        return true;
    }
    return false;
}

void Game::NetObject::destroy(){
	if(container == NULL || destroyed) return; 
    UInt16 _id = this->id;
    container->remove(this->id);
    if(sv != NULL){
        nite::Packet desUpt;
        desUpt.setHeader(Game::PacketType::SV_DESTROY_OBJECT);
        desUpt.write(&_id, sizeof(_id));	
        sv->sendPersPacketForMany(sv->getAllClientsIps(), desUpt, sv->getAllClientsAcks());
    }
    destroyed = true;  
}

float Game::NetObject::getDistance(Game::NetObject *other){
	return nite::sqrt((other->position.x - position.x) * (other->position.x - position.x) + (other->position.y - position.y) * (other->position.y - position.y));
}

static inline float veryFastDistance(float x, float y){
	return x * x + y * y;
}

void Game::NetObject::clearDeltaUpdates(){
    deltaUpdates = 0;
}

void Game::NetObject::issueDeltaUpdate(UInt8 type){
    deltaUpdates |= (1 << type);
}
