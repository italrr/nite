
#include "Effect.hpp"
#include "Base.hpp"

bool Game::EffectStat::add(Game::Effect status){
    auto copy = status;
    copy.start();
    effects.push_back(copy);
    return true;
}

bool Game::EffectStat::remove(UInt16 type){
    bool success = false;
    for(int i = 0; i < effects.size(); ++i){
        if(effects[i].type == type){
            effects.erase(effects.begin() + i);
            --i;
            success = true;
            continue;
        }
    } 
    return success;
}

bool Game::EffectStat::remove(const String &name){
    bool success = false;
    for(int i = 0; i < effects.size(); ++i){
        if(effects[i].name == name){
            effects.erase(effects.begin() + i);
            --i;
            success = true;
            continue;
        }
    }    
    return success;
}

bool Game::EffectStat::isOn(UInt16 type){
    for(int i = 0; i < effects.size(); ++i){
        if(effects[i].type == type){
            return true;
        }
    }
    return false;
}

void Game::EffectStat::removeAll(){
    this->effects.clear();
}

void Game::EffectStat::update(Game::EntityBase &ent){
    for(int i = 0; i < effects.size(); ++i){
        auto &ef = effects[i];
        if(nite::getTicks()-ef.started > ef.duration){
            ef.onEnd();
            --i;
            continue;
        }
        ef.step(ent);
    }
}



/*
    EFFECTS
*/

// Eff_EffHeal
void Game::Eff_EffHeal::step(Game::EntityBase &ent){
    UInt16 diff = (nite::getTicks() - this->lastStep) / 1000;
    if(diff < 1){
        return;
    }
    this->lastStep = nite::getTicks();
    UInt16 toHeal = diff * amnt;
    ent.heal(toHeal, 0, 0);
}