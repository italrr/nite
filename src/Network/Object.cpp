#include "Object.hpp"

#include "../Entity.hpp"
#include "../Player.hpp"

/*
    All the different types of objects must be created from here
*/
Shared<Game::NetObject> Game::createNetObject(UInt16 id, UInt16 sig, float x, float y){
    switch(sig){
        case Game::ObjectSig::Player: {
            auto player = Shared<Game::NetObject>(new Player());
            player->position.set(x, y);
            player->id = id;
            return player;
        } break;
        default:    
            nite::print("failed to created object of sig "+nite::toStr(sig)+": it's undefined");
            return Shared<Game::NetObject>(NULL);
    }
}