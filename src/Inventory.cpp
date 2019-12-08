#include "Inventory.hpp"
#include "Engine/Tools/Tools.hpp"

// Game::BaseItem::BaseItem(){
// 	amountable = false;
// }

Game::Inventory::Inventory(){
	carrySeedIndex = nite::randomInt(350, 750);
	// nite::print(carrySeedIndex);
}

int Game::Inventory::addItem(Shared<Game::BaseItem> &item){
	if(item.get() == NULL){
		return -1;
	}
	carrySeedIndex += 1;
	int finalId = -1;
	int id = item->id;
	auto ins = getItem(item->id); // check if this item is already in the carry
	if(ins.get() == NULL || !item->amountable){
		itemCarry[carrySeedIndex] = item;	
		item->inCarryId = carrySeedIndex;
		item->container = this;
		item->onCarryAdd();
		finalId = carrySeedIndex;
	}else{
		ins->quantity += item->quantity;
		finalId = ins->inCarryId;
	}
	return finalId;
}

void Game::Inventory::setActiveItem(int slotId, Shared<Game::BaseItem> &item){
	int inCarryId = item->inCarryId;
	if(!containsItem(inCarryId)){
		nite::print("entity: can't set active "+nite::toStr(item->id)+" "+item->name+". entity is not carrying it");
		return;
	}
	switch(slotId){
		case Game::InventoryActiveSlot::Main:
		case Game::InventoryActiveSlot::Secondary:
			nite::print("entity: set active "+nite::toStr(item->id)+" "+item->name);
			itemHandling[slotId] = item->inCarryId;
			onActiveItemSwitch(item);
			break;
		default:
			return;
	}
}

void Game::Inventory::setUnactiveItem(int slotId){
	switch(slotId){
		case Game::InventoryActiveSlot::Main: 
		case Game::InventoryActiveSlot::Secondary:
			itemHandling[slotId] = -1;
			break;
		default:
			return;
	}
}

void Game::Inventory::setUnactiveItem(Shared<Game::BaseItem> &item){
	int inCarryId = item->inCarryId;
	if(!containsItem(inCarryId)){
		nite::print("entity: can't unset active "+nite::toStr(item->id)+" "+item->name+". entity is not carrying it");
		return;
	}
	for (auto& pair : itemHandling) {
		if(pair.second == inCarryId){
			itemHandling[pair.first] = -1;
		}
	}	
}

Shared<Game::BaseItem> Game::Inventory::getItem(int itemId){
	for (auto& pair : itemCarry) {
		if(pair.second->id == itemId){
			return itemCarry[itemId];
		}
	}	
	return Shared<Game::BaseItem>(NULL);
}

bool Game::Inventory::containsItem(int id){
	return itemCarry.count(id);
}

bool Game::Inventory::removeItem(int id){
	if(!itemCarry.count(id)){
		return false;
	}
	itemCarry[id]->onCarryRemove();
	itemCarry[id]->inCarryId = -1;
	itemCarry[id]->container = NULL;
	itemCarry.erase(id);
}

void Game::BaseItem::load(const String &path){
	Jzon::Parser parser;
	Jzon::Node node = parser.parseFile(path);
	if (!node.isValid()){
	   nite::print("Failed to read file "+path+": '"+parser.getError()+"'");
	   return;
	}
	// Basic data
	spriteSize.set(node.get("sprieSize").get("width").toFloat(), node.get("sprieSize").get("height").toFloat());	
	frameSize.set(node.get("frameSize").get("width").toFloat(), node.get("frameSize").get("height").toFloat());	
	id = node.get("id").toInt();
	type = node.get("type").toInt();
	name = node.get("name").toString();
	description = node.get("description").toString();
	weight = node.get("weight").toFloat();
	amountable = node.get("amountable").toBool();
	auto hitbox = node.get("hitbox");
	for(int i = 0; i < hitbox.getCount(); ++i){
		hitboxes.push_back(nite::Hitbox(hitbox.get(i)));		
	}
	loadSpecific(path);
}

nite::Hitbox Game::BaseEquip::correctHitbox(nite::Hitbox &hb, float angle, nite::Vec2 &origin){
	auto hitbox = nite::Hitbox();
	hitbox.position = hb.position - origin;
	return hitbox;
}
