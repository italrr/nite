#include "Stat.hpp"

Game::ComplexStat::ComplexStat() {
	maxCarry = 25;
	walkRate = 0.25f;
	atk = 0;
	magicAtk = 0;
	def = 0;
	magicDef = 0; 
	critRate = 0;
	atkRate = 0;
	precsRate = 0;
	persuasionRate = 0;
	charmRate = 0;
}

Game::BaseStat::BaseStat(){
	strStat = 0;
	agiStat = 0;
	dexStat = 0;
	enduStat = 0;
	lukStat = 0;
	intStat = 0;
	charismaStat = 0;
}

void Game::Stat::recalculateHealthStats(){
	healthStat.maxHealth = nite::ceil((12.5f + baseStat.enduStat * 1.5f)  * GAME_STAT_BASE_SCALE * healthStat.lv);
	healthStat.maxMana = nite::ceil((4.5f + baseStat.intStat * 0.5f) * GAME_STAT_BASE_SCALE * healthStat.lv);
	healthStat.maxStamina = nite::ceil((1.9f + baseStat.dexStat * 0.35f + baseStat.enduStat * 0.15f) * GAME_STAT_BASE_SCALE * healthStat.lv);
}

void Game::Stat::recalculateComplexStats(){
	complexStat.maxCarry = 25 + nite::ceil(baseStat.strStat * GAME_STAT_BASE_SCALE * 4.0f + baseStat.enduStat * GAME_STAT_BASE_SCALE * 2.0f);
	complexStat.atk = nite::ceil(baseStat.strStat * 8.5f + baseStat.enduStat * 1.2f);
	complexStat.magicAtk = nite::ceil(baseStat.intStat * 8.5f);
	complexStat.def = nite::ceil(baseStat.enduStat * 5.8f + healthStat.lv * 0.7f);
	complexStat.magicDef = nite::ceil(baseStat.intStat * 4.5f + baseStat.enduStat * 1.2f);		
	complexStat.walkRate = nite::ceil(0.18f * baseStat.agiStat); 
	complexStat.critRate = (float)baseStat.lukStat / (float)GAME_MAX_STAT;
	complexStat.precsRate = baseStat.dexStat * 1.25f;
	complexStat.atkRate = baseStat.agiStat * (3.85f + healthStat.lv * 0.15f) + baseStat.dexStat * (1.1f + healthStat.lv * 0.1f);
	complexStat.persuasionRate = nite::ceil(0.45f * healthStat.lv + baseStat.charismaStat * 5.96f + baseStat.intStat * 2.25f);
	complexStat.charmRate = nite::ceil(0.5f * healthStat.lv + baseStat.charismaStat * 5.96f);
}

void Game::Stat::printInfo(){
	// #define str nite::toStr
	// String ms = str(GAME_MAX_STAT);
	// nite::print("ID "+str(id)+" | Name: "+name+" | LV "+str(lv)+" | Carry "+str(this->maxCarry));
	// nite::print("HP "+str(maxHealth)+" | Mana "+str(maxMana)+" | Stamina "+str(maxStamina)+" | StatPoints "+str(statPoints));
	// nite::print("Str "+str(strStat)+"/"+ms+" | Agi "+str(agiStat)+"/"+ms+
	// " | Dex "+str(dexStat)+"/"+ms+" | Endurance "+str(enduStat)+"/"+ms+
	// "| Luk "+str(lukStat)+"/"+ms+" | Int "+str(intStat)+"/"+ms+
	// " | Char "+str(charismaStat)+"/"+ms);
	// nite::print("Atk "+str(this->atk)+" "+"MAtk "+str(this->magicAtk)+" "+"Def "+str(this->def)+" "+"MDef "+str(this->magicDef));
	// nite::print("WalkRate "+str(this->walkRate));
	// nite::print("CritRate "+str(this->critRate));
	// nite::print("PrecsRate "+str(this->precsRate));
	// nite::print("AtkRate "+str(this->atkRate));
	// nite::print("CharmRate "+str(this->charmRate));
	// nite::print("PersuasionRate "+str(this->persuasionRate));    
	// #undef str    
}

Int32 Game::Stat::addBaseStat(UInt8 type, UInt32 amnt){
	Int32 toAdd = 0;
	auto inc = [&](UInt32 &target){
		int toRest = baseStat.statPoints;
		// handle remaining points
		if(amnt <= toRest){
			toAdd +=  amnt;
			toRest -= amnt;
		}else{
			toAdd = toRest;
			toRest = 0;
		}
		// handle excess points
		if(toAdd + target > GAME_MAX_STAT){
			toAdd = GAME_MAX_STAT - target;
			toRest = baseStat.statPoints - toAdd;
		}
		baseStat.statPoints = toRest;
		target += toAdd;
	};
	switch(type){
		case BaseStatType::Strength:{
			inc(baseStat.strStat);
		} break;
		case BaseStatType::Agility:{
			inc(baseStat.agiStat);
		} break;
		case BaseStatType::Dexterity:{
			inc(baseStat.dexStat);
		} break;
		case BaseStatType::Endurance:{
			inc(baseStat.enduStat);
		} break;
		case BaseStatType::Luck:{
			inc(baseStat.lukStat);
		} break;
		case BaseStatType::Intelligence:{
			inc(baseStat.intStat);
		} break;
		case BaseStatType::Charisma:{
			inc(baseStat.charismaStat);
		} break;
	}
	recalculateStats();
	return toAdd;
}


void Game::Stat::recalculateStats(){
	recalculateHealthStats();
	recalculateComplexStats();	
}

void Game::Stat::resetBaseStat(UInt8 type){
	baseStat.statPoints = GAME_STAT_POINTS_PER_LV * this->healthStat.lv;
	switch(type){
		case BaseStatType::Strength:{
			baseStat.strStat = 0;
		} break;
		case BaseStatType::Agility:{
			baseStat.agiStat = 0;
		} break;
		case BaseStatType::Dexterity:{
			baseStat.dexStat = 0;
		} break;
		case BaseStatType::Endurance:{
			baseStat.enduStat = 0;
		} break;
		case BaseStatType::Luck:{
			baseStat.lukStat = 0;
		} break;
		case BaseStatType::Intelligence:{
			baseStat.intStat = 0;
		} break;
		case BaseStatType::Charisma:{
			baseStat.charismaStat = 0;
		} break;
	}
	recalculateStats();	
}

void Game::Stat::fullHeal(){
	healthStat.health = healthStat.maxHealth;
	healthStat.mana = healthStat.maxMana;
	healthStat.stamina = healthStat.maxStamina;
}

bool Game::Stat::lvUp(){
	if(healthStat.lv >= GAME_MAX_LEVEL){
		return false;
	}
	++healthStat.lv;
	baseStat.statPoints += GAME_STAT_POINTS_PER_LV;
	healthStat.exp = 0;
	healthStat.nextExp = nite::ceil((healthStat.lv + 1) * GAME_STAT_BASE_SCALE * 250);
	recalculateStats();
	return true;
}

void Game::Stat::setupStat(UInt16 lv){
	healthStat.lv = 0;
	healthStat.exp = 0;
	healthStat.dead = false;
	baseStat.statPoints = 12;
	while(healthStat.lv < lv && lvUp());
	// add base (start) skills. might be cool to get these from a json
	skillStat.add(Game::SkillList::BA_ATTACK, 1);
	skillStat.add(Game::SkillList::BA_BASH, 1);
	skillStat.add(Game::SkillList::BA_DODGE, 1);
	skillStat.add(Game::SkillList::BA_PARRY, 1);
	skillStat.add(Game::SkillList::BA_FIRST_AID, 1);
	effectStat.removeAll();
	fullHeal();
}

void Game::Stat::heal(UInt32 hp, UInt32 mana, UInt32 stamina){
	healthStat.health += hp;
	if(healthStat.health > healthStat.maxHealth){
		healthStat.health = healthStat.maxHealth;
	}

	healthStat.mana += mana;
	if(healthStat.mana > healthStat.maxMana){
		healthStat.mana = healthStat.maxMana;
	}

	healthStat.stamina += stamina;
	if(healthStat.stamina > healthStat.maxStamina){
		healthStat.stamina = healthStat.maxStamina;
	}		
}