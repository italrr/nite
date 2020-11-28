#include "../Core/Server.hpp"
#include "../Core/Client.hpp"

#include "Skill.hpp"
#include "Base.hpp"

static void notifyAddSkill(Game::EntityBase *ent, UInt16 skillId, UInt8 lv){
    if(ent != NULL && ent->sv != NULL){
        if(auto cl = ent->sv->getClientByEntityId(ent->id)){
            ent->sv->notifyAddSkill(cl->clientId, skillId, lv);
        }
    }
}

static void notifyRemoveSkill(Game::EntityBase *ent, UInt16 skillId){
    if(ent != NULL && ent->sv != NULL){
        if(auto cl = ent->sv->getClientByEntityId(ent->id)){
            ent->sv->notifyRemoveSkill(cl->clientId, skillId);
        }
    }
}

static Jzon::Node db = Jzon::object();

void Game::DBLoadSkill(const String &path){
    String msg = "failed to load Skill Db '"+path+"': ";
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
    db = Jzon::object();
    for(auto &obj : root){
        if(obj.first == "__root__"){
            continue;
        }
        db.add(obj.first, obj.second);
    }
    nite::print("loaded Skill Db '"+path+"': added "+nite::toStr(root.getCount()-1)+" object(s)");
}


void Game::Skill::parse(Jzon::Node &obj){
    name = obj.get("name").toString("None");
    desc = obj.get("desc").toString("None");
    for(auto &it : obj.get("reqs")){
        reqs.push_back(it.second.toInt(0));
    }
    iconId = obj.get("iconId").toInt(0);
    type = obj.get("type").toInt(0);
    maxLv = obj.get("maxLv").toInt(1);
    manaCost = obj.get("manaCost").toInt(0);
    staminaCost = obj.get("staminaCost").toInt(0);
    family = obj.get("family").toInt(0);
    range = obj.get("range").toInt(1);
    cooldown = obj.get("cooldown").toInt(1000);
    castDelay = obj.get("castDelay").toInt(100);
    usageType = obj.get("usageType").toInt(Game::SkillUsageType::Self);
    minUseLv = obj.get("minUseLv").toInt(1);
    skPointsPerLv = obj.get("skPointsPerLv").toInt(1);
    baseDmg = obj.get("baseDmg").toInt(0);
}

bool Game::Skill::isReady(Game::EntityBase *who){
    Int64 cd = (cooldown - cooldown * who->complexStat.cooldownRedRate);
    if(cd < 0){
        cd = cooldown;
    }
    return nite::getTicks() - lastUse > cd;
}

void Game::Skill::writeUpdate(nite::Packet &packet){
    packet.write(&lastUse, sizeof(lastUse));
}

UInt64 Game::Skill::getCooldown(EntityBase *who){
    Int64 cd = (cooldown - cooldown * who->complexStat.cooldownRedRate);
    if(cd < 0){
        cd = cooldown;
    }
    return cd;
}

void Game::Skill::readUpdate(nite::Packet &packet){
    packet.read(&lastUse, sizeof(lastUse));
}

Shared<Game::Skill> Game::getSkill(UInt16 id, UInt8 lv){
	auto dbInfo = db.get(nite::toStr(id));
    auto sk = Shared<Game::Skill>(NULL);
    switch(id){
        case Game::SkillList::BA_ATTACK: {
            sk = Shared<Game::Skill>(new Game::Skills::BA_Attack());
        } break;
        case Game::SkillList::BA_BASH: {
            sk = Shared<Game::Skill>(new Game::Skills::BA_Bash());
        } break;
        case Game::SkillList::BA_DASH: {
            sk = Shared<Game::Skill>(new Game::Skills::BA_Dash());
        } break;
        case Game::SkillList::BA_PARRY: {
            sk = Shared<Game::Skill>(new Game::Skills::BA_Parry());
        } break;
        case Game::SkillList::BA_FIRST_AID: {
            sk = Shared<Game::Skill>(new Game::Skills::BA_FIRST_AID());
        } break;
    }
    if(sk.get() != NULL){
        sk->lv = lv;
        if(db.has(nite::toStr(id))){
            sk->parse(dbInfo);
            sk->parseSpecial(dbInfo);
        }
    }
    return sk;
}

bool Game::SkillStat::add(Shared<Game::Skill> &sk){
    if(contains(sk->id)){
        return false;
    }
    this->skills[sk->id] = sk;
    // server-side only
    notifyAddSkill(owner, sk->id, sk->lv);
    owner->recalculateStats();
    return true;
}

bool Game::SkillStat::remove(UInt16 id){
    auto it = skills.find(id);
    if(it == skills.end()){
        return false;
    }
    skills.erase(it);
    // server-side only
    notifyRemoveSkill(owner, id);
    owner->recalculateStats();
    return true;
}

bool Game::SkillStat::contains(UInt16 id){
    return skills.find(id) != skills.end();
}

Game::Skill *Game::SkillStat::get(UInt16 id){
    auto it = skills.find(id);
    if(it == skills.end()){
        return NULL;
    }
    return it->second.get();
}

bool Game::SkillStat::lvUp(UInt16 id){
    if(contains(id)){
        return false;
    }
    if(skills[id]->lv >= skills[id]->maxLv){
        return false;
    }
    ++skills[id]->lv;
    return true;
}

// SKILLS

/*
    SK_BA_Attack
*/
bool Game::Skills::BA_Attack::use(Game::EntityBase *who, Game::EntityBase *to, const nite::Vec2 &at){
    if(!isReady(who) || this->locked){
        return false;
    }
    this->lastUse = nite::getTicks();
    if(who == NULL){
        return false;
    }
    who->useBaseAttack();
    this->locked = true;
    return true;
}

/*
    SK_BA_Bash
*/
bool Game::Skills::BA_Bash::use(Game::EntityBase *who, Game::EntityBase *to, const nite::Vec2 &at){
    this->lastUse = nite::getTicks();
    nite::print("bash");
    return true;
}

/*
    SK_BA_Dodge
*/
bool Game::Skills::BA_Dash::use(Game::EntityBase *who, Game::EntityBase *to, const nite::Vec2 &at){
    if(!isReady(who)){
        return false;
    }
    this->lastUse = nite::getTicks();
    if(who == NULL){
        return false;
    } 
    // nite::Vec2 origp = who->position;
    float dist = 128.0f;
    // float step = dist / 8.0f;
    auto ang = who->direction;
    // Game::NetObject *other;
    // who->position = who->position + nite::Vec2(nite::cos(ang) * dist, nite::sin(ang) * dist);
    // do {
    //     who->position = who->position - nite::Vec2(nite::cos(ang) * step, nite::sin(ang) * step);
    // } while(who->isCollidingWithSomething(&other));
    // who->setState(EntityState::SP_DASHING, EntityStateSlot::SPECIAL, 0, true);
    // who->setPosition(who->position);
    who->push(ang, dist);
    return true;
}

/*
    SK_BA_Parry
*/
bool Game::Skills::BA_Parry::use(Game::EntityBase *who, Game::EntityBase *to, const nite::Vec2 &at){
    this->lastUse = nite::getTicks();
    nite::print("parry");
    return true;
}

/*
    SK_BA_FIRST_AID
*/
bool Game::Skills::BA_FIRST_AID::use(Game::EntityBase *who, Game::EntityBase *to, const nite::Vec2 &at){
    this->lastUse = nite::getTicks();
    nite::print("first aid");
    return true;
}
