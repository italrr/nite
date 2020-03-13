
#include "Effect.hpp"
#include "Base.hpp"

static UInt16 getUniqueId(){
    static UInt16 seed = nite::randomInt(5, 25);
    return ++seed;
}

bool Game::EffectStat::add(Shared<Game::Effect> eff){
    eff->start(owner);
    eff->insId = getUniqueId();
    effects[eff->insId] = eff;
    return true;
}

bool Game::EffectStat::remove(UInt16 insId){
    auto it = effects.find(insId);
    if(it == effects.end()) return false;
    effects.erase(insId);
}

bool Game::EffectStat::remove(const String &name){
    for(auto &it : effects){
        auto ef = effects[it.first];
        if(ef->name == name){
            ef->onEnd(owner);
            effects.erase(it.first);
            return true;
        }
    }
    return false;
}

bool Game::EffectStat::isOn(UInt16 insId){
    for(auto &it : effects){
        auto ef = effects[it.first];
        if(ef->insId == insId){
            return true;
        }
    }
    return false;
}

void Game::EffectStat::removeAll(){
    this->effects.clear();
}

void Game::EffectStat::update(){
    for(auto it : effects){
        auto &ef = effects[it.first];
        ef->step(owner);
        if(nite::getTicks()-ef->started > ef->duration){
            remove(ef->insId);
        }        
    }
}

bool Game::EffectStat::isOnByType(UInt16 type){
    for(auto &it : effects){
        auto ef = effects[it.first];
        if(ef->type == type){
            return true;
        }
    }
    return false;
}

bool Game::EffectStat::removeByType(UInt16 type){
    for(auto &it : effects){
        auto ef = effects[it.first];
        if(ef->type == type){
            ef->onEnd(owner);
            effects.erase(it.first);
            return true;
        }
    }
    return false;
}

Shared<Game::Effect> Game::getEffect(UInt16 type){
    switch(type){
        case Game::EffectList::EF_HEAL: {
            auto ef = Shared<Game::Effect>(new Game::Effects::EffHeal());

            return ef;
        };
        default:
            return Shared<Game::Effect>(NULL);               
    }
}

/*
    EFFECTS
*/

// EffHeal
void Game::Effects::EffHeal::step(){
    UInt16 diff = (nite::getTicks() - this->lastStep) / 1000;
    if(diff < 1){
        return;
    }
    this->lastStep = nite::getTicks();
    UInt16 toHeal = diff * amnt;
    owner->heal(toHeal, 0, 0);
}