#include "Skill.hpp"


Game::Skill::Skill(){
    id =  Game::SkillList::NONE;
    name = "NONE";
    desc = "NONE";
    type = Game::SkillType::Active;
    maxLv = 1;
    family = 0;
    staminaCost = 1;
    manaCost = 1;
    range = 1;
    minUseLv = 1;
    delay = 1000;
    adelay = 100;
    skPointsPerLv = 1;
}

void Game::Skill::beforeUse(Shared<Game::EntityBase> who){

}

void Game::Skill::afterUse(Shared<Game::EntityBase> who){

}

void Game::Skill::use(Shared<Game::EntityBase> who, Shared<Game::EntityBase> to, const nite::Vec2 &at){
    this->lastUse = nite::getTicks();
}


Shared<Game::Skill> Game::getSkill(UInt16 id, UInt8 lv){
    switch(id){
        case Game::SkillList::BA_ATTACK: {
            auto sk = Shared<Game::Skill>(new Game::Skills::BA_Attack());
            sk->lv = lv;
            return sk;
        };
        case Game::SkillList::BA_BASH: {
            auto sk = Shared<Game::Skill>(new Game::Skills::BA_Bash());
            sk->lv = lv;
            return sk;
        };
        case Game::SkillList::BA_DODGE: {
            auto sk = Shared<Game::Skill>(new Game::Skills::BA_Dodge());
            sk->lv = lv;
            return sk;
        };
        case Game::SkillList::BA_PARRY: {
            auto sk = Shared<Game::Skill>(new Game::Skills::BA_Parry());
            sk->lv = lv;
            return sk;
        };    
        case Game::SkillList::BA_FIRST_AID: {
            auto sk = Shared<Game::Skill>(new Game::Skills::BA_FIRST_AID());
            sk->lv = lv;
            return sk;
        };
        default:
            return Shared<Game::Skill>(NULL);               
    }
}

bool Game::SkillStat::addSkill(UInt16 id, UInt8 lv){
    if(hasSkill(id)){
        return false;
    }   
    if(this->skills[id] >= lv){
        return false;
    }
    this->skills[id] = lv;
    return true;
}

bool Game::SkillStat::removeSkill(UInt16 id){
    auto it = skills.find(id);
    if(it == skills.end()){
        return false;
    }
    skills.erase(it);
}

bool Game::SkillStat::hasSkill(UInt16 id){
    return skills.find(id) != skills.end();
}

bool Game::SkillStat::lvUpSkill(UInt16 id){
    if(hasSkill(id)){
        return false;
    }    
    skills[id] += skills[id] + 1;
    return true;
}

// SKILLS

/*
    SK_BA_Attack
*/
Game::Skills::BA_Attack::BA_Attack(){
    this->name = "Basic Attack";
    this->desc = "Inflicts [3 + ATK/DEX + 30%WEAP] physical damage using current wielded weapon.";
    this->id = Game::SkillList::BA_ATTACK;
    this->lv = 1;
    this->type = Game::SkillType::Active;
    this->family = 0; // no family tree
    this->delay = 100;
    this->iconId = 16 * 6 + 15;
    this->adelay = 50;
    this->minUseLv = 1;
    this->maxLv = 1;
    this->range = 2; // doesn't apply using bow
}

void Game::Skills::BA_Attack::use(Shared<Game::EntityBase> who, Shared<Game::EntityBase> to, const nite::Vec2 &at){
    this->lastUse = nite::getTicks();
}

/*
    SK_BA_Bash
*/
Game::Skills::BA_Bash::BA_Bash(){
    this->name = "Bash";
    this->desc = "Inflicts [8 + ATK/DEX + 30%WEAP] physical damage using current wielded weapon.";
    this->id = Game::SkillList::BA_BASH;
    this->lv = 1;
    this->type = Game::SkillType::Active;
    this->family = 0; // no family tree
    this->delay = 2500;
    this->iconId = 16 * 4 + 11;
    this->adelay = 400;
    this->minUseLv = 1;
    this->maxLv = 1;
    this->range = 2;
}

void Game::Skills::BA_Bash::use(Shared<Game::EntityBase> who, Shared<Game::EntityBase> to, const nite::Vec2 &at){
    this->lastUse = nite::getTicks();
}

/*
    SK_BA_Dodge
*/
Game::Skills::BA_Dodge::BA_Dodge(){
    this->name = "Dodge";
    this->desc = "Makes a quick dash helping avoid hits.";
    this->id = Game::SkillList::BA_DODGE;
    this->lv = 1;
    this->type = Game::SkillType::Active;
    this->family = 0; // no family tree
    this->delay = 8000;
    this->iconId = 16 * 5 + 8;
    this->adelay = 400;
    this->minUseLv = 1;
    this->maxLv = 1;
    this->range = 0;
}

void Game::Skills::BA_Dodge::use(Shared<Game::EntityBase> who, Shared<Game::EntityBase> to, const nite::Vec2 &at){
    this->lastUse = nite::getTicks();
}


/*
    SK_BA_Parry
*/
Game::Skills::BA_Parry::BA_Parry(){
    this->name = "Parry";
    this->desc = "If used in the right time, blocks between 45-90% of the next incoming physical (ranged or close) attack";
    this->id = Game::SkillList::BA_PARRY;
    this->lv = 1;
    this->type = Game::SkillType::Active;
    this->family = 0; // no family tree
    this->delay = 3000;
    this->iconId = 16 * 4 + 2;
    this->adelay = 190;
    this->minUseLv = 1;
    this->maxLv = 1;
    this->range = 0;
}

void Game::Skills::BA_Parry::use(Shared<Game::EntityBase> who, Shared<Game::EntityBase> to, const nite::Vec2 &at){
    this->lastUse = nite::getTicks();
}

/*
    SK_BA_FIRST_AID
*/
Game::Skills::BA_FIRST_AID::BA_FIRST_AID(){
    this->name = "First Aid";
    this->desc = "Heals up to 20% of HP over 30 seconds. Doesn't work with >50% of current HP.";
    this->id = Game::SkillList::BA_FIRST_AID;
    this->lv = 1;
    this->type = Game::SkillType::Active;
    this->family = 0; // no family tree
    this->delay = 60 * 1000;
    this->iconId = 16 * 4 + 6;
    this->adelay = 250;
    this->minUseLv = 1;
    this->maxLv = 1;
    this->range = 0;
}

void Game::Skills::BA_FIRST_AID::use(Shared<Game::EntityBase> who, Shared<Game::EntityBase> to, const nite::Vec2 &at){
    this->lastUse = nite::getTicks();
}