#include "../Core/Server.hpp"
#include "../Core/Client.hpp"

#include "Effect.hpp"
#include "Base.hpp"

static UInt16 getUniqueId(){
    static UInt16 seed = nite::randomInt(5, 25);
    return ++seed;
}

static void notifyAddEffect(Game::EntityBase *ent, UInt16 type, UInt16 insId){
    if(ent != NULL && ent->sv != NULL){
        if(auto cl = ent->sv->getClientByEntityId(ent->id)){
            ent->sv->notifyAddEffect(cl->clientId, type, insId);
        }
    }
}

static void notifyRemoveEffect(Game::EntityBase *ent, UInt16 insId){
    if(ent != NULL && ent->sv != NULL){
        if(auto cl = ent->sv->getClientByEntityId(ent->id)){
            ent->sv->notifyRemoveEffect(cl->clientId, insId);
        }
    }
}

static Jzon::Node db = Jzon::object();

void Game::DBLoadEffect(const String &path){
    String msg = "failed to load Effect Db '"+path+"': ";
    if(!nite::fileExists(path)){
        nite::print(msg+"it doesn't exist");
        return;
    }
    Jzon::Parser parser;
    auto root  = parser.parseFile(path);
    if(!root.isValid()){
        nite::print(msg+parser.getError());
        return;
    }
    if(!root.isObject() || root.getCount() == 0){
        nite::print("root must be an object");
        return;
    }
    for(auto &obj : root){
        if(obj.first == "__root__"){
            continue;
        }
        db.add(obj.first, obj.second);
    }
    nite::print("loaded Effect Db '"+path+"': added "+nite::toStr(root.getCount()-1)+" object(s)");
}

void Game::Effect::parse(Jzon::Node &obj){
    name = obj.get("name").toString("None");
    desc = obj.get("desc").toString("None");
    duration = obj.get("duration").toInt(1000);
    iconId = obj.get("iconId").toInt(0);
    amnt = obj.get("amnt").toInt(0);    
    amntpersecond = obj.get("amntpersecond").toInt(0);    
    color = nite::Color(obj.get("color").toString());
}

bool Game::EffectStat::add(Shared<Game::Effect> eff){
    eff->start(owner);
    eff->insId = getUniqueId();
    effects[eff->insId] = eff;
    hasChanged = true;

    // server-side (notify owner specifically)
    notifyAddEffect(owner, eff->type, eff->insId);

    owner->recalculateStats();

    return true;
}

bool Game::EffectStat::add(Shared<Game::Effect> eff, UInt16 insId){
    eff->start(owner);
    eff->insId = insId;
    effects[insId] = eff;
    hasChanged = true;

    // server-side (notify owner specifically)
    notifyAddEffect(owner, eff->type, eff->insId);

    owner->recalculateStats();

    return true;
}

bool Game::EffectStat::remove(UInt16 insId){
    auto it = effects.find(insId);
    if(it == effects.end()) return false;
    hasChanged = true;
    it->second->onEnd(owner);
    effects.erase(insId);

    // server-side (notify owner specifically)
    notifyRemoveEffect(owner, insId);

    owner->recalculateStats();

    return true;
}

bool Game::EffectStat::remove(const String &name){
    for(auto &it : effects){
        auto ef = effects[it.first];
        if(ef->name == name){
            ef->onEnd(owner);

            // server-side (notify owner specifically)
            notifyRemoveEffect(owner, ef->insId);
            effects.erase(it.first);
            hasChanged = true;
            owner->recalculateStats();
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
    hasChanged = true;
    Vector<UInt16> queue;
    for(auto &it : effects){
        queue.push_back(it.second->insId);
    }
    for(int i = 0; i < queue.size(); ++i){
        remove(queue[i]);
    }
}

bool Game::EffectStat::update(){
    if(hasChanged){
        hasChanged = false;
        return true;
    }
    for(auto it : effects){
        auto &ef = effects[it.first];
        auto updated = ef->step(owner);
        if(nite::getTicks()-ef->started > ef->duration){
            remove(ef->insId);
            return true;
        }
        if(updated){
            return true;
        }
    }
    return false;
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
            notifyRemoveEffect(owner, ef->insId);
            effects.erase(it.first);
            hasChanged = true;
            return true;
        }
    }
    return false;
}

Shared<Game::Effect> Game::getEffect(UInt16 type){
    auto dbInfo = db.get(nite::toStr(type));
    auto ef = Shared<Game::Effect>(NULL);
    switch(type){
        case Game::EffectList::EF_HEAL: {
            ef = Shared<Game::Effect>(new Game::Effects::EffHeal());
        } break;            
    }
    if(ef.get() != NULL && db.has(nite::toStr(type))){		
		ef->parse(dbInfo);
	    ef->parseSpecial(dbInfo);
    }
    return ef;
}

/*
    EFFECTS
*/

// EffHeal
bool Game::Effects::EffHeal::step(Game::EntityBase *owner){
    UInt16 diff = (nite::getTicks() - this->lastStep) / 1000;
    if(diff < 1){
        return false;
    }
    this->lastStep = nite::getTicks();
    UInt16 toHeal = diff * amntpersecond;
    owner->heal(toHeal, 0, 0);

    return true;
}

void Game::Effects::EffHeal::setup(UInt16 amnt, UInt64 time){ // time is msecs
    this->amnt = amnt;
    this->duration = time;  
    this->amntpersecond = amnt / (time / 1000);                
}

String Game::Effects::EffHeal::getStatus(Game::EntityBase *owner){
    UInt64 target = this->started + this->duration;
    UInt64 current = owner->net->clock.time >= target ? 0 : target - owner->net->clock.time; // we use server's remote time
    return nite::toStr(((current / 1000) + 1))+"s";
}