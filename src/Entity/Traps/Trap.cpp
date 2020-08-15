#include "Trap.hpp"
#include "../../Core/Network.hpp"
#include "../../Core/Object.hpp"
#include "../../Core/World.hpp"



UInt16 Game::TrapDevice::add(const Shared<Trap> &trap, int useId){
    UInt16 cid = useId == -1 ? lastId++ : useId;
    trap->id = cid;
    this->traps[cid] = trap;
    auto *mask = new Game::GhostMask();
    mask->position = trap->position;
    mask->size = trap->size;
    mask->callback = trap->callback;
    mask->solid = true;
    mask->unmovable = true;
    auto gid = host->world.addGhostMask(mask);
    trap->mask = mask;
    mask->setPosition(trap->position);
    return cid;
}

void Game::TrapDevice::start(Game::Net *host){
    this->host = host;
}

void Game::TrapDevice::remove(UInt16 id){
    auto it = traps.find(id);
    if(it == traps.end()){
        return;
    }
    if(it->second->mask != NULL){
        host->world.removeGhostMask(it->second->mask);
    }
    traps.erase(it->first);
}

Vector<UInt16> Game::TrapDevice::update(){
    Vector<UInt16> changes;
    for(auto &it : traps){
        int lstSt = it.second->state;
        it.second->update(host->map, this->host->world);
        if(it.second->state != lstSt){
            changes.push_back(it.second->id);
        }
    }
    return changes;
}

Game::Trap *Game::TrapDevice::getTrap(UInt16 id){
    auto it = traps.find(id);
    if(it == traps.end()){
        return NULL;
    }
    return it->second.get();
}

void Game::TrapDevice::clear(){
    lastId = 0;
    Vector<int> ids;
    for(auto &it : traps){
        ids.push_back(it.second->id);
    }
    for(int i = 0; i < ids.size(); ++i){
        remove(ids[i]);
    }
    traps.clear();
}



void Game::TrapNeedles::update(const Shared<nite::Map> &map, Game::NetWorld &world){
   if(nite::getTicks()-initTime > timeout){
       switch(state){
            case 0: {
                setState(1, map, world);
            } break;
            default: 
            case 1: {
                setState(0, map, world);
            } break;
       }
       initTime = nite::getTicks();
   }
}

void Game::TrapNeedles::setState(int state, const Shared<nite::Map> &map, Game::NetWorld &world){
    this->state = state;
    if(this->dynTile > 0){
        map->setDynamicTileState(dynTile, state);
    }
      
}



Shared<Game::Trap> Game::getTrap(int type){
    switch(type){
        case TrapType::NEEDLES: {
            return Shared<Game::Trap>(new Game::TrapNeedles());
        }
        case TrapType::NONE:
        default: {
            return Shared<Game::Trap>(NULL);
        }
    }
}


Shared<Game::Trap> Game::getTrap(const String &type){
    if(type == "needles"){
        return getTrap(Game::TrapType::NEEDLES);
    }
    return getTrap(0);
}
