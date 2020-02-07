#include "Skills.hpp"


Game::Skill::Skill(){
    id =  Game::SkillList::NONE;
    name = "NONE";
    desc = "NONE";
    type = Game::SkillType::ACTIVE;
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

void Game::Skill::beforeUse(Shared<Game::Entity> who){

}

void Game::Skill::afterUse(Shared<Game::Entity> who){

}

void Game::Skill::use(Shared<Game::Entity> who){
    use(who, Shared<Game::Entity>(NULL), nite::Vec2(0.0f));
}

void Game::Skill::use(Shared<Game::Entity> who, Shared<Game::Entity> to){
    use(who, to, nite::Vec2(0.0f));
}

void Game::Skill::use(Shared<Game::Entity> who, Shared<Game::Entity> to, const nite::Vec2 &at){

}