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

bool nite::Hitbox::collision(const Hitbox &box){
	return ((position.x + size.x * 0.5 > box.position.x-box.size.x * 0.5 &&
		position.x - size.x * 0.5 < box.position.x+box.size.x * 0.5) &&
		(position.y + size.y * 0.5 > box.position.y-box.size.y * 0.5 &&
		position.y - size.y * 0.5 < box.position.y+box.size.y * 0.5));
}
