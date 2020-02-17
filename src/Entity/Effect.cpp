
#include "Effect.hpp"
#include "Base.hpp"

void Game::EffectStat::addEffect(Game::Effect status){
    auto copy = status;
    copy.start();
    effects.push_back(copy);
}

void Game::EffectStat::removeEffect(UInt16 type){
    for(int i = 0; i < effects.size(); ++i){
        if(effects[i].type == type){
            effects.erase(effects.begin() + i);
            --i;
            continue;
        }
    } 
}

void Game::EffectStat::removeEffect(const String &name){
    for(int i = 0; i < effects.size(); ++i){
        if(effects[i].name == name){
            effects.erase(effects.begin() + i);
            --i;
            continue;
        }
    }    
}

bool Game::EffectStat::isEffectOn(UInt16 type){
    for(int i = 0; i < effects.size(); ++i){
        if(effects[i].type == type){
            return true;
        }
    }
    return false;
}

void Game::EffectStat::removeAllEffects(){
    this->effects.clear();
}

void Game::EffectStat::updateEffects(){
    if(this->container == NULL){
        return;
    }
    for(int i = 0; i < effects.size(); ++i){
        auto &ef = effects[i];
        if(nite::getTicks()-ef.started > ef.duration){
            ef.onEnd();
            --i;
            continue;
        }
        ef.step(*this->container);
    }
}



/*
    EFFECTS
*/

// Eff_EffHeal
void Game::Eff_EffHeal::step(Game::NetWorld &container){
    UInt16 diff = (nite::getTicks() - this->lastStep) / 1000;
    if(diff < 1){
        return;
    }
    this->lastStep = nite::getTicks();
    UInt16 toHeal = diff * amnt;
    auto owner = container.objects.find(this->owner);
    if(owner != container.objects.end()){
        if(auto ent = dynamic_cast<Game::EntityBase*>(owner->second.get())){
            ent->heal(toHeal, 0, 0);
        }
    }
}