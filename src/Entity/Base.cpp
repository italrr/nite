#include "../Engine/Graphics.hpp"
#include "../Engine/Texture.hpp"
#include "../Engine/UI/UI.hpp"
#include "../Engine/Console.hpp"

#include "../Core/Network.hpp"
#include "../Core/Server.hpp"

#include "Base.hpp"

static bool showHitboxes = false;
static nite::Console::CreateProxy cpshowHitboxes("cl_show_hitbox", nite::Console::ProxyType::Bool, sizeof(bool), &showHitboxes);


static UInt64 EntityDamageRecover = 250;
static nite::Console::CreateProxy cpEntityDamageRecover("gm_entity_damage_rcv_time", nite::Console::ProxyType::Int, sizeof(int), &EntityDamageRecover, true, true);

static void notifyEntityDeath(Game::EntityBase *ent){
    if(ent != NULL && ent->sv != NULL){
        if(auto cl = ent->sv->getClientByEntityId(ent->id)){
            ent->sv->notifyDeath(cl->clientId);
        }
    }
}

void Game::AIDriver::set(Game::EntityBase *subject){
	this->subject = subject;
}

void Game::AIDriver::update(){
	for(int i = 0; i < behaviors.size(); ++i){
		auto beh = behaviors[i];
		if(beh->isReady()){
			beh->think(subject);
		}
	}
}

void Game::AIDriver::add(Shared<Game::AI::BaseBehavior> behavior){
	behaviors.push_back(behavior);
	behavior->init();
}

Game::EntityBase::EntityBase(){
	this->isCasting = false;
	this->effectStat.owner = this;
	this->skillStat.owner = this;
	this->invStat.owner = this;
	this->lastMeleeHit = nite::getTicks();
	this->lastUpdateStats = nite::getTicks();
	this->objType = ObjectType::Entity;
	this->lastDmgd = nite::getTicks();
	this->currentCasting = Shared<Game::EntityCasting>(NULL);
	setState(EntityState::IDLE, EntityStateSlot::MID, 0);
	setState(EntityState::IDLE, EntityStateSlot::BOTTOM, 0);
	walkStepTick = 0;
	aidriver.set(this);
}

bool Game::EntityBase::canDamage(){
	return nite::getTicks()-lastDmgd > EntityDamageRecover;
}

void Game::EntityBase::entityMove(const nite::Vec2 &dir, bool holdStance){  // more like hold direction
	if(healthStat.dead){
		return;
	}
	if(!holdStance && dir.x > 0){
		faceDirection = EntityFacing::Right;
	}
	if(!holdStance && dir.x < 0){
		faceDirection = EntityFacing::Left;
	}
	if(state[EntityStateSlot::BOTTOM] != EntityState::WALKING){
		setState(EntityState::WALKING, EntityStateSlot::BOTTOM, 0);
	}
	isMoving = true;
	move((nite::Vec2(5.8f) + nite::Vec2(complexStat.walkRate)) * dir);
}

void Game::EntityBase::kill(){
	nite::print("killed entity id "+nite::toStr(id)+" '"+name+"'");
	if(healthStat.dead){
		return;
	}
	healthStat.dead = true;
	onDeath();
}

void Game::EntityBase::onCreate(){
    unmovable = false;
    solid = true;
    friction = 0.87f;
    mass = 2.8f;
    healthStat.dead = false;
    size.set(128, 128);
    name = "Base Entity Type";
}

void Game::EntityBase::printInfo(){
	String p = this->position.str();
	#define str nite::toStr
	String ms = str(GAME_MAX_STAT);
	nite::print("ID "+str(id)+" | Name: "+name+" | LV "+str(healthStat.lv)+" | Carry "+str(complexStat.maxCarry)+" | Position "+p);
	nite::print("HP "+str(healthStat.maxHealth)+" | Mana "+str(healthStat.maxMana)+" | Stamina "+str(healthStat.maxStamina)+" | StatPoints "+str(baseStat.statPoints));
	nite::print("Str "+str(baseStat.strAdd)+"/"+ms+" | Agi "+str(baseStat.agiAdd)+"/"+ms+
	" | Dex "+str(baseStat.dexAdd)+"/"+ms+" | Endurance "+str(baseStat.enduAdd)+"/"+ms+
	"| Luk "+str(baseStat.lukAdd)+"/"+ms+" | Int "+str(baseStat.intelAdd)+"/"+ms+
	" | Char "+str(baseStat.charmAdd)+"/"+ms);
	nite::print("Atk "+str(complexStat.atk)+" "+"MAtk "+str(complexStat.magicAtk)+" "+"Def "+str(complexStat.def)+" "+"MDef "+str(complexStat.magicDef));
	nite::print("WalkRate "+str(complexStat.walkRate));
	nite::print("CritRate "+str(complexStat.critRate));
	nite::print("PrecsRate "+str(complexStat.precsRate));
	nite::print("AtkRate "+str(complexStat.atkRate));
	nite::print("CharmRate "+str(complexStat.charmRate));
	nite::print("PersuasionRate "+str(complexStat.persuasionRate));
	#undef str
}

Vector<nite::Hitbox> Game::EntityBase::getHitbox(){
	auto initp = (position + size * nite::Vec2(0.5f)) - (anim.frameSize * nite::Vec2(0.5f));
	auto copy = anim.hitboxes;
	for(int i = 0; i < copy.size(); ++i){
		auto &hb = copy[i];
		hb.position.x = initp.x + (faceDirection == EntityFacing::Right ? hb.position.x : (anim.frameSize.x - hb.position.x) - hb.size.x);
		hb.position.y = initp.y + hb.position.y;
	}
	return copy;
}

void Game::EntityBase::draw(){
	UInt8 bot = EntityState::stateToAnimType[state[EntityStateSlot::BOTTOM]][EntityStateSlot::BOTTOM];
	UInt8 mid = EntityState::stateToAnimType[state[EntityStateSlot::MID]][EntityStateSlot::MID];
	UInt8 anims[AnimPart::total] = {bot, mid, AnimType::TOP_NEUTRAL};
	UInt8 numbs[AnimPart::total] = {stNum[EntityStateSlot::BOTTOM], stNum[EntityStateSlot::MID], 0};
	anim.setState(anims, numbs);
	nite::Vec2 rp = position + size * 0.5f;

    nite::setRenderTarget(nite::RenderTargetGame);
	nite::setColor(1.0f, 1.0f, 1.0f, canDamage() ? 1.0f : 0.80f);
	int bodyDepth = -rp.y - anim.bodyDepthOffset;
	nite::setDepth(bodyDepth);
	float reversed = faceDirection == EntityFacing::Left ? -1.0f : 1.0f;
	auto ref = anim.batch.draw(rp.x, rp.y, anim.frameSize.x * reversed, anim.frameSize.y, 0.5f, 0.5f, 0.0f);

	if(showHitboxes){
		static nite::Texture empty("data/texture/empty.png");
		nite::setColor(0.95f, 0.25f, 04.0f, 0.35f);
		auto hitboxes = getHitbox();
		for(int i = 0; i < hitboxes.size(); ++i){
			auto &hb = hitboxes[i];
			empty.draw(hb.position.x, hb.position.y, hb.size.x, hb.size.y, 0.0f, 0.0f, 0.0f);
		}
	}

	nite::setColor(1.0f, 1.0f, 1.0f, 1.0f);
	nite::setDepth(bodyDepth);
	bool castingSt = state[EntityStateSlot::MID] == EntityState::CASTING && castingMsg.get() != NULL;
	nite::cInterpDiscrete(castingMsgAlpha, castingSt ? 100.0f : 0.0f, 0.30f);
	if(castingMsgAlpha > 0.0f){
		if(!castingMsg->visible){
			castingMsg->setVisible(true);
		}
		auto self = static_cast<nite::PanelUI*>(castingMsg.get());
		auto renderPanel = [&](const nite::Vec2 &offset){
			if(currentCasting.get() != NULL){
				auto sk = skillStat.get(currentCasting->id);
				if(sk != NULL){
					auto cmp = self->getComponentByType("text");
					if(cmp.get() != NULL && cmp->type == "text"){
						static_cast<nite::TextUI*>(cmp.get())->setText(nite::toUpper(sk->name));
					}
				}
			}
			self->recalculate();
			auto cps = self->computeSize();
			nite::Vec2 rp = self->margin * 0.5f + offset; // offset

			// rp = rp + nite::Vec2(nite::randomInt(-2, 2), nite::randomInt(-2, 2));
			float an = 0.0f;

			// Render batch
			nite::setRenderTarget(nite::RenderTargetGame);
			nite::setDepth(bodyDepth);

			nite::setColor(1.0f, 1.0f, 1.0f, castingMsgAlpha / 100.0f);

			auto ref = self->batch.draw(rp.x, rp.y, self->size.x, self->size.y, 0.5f, 0.5f, an);
			// static nite::Shader dummy("data/shaders/ui_ovw_channelingmsg_f.glsl", "data/shaders/ui_ovw_channelingmsg_v.glsl");
			// if(ref != NULL){
			// 	nite::Uniform uni;
			// 	uni.add("p_size", self->size);
			// 	uni.add("p_alpha", 1.0f);
			// 	uni.add("p_cshade", nite::Color(0.90f, 0.10f, 0.10f));
			// 	uni.add("p_cshade", nite::Color(1.0f, 1.0f, 1.0f));
			// 	ref->apply(dummy, uni);
			// }
		};
		renderPanel(rp + nite::Vec2(0.0f, -size.y * 0.55f));
	}
	if(state[EntityStateSlot::MID] != EntityState::CASTING && castingMsg.get() != NULL){
		if(castingMsg->visible){
			castingMsg->setVisible(false);
		}
	}

	nite::cInterpDiscrete(castingBall.alpha, castingSt ? 100.f : 0.0f, 0.35f);
	if(castingBall.alpha > 0.0f){
		castingBall.draw(rp + nite::Vec2(0.0f, -0.35f * size.y)); // TODO: load head position from json
	}
	nite::setDepth(nite::DepthMiddle);
}

void Game::Gfx_CastingBall::draw(const nite::Vec2 &p){
	step += 1 * nite::getDelta();
	// nite::setRenderTarget(nite::RenderTargetGame);
	// nite::setColor(1.0f, 1.0f, 1.0f, 1.0f);
	// float r = 32.0f;
	// for(int i = 0; i < 3; ++i){
	// 	int _an = step + i * 120;
	// 	float an = nite::toRadians(_an);
	// 	float _x = nite::cos(an) * r;
	// 	float _y = nite::sin(an) * r;
	// 	// nite::setDepth(p.y + nite::arctan(_y, _x) * r * 1000);
	// 	_y -= ((_an % 360) / 360.0f) * 8.0f;
	// 	tex.draw(p.x + _x, p.y + _y, 24, 24, 0.5f, 0.5f, 0.0f);
	// }
}

void Game::Gfx_CastingBall::init(const nite::Vec2 &p){
	pos = p;
	step = 0;
	rPos = p;
	rZ = 0.0f;
	z = 0.0f;
	alpha = 0.0f;
	tex.load("data/texture/gfx/casting_ball.png", nite::Color(1.0f, 1.0f, 1.0f, 1.0f));
}


void Game::EntityBase::entityStep(){
	if(healthStat.health == 0 && !healthStat.dead){
		healthStat.dead = true;
		notifyEntityDeath(this);
		onDeath();
	}
	updateStance();
	solveCasting();
	aidriver.update();
}

void Game::EntityBase::setState(UInt8 nstate, UInt8 slot, UInt8 n){
	if(nstate > EntityState::total || slot > EntityStateSlot::total){
		return;
	}
	auto nanim = anim.getAnim(EntityState::stateToAnimType[nstate][slot]);
	if(nanim != NULL && nanim->n > 0 && n > nanim->n){
		n = n % nanim->n;
	}
	bool update = true;
	// if(state[slot] != nstate){
	// 	update = true;
	// }
	state[slot] = nstate;
	lastStateTime[slot] = nite::getTicks();
	lastFrameTime[slot] = nite::getTicks();
	stNum[slot] = n;
	// issue anim update to clients
	if(update && sv != NULL){
		nite::Packet stance;
		stance.setHeader(Game::PacketType::SV_UPDATE_ENTITY_STANCE_STATE);
		stance.write(&this->id, sizeof(this->id));
		stance.write(&faceDirection, sizeof(faceDirection));
		stance.write(&state[EntityStateSlot::BOTTOM], sizeof(UInt8));
		stance.write(&stNum[EntityStateSlot::BOTTOM], sizeof(UInt8));
		stance.write(&state[EntityStateSlot::MID], sizeof(UInt8));
		stance.write(&stNum[EntityStateSlot::MID], sizeof(UInt8));
		// TODO: filter by whether this entity is actually in the clients view
		sv->sendAll(stance);
	}
}

void Game::EntityBase::switchFrame(UInt8 slot, UInt8 n){
	if(slot >= EntityStateSlot::total){
		return;
	}
	lastFrameTime[slot] = nite::getTicks();
	stNum[slot] = n;
}

void Game::EntityBase::throwMelee(float x, float y){
	// server-side only
	if(sv == NULL){
		return;
	}

	container->getQuadrant(position.x - 256,  position.y - 256, 512, 512, locals);
	nite::Hitbox *hb = NULL;
	nite::Hitbox cpy;
	auto initp = (position + size * nite::Vec2(0.5f)) - (anim.frameSize * nite::Vec2(0.5f));
	// no weap
	if(state[EntityStateSlot::MID] == EntityState::MELEE_NOWEAP){
		cpy = anim.meleeNoWeapHb;
		hb = &cpy;
		cpy.position.x = initp.x + (faceDirection == EntityFacing::Right ? cpy.position.x : (anim.frameSize.x - cpy.position.x) - cpy.size.x);
		cpy.position.y = initp.y + cpy.position.y;
	}

	if(hb == NULL){
		return;
	}

	// if(showHitboxes){
	// 	static nite::Texture empty("data/texture/empty.png");
	// 	nite::setRenderTarget(nite::RenderTargetGame);
	// 	nite::setDepth(nite::DepthTop);
	// 	nite::setColor(0.95f, 0.25f, 04.0f, 0.35f);
	// 	empty.draw(hb->position.x, hb->position.y, hb->size.x, hb->size.y, 0.0f, 0.0f, 0.0f);
	// }

	for(int i = 0; i < locals.size(); ++i){
		if(locals[i]->objType != ObjectType::Entity || locals[i] == this){
			continue;
		}
		auto ent = static_cast<Game::EntityBase*>(locals[i]);
		// TODO: move this from here
		if(nite::getTicks() - ent->lastMeleeHit < 350){
			continue;
		}
		auto ohbs = ent->getHitbox();
		for(int j = 0; j < ohbs.size(); ++j){
			if(hb->collision(ohbs[j])){
				nite::print("detect");
				ent->lastMeleeHit = nite::getTicks();
				break;
			}
		}
	}
}

void Game::EntityBase::updateStance(){
	// this is for humanoid basically
	for(int i = 0; i < EntityStateSlot::total; ++i){
		auto &part = i;
		switch(state[i]){
            case EntityState::IDLE: {
			} break;
            case EntityState::WALKING: {
				if(part != EntityStateSlot::BOTTOM){
					break;
				}
				auto canim = this->anim.getAnim(EntityState::stateToAnimType[EntityState::WALKING][EntityStateSlot::BOTTOM]);
				if(canim == NULL){
					break;
				}
				UInt64 walkRateDiff = complexStat.walkRate * 8;
				UInt64 walkAnimTime = canim->spd - (walkRateDiff > walkAnimTime ? ((UInt64)canim->spd*0.05f) : walkRateDiff);
				UInt64 currentTime = nite::getTicks()-lastStateTime[EntityStateSlot::BOTTOM];
				if(!isMoving && currentTime > walkAnimTime){
					setState(EntityState::IDLE, EntityStateSlot::BOTTOM, 0);
				}else
				if(currentTime > walkAnimTime){
					setState(EntityState::WALKING, EntityStateSlot::BOTTOM, stNum[EntityStateSlot::BOTTOM] + 1);
				}
			} break;
			case EntityState::JUMPING: {
			} break;
			case EntityState::SHOOTING_HANDGUN: {
			} break;
			case EntityState::SHOOTING_BOW: {
			} break;
			case EntityState::IDLE_HANDGUN: {
			} break;
			case EntityState::IDLE_BOW: {
			} break;
			case EntityState::IDLE_SWORD: {
			} break;
            case EntityState::IDLE_KNIFE: {
			} break;
            case EntityState::WAVING_KNIFE: {
			} break;
			case EntityState::WAVING_SWORD: {
			} break;
            case EntityState::MELEE_NOWEAP: {
				if(part != EntityStateSlot::MID){
					break;
				}
				auto canim = this->anim.getAnim(EntityState::stateToAnimType[EntityState::MELEE_NOWEAP][EntityStateSlot::MID]);
				switch(stNum[EntityStateSlot::MID]){
					case 0:{
						if(nite::getTicks()-lastStateTime[EntityStateSlot::MID] > canim->spd){
							switchFrame(EntityStateSlot::MID, stNum[EntityStateSlot::MID] + 1);
						}
					} break;
					default:
					case 1: {
						throwMelee(0.0f, 0.0f);
						if(nite::getTicks()-lastStateTime[EntityStateSlot::MID] > 400){ // 200 hard coded for now
							setState(EntityState::IDLE_FIST, EntityStateSlot::MID, 0);
						}
					} break;
				}
			} break;
			case EntityState::IDLE_FIST: {
				if(nite::getTicks()-lastStateTime[EntityStateSlot::MID] > 1500){
					setState(EntityState::IDLE, EntityStateSlot::MID, 0);
				}
			} break;
			case EntityState::CASTING: {
				if(nite::getTicks()-lastStateTime[EntityStateSlot::MID] > 500 && !isCasting){
					setState(EntityState::IDLE, EntityStateSlot::MID, 0);
				}
			} break;
		}
	}
	isMoving = false;
}

void Game::EntityBase::invokeUse(UInt16 targetId, UInt8 type, UInt32 id, float x, float y){
	if(this->sv == NULL || currentCasting.get() != NULL){
		return;
	}
	switch(type){
		case ActionableType::Skill: {
			auto sk = skillStat.get(id);
			if(sk != NULL && sk->isReady(this)){
				currentCasting = Shared<Game::EntityCasting>(new Game::EntityCasting());
				currentCasting->id = id;
				currentCasting->type = type;
				currentCasting->p.set(x, y);
				currentCasting->target = targetId;
				currentCasting->startTime = nite::getTicks();
				currentCasting->time = sk->castDelay;
				if(sk->castDelay > 0){
					setState(EntityState::CASTING, EntityStateSlot::MID, 0);
					isCasting = true;
				}
				auto cl = sv->getClientByEntityId(this->id);
				if(cl != NULL){
					nite::Packet update(++cl->svOrder);
					update.setHeader(Game::PacketType::SV_UPDATE_ENTITY_SET_CASTING_STATE);
					update.write(&this->id, sizeof(this->id));
					update.write(&id, sizeof(id));
					update.write(&type, sizeof(type));
					update.write(&targetId, sizeof(targetId));
					update.write(&currentCasting->startTime, sizeof(currentCasting->startTime));
					update.write(&sk->castDelay, sizeof(sk->castDelay));
					update.write(&x, sizeof(x));
					update.write(&y, sizeof(y));
					sv->persSend(cl->cl, update, 1000, -1);
				}
			}
		} break;
		case ActionableType::Item: {
			nite::print("USE ITEM TODO");
		} break;
	}
	solveCasting();
}

void Game::EntityBase::solveCasting(){
	if(this->sv == NULL || currentCasting.get() == NULL){
		return;
	}
	if(nite::getTicks()-currentCasting->startTime < currentCasting->time){
		return;
	}
	auto sendSkillState = [&](Game::Skill *sk){
		auto cl = sv->getClientByEntityId(this->id);
		if(cl == NULL){
			return;
		}
		nite::Packet update(++cl->svOrder);
		update.setHeader(Game::PacketType::SV_UPDATE_SKILL_STATE);
		update.write(&this->id, sizeof(UInt16));
		update.write(&sk->id, sizeof(UInt16));
		sk->writeUpdate(update);
		sv->persSend(cl->cl, update, 1000, -1);
	};
	auto target = this->sv->getEntity(currentCasting->target);
	switch(currentCasting->type){
		case ActionableType::Item: {

		} break;
		case ActionableType::Skill: {
			auto sk = skillStat.get(currentCasting->id);
			if(sk->use(this, target, nite::Vec2(currentCasting->p.x, currentCasting->p.y))){
				sendSkillState(sk);
			}
		} break;
	}
	isCasting = false;
	currentCasting = Shared<Game::EntityCasting>(NULL);
}

void Game::EntityBase::recalculateStats(){

	if(this->healthStat.dead){
		return;
	}

	baseStat.resetAdd();
	resetComplexStats();

	// recalculate effects-given stats
	auto &eff = effectStat.effects;
	for(auto &ef : eff){
		ef.second->onRecalculateStat(this);
	}

	// recalculate passive-items-given stats
	auto &carry = invStat.carry;
	for(auto &item : carry){
		item.second->onRecalculateStat(this);
	}

	// recalculate passive-skill-given stats
	auto &skills = skillStat.skills;
	for(auto &sk : carry){
		sk.second->onRecalculateStat(this);
	}

	// recalculate local stats
	recalculateBaseStats();
	recalculateHealthStats();
	recalculateComplexStats();

	// normalize in case of overflows
	normalizeComplexStats();

	// server-side only (notify client owner)
	if(sv != NULL){
		auto cl = sv->getClientByEntityId(this->id);
		if(cl == NULL){
			return;
		}
		nite::Packet notify(++cl->svOrder);
		notify.setHeader(Game::PacketType::SV_UPDATE_ENTITY_ALL_STAT);
		notify.write(&id, sizeof(id));
		writeAllStatState(notify);
		sv->persSend(cl->cl, notify, 750, -1);
	}

}

void Game::EntityBase::loadAnim(){
	anim.load("data/anim/anim_humanoid.json");
	this->size.set(anim.maskSize);
	// server cannot load textures (headless doesnt' run opengl)
	if(sv == NULL){
		anim.anim.load(anim.source.path, anim.transparency);
		castingMsg = nite::UI::build("data/ui/overworld/entity_casting_messagebox.json");
		castingMsg->onCreate();
		castingMsg->setVisible(false);
		castingMsgAlpha = 0.0f;
		castingBall.init(position);
	}
}

void Game::EntityBase::markDamaged(){
	lastDmgd = nite::getTicks();
}

bool Game::EntityBase::damage(const Game::DamageInfo &dmg){
	if(this->healthStat.dead || !canDamage() || this->sv == NULL){
		return false;
	}
	markDamaged();
	Int32 def = 0, mdef = 0;
	auto item = invStat.slots[Game::EquipSlot::Chest];
	Game::ItemBase *armor = item.get() != NULL ? static_cast<Game::EquipItem*>(item.get()) : NULL;
	auto efVal = Game::Element::isEffective(dmg.elmnt, armor == NULL ? Game::Element::Neutral : armor->elemnt); // entities are neutral by default
	Int32 dmgdone = dmg.amnt * efVal * (dmg.isCrit ? 2.25f : 1.0f); // damage cannot be negative at the end
	for(int i = 0; i < EquipSlot::TOTAL; ++i){
		auto item = invStat.slots[i];
		if(item.get() == NULL || i == EquipSlot::LeftAcc || i == EquipSlot::RightAcc) continue;
		auto equip = static_cast<Game::EquipItem*>(item.get());
		def += equip->def;
		mdef += equip->mdef;
		if(!dmg.truedmg){ // truedmg ignores damage buffs from weaps and items
			dmgdone = equip->onDamageRecv(dmgdone, dmg);
		}
	}
	switch(dmg.dmgtype){
		case Game::DamageType::Magical: {
			dmgdone -= mdef;
		} break;
		case Game::DamageType::Physical: {
			dmgdone -= def;
		} break;
		case Game::DamageType::Ranged: {
			dmgdone = dmgdone * 0.90f - def; // ranged attacks are always 90% of the real value
		} break;
        default: {
            dmgdone = 0;
            nite::print("entity '"+nite::toStr(this->id)+"' received damage of unknown type");
        } break;
	}
	auto &health = this->healthStat.health;
	auto orig = health;
	if(dmgdone > 0){
		if(dmgdone > health){
			health = 0;
		}else{
			health -= dmgdone;
		}
	}
    nite::print("dealt "+nite::toStr(dmgdone)+" DMG to entity '"+nite::toStr(this->id)+"'");
	// server-side only
	if(orig != health && sv != NULL){
		// send new health
		nite::Packet notify;
		notify.setHeader(Game::PacketType::SV_UPDATE_ENTITY_HEALTH_STAT);
		notify.write(&id, sizeof(id));
		writeHealthStatState(notify);
		sv->persSendAll(notify, 750, -1);
		// send damage notification
		nite::Packet notifydmg;
		notifydmg.setHeader(Game::PacketType::SV_NOTIFY_ENTITY_DAMAGE);
		notifydmg.write(&id, sizeof(id));
		notifydmg.write(&dmgdone, sizeof(dmgdone));
		sv->sendAll(notifydmg);
	}
	return dmgdone > 0;
}

void Game::EntityBase::writeAllStatState(nite::Packet &packet){
	packet.write(&healthStat, sizeof(healthStat));
	packet.write(&baseStat, sizeof(baseStat));
	packet.write(&complexStat, sizeof(complexStat));
}

void Game::EntityBase::readAllStatState(nite::Packet &packet){
	packet.read(&healthStat, sizeof(healthStat));
	packet.read(&baseStat, sizeof(baseStat));
	packet.read(&complexStat, sizeof(complexStat));
}

void Game::EntityBase::writeHealthStatState(nite::Packet &packet){
	packet.write(&healthStat, sizeof(healthStat));
}

void Game::EntityBase::readHealthStatState(nite::Packet &packet){
	packet.read(&healthStat, sizeof(healthStat));
}

void Game::EntityBase::writeInitialState(nite::Packet &packet){
	writeAllStatState(packet);
}

void Game::EntityBase::readInitialState(nite::Packet &packet){
	readAllStatState(packet);
}
