#include "Entity.hpp"
#include "Engine/Graphics.hpp"
#include "Engine/Texture.hpp"
#include "Engine/Input.hpp"
#include "Engine/Font.hpp"
#include "Sword.hpp"
#include "Gun.hpp"
#include "Bullet.hpp"
#include "Engine/Tools/Tools.hpp"
#include "Engine/World.hpp"
#include "Engine/Shapes.hpp"
#include "Engine/Console.hpp"
#include "Engine/Shader.hpp"
#include "Engine/View.hpp"
#include "Game.hpp"

static UInt64 animationDataReloadTimeout = 1000;
static nite::Console::CreateProxy cpAnDatTo("cl_anchecktimeout", nite::Console::ProxyType::Int, sizeof(int), &animationDataReloadTimeout);
static bool animationDataReload = true;
static nite::Console::CreateProxy cpGlobalTimescale("cl_anautocheck", nite::Console::ProxyType::Bool, sizeof(bool), &animationDataReload);

/////////////
// COMMAND: ent_jump_to_mouse
////////////
static void cfEntityJumpToMouse(Vector<String> params){
	static auto game = Game::getInstance();
	if(params.size() < 1){
		nite::Console::add("Not enough parameters(1)", nite::Color(0.80f, 0.15f, 0.22f, 1.0f));
		return;
	}
	String &param0 = params[0];
	if(!nite::isNumber(param0)){
		nite::Console::add("'"+param0+"' is not a valid parameter", nite::Color(0.80f, 0.15f, 0.22f, 1.0f));
		return;
	}
	int id = nite::toInt(param0);	
	if(!game->world.exists(id)){
		nite::Console::add("Entity id '"+param0+"' does not exist", nite::Color(0.80f, 0.15f, 0.22f, 1.0f));
		return;
	}
	auto entity = static_cast<Game::Entity*>(game->world.objects[id].get());
	auto mp = nite::getView(nite::RenderTargetGame) + nite::mousePosition();
	entity->position = mp;
	nite::print("Entity id "+param0+" '"+entity->name+"' was manually moved to "+mp.str());
}
static auto cfEntityJumpToMouseIns = nite::Console::CreateFunction("ent_jump_to_mouse", &cfEntityJumpToMouse); 

/////////////
// COMMAND: ent_move
////////////
static void cfEntityMove(Vector<String> params){
	static auto game = Game::getInstance();
	if(params.size() < 1){
		nite::Console::add("Not enough parameters(3)", nite::Color(0.80f, 0.15f, 0.22f, 1.0f));
		return;
	}
	String &param0 = params[0]; // ent id
	String &param1 = params[1]; // x
	String &param2 = params[2]; // y

	if(!nite::isNumber(param0)){
		nite::Console::add("'"+param0+"' is not a valid parameter. entity id is expected", nite::Color(0.80f, 0.15f, 0.22f, 1.0f));
		return;
	}
	if(!nite::isNumber(param1)){
		nite::Console::add("'"+param1+"' is not a valid parameter. x position is expected", nite::Color(0.80f, 0.15f, 0.22f, 1.0f));
		return;
	}
	if(!nite::isNumber(param2)){
		nite::Console::add("'"+param2+"' is not a valid parameter. y position is expected", nite::Color(0.80f, 0.15f, 0.22f, 1.0f));
		return;
	}
	int id = nite::toInt(param0);	
	nite::Vec2 position(nite::toFloat(param1), nite::toFloat(param2));	
	if(!game->world.exists(id)){
		nite::Console::add("Entity id '"+param0+"' does not exist", nite::Color(0.80f, 0.15f, 0.22f, 1.0f));
		return;
	}
	auto entity = static_cast<Game::Entity*>(game->world.objects[id].get());
	entity->position = position;
	nite::print("Entity id "+param0+" '"+entity->name+"' was manually moved to "+position.str());
}
static auto cfEntityMoveIns = nite::Console::CreateFunction("ent_move", &cfEntityMove); 

/////////////
// COMMAND: ent_show
////////////
static void cfShowEntities(Vector<String> params){
	static auto game = Game::getInstance();
	String output;
	for (auto& it : game->world.objects){
		auto current = it.second;
		if(auto ent = dynamic_cast<Game::Entity*>(current.get())){
			output += "'"+ent->name+"' id: "+nite::toStr(ent->id)+", ";
		}
	}	
	nite::print("Active entities: "+output);
	
}
static auto cfShowEntitiesIns = nite::Console::CreateFunction("ent_show", &cfShowEntities); 


/////////////
// COMMAND: ent_reloadanims
////////////
static void cfReloadEntityAnims(Vector<String> params){
	static auto game = Game::getInstance();
	if(params.size() < 1){
		nite::Console::add("Not enough parameters(1)", nite::Color(0.80f, 0.15f, 0.22f, 1.0f));
		return;
	}
	String &param0 = params[0];
	if(!nite::isNumber(param0)){
		nite::Console::add("'"+param0+"' is not a valid parameter", nite::Color(0.80f, 0.15f, 0.22f, 1.0f));
		return;
	}
	int id = nite::toInt(param0);	
	if(!game->world.exists(id)){
		nite::Console::add("Entity id '"+param0+"' does not exist", nite::Color(0.80f, 0.15f, 0.22f, 1.0f));
		return;
	}
	auto entity = static_cast<Game::Entity*>(game->world.objects[id].get());	
	entity->entityReloadAnimation();
}
static auto cfReloadEntityAnimsIns = nite::Console::CreateFunction("ent_reloadanims", &cfReloadEntityAnims);  

/////////////
// COMMAND: ent_info
////////////
static void cfPrintEntityInfo(Vector<String> params){
	static auto game = Game::getInstance();
	if(params.size() < 1){
		nite::Console::add("Not enough parameters(1)", nite::Color(0.80f, 0.15f, 0.22f, 1.0f));
		return;
	}
	String &param0 = params[0];
	if(!nite::isNumber(param0)){
		nite::Console::add("'"+param0+"' is not a valid parameter", nite::Color(0.80f, 0.15f, 0.22f, 1.0f));
		return;
	}
	int id = nite::toInt(param0);	
	if(!game->world.exists(id)){
		nite::Console::add("Entity id '"+param0+"' does not exist", nite::Color(0.80f, 0.15f, 0.22f, 1.0f));
		return;
	}
	auto entity = static_cast<Game::Entity*>(game->world.objects[id].get());
	entity->printEntity();
}
static auto cfPrintEntityInfoIns = nite::Console::CreateFunction("ent_info", &cfPrintEntityInfo);  

/////////////
// COMMAND: ent_kill
////////////
static void cfEntityKill(Vector<String> params){
	static auto game = Game::getInstance();
	if(params.size() < 1){
		nite::Console::add("Not enough parameters(1)", nite::Color(0.80f, 0.15f, 0.22f, 1.0f));
		return;
	}
	String &param0 = params[0];
	if(!nite::isNumber(param0)){
		nite::Console::add("'"+param0+"' is not a valid parameter", nite::Color(0.80f, 0.15f, 0.22f, 1.0f));
		return;
	}
	int id = nite::toInt(param0);	
	if(!game->world.exists(id)){
		nite::Console::add("Entity id '"+param0+"' does not exist", nite::Color(0.80f, 0.15f, 0.22f, 1.0f));
		return;
	}
	auto entity = static_cast<Game::Entity*>(game->world.objects[id].get());
	entity->kill();
}
static auto cfEntityKillIns = nite::Console::CreateFunction("ent_kill", &cfEntityKill);  

/////////////
// COMMAND: ent_statadd
////////////
static void cfEntityStatUp(Vector<String> params){
	static auto game = Game::getInstance();
	if(params.size() < 3){
		nite::Console::add("Not enough parameters(1)", nite::Color(0.80f, 0.15f, 0.22f, 1.0f));
		return;
	}
	String &param0 = params[0];
	String &stat = params[1];
	String &amount = params[2];	
	if(!nite::isNumber(param0)){
		nite::Console::add("'"+param0+"' is not a valid parameter", nite::Color(0.80f, 0.15f, 0.22f, 1.0f));
		return;
	}
	int id = nite::toInt(param0);	
	if(!game->world.exists(id)){
		nite::Console::add("Entity id '"+param0+"' does not exist", nite::Color(0.80f, 0.15f, 0.22f, 1.0f));
		return;
	}
	auto entity = static_cast<Game::Entity*>(game->world.objects[id].get());
	if(!nite::isNumber(amount)){
		nite::Console::add("'"+amount+"' is not a valid parameter", nite::Color(0.80f, 0.15f, 0.22f, 1.0f));
		return;
	}	
	int amnt = nite::toInt(amount);
	int added = 0;
	bool success = true;
	if(stat == "str"){
		added = entity->addBaseStat(Game::BaseStatType::Strength, amnt);
	}else
	if(stat == "agi"){
		added = entity->addBaseStat(Game::BaseStatType::Agility, amnt);
	}else
	if(stat == "dex"){
		added = entity->addBaseStat(Game::BaseStatType::Dexterity, amnt);
	}else	
	if(stat == "end"){
		added = entity->addBaseStat(Game::BaseStatType::Endurance, amnt);
	}else	
	if(stat == "luk"){
		added = entity->addBaseStat(Game::BaseStatType::Luck, amnt);
	}else	
	if(stat == "cha"){
		added = entity->addBaseStat(Game::BaseStatType::Charisma, amnt);
	}else		
	if(stat == "int"){
		added = entity->addBaseStat(Game::BaseStatType::Intelligence, amnt);
	}else{
		success = false;
		nite::print("Invalid stat "+stat);
	}
	if(success){
		nite::print("Entity id "+param0+" '"+entity->name+"' stat "+stat+" by "+nite::toStr(added));
	}
	
}
static auto cfEntityStatUpIns = nite::Console::CreateFunction("ent_statadd", &cfEntityStatUp);  

/////////////
// COMMAND: ent_statreset
////////////
static void cfEntityStatReset(Vector<String> params){
	static auto game = Game::getInstance();
	if(params.size() < 1){
		nite::Console::add("Not enough parameters(2)", nite::Color(0.80f, 0.15f, 0.22f, 1.0f));
		return;
	}
	String &param0 = params[0];
	String &stat = params[1];
	if(!nite::isNumber(param0)){
		nite::Console::add("'"+param0+"' is not a valid parameter", nite::Color(0.80f, 0.15f, 0.22f, 1.0f));
		return;
	}
	int id = nite::toInt(param0);	
	if(!game->world.exists(id)){
		nite::Console::add("Entity id '"+param0+"' does not exist", nite::Color(0.80f, 0.15f, 0.22f, 1.0f));
		return;
	}
	auto entity = static_cast<Game::Entity*>(game->world.objects[id].get());	

	bool success = true;
	if(stat == "str"){
		entity->resetStat(Game::BaseStatType::Strength);
	}else
	if(stat == "agi"){
		entity->resetStat(Game::BaseStatType::Agility);
	}else
	if(stat == "dex"){
		entity->resetStat(Game::BaseStatType::Dexterity);
	}else	
	if(stat == "end"){
		entity->resetStat(Game::BaseStatType::Endurance);
	}else	
	if(stat == "luk"){
		entity->resetStat(Game::BaseStatType::Luck);
	}else
	if(stat == "cha"){
		entity->resetStat(Game::BaseStatType::Charisma);
	}else	
	if(stat == "int"){
		entity->resetStat(Game::BaseStatType::Intelligence);
	}else{
		success = false;
		nite::print("Invalid stat "+stat);
	}
	if(success){
		nite::print("Reset entity id "+param0+" '"+entity->name+"' stat "+stat+" to 0");
	}
	
}
static auto cfEntityStatResetIns = nite::Console::CreateFunction("ent_statreset", &cfEntityStatReset); 

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

void Game::Entity::onActiveItemSwitch(Shared<Game::BaseItem> &item){
	switch(item->type){
		case Game::ItemType::Equip: {
			auto &equip = *static_cast<Game::BaseEquip*>(item.get()); // cast as BaseEquip
			switch(equip.equipType){
				case Game::EquipType::Sword: {
					currentStance = Game::EntityStance::Melee;
				} break;
				case Game::EquipType::Gun: {
					currentStance = Game::EntityStance::Gun;
				} break;				
				default: {
					nite::print("entity: no stance for item '"+item->name+"' "+nite::toStr(item->id)+". unknown type");
				} break;
			}
		} break;
		default: {
			nite::print("entity: no stance for item '"+item->name+"' "+nite::toStr(item->id));
			return;
		} break;
	}	
}

void Game::Entity::entityUseActiveSlot(int slotId){
	// METHOD USAGE ACCORDING TO TYPE
	// WEAPON
	auto onSwordUse = [&](const Game::BaseItem &item){
		if(isSwordSwinging) return;
		swordSwingLastStep = 0;
		isSwordSwinging = true;
		swordSwingTimeout = nite::getTicks();
	};
	auto onGunUse = [&](const Game::BaseItem &item){
		if(isShootingGun) return;
		gunShootLastStep = 0;
		isShootingGun = true;
		gunShootTimeout = nite::getTicks();		
	};	
	// USABLE
	if(itemHandling.count(slotId) == 0){
		nite::print("entity: nothing active on slot "+nite::toStr(slotId));
		return;
	}
	if(isSwordSwinging || isShootingGun){
		return;	
	}
	auto &item = *itemCarry[itemHandling[slotId]].get();
	switch(item.type){
		case Game::ItemType::Equip: {
			auto &equip = *static_cast<Game::BaseEquip*>(itemCarry[itemHandling[slotId]].get()); // cast as BaseEquip
			switch(equip.equipType){
				case Game::EquipType::Sword: {
					onSwordUse(equip);
				} break;
				case Game::EquipType::Gun: {
					onGunUse(equip);
				} break;				
				default: {
					nite::print("entity: cannot use equip item '"+item.name+"' "+nite::toStr(item.id)+". unknown type");
				} break;
			}
		} break;
		default: {
			nite::print("entity: cannot equip item '"+item.name+"' "+nite::toStr(item.id));
			return;
		} break;
	}
}

void Game::Entity::entityStep(){
  nite::lerp(entityAlpha, dead ? 0.0f : 100.0f, 0.25f);
  dims.update();
  
	// TODO: Proper death sequence
  if(dead){
		destroy();
		return;
	}
  
  // Alive tasks
  if(health <= 0){
    kill();
  }		

  auto fixStance = [&](){
	if(currentStance == Game::EntityStance::Neutral){
			animCurrentTop = animTopIdle;
			animCurrentBottom = animBottomIdle;
	  }
	  if(currentStance == Game::EntityStance::Melee){
			animCurrentTop = animTopSwordSwinging;
			animCurrentBottom = animBottomIdle;
	  }
	  if(currentStance == Game::EntityStance::Gun){
			animCurrentTop = animTopGunOnHand;
			animCurrentBottom = animBottomIdle;
	  }	  
	  if(currentStance == Game::EntityStance::KnockedBack){
			animCurrentTop = animTopKnockback;
			animCurrentBottom = animBottomKnockback;
			
	  }	  
  };
	
  fixStance();
  int acvslot = Game::InventoryActiveSlot::Main;
  // Knockback
  if(isKnockedback && currentStance == Game::EntityStance::KnockedBack){
	  animCurrentTop = animTopKnockback;
	  animCurrentBottom = animBottomKnockback;	  
	  if(nite::getTicks() - knockbackTimeout > 200){
		  isKnockedback = false;
		  reverseStance();
	  }
  }else
  // Sword Swinging & Hand hold state
  if(isSwordSwinging && currentStance == Game::EntityStance::Melee){
		animCurrentTop = animTopSwordSwinging;
		int _rate = atkRate / 35.0f;
		UInt64 nextStepTimeout = dims.faSwordForward.frames[swordSwingLastStep % dims.faSwordForward.frames.size()].time;
		nextStepTimeout = _rate > nextStepTimeout ? 0 : nextStepTimeout - _rate;
		auto &weap = *static_cast<Game::Sword*>(itemCarry[itemHandling[acvslot]].get()); // assuming it is a sword, might have to add some extra checks for this
		if(nite::getTicks() - swordSwingTimeout > nite::timescaled(nextStepTimeout)){
			if(swordSwingLastStep == 0){
				weap.onSwingStart(this);
			}
			swordSwingTimeout = nite::getTicks();
			if(swordSwingLastStep > 0){
				throwMelee(&weap);
			}		
			++swordSwingLastStep;
			if(swordSwingLastStep >= dims.faSwordForward.frames.size()){
				weap.onSwingFinish(this);
				isSwordSwinging = false;
				animCurrentTop = animTopSwordOnHand;
			}
		}
		entityAnim.setManualClicking(animTopSwordSwinging, true);
		entityAnim.setFrame(animTopSwordSwinging, swordSwingLastStep % entityAnim.animations[animTopSwordSwinging].index.size());
  }else
  if(!isSwordSwinging && currentStance == Game::EntityStance::Melee){
		animCurrentTop = animTopSwordOnHand;
  }else
  // Shoot shooting & Hand hold state
  if(isShootingGun && currentStance == Game::EntityStance::Gun){
		animCurrentTop = animTopGunShoot;
		int _rate = atkRate / 35.0f; // shoot speed, might consider another stat for this
		auto &frame = dims.faGunShoot.frames[gunShootLastStep % dims.faGunShoot.frames.size()];
		UInt64 nextStepTimeout = frame.time;
		nextStepTimeout = _rate > nextStepTimeout ? 0 : nextStepTimeout - _rate;
		auto &weap = *static_cast<Game::Gun*>(itemCarry[itemHandling[acvslot]].get()); // assuming it is a gun, might have to add some extra checks for this
		if(nite::getTicks() - gunShootTimeout > nite::timescaled(nextStepTimeout)){
			gunShootTimeout = nite::getTicks();
			if(gunShootLastStep > 0 && frame.damage){ // 'damage' for gun is when the bullet is shot. there must be only one frame with 'damage'
				weap.onShot(this);
				shootBullet(&weap);
			}		
			++gunShootLastStep;
			if(gunShootLastStep >= dims.faGunShoot.frames.size()){
				isShootingGun = false;
				animCurrentTop = animTopGunOnHand;
			}
		}
		entityAnim.setManualClicking(animTopGunShoot, true);
		entityAnim.setFrame(animTopGunShoot, gunShootLastStep % entityAnim.animations[animTopGunShoot].index.size());
  }else
  if(!isShootingGun && currentStance == Game::EntityStance::Gun){
		animCurrentTop = animTopGunOnHand;
  }

  // Walking State
  if(!isKnockedback && isWalking){
    animCurrentBottom = animBottomWalking;
    if(nite::getTicks()-walkingStepTimeout > nite::timescaled(128 - walkRate * 2.0)){
      walkingStepTimeout = nite::getTicks();
      ++walkingLastStep;
    }
    if(nite::getTicks()-walkingTimeout > nite::timescaled(64)){
      isWalking = false;
    }
    entityAnim.setManualClicking(animBottomWalking, true);
    entityAnim.setFrame(animBottomWalking, walkingLastStep % entityAnim.animations[animBottomWalking].index.size());	
  }
}

void Game::Entity::recalculateHealthStats(){
  this->maxHealth = nite::ceil((12.5f + enduStat * 1.5f)  * baseScaleEntity * lv);
  this->maxMana = nite::ceil((4.5f + intStat * 0.5f) * baseScaleEntity * lv);
  this->maxStamina = nite::ceil((1.9f + dexStat * 0.35f + enduStat * 0.15f) * baseScaleEntity * lv);
}

void Game::Entity::recalculateComplexStats(){
  this->maxCarry = 25 + nite::ceil(strStat * baseScaleEntity * 4.0f + enduStat * baseScaleEntity * 2.0f);
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

void Game::Entity::printEntity(){
  #define str nite::toStr
	String ms = str(GAME_MAX_STAT);
  nite::print("ID "+str(id)+" | Name: "+name+" | LV "+str(lv)+" | Carry "+str(this->maxCarry));
  nite::print("HP "+str(maxHealth)+" | Mana "+str(maxMana)+" | Stamina "+str(maxStamina)+" | StatPoints "+str(statPoints));
  nite::print("Str "+str(strStat)+"/"+ms+" | Agi "+str(agiStat)+"/"+ms+
	" | Dex "+str(dexStat)+"/"+ms+" | Endurance "+str(enduStat)+"/"+ms+
	"| Luk "+str(lukStat)+"/"+ms+" | Int "+str(intStat)+"/"+ms+
	" | Char "+str(charismaStat)+"/"+ms);
  nite::print("Atk "+str(this->atk)+" "+"MAtk "+str(this->magicAtk)+" "+"Def "+str(this->def)+" "+"MDef "+str(this->magicDef));
  nite::print("WalkRate "+str(this->walkRate));
  nite::print("CritRate "+str(this->critRate));
  nite::print("PrecsRate "+str(this->precsRate));
  nite::print("AtkRate "+str(this->atkRate));
  nite::print("CharmRate "+str(this->charmRate));
  nite::print("PersuasionRate "+str(this->persuasionRate));    
  #undef str
}

void Game::Entity::recalculateStats(){
  recalculateHealthStats();
  recalculateComplexStats();
}

void Game::Entity::resetStat(int type){
	statPoints += agiStat;
	agiStat = 0;
	recalculateStats();	
}

int Game::Entity::addBaseStat(int type, int amnt){
  int toAdd = 0;
  auto inc = [&](int &target){
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

void Game::Entity::kill(){
	nite::print("Killed entity id "+nite::toStr(id)+" '"+name+"'");
  if(dead){
	  return;
  }
  dead = true;
  onDeath();
}

void Game::Entity::switchStance(int stance){
	if(stance == currentStance) return;
	lastStance = currentStance;
	currentStance = stance;
}

void Game::Entity::reverseStance(){
	int w = currentStance;
	currentStance = lastStance;
	lastStance = w;
}

void Game::Entity::setupEntity(int lv, float baseScale){
  this->lv = 0;
  this->baseScaleEntity = baseScale;
  this->exp = 0;
  this->dead = false;
  this->statPoints = 12;
  while(this->lv < lv && lvUp());
  fullHeal();
  entityAlpha = 0.0f;
}

void Game::Entity::fullHeal(){
  health = maxHealth;
  mana = maxMana;
  stamina = maxStamina;
}

bool Game::Entity::lvUp(){
  if(lv >= GAME_MAX_LEVEL){
    return false;
  }
  ++lv;
  this->statPoints += 6;
  this->exp = 0;
  this->nextExp = nite::ceil((lv + 1) * baseScaleEntity * 250);
  return true;
}

void Game::FrameData::parse(const Jzon::Node &node){
  index.set(node.get("index").get("x").toFloat(), node.get("index").get("y").toFloat());
  n = node.get("n").toInt();
  reversed = node.get("reversed").toBool();
  vertical = node.get("vertical").toBool();
  speed = node.get("speed").toFloat();
}

bool Game::AnimationData::load(const String &path){
	clear();	
	filePath = path;
	lastHash = nite::hashFile(path);
	Jzon::Parser parser;
	Jzon::Node node = parser.parseFile(path);
	if (!node.isValid()){
		nite::print("Failed to read file "+path+": '"+parser.getError()+"'");
		return false;
	}
	// Basic data
	headPosition.set(node.get("headPosition").get("x").toFloat(), node.get("headPosition").get("y").toFloat());
	headSize.set(node.get("headSize").get("width").toFloat(), node.get("headSize").get("height").toFloat());
	frameSize.set(node.get("frameSize").get("width").toFloat(), node.get("frameSize").get("height").toFloat());
	spriteSize.set(node.get("spriteSize").get("width").toFloat(), node.get("spriteSize").get("height").toFloat());
	defaultPhysicalSize.set(node.get("defaultPhysicalSize").get("width").toFloat(), node.get("defaultPhysicalSize").get("height").toFloat());
	frameDepthOffset = node.get("frameDepthOffset").toFloat();
	faceDirBias = node.get("faceDirBias").toFloat();
	globalDepthOffsetY = node.get("globalDepthOffsetY").toFloat();
	walkRightStepOffset = node.get("walkRightStepOffset").toFloat();
	spriteFilename = node.get("spriteFilename").toString();
	gunOnHandPosition.set(node.get("gunPositionOnHand").get("x").toFloat(), node.get("gunPositionOnHand").get("y").toFloat());
	// Frames
	topIdle.parse(node.get("animations").get("TopIdle"));
	topWalking.parse(node.get("animations").get("TopIdle"));
	bottomWalking.parse(node.get("animations").get("BottomWalking"));
	bottomIdle.parse(node.get("animations").get("BottomIdle"));
	topSwordSwing.parse(node.get("animations").get("TopSwordSwing"));
	topSwordOnHand.parse(node.get("animations").get("TopSwordOnHand"));
	topKnockback.parse(node.get("animations").get("TopKnockback"));
	bottomKnockback.parse(node.get("animations").get("BottomKnockback"));
	topGunOnHand.parse(node.get("animations").get("TopGunOnHand"));
	topGunShoot.parse(node.get("animations").get("TopGunShoot"));
	topCast.parse(node.get("animations").get("TopCast"));
	// Frame Animations
	auto faNode = node.get("frames");
	faSwordForward.load(faNode.get("SwordForward"));
	faSwordOnHand.load(faNode.get("SwordIdle"));
	faGunOnHand.load(faNode.get("GunIdle"));
	faGunShoot.load(faNode.get("GunShoot"));
	nite::print("loaded entity data '"+path+"'");
	// Hitboxes
	auto hbNode = node.get("hitbox");
	hitboxes.clear();
	for(int i = 0; i < hbNode.getCount(); ++i){
		hitboxes.push_back(nite::Hitbox(hbNode.get(i)));
	}
	return true;
}

void Game::AnimationData::update(){
	if(animationDataReload && nite::getTicks() - lastFileCheck > animationDataReloadTimeout){
		if(nite::fileExists(filePath)){
			auto hash = nite::hashFile(filePath);
			if(hash != lastHash){
				// reload
				if(host != NULL){
					host->entityReloadAnimation();
				}
			}
		}
		lastFileCheck = nite::getTicks();
	}
}

void Game::AnimationData::clear(){
	faSwordForward.clear();
	faSwordOnHand.clear();
	faGunShoot.clear();
	faGunOnHand.clear();
}

static unsigned parseAnimation(nite::Animation &an, const nite::Vec2 &ss, const Game::FrameData &fd){
  return an.add(ss.x * fd.index.x, ss.y * fd.index.y, ss.x, ss.y, fd.n, fd.speed, fd.vertical, fd.reversed);
}

void Game::Entity::entityReloadAnimation(){
	entityAnim.clear();
	String path = "data/entity/basic_template.json";
	dims.load(path);
	entityAnim.load(dims.spriteFilename, nite::Color(1.0f, 1.0f, 1.0f));
	auto &ss = dims.spriteSize;
	auto &frameSize = dims.frameSize;
	size.set(dims.defaultPhysicalSize);    
	auto &frameDepthOffset = dims.frameDepthOffset;
	batch.init(ss.x, ss.y); 
	animTopIdle = parseAnimation(entityAnim, ss, dims.topIdle);
	animTopWalking = parseAnimation(entityAnim, ss, dims.topWalking);
	animBottomIdle = parseAnimation(entityAnim, ss, dims.bottomIdle);
	animBottomWalking = parseAnimation(entityAnim, ss, dims.bottomWalking);
	animTopSwordSwinging = parseAnimation(entityAnim, ss, dims.topSwordSwing);
	animTopSwordOnHand = parseAnimation(entityAnim, ss, dims.topSwordOnHand);
	animTopKnockback = parseAnimation(entityAnim, ss, dims.topKnockback);
	animBottomKnockback = parseAnimation(entityAnim, ss, dims.bottomKnockback);
	animTopGunOnHand = parseAnimation(entityAnim, ss, dims.topGunOnHand);
	animTopGunShoot = parseAnimation(entityAnim, ss, dims.topGunShoot);
	animTopCast = parseAnimation(entityAnim, ss, dims.topCast);
	nite::print("reloaded entity '"+path+"' animations");
}

void Game::Entity::entityInit(){
  dims.host = this;
  faceDirection = EntityFacing::Right;
  entityShadow.load("data/sprite/shadow.png");
  // DEFAULT
  entityReloadAnimation();
  animCurrentTop = animTopIdle;
  animCurrentBottom = animBottomIdle;
  entityAlpha = 0.0f;
  switchStance(Game::EntityStance::Neutral);
}

void Game::Entity::onCreate(){
  unmovable = false;
  solid = true;
  friction = 0.25f; 
  mass = 2.8f;
  dead = false;
  walkPushRate = 5.0f;
  name = "Base Entity Type";
}

void Game::Entity::onDestroy(){
  
}

// This method must be used to move entities
// Do not use PhyshicsObject's methods nor manually modifiying Vec2 position
// NOTE: Angle is radians
void Game::Entity::entityMove(float angle, float mod, bool holdStance){ 
  if(!holdStance && (nite::toDegrees(angle) <= 45 || nite::toDegrees(angle) >= 315)){
	  faceDirection = EntityFacing::Right;
  }
  if(!holdStance && (nite::toDegrees(angle) >= 135 && nite::toDegrees(angle) <= 225)){
	  faceDirection = EntityFacing::Left;
  }
  isWalking = true;
  walkingTimeout = nite::getTicks();
  move(angle, mod + 0.25f * walkRate);
}

void Game::Entity::step(){
				
}

void Game::Entity::dealDamage(const Game::DamageInfo &dmg){
	int fdamage = dmg.amount - this->def;
	// int fdamage = 10;
	if(fdamage <= 0){
		nite::print("0 damage to entity '"+this->name+"'");
		return;
	}
	if(dmg.knockback){
		float an = nite::arctan(position.y - dmg.owner->position.y, position.x - dmg.owner->position.x);
		float mod = 50.57f;
		this->knockback(an, mod);
	}
	nite::print("dealt "+nite::toStr(fdamage)+" entity '"+this->name+"'");
	
}

void Game::Entity::knockback(float angle, float intensity){
	if(isKnockedback) return;
	switchStance(Game::EntityStance::KnockedBack);
	knockbackTimeout = nite::getTicks();
	isKnockedback = true;	
	push(angle, intensity);
}

void Game::Entity::shootBullet(Game::BaseEquip *_weap){
	auto &weap = *static_cast<Game::Gun*>(_weap);
	auto bullet = Shared<Game::BaseBullet>(new Game::BaseBullet());
	float an = faceDirection == Game::EntityFacing::Right ? 0.0f : 3.142f;
	float mod = 42.0f;
	nite::Vec2 flip = nite::Vec2(faceDirection == 1.0f ? -1.0f : 1.0f, 1.0f);
	bullet->position.set(position + dims.gunOnHandPosition * flip + weap.barrelPosition * flip);
	bullet->setup(this, mod, an, 0, 0, 2000);
	container->add(bullet);
}

void Game::Entity::throwMelee(Game::BaseEquip *_weap){
	auto &weap = *static_cast<Game::Sword*>(_weap);
	if(container == NULL){
		return;
	}
	nite::Hitbox melee = getMeleeHitbox();
	int acvslot = Game::InventoryActiveSlot::Main;
	auto sendDamage = [&](MeleeHitInformation &data){
		Game::DamageInfo dmg;
		dmg.subject = data.subject;
		dmg.owner = this;
		dmg.amount = weap.damage + this->atk; // TODO: Add damage specification from entity to weapons
		dmg.element = weap.getEnchant();
		dmg.knockback = true;
		data.subject->dealDamage(dmg);
	};
	for (auto &it : container->objects){
		auto current = it.second;
		if(current.get() == NULL){
			continue;
		}
		if(Game::Entity *_ent = dynamic_cast<Game::Entity*>(current.get())){
			auto data = _ent->receiveMelee(melee);
			if(data.hit){
				// nite::shakeScreen(nite::RenderTargetGame, 2.2f, 200);
				weap.onHit(this, _ent);
				sendDamage(data);
				break;
			}
		}		
	}	
}

Game::MeleeHitInformation Game::Entity::receiveMelee(nite::Hitbox &input){
	for(int i = 0; i < dims.hitboxes.size(); ++i){
		auto &current = dims.hitboxes[i];
		nite::Hitbox corrected = current;
		corrected.position = position - dims.frameSize * 0.5f + current.position;
		corrected.size =  current.size;
		if(corrected.collision(input)){
			Game::MeleeHitInformation data;
			data.hitbox = corrected;
			data.hit = true;
			data.subject = this;
			return data;
		}
	}
	return Game::MeleeHitInformation();
}

nite::Hitbox Game::Entity::getMeleeHitbox(){
	float faceDir = dims.faceDirBias * (EntityFacing::Right == faceDirection ? 1.0f : -1.0f);	
	int acvslot = Game::InventoryActiveSlot::Main;	
	float stepOffset = (isWalking && walkingLastStep % 2 == 0) ? dims.walkRightStepOffset : 0.0f;	
	if(!(itemHandling.count(acvslot) > 0 && itemHandling[acvslot] != -1)){
		return nite::Hitbox();
	}
	auto &weap = *static_cast<Sword*>(itemCarry[itemHandling[acvslot]].get());
	nite::Vec2 _orig(weap.origin / weap.frameSize);
	auto &swingFrame = dims.faSwordForward.frames[swordSwingLastStep % dims.faSwordForward.frames.size()]; 
	auto &onHandFrame = dims.faSwordOnHand.frames[swordSwingLastStep % dims.faSwordOnHand.frames.size()];
	auto &animFrame = isSwordSwinging ? swingFrame : onHandFrame;
	nite::Vec2 ofs = animFrame.position - weap.frameSize * 0.5f;
	nite::Vec2 p(0.0f + ofs.x, 0.0f + stepOffset + ofs.y);
	if(faceDir == 1.0f){
		p.x *= -1.0f;
	}
	p = p + position;
	float devi = (weap.frameSize.x - nite::cos(nite::toRadians(animFrame.angle * faceDir)) * weap.frameSize.x) * faceDir * dims.faceDirBias;
	nite::Hitbox hb;
	hb.position.set(p - weap.frameSize * _orig + nite::Vec2(devi, 0.0f));
	hb.size.set(weap.frameSize);
	return hb;
}

void Game::Entity::draw(){
	static nite::Font font(nite::DefaultFontPath, 16);
	nite::setRenderTarget(nite::RenderTargetGame);
	nite::setColor(1.0f, 1.0f, 1.0f, 1.0f);
	nite::setDepth(nite::DepthMiddle);
	float faceDir = dims.faceDirBias * (EntityFacing::Right == faceDirection ? 1.0f : -1.0f);

	nite::Vec2 &frameSize = dims.frameSize;
	float &frameDepthOffset = dims.frameDepthOffset;

	batch.begin();
	auto *refTop = entityAnim.draw(animCurrentTop, 0.0f, 0.0f, frameSize.x, frameSize.y, 0.0f, 0.0f, 0.0f);
	auto *refBottom = entityAnim.draw(animCurrentBottom, 0.0f, 0.0f, frameSize.x, frameSize.y, 0.0f, 0.0f, 0.0f);
	batch.end();
	batch.flush();

	float stepOffset = (isWalking && walkingLastStep % 2 == 0) ? dims.walkRightStepOffset : 0.0f;
	nite::setColor(0.7f, 0.7f, 0.7f, 0.5f);
	// entityShadow.draw(position.x, position.y + dims.frameSize.y * 0.25f, entityShadow.getWidth(), entityShadow.getHeight(), 0.5f, 0.5f, 0.0f);
	nite::setColor(1.0f, 1.0f, 1.0f, 1.0f);
	nite::setDepth(-position.y + dims.globalDepthOffsetY); // dynamic depth using y
	auto *ref = batch.draw(position.x, position.y - frameDepthOffset + stepOffset, frameSize.x * faceDir, frameSize.y, 0.5f, 0.5f, 0.0f);
	if(ref != NULL){
		ref->smooth = true;  
	}
	nite::setDepth(nite::DepthMiddle);
	// Draw stance
	int acvslot = Game::InventoryActiveSlot::Main;
	auto swordStance = [&](Game::Sword &weap){
		nite::Vec2 _orig(weap.origin / weap.frameSize);
		nite::setColor(1.0f, 1.0f, 1.0f, 1.0f);
		auto &swingFrame = dims.faSwordForward.frames[swordSwingLastStep % dims.faSwordForward.frames.size()]; 
		auto &onHandFrame = dims.faSwordOnHand.frames[swordSwingLastStep % dims.faSwordOnHand.frames.size()];
		auto &animFrame = isSwordSwinging ? swingFrame : onHandFrame;
		nite::Vec2 ofs = animFrame.position - weap.frameSize * 0.5f;
		nite::Vec2 p(0.0f + ofs.x, 0.0f + stepOffset + ofs.y);
		nite::Vec2 blurDir(0.032f, 0.032f);
		static nite::Shader dirShader("data/shaders/directional_blur_f.glsl", "data/shaders/directional_blur_v.glsl");
		if(faceDir == 1.0f){
			//nite::print(nite::toStr(frameSize.x)+" "+nite::toStr(p.x));
			p.x *= -1.0f;
		}
		p = p + position;
		if(swordSwingLastStep > 0 && isSwordSwinging){
			auto &currentFrame = dims.faSwordForward.frames[(swordSwingLastStep % dims.faSwordForward.frames.size()) - 1];
			nite::Vec2 _dif = (currentFrame.position - weap.frameSize * 0.5f) - ofs;
			if(faceDir == 1.0f){
				_dif.x *= -1.0f;
			}
			blurDir.set(nite::Vec2(nite::getSign(_dif.x) * 0.03f, nite::getSign(_dif.y)));
		}
		auto *ref = weap.equipAnim.draw(weap.animNormal, p.x, p.y, weap.frameSize.x, weap.frameSize.y, _orig.x, _orig.y, 0.0f);
		if(ref != NULL){
			ref->angle = animFrame.angle * faceDir * dims.faceDirBias;
			ref->smooth = true;
			if(isSwordSwinging){
				nite::Uniform uniforms;
				// uniforms.add("dir", blurDir);
				ref->apply(dirShader, uniforms);
			}
		}

		if(container != NULL && container->debugPhysics){
			// for(int i = 0; i < weap.hitboxes.size(); ++i){
			// auto &hit = weap.hitboxes[i];
			// Using weap's own frameSize as hitbox for now
			nite::setColor(nite::Color(1.0f, 0.5f, 0));
			auto devi = (weap.frameSize.x - nite::cos(nite::toRadians(animFrame.angle * faceDir)) * weap.frameSize.x) * faceDir * dims.faceDirBias;
			nite::Draw::Rectangle(p - weap.frameSize * _orig + nite::Vec2(devi, 0.0f), weap.frameSize, false, nite::Vec2(0.0f), 0);
			// }  
		}
	};
	auto gunStance = [&](Game::Gun &weap){
		auto &shootFrame = dims.faGunShoot.frames[gunShootLastStep % dims.faGunShoot.frames.size()]; 
		auto &onHandFrame = dims.faGunOnHand.frames[gunShootLastStep % dims.faGunOnHand.frames.size()];
		auto &animFrame = isShootingGun ? shootFrame : onHandFrame;				
		nite::Vec2 _orig(weap.origin / weap.frameSize);
		nite::Vec2 ofs = animFrame.position - weap.frameSize * 0.5f;
		nite::Vec2 p(0.0f + ofs.x, 0.0f + stepOffset + ofs.y);
		if(faceDir == 1.0f){
			//nite::print(nite::toStr(frameSize.x)+" "+nite::toStr(p.x));
			p.x *= -1.0f;
		}
		p = p + position;		
		auto *ref = weap.equipAnim.draw(weap.animNormal, p.x, p.y, weap.frameSize.x * (faceDir == 1.0f ? -1.0f : 1.0f), weap.frameSize.y, _orig.x, _orig.y, 0.0f);
		if(ref != NULL){
			ref->angle = animFrame.angle * faceDir * dims.faceDirBias;
		}		
	};
	if(itemHandling.count(acvslot) > 0 && itemHandling[acvslot] != -1){
		auto item = static_cast<Game::BaseEquip*>(itemCarry[itemHandling[acvslot]].get()); // assuming it's always an equip for now
		switch(item->equipType){
			case Game::EquipType::Sword: {
				swordStance(*static_cast<Game::Sword*>(item));
			} break;
			case Game::EquipType::Gun: {
				gunStance(*static_cast<Game::Gun*>(item));
			} break;			
		}
	}  
	if(container != NULL && container->debugPhysics){
		// Debugging
		for(int i = 0; i < dims.hitboxes.size(); ++i){
		auto &hit = dims.hitboxes[i];
		nite::setColor(nite::Color(1.0f, 0.5f, 0));
		nite::Draw::Rectangle(position - frameSize * 0.5f + hit.position, hit.size, false, nite::Vec2(0.0f), 0);
		}  
		nite::setColor(nite::Color(1.0f, 0.0f, 0.5f));	
		nite::Draw::Rectangle(position - frameSize * 0.5f , frameSize, false, nite::Vec2(0.0f), 0);
	}
}

void Game::FrameAnimation::clear(){
	frames.clear();
}

void Game::FrameAnimation::load(const Jzon::Node &node){
	auto getFrame = [&](const Jzon::Node &node){
		Game::FrameAnimationData data;
		data.position.set(node.get("x").toFloat(), node.get("y").toFloat());
		data.angle = node.get("angle").toFloat();
		data.time = node.get("time").toInt();
		data.damage = node.get("damage").toBool();
		return data;
	};
	if(node.isArray()){
		for(int i = 0; i < node.getCount(); ++i){
			frames.push_back(getFrame(node.get(i)));
		}
	}else{
		frames.push_back(getFrame(node));
	}
}
