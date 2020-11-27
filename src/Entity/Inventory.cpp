#include "../Core/Client.hpp"
#include "../Core/Server.hpp"

#include "Inventory.hpp"
#include "../Engine/Tools/Tools.hpp"
#include "Base.hpp"

static void notifyAddItem(Game::EntityBase *ent, UInt16 itemId, UInt16 slotId, UInt16 qty){
    if(ent != NULL && ent->sv != NULL){
        if(auto cl = ent->sv->getClientByEntityId(ent->id)){
            ent->sv->notifyAddItem(cl->clientId, itemId, slotId, qty);
        }
    }
}

static void notifyRemoveItem(Game::EntityBase *ent, UInt16 itemId, UInt16 slotId, UInt16 qty){
    if(ent != NULL && ent->sv != NULL){
        if(auto cl = ent->sv->getClientByEntityId(ent->id)){
            ent->sv->notifyRemoveItem(cl->clientId, itemId, slotId, qty);
        }
    }
}

static void notifyEquipItem(Game::EntityBase *ent, UInt16 itemId, UInt16 slotId){
    if(ent != NULL && ent->sv != NULL){
        if(auto cl = ent->sv->getClientByEntityId(ent->id)){
            ent->sv->notifyEquipItem(cl->clientId, itemId, slotId);
        }
    }
}

static void notifyUnequipItem(Game::EntityBase *ent, UInt16 itemId, UInt16 slotId){
    if(ent != NULL && ent->sv != NULL){
        if(auto cl = ent->sv->getClientByEntityId(ent->id)){
            ent->sv->notifyUnequipItem(cl->clientId, itemId, slotId);
        }
    }
}

Game::InventoryStat::InventoryStat(){
	seedIndex = nite::randomInt(5, 35);
	for(int i = 0; i < EquipSlot::TOTAL; ++i){
		slots[i] = Shared<Game::EquipItem>(NULL);
	}
}

static Jzon::Node db = Jzon::object();

void Game::DBLoadInventory(const String &path){
    String msg = "failed to load Inventory Db '"+path+"': ";
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
    nite::print("loaded Inventory Db '"+path+"': added "+nite::toStr(root.getCount()-1)+" object(s)");
}

bool Game::EquipAnim::load(const String &path){
	String errmsg = "failed to load '"+path+"': ";
	if(!nite::fileExists(path)){
		nite::print(errmsg+"it doesn't exist");
		return false;
	}
    Jzon::Parser parser;
    Jzon::Node node = parser.parseFile(path);
    if(!node.isValid()){
        nite::print(errmsg+"invalid json: "+parser.getError());
        return false;
    }	
	auto indexer = nite::getIndexer();
	frameSize.set(node.get("frameSize").get("w").toInt(128), node.get("frameSize").get("h").toInt(192));
	String hash = node.get("source").toString("");
	auto ifile = indexer->get(hash);
    if(ifile == NULL){
        nite::print(errmsg+"source file '"+hash+"' was not found");
        return false;
    }
    if(ifile != NULL && !ifile->isIt("spritesheet")){
        nite::print(errmsg+"source file is not a spritesheet");
        return false;
    }		
	this->source = *ifile;
	this->texture.load(this->source.path);
	this->textureSize = nite::Vec2(node.get("textureSize").get("w").toInt(0), node.get("textureSize").get("h").toInt(0));
	this->origin = nite::Vec2(node.get("origin").get("x").toInt(0), node.get("origin").get("y").toInt(0));
	for(auto &it : node.get("frames")){
		auto frame = it.second;
		this->inTexCoors.push_back(nite::Vec2(frame.get("x").toInt(0), frame.get("y").toInt(0)));
	}
	for(auto &it : node.get("modes")){
		this->modes[it.first] = it.second.get("n").toInt(0);;
	}
	return true;
}

void Game::ItemBase::parse(Jzon::Node &obj){
	// we won't be doing sanity checks here (maybe TODO)
	name = obj.get("name").toString("Undefined");
	desc = obj.get("desc").toString("Undefined");
	type = obj.get("type").toInt(0);
	weight = obj.get("weight").toInt(0);
	elemnt = obj.get("element").toInt(0);
	iconId = obj.get("iconId").toInt(0);
	amnt = obj.get("amnt").toBool(false);
}

UInt16 Game::InventoryStat::add(Shared<Game::ItemBase> &item, UInt16 slotId){
	if(item.get() == NULL){
		return 0;
	}
	int _seedIndex = slotId == 0 ?  ++seedIndex : slotId;
	int finalId = 0;
	int id = item->id;
	auto ins = getById(item->id); // check if this item is already in the carry
	// if(item->weight * item->qty + owner->complexStat.carry > owner->complexStat.maxCarry){ // too heavy
	// 	return finalId;
	// }
	if(ins.get() == NULL || !item->amnt){
		carry[_seedIndex] = item;
		item->slotId = _seedIndex;
		item->onCarryAdd(owner);
		finalId = _seedIndex;
		owner->complexStat.carry += item->weight;
		notifyAddItem(owner, item->id, item->slotId, item->qty);
	}else{
		ins->qty += item->qty;
		finalId = ins->slotId;
		owner->complexStat.carry += ins->weight * item->qty;
		notifyAddItem(owner, ins->id, ins->slotId, item->qty);
	}
	if(finalId > 0){
		owner->recalculateStats();
	}
	return finalId;
}


bool Game::InventoryStat::equip(Shared<Game::ItemBase> &item){
	auto it = carry.find(item->slotId);
	if(it == carry.end()){
		nite::print("failure: cannot equip an item outside of the carry");
		return false;
	}
	if(item->type != ItemType::Equip){
		nite::print("failure: cannot equip a non-equip item");
		return false;
	}
	auto equip = static_cast<Game::EquipItem*>(item.get());
	slots[equip->equipType] = item;
	equip->onEquip(owner);
	owner->recalculateStats();
	nite::print("equipped "+item->name);
	notifyEquipItem(this->owner, item->id, item->slotId);
	return true;
}

bool Game::InventoryStat::unequip(UInt16 itemId){
	for(int i = 0; i < EquipSlot::TOTAL; ++i){
		for(auto &it : carry){
			if(it.second->id == itemId){
				auto equip = static_cast<Game::EquipItem*>(it.second.get());
				equip->onUnequip(owner);
				slots[equip->equipType] = Shared<Game::EquipItem>(NULL);
				owner->recalculateStats();
				notifyEquipItem(this->owner, equip->id, equip->slotId);
				return true;
			}
		}
	}
	nite::print("failure: no item with such slotId '"+nite::toStr(itemId)+"' to unequip");
	return false;
}

UInt16 Game::InventoryStat::add(Shared<Game::ItemBase> &item){
	return add(item, 0);
}

Shared<Game::ItemBase> Game::InventoryStat::getById(UInt16 id){
	for(auto &item : carry){
		if(item.second->id == id){
			return item.second;
		}
	}
	return Shared<Game::ItemBase>(NULL);
}

Shared<Game::ItemBase> Game::InventoryStat::get(UInt16 slotId){
	auto it = carry.find(slotId);
	return it == carry.end() ? Shared<Game::ItemBase>(NULL) : carry[it->first];
}

bool Game::InventoryStat::contains(UInt16 id){
    for(auto &it : carry){
		if(it.second->id == id){
			return true;
		}
	}
	return false;
}

void Game::InventoryStat::clear(){
    for(auto &it : carry){
        remove(it.second->id, 0); // 0 = means ALL
    }
}

bool Game::InventoryStat::remove(UInt16 id, UInt16 qty){
	if(!contains(id)){
		return false;
	}
	for(auto &it : carry){
        auto &item = *it.second.get();
        if(item.id == id){
            if(item.amnt){
                item.qty -= qty;
            }
			int origSlot = item.slotId;
            if(qty == 0 || item.qty <= 0){
                item.onCarryRemove(owner);
				owner->complexStat.carry -= item.weight * item.qty;
                item.slotId = 0;
                carry.erase(it.first);
			}
			notifyRemoveItem(owner, id, origSlot, qty);
			owner->recalculateStats();
			return true;
        }
    }
    return true;
}

bool Game::InventoryStat::removeBySlotId(UInt16 slotId, UInt16 qty){

	auto it = carry.find(slotId);
	if(it == carry.end()){
		return false;
	}
	auto item = carry[it->first];
	if(item->amnt){
		item->qty -= qty;
	}

	if(qty == 0 || item->qty <= 0){
		item->onCarryRemove(owner);
		owner->complexStat.carry -= item->weight * item->qty;
		item->slotId = 0;
		carry.erase(it->first);
	}
	notifyRemoveItem(owner, item->id, slotId, qty);
	owner->recalculateStats();
	return true;
}


Shared<Game::ItemBase> Game::getItem(UInt16 id, UInt16 qty){
	auto dbInfo = db.get(nite::toStr(id));
	auto it = Shared<Game::ItemBase>(NULL);
	switch(id){
		case Game::ItemList::U_APPLE: {
			it = Shared<Game::ItemBase>(new Items::HealthPotion());
		} break;
		case Game::ItemList::W_BOW: {
			it = Shared<Game::ItemBase>(new Items::BowWeapon());
		} break;		
	}
	if(it.get() != NULL){
		it->qty = qty;
		if(db.has(nite::toStr(id))){
			it->parse(dbInfo);
			it->parseSpecial(dbInfo);
		}
	}
	return it;
}

void Game::EquipItem::parseSpecial(Jzon::Node &obj){
	equipType = obj.get("equipType").toInt(0);
	dmg = obj.get("dmg").toInt(0);
	mdmg = obj.get("mdmg").toInt(0);
	def = obj.get("def").toInt(0);
	mdef = obj.get("mdef").toInt(0);
}

void Game::EquipWeapon::parseSpecial(Jzon::Node &obj){
	equipType = obj.get("equipType").toInt(0);
	weaponType = obj.get("weaponType").toInt(0);
	dmg = obj.get("dmg").toInt(0);
	mdmg = obj.get("mdmg").toInt(0);
	def = obj.get("def").toInt(0);
	mdef = obj.get("mdef").toInt(0);
	this->anim.load("data/anim/"+obj.get("anim").toString("weap_bow.json"));
}
