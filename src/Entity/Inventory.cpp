#include "Inventory.hpp"
#include "../Engine/Tools/Tools.hpp"

Game::InventoryStat::InventoryStat(){
	seedIndex = nite::randomInt(5, 35);
}

UInt16 Game::InventoryStat::add(Shared<Game::BaseItem> &item){
	if(item.get() == NULL){
		return 0;
	}
	seedIndex += 1;
	int finalId = -1;
	int id = item->id;
	auto ins = get(item->id); // check if this item is already in the carry
	if(ins.get() == NULL || !item->amnt){
		carry[seedIndex] = item;	
		item->slotId = seedIndex;
		item->onCarryAdd();
		finalId = seedIndex;
	}else{
		ins->qty += item->qty;
		finalId = ins->slotId;
	}
	return finalId;
}

Shared<Game::BaseItem> Game::InventoryStat::get(UInt16 itemId){
	for (auto &pair : carry) {
		if(pair.second->id == itemId){
			return carry[itemId];
		}
	}	
	return Shared<Game::BaseItem>(NULL);
}

bool Game::InventoryStat::contains(UInt16 id){
    return carry.find(id) != carry.end();
}

void Game::InventoryStat::clear(){
    for(auto &it : carry){
        remove(it.second->id, 0); // 0 = means ALL
    }
}

bool Game::InventoryStat::remove(UInt16 id, UInt16 amnt){
	if(!contains(id)){
		return false;
	}
	for(auto &it : carry){
        auto &item = *it.second.get();
        if(item.id == id){
            if(item.amnt){
                item.qty -= amnt;
            }
            if(amnt == 0 || item.qty < 0){
                item.onCarryRemove();
                item.slotId = 0;
                carry.erase(it.first);                
            }              
        }
    }
    return true;
}
