#include "Object.hpp"
#include "Network.hpp"
#include "World.hpp"

#include "../Entity/Base.hpp"

/*
    All the different types of objects must be created from here
*/
Shared<Game::NetObject> Game::createNetObject(UInt16 id, UInt16 sig, float x, float y){
    switch(sig){
        case Game::ObjectSig::Player: {
            auto player = Shared<Game::NetObject>(new EntityBase()); // TODO: replace for player
            player->setPosition(x, y);
            player->id = id;
            return player;
        } break;
        default:    
            nite::print("[netplay] failed to created object of sig '"+nite::toStr(sig)+"': it's undefined");
            return Shared<Game::NetObject>(NULL);
    }
}

void Game::NetObject::destroy(){
	if(container == NULL) return;
    clearQuadrant();
	container->remove(this->id);
}

void Game::NetObject::setPosition(const nite::Vec2 &p){
    this->position.set(p);
    this->lerpPosition.set(p);
    updateQuadrant();
}

void Game::NetObject::updateQuadrant(){
    if(container != NULL && container->cells != NULL){
        int _x = nite::round(position.x / (float)container->cusize);
        int _y = nite::round(position.y / (float)container->cusize);
        UInt16 nq = _x + container->cwidth * _y;
        if(quadrant != -1 && nq != quadrant && container->cells[quadrant] == this){
            container->cells[quadrant] = NULL;
        }
        quadrant = nq;
        container->cells[nq] = this;
    }    
}

void Game::NetObject::clearQuadrant(){
    if(container != NULL && quadrant != -1  && container->cells[quadrant] == this){
        container->cells[quadrant] = NULL;
    }
    quadrant = -1;
}