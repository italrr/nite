#include "Object.hpp"

#include "../Entity.hpp"

/*
    All the different types of objects must be created from here
*/
Shared<Game::NetObject> Game::createNetObject(UInt8 type){
    switch(type){
        case Game::ObjectType::Base: {
            auto obj = Shared<Game::NetObject>(new NetObject());
            obj->type = type;
            return obj;
        }
        case Game::ObjectType::Entity: {
            auto obj = Shared<Game::NetObject>(new Game::Entity());
            obj->type = type;
            return obj;
        }
        default: { 
            nite::print("failed to create netobject type '"+nite::toStr(type)+"': undefined");
            return Shared<Game::NetObject>(NULL);
        }
    }
}