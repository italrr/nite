#include <cmath>

#include "Object.hpp"
#include "Types.hpp"
#include "Tools/Tools.hpp"

nite::Hitbox::Hitbox(const Jzon::Node &node){
	load(node);
}

nite::Hitbox::Hitbox(){

}

void nite::Hitbox::load(const Jzon::Node &node){
	position.set(node.get("position").get("x").toFloat(), node.get("position").get("y").toFloat());
	size.set(node.get("size").get("width").toFloat(), node.get("size").get("height").toFloat());
	type = node.get("type").toInt();
	name = node.get("name").toString();
}

bool nite::Hitbox::collision(const Hitbox &b){

	bool withinTopY = this->position.y <= b.position.y + b.size.y && this->position.y >= b.position.y;
	bool withinBottomY = this->position.y + this->size.y <= b.position.y + b.size.y && this->position.y + this->size.y >= b.position.y;
	
	bool withinTopX = this->position.x <= b.position.x + b.size.x && this->position.x >= b.position.x;
	bool withinBottomX = this->position.x + this->size.x <= b.position.x + b.size.x && this->position.x + this->size.x >= b.position.x;

	if(position.x + size.x > b.position.x && position.x < b.position.x + b.size.x && (withinTopY || withinBottomY)){
		return true;
	}
	
	if(position.x < b.position.x + b.size.x && position.x > b.position.x && (withinTopY || withinBottomY)){
		return true;
	}

	if(position.y + size.y > b.position.y && position.y < b.position.y + b.size.y && (withinTopX || withinBottomX)){
		return true;
	}
	if(position.y < b.position.y + b.size.y && position.y > b.position.y && (withinTopX || withinBottomX)){
		return true;
	}


	return false;
}
