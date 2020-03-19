#include "Inventory.hpp"
#include "../Engine/Tools/Tools.hpp"
#include "Base.hpp"

Game::InventoryStat::InventoryStat(){
	seedIndex = nite::randomInt(5, 35);
	for(int i = 0; i < EquipSlot::TOTAL; ++i){
		slots[i] = Shared<Game::EquipItem>(NULL);
	}
}

UInt16 Game::InventoryStat::add(Shared<Game::ItemBase> &item, UInt16 slotId){
	if(item.get() == NULL){
		return 0;
	}
	int _seedIndex = slotId == 0 ?  ++seedIndex : slotId;
	int finalId = 0;
	int id = item->id;
	auto ins = get(_seedIndex); // check if this item is already in the carry
	// if(item->weight * item->qty + owner->complexStat.carry > owner->complexStat.maxCarry){ // too heavy
	// 	return finalId;
	// }
	if(ins.get() == NULL || !item->amnt){
		carry[_seedIndex] = item;	
		item->slotId = _seedIndex;
		item->onCarryAdd(owner);
		finalId = _seedIndex;
		owner->complexStat.carry += item->weight;
		owner->recalculateStats();
	}else{
		ins->qty += item->qty;
		finalId = ins->slotId;
		owner->complexStat.carry += ins->weight * item->qty;
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
	nite::print("equipped "+item->name);
	return true;
}

bool Game::InventoryStat::unequip(UInt16 itemId){
	for(int i = 0; i < EquipSlot::TOTAL; ++i){
		for(auto &it : carry){
			if(it.second->id == itemId){
				auto equip = static_cast<Game::EquipItem*>(it.second.get());
				equip->onUnequip(owner);				
				slots[equip->equipType] = Shared<Game::EquipItem*>(NULL);
				return true;
			}
		}
	}
	nite::print("failure: no item with such id to unequip");
	return false;
}

UInt16 Game::InventoryStat::add(Shared<Game::ItemBase> &item){
	add(item, 0);
}

Shared<Game::ItemBase> Game::InventoryStat::get(UInt16 slotId){
	auto it = carry.find(slotId);
	return it == carry.end() ? Shared<Game::ItemBase>(NULL) : carry[it->first];
}

bool Game::InventoryStat::contains(UInt16 id){
    return carry.find(id) != carry.end();
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
            if(qty == 0 || item.qty < 0){
                item.onCarryRemove(owner);
				owner->complexStat.carry -= item.weight * item.qty;
                item.slotId = 0;
                carry.erase(it.first);             
				owner->recalculateStats();   
            }              
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
	if(qty == 0 || item->qty < 0){
		item->onCarryRemove(owner);
		owner->complexStat.carry -= item->weight * item->qty;
		item->slotId = 0;
		carry.erase(it->first);             
		owner->recalculateStats();   
	} 	
	return true;
}


Shared<Game::ItemBase> Game::getItem(UInt16 id, UInt16 qty){
	switch(id){
		case Game::ItemList::U_APPLE: {
			auto r = Shared<Game::ItemBase>(new Items::HealthPotion());
			r->qty = qty;
			return r;
		} break;
		default:
			return Shared<Game::ItemBase>(NULL);
	}
}