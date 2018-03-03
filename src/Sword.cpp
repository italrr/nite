#include "Sword.hpp"

void Game::Sword::loadSpecific(const String &path){
	Jzon::Parser parser;
	Jzon::Node node = parser.parseFile(path);
	if (!node.isValid()){
	   nite::print("Failed to read file "+path+": '"+parser.getError()+"'");
	   return;
	}
	equipType = node.get("equipType").toInt();
	damage = node.get("damage").toFloat();
	defense = node.get("defense").toFloat();
	lifespan = node.get("lifespan").toFloat();
	handling = node.get("handling").toFloat();
	origin.set(node.get("origin").get("x").toFloat(), node.get("origin").get("y").toFloat());
	equipAnim.load(node.get("spriteFilename").toString(), nite::Color(1.0f, 1.0f, 1.0f, 1.0f));		
	// load animations
	auto objAnimNormal = node.get("animations").get("normal");
	animNormal = equipAnim.add(spriteSize, objAnimNormal);
}