#include <string.h>
#include <limits>

#include "../Engine/Shapes.hpp"
#include "../Engine/Tools/Tools.hpp"
#include "../Engine/Graphics.hpp"
#include "World.hpp"
#include "Network.hpp"


UInt16 Game::NetWorld::generateId(){
	return ++seedId; // networld ids are never to be 0. so a response with 0 means error
}

void Game::NetWorld::setSize(int w, int h, int unitsize, float gridSpec){
	if(w <= 0.0f || h <= 0.0f || unitsize <= 0){
		return;
	}
	if(cells != NULL){
		delete cells;
	}
	if(ghosts != NULL){
		delete ghosts;
	}
	this->gridSpec = nite::Vec2(gridSpec);
	this->size.set(w, h);
	this->cusize = unitsize;
	this->cwidth = w / this->cusize;
	this->cheight = h / this->cusize;
	this->ctotal = this->cwidth * this->cheight;
	this->cells = new Game::NetObject*[this->ctotal];
	this->ghosts = new Game::NetObject*[this->ctotal];
	for(int i = 0; i < this->ctotal; ++i){
		this->cells[i] = NULL;
		this->ghosts[i] = NULL;
	}
	nite::print("set world size "+nite::toStr(this->cwidth)+"x"+nite::toStr(this->cheight)+"("+nite::toStr(unitsize)+") | cells "+nite::toStr(this->ctotal));
}

void Game::NetWorld::getQuadrant(int x, int y, int w, int h, Vector<Game::NetObject*> &quadrant){
	if(cells == NULL || ghosts == NULL){
		return;
	}
	x /= cusize;
	y /= cusize;
	w /= cusize;
	h /= cusize;
	for(int _x = 0; _x < w; ++_x){
		for(int _y = 0; _y < h; ++_y){
			int rx = x + _x;
			int ry = y + _y;
			int ind = rx + ry * cwidth;
			if(ind < 0 || ind >= ctotal){
				continue;
			}
			auto c = cells[ind];
			auto g = ghosts[ind];
			if(c != NULL){
				quadrant.push_back(c);
			}
			if(g != NULL){
				quadrant.push_back(g);
			}

		}
	}
	return;
}

Game::NetWorld::~NetWorld(){
	clearWallMasks();
	clearGhostMasks();
	if(cells != NULL){
		delete cells;
	}
	if(ghosts != NULL){
		delete ghosts;
	}
}

Game::NetWorld::NetWorld(){
	timescale = 1.0f;
	debugPhysics = false;
	cells = NULL;
	ghosts = NULL;
	seedNId = -1;
	seedId = nite::randomInt(25, 50);
	snapshotOrder = 0;
	tickrate = 1000 / 33;
	int reserve = 1000 * 30;
	objects.reserve(reserve);
	nite::print("networld reserved "+nite::toStr(sizeof(Shared<Game::NetObject>) * reserve)+" bytes");
}

bool Game::NetWorld::exists(UInt16 id){
	return id != -1 ? objects.find(id) != objects.end() : false;
}

UInt16 Game::NetWorld::add(Shared<Game::NetObject> &obj, int useId){
	int id = useId == -1 ? generateId() : useId;
	objects[id] = obj;
	obj->id = id;
	obj->container = this;
	obj->onCreate();
	obj->updateQuadrant();
	return id;
}

void Game::NetWorld::clearWallMasks(){
	for(int i = 0; i < wallMasks.size(); ++i){
		wallMasks[i]->clearQuadrant();
		delete wallMasks[i];
	}
	wallMasks.clear();
}

bool Game::NetWorld::removeWallMask(Game::NetObject *mask){
	for(int i = 0; i < wallMasks.size(); ++i){
		if(wallMasks[i] == mask){
			wallMasks[i]->clearQuadrant();
			delete mask;
			wallMasks.erase(wallMasks.begin() + i);
			return true;
		}
	}
	return false;
}

int Game::NetWorld::addWallMask(Game::NetObject *mask){
	mask->localId = --seedNId;
	mask->id = 0;
	mask->container = this;
	mask->onCreate();
	mask->updateQuadrant();
	wallMasks.push_back(mask);
	return mask->localId;
}


void Game::NetWorld::clearGhostMasks(){
	for(int i = 0; i < ghostMasks.size(); ++i){
		ghostMasks[i]->clearQuadrant();
		delete ghostMasks[i];
	}
	ghostMasks.clear();
}

int Game::NetWorld::addGhostMask(Game::NetObject *mask){
	mask->localId = --seedNId;
	mask->id = 0;
	mask->container = this;
	mask->onCreate();
	mask->updateQuadrant();
	ghostMasks.push_back(mask);
	return mask->localId;
}

bool Game::NetWorld::removeGhostMask(Game::NetObject *mask){
	for(int i = 0; i < ghostMasks.size(); ++i){
		if(ghostMasks[i] == mask){
			ghostMasks[i]->clearQuadrant();
			delete mask;
			ghostMasks.erase(ghostMasks.begin() + i);
			return true;
		}
	}
	return false;
}


void Game::NetWorld::clear(){
	objects.clear();
	clearWallMasks();
	clearGhostMasks();
}

void Game::NetWorld::remove(int objectId){
	auto search = objects.find(objectId);
	if(search != objects.end()) {
		for(int i = 0; i < removeQueue.size(); ++i){
			if(removeQueue[i] == objectId) return;
		}
		removeQueue.push_back(objectId);
	}else{
		nite::print("[NetWorld] unable to remove object id '"+nite::toStr(objectId)+"': it doesn't exist.");
	}
}

void Game::NetWorld::remove(Game::NetObject *obj){
	this->remove(obj->id);
}

Game::NetObject *Game::NetWorld::get(UInt16 id){
	auto it = objects.find(id);
	if(it == objects.end()){
		return NULL;
	}
	return it->second.get();
}

void Game::NetWorld::step(){
  for (auto& it : objects){
		auto current = it.second;
		current->step();
		current->onStep();
	}
}

bool Game::NetWorld::testCollision(Game::NetObject *a, Game::NetObject *b, const nite::Vec2 &diff, nite::Vec2 &limit, nite::Vec2 &normal){
	bool collision = false;
	normal.x = 0.0f;
	normal.y = 0.0f;
	bool withinTopY = a->position.y <= b->position.y + b->size.y && a->position.y >= b->position.y;
	bool withinBottomY = a->position.y + a->size.y <= b->position.y + a->size.y && a->position.y + a->size.y >= b->position.y;

	bool withinTopX = a->position.x <= b->position.x + b->size.x && a->position.x >= b->position.x;
	bool withinBottomX = a->position.x + a->size.x <= b->position.x + b->size.x && a->position.x + a->size.x >= b->position.x;

	if(diff.x > 0){
		float ray = a->position.x + a->size.x + diff.x;
		if(ray >= b->position.x && (a->position.x + a->size.x) <= b->position.x && (withinTopY || withinBottomY)){
			float lim = (b->position.x - (a->position.x + a->size.x)) / diff.x;
			if(lim < limit.x){
				limit.x = lim;
			}
			normal.x = 1.0f;
			collision = true;
		}
	}else
	if(diff.x < 0){
		float ray = a->position.x + diff.x;
		if(ray <= b->position.x + b->size.x && a->position.x >= b->position.x + b->size.x && (withinTopY || withinBottomY)){
			float lim = (a->position.x - (b->position.x + b->size.x)) / (diff.x * -1.0f);
			if(lim < limit.x){
				limit.x = lim;
			}			
			normal.x = -1.0f;
			collision = true;
		}
	}

	if(diff.y > 0){
		float ray = a->position.y + a->size.y + diff.y;
		if(ray >= b->position.y && (a->position.y + a->size.y) <= b->position.y && (withinTopX || withinBottomX)){
			float lim = (b->position.y - (a->position.y + a->size.y)) / diff.y;
			if(lim < limit.y){
				limit.y = lim;
			}			
			normal.y = 1.0f;
			collision = true;
		}
	}else
	if(diff.y < 0){
		float ray = a->position.y + diff.y;
		if(ray <= b->position.y + b->size.y && a->position.y >= b->position.y + b->size.y && (withinTopX || withinBottomX)){
			float lim = (a->position.y - (b->position.y + b->size.y)) / (diff.y * -1.0f);
			if(lim < limit.y){
				limit.y = lim;
			}			
			normal.y = -1.0f;
			collision = true;
		}
	}

	// normalize limit
	// if(limit.x < 0.0f) limit.x = 0.0f;
	// if(limit.x > 1.0f) limit.x = 1.0f;	

	// if(limit.y < 0.0f) limit.y = 0.0f;
	// if(limit.y > 1.0f) limit.y = 1.0f;		

	return collision;
}

void Game::NetWorld::updateObject(Game::NetObject *obj){
	if(obj->unmovable || obj == NULL) return;
	nite::Vec2 origp = obj->position;

	// we're gonna use a fixed size to look for quadrants for now(3000x3000 from center/origin)
	// ideally we should use a size relative to the object's speed
	// TODO: this ^
	
	Vector<Game::NetObject*> nearObjs;
	getQuadrant(obj->position.x - 3000,  obj->position.y - 3000, 6000, 6000, nearObjs);
	float xdiff = obj->speed.x * this->timescale * obj->relativeTimescale * currentTickRate * nite::getTimescale() * 0.067f;
	float ydiff = obj->speed.y * this->timescale * obj->relativeTimescale * currentTickRate * nite::getTimescale() * 0.067f;
	auto diff = nite::Vec2(xdiff, ydiff);
	auto normal = nite::Vec2(0.0f);
	auto limit = nite::Vec2(1.0f);
	Game::NetObject *otherP;
	for(int i = 0; i < nearObjs.size(); ++i){
		auto other = nearObjs[i];
		if(obj->id == other->id) continue;
		if(!other->solid && other->objType != ObjectType::Ghost) continue;
		if(testCollision(obj, other, diff, limit, normal)){
			if(other->objType == ObjectType::Ghost){
				auto ghost = static_cast<Game::GhostMask*>(other);
				ghost->callback(obj);
				limit = nite::Vec2(1.0f);
				continue; // we do not handle collision if it's a ghost object
			}
			obj->collided = true;
			obj->onCollision(other);		
			if(limit.x == 0.0f && limit.y == 0.0f){
				otherP = other;
				break;
			}
		}
	}
	float offset = 1.0f;
	obj->position = obj->position + nite::Vec2(diff.x * limit.x, diff.y * limit.y);
	if(limit.x == 0.0f){
		if(normal.x > 0.0f){
			obj->position.x = otherP->position.x - obj->size.x - offset;
		}else
		if(normal.x < 0.0f){
			obj->position.x = otherP->position.x + otherP->size.x + offset;
		}
	}
	if(limit.y == 0.0f){
		if(normal.y > 0.0f){
			obj->position.y = otherP->position.y - obj->size.y - offset;
		}else
		if(normal.y < 0.0f){
			obj->position.y = otherP->position.y + otherP->size.y + offset;
		}
	}	
	if(origp.x != obj->position.x || origp.y != obj->position.y){
		obj->updateQuadrant();
	}
}

void Game::NetWorld::setVisibleQuadrant(int x, int y, int w, int h){

}

void Game::NetWorld::renderDbug(){
	// debug: shows masks
	if(debugPhysics){
		for (auto it : objects){
			auto current = it.second;
			nite::setDepth(nite::DepthTop);
			nite::setRenderTarget(nite::RenderTargetGame);
			nite::setColor(current->collided ? nite::Color(1.0f, 0, 0) : nite::Color(0, 1.0f, 0));
			nite::Draw::Rectangle(current->position, current->size, false, nite::Vec2(0.0, 0.0), 0);
			nite::setDepth(0);
			nite::resetColor();
		}
		for(int i = 0; i < wallMasks.size(); ++i){
			auto mask = wallMasks[i];
			nite::setDepth(nite::DepthTop);
			nite::setRenderTarget(nite::RenderTargetGame);
			nite::setColor(mask->collided ? nite::Color(1.0f, 0, 0) : nite::Color(0, 1.0f, 0));
			nite::Draw::Rectangle(mask->position, mask->size, false, nite::Vec2(0.0, 0.0), 0);
			nite::setDepth(0);
			nite::resetColor();
		}
		for(int i = 0; i < ghostMasks.size(); ++i){
			auto mask = ghostMasks[i];
			nite::setDepth(nite::DepthTop);
			nite::setRenderTarget(nite::RenderTargetGame);
			nite::setColor(nite::Color(1.0f, 0, 1.0f));
			nite::Draw::Rectangle(mask->position, mask->size, false, nite::Vec2(0.0, 0.0), 0);
			nite::setDepth(0);
			nite::resetColor();
		}
	}
}

void Game::NetWorld::update(){
	renderDbug();

	if(nite::getTicks()-lastTick < tickrate){
		return;
	}

	delta = nite::getTicks() - lastTick;
	lastTick = nite::getTicks();
	currentTickRate = delta;

  	for (auto it : objects){
		auto current = it.second;
		nite::Vec2 lastPos = current->position;
		current->collided = false;
		// movement
		updateObject(it.second.get());
		// friction
		for(int i = 0; i < currentTickRate / tickrate; ++i){
			current->speed.x *= 1.0f - (current->friction * nite::getTimescale() * timescale * current->relativeTimescale);
			current->speed.y *= 1.0f - (current->friction * nite::getTimescale() * timescale * current->relativeTimescale);
		}
		// position changed, send updates to clients (client->side only)
		if(current->position.x != lastPos.x || current->position.y != lastPos.y){
			this->updateQueue[current->id] = current.get();
			current->nextPosition.push_back(current->position);
		}
	}

	++snapshotOrder;
	// clean up
	if(removeQueue.size() > 0){
		for(int i = 0; i < removeQueue.size(); ++i){
			objects[removeQueue[i]]->onDestroy();
			objects[removeQueue[i]]->id = 0;
			objects.erase(removeQueue[i]);
		}
		removeQueue.clear();
	}
}

void Game::NetWorld::render(){
  for (auto& it : objects){
		auto current = it.second;
		current->draw();
		current->onDraw();
	}
}
