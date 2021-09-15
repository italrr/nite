#include "../Entity.hpp"
#include "BattlePlayerInfo.hpp"

void Game::UIBattlePlayerInfo::onCreate(){
    size.set(nite::Vec2(200, 150));
    secondBar = entity->healthStat.health;
    affected = false;
}

void Game::UIBattlePlayerInfo::onDestroy(){

}

void Game::UIBattlePlayerInfo::step(){
    if(affected && nite::getTicks()-affectedTick > 800 && nite::lerpDiscrete(secondBar, entity->healthStat.health, 0.1f)){
        affected = false;
    }
}

void Game::UIBattlePlayerInfo::setAffected(){
    affected = true;
    affectedTick = nite::getTicks();
}

void Game::UIBattlePlayerInfo::render(){
    if(entity.get() == NULL || theme.get() == NULL){
        return;
    }
    nite::Vec2 p = this->position;

    nite::Vec2 margin = nite::Vec2(16.0f);

    float ycursor = 0.0f;

    // base
    nite::setColor(theme->backgroundColor);
    theme->base.draw(position.x, position.y, size.x, size.y, 0.0f, 0.0f, 0.0f);


    String header = entity->nickname+" | Lv. "+nite::toStr(entity->healthStat.lv);

    float headerWidth = theme->bigFont.getWidth(header);
    float headerHeight = theme->bigFont.getHeight();

    nite::Color(theme->fontColor);
    theme->bigFont.draw(header, p.x + size.x * 0.5f + margin.x * 0.5f - headerWidth * 0.5f, p.y + margin.y * 0.5f, 0.0f, 0.0f, 0.0f);

    ycursor += headerHeight;
    float sectHeight = 50;
    float firstCol = 40;

    // hp
    float hpRate = (float)entity->healthStat.health / (float)entity->healthStat.maxHealth;
    float hpRateSecBar = secondBar / (float)entity->healthStat.maxHealth;
    String hpText = nite::toStr(entity->healthStat.health)+" / "+nite::toStr(entity->healthStat.maxHealth);
    float hpTextWidth = theme->smallFont.getWidth(hpText);
    nite::setColor(theme->solidColor);
    theme->regularFont.draw("HP", p.x + margin.x * 0.5f, p.y + ycursor + sectHeight * 0.5f, 0.0f, 0.5f, 0.0f);
    
    nite::setColor(theme->subColor);
    theme->base.draw(p.x + margin.x * 0.5f + firstCol, p.y + ycursor + sectHeight * 0.5f, (size.x - margin.x - firstCol) * hpRateSecBar, sectHeight * 0.5f, 0.0f, 0.5f, 0.0f);

    nite::setColor(theme->solidColor);
    theme->base.draw(p.x + margin.x * 0.5f + firstCol, p.y + ycursor + sectHeight * 0.5f, (size.x - margin.x - firstCol) * hpRate, sectHeight * 0.5f, 0.0f, 0.5f, 0.0f);
    
    theme->smallFont.draw(hpText, p.x + size.x - margin.x * 0.5f - hpTextWidth, p.y + ycursor + sectHeight * 0.75f, 0.0f, 0.0f, 0.0f);


    ycursor += sectHeight;


}