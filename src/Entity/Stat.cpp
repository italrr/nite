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
	this->maxHealth = nite::ceil((12.5f + enduStat * 1.5f)  * GAME_STAT_BASE_SCALE * lv);
	this->maxMana = nite::ceil((4.5f + intStat * 0.5f) * GAME_STAT_BASE_SCALE * lv);
	this->maxStamina = nite::ceil((1.9f + dexStat * 0.35f + enduStat * 0.15f) * GAME_STAT_BASE_SCALE * lv);
}

void Game::Stat::recalculateComplexStats(){
	this->maxCarry = 25 + nite::ceil(strStat * GAME_STAT_BASE_SCALE * 4.0f + enduStat * GAME_STAT_BASE_SCALE * 2.0f);
	this->atk = nite::ceil(strStat * 8.5f + enduStat * 1.2f);
	this->magicAtk = nite::ceil(intStat * 8.5f);
	this->def = nite::ceil(enduStat * 5.8f + lv * 0.7f);
	this->magicDef = nite::ceil(intStat * 4.5f + enduStat * 1.2f);		
	this->walkRate = nite::ceil(0.18f * agiStat); 
	this->critRate = (float)lukStat / (float)GAME_MAX_STAT;
	this->precsRate = dexStat * 1.25f;
	this->atkRate = agiStat * (3.85f + lv * 0.15f) + dexStat * (1.1f + lv * 0.1f);
	this->persuasionRate = nite::ceil(0.45f * lv + charismaStat * 5.96f + intStat * 2.25f);
	this->charmRate = nite::ceil(0.5f * lv + charismaStat * 5.96f);
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
		int toRest = statPoints;
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
			toRest = statPoints - toAdd;
		}
		statPoints = toRest;
		target += toAdd;
	};
	switch(type){
		case BaseStatType::Strength:{
			inc(strStat);
		} break;
		case BaseStatType::Agility:{
			inc(agiStat);
		} break;
		case BaseStatType::Dexterity:{
			inc(dexStat);
		} break;
		case BaseStatType::Endurance:{
			inc(enduStat);
		} break;
		case BaseStatType::Luck:{
			inc(lukStat);
		} break;
		case BaseStatType::Intelligence:{
			inc(intStat);
		} break;
		case BaseStatType::Charisma:{
			inc(charismaStat);
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
	statPoints += agiStat;
	switch(type){
		case BaseStatType::Strength:{
			strStat = 0;
		} break;
		case BaseStatType::Agility:{
			agiStat = 0;
		} break;
		case BaseStatType::Dexterity:{
			dexStat = 0;
		} break;
		case BaseStatType::Endurance:{
			enduStat = 0;
		} break;
		case BaseStatType::Luck:{
			lukStat = 0;
		} break;
		case BaseStatType::Intelligence:{
			intStat = 0;
		} break;
		case BaseStatType::Charisma:{
			charismaStat = 0;
		} break;
	}
	recalculateStats();	
}

void Game::Stat::fullHeal(){
	health = maxHealth;
	mana = maxMana;
	stamina = maxStamina;
}

bool Game::Stat::lvUp(){
	if(lv >= GAME_MAX_LEVEL){
		return false;
	}
	++lv;
	this->statPoints += 6;
	this->exp = 0;
	this->nextExp = nite::ceil((lv + 1) * GAME_STAT_BASE_SCALE * 250);
	return true;
}

void Game::Stat::setupStat(UInt16 lv){
	this->lv = 0;
	this->exp = 0;
	this->dead = false;
	this->statPoints = 12;
	while(this->lv < lv && lvUp());
	fullHeal();
}