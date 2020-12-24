#include <string.h>
#include <limits>
#include <cmath>
#include  <algorithm>

#include "../Engine/Shapes.hpp"
#include "../Engine/Tools/Tools.hpp"
#include "../Engine/Graphics.hpp"
#include "../Engine/View.hpp"

#include "World.hpp"
#include "Network.hpp"
#include "../Entity/Base.hpp"


// UInt16 Game::NetWorld::generateId(){
// 	return ++seedId; // networld ids are never to be 0. so a response with 0 means error
// }

// void Game::NetWorld::setSize(int w, int h, int unitsize, float gridSpec){
// 	if(w <= 0.0f || h <= 0.0f || unitsize <= 0){
// 		return;
// 	}
// 	if(cells != NULL){
// 		delete cells;
// 	}
// 	if(ghosts != NULL){
// 		delete ghosts;
// 	}
// 	this->gridSpec = nite::Vec2(gridSpec);
// 	this->size.set(w, h);
// 	this->cusize = unitsize;
// 	this->cwidth = w / this->cusize;
// 	this->cheight = h / this->cusize;
// 	this->ctotal = this->cwidth * this->cheight;
// 	this->cells = new Game::NetObject*[this->ctotal];
// 	this->ghosts = new Game::NetObject*[this->ctotal];
// 	for(int i = 0; i < this->ctotal; ++i){
// 		this->cells[i] = NULL;
// 		this->ghosts[i] = NULL;
// 	}
// 	nite::print("set world size "+nite::toStr(this->cwidth)+"x"+nite::toStr(this->cheight)+"("+nite::toStr(unitsize)+") | cells "+nite::toStr(this->ctotal));
// }

// void Game::NetWorld::getQuadrant(int x, int y, int w, int h, Vector<Game::NetObject*> &quadrant){
// 	if(cells == NULL || ghosts == NULL){
// 		return;
// 	}
// 	x /= cusize;
// 	y /= cusize;
// 	w /= cusize;
// 	h /= cusize;
// 	for(int _x = 0; _x < w; ++_x){
// 		for(int _y = 0; _y < h; ++_y){
// 			int rx = x + _x;
// 			int ry = y + _y;
// 			int ind = rx + ry * cwidth;
// 			if(ind < 0 || ind >= ctotal){
// 				continue;
// 			}
// 			auto c = cells[ind];
// 			auto g = ghosts[ind];
// 			if(c != NULL){
// 				quadrant.push_back(c);
// 			}
// 			if(g != NULL){
// 				quadrant.push_back(g);
// 			}

// 		}
// 	}
// 	return;
// }

// Game::NetWorld::~NetWorld(){
// 	clear();
// }

// Game::NetWorld::NetWorld(){
// 	timescale = 1.0f;
// 	debugPhysics = false;
// 	cells = NULL;
// 	ghosts = NULL;
// 	seedNId = -1;
// 	seedId = nite::randomInt(101, 151);
// 	tickrate = 1000 / 33;
// 	int reserve = 1000 * 30;
// 	objects.reserve(reserve);
// 	nite::print("networld reserved "+nite::toStr((long long unsigned int)(sizeof(Shared<Game::NetObject>) * reserve))+" bytes");
// }

// bool Game::NetWorld::exists(UInt16 id){
// 	return id != -1 ? objects.find(id) != objects.end() : false;
// }

// UInt16 Game::NetWorld::add(Shared<Game::NetObject> &obj, int useId){
// 	int id = useId == -1 ? generateId() : useId;
// 	objects[id] = obj;
// 	obj->id = id;
// 	obj->container = this;
// 	obj->onCreate();
// 	obj->updateQuadrant();
// 	return id;
// }

// void Game::NetWorld::clearWallMasks(){
// 	for(int i = 0; i < wallMasks.size(); ++i){
// 		wallMasks[i]->clearQuadrant();
// 		delete wallMasks[i];
// 	}
// 	wallMasks.clear();
// }

// bool Game::NetWorld::removeWallMask(Game::NetObject *mask){
// 	for(int i = 0; i < wallMasks.size(); ++i){
// 		if(wallMasks[i] == mask){
// 			wallMasks[i]->clearQuadrant();
// 			delete mask;
// 			wallMasks.erase(wallMasks.begin() + i);
// 			return true;
// 		}
// 	}
// 	return false;
// }

// int Game::NetWorld::addWallMask(Game::NetObject *mask){
// 	mask->localId = --seedNId;
// 	mask->id = 0;
// 	mask->container = this;
// 	mask->onCreate();
// 	mask->updateQuadrant();
// 	wallMasks.push_back(mask);
// 	return mask->localId;
// }


// void Game::NetWorld::clearGhostMasks(){
// 	for(int i = 0; i < ghostMasks.size(); ++i){
// 		ghostMasks[i]->clearQuadrant();
// 		delete ghostMasks[i];
// 	}
// 	ghostMasks.clear();
// }

// int Game::NetWorld::addGhostMask(Game::NetObject *mask){
// 	mask->localId = --seedNId;
// 	mask->id = 0;
// 	mask->container = this;
// 	mask->onCreate();
// 	mask->updateQuadrant();
// 	ghostMasks.push_back(mask);
// 	return mask->localId;
// }

// bool Game::NetWorld::removeGhostMask(Game::NetObject *mask){
// 	for(int i = 0; i < ghostMasks.size(); ++i){
// 		if(ghostMasks[i] == mask){
// 			ghostMasks[i]->clearQuadrant();
// 			delete mask;
// 			ghostMasks.erase(ghostMasks.begin() + i);
// 			return true;
// 		}
// 	}
// 	return false;
// }


// void Game::NetWorld::clear(){
// 	objects.clear();
// 	clearWallMasks();
// 	clearGhostMasks();
// 	updateQueue.clear();
// 	removeQueue.clear();
// 	if(cells != NULL){
// 		delete cells;
// 	}
// 	if(ghosts != NULL){
// 		delete ghosts;
// 	}	
// 	cells = NULL;
// 	ghosts = NULL;
// 	seedNId = -1;
// 	seedId = nite::randomInt(25, 50);
// }

// void Game::NetWorld::remove(int objectId){
// 	auto it = objects.find(objectId);
// 	if(it == objects.end()) {
// 		nite::print("[NetWorld] unable to remove object id '"+nite::toStr(objectId)+"': it doesn't exist.");
// 	}
// 	removeQueue.push_back(objectId);
// }

// void Game::NetWorld::remove(Game::NetObject *obj){
// 	this->remove(obj->id);
// }

// Game::NetObject *Game::NetWorld::get(UInt16 id){
// 	auto it = objects.find(id);
// 	if(it == objects.end()){
// 		return NULL;
// 	}
// 	return it->second.get();
// }

// static nite::Vec2 perp(const nite::Vec2 &in){
// 	return nite::Vec2(in.y, -in.x);
// }

// static Vector<nite::Vec2> getAxes(const nite::Polygon &polygon){
// 	Vector<nite::Vec2> axes(polygon.vert.size(), nite::Vec2(0.0f));
// 	for (int i = 0; i < polygon.vert.size(); i++) {
// 		nite::Vec2 p1 = polygon.vert[i];
// 		nite::Vec2 p2 = polygon.vert[(i + 1) == polygon.vert.size() ? 0 : (i + 1)];
// 		nite::Vec2 edge = p1 - p2;
// 		nite::Vec2 normal = perp(edge);
// 		axes[i] = normal;
// 	}
// 	return axes;
// }

// struct Projection {
// 	float min;
// 	float max;
// };

// struct Penetration {
// 	nite::Vec2 normal;
// 	float depth;
// };

// static bool overlaps(const Projection &p1, const Projection &p2){
// 	return !(p1.min > p2.max || p2.min > p1.max);
// }

// static float getOverlap(const Projection &p1, const Projection &p2){
// 	return overlaps(p1, p2) ? (std::min(p1.max, p2.max) - std::max(p1.min, p2.min)): 0.0f;	
// }

// static Projection project(const nite::Polygon &polygon, nite::Vec2 axis){
// 	float min = axis.dotProduct(polygon.vert[0]);
// 	float max = min;
// 	for (int i = 1; i < polygon.vert.size(); i++) {
// 		double p = axis.dotProduct(polygon.vert[i]);
// 		if(p < min){
// 			min = p;
// 		} else
// 		if(p > max){
// 			max = p;
// 		}
// 	}
// 	Projection proj;
// 	proj.min = min;
// 	proj.max = max;
// 	return proj;	
// }

// static bool test(Game::NetObject *_a, Game::NetObject *_b, Penetration &penetration){
// 	if(_a->body.vert.size() == 0 || _b->body.vert.size() == 0){
// 		return false;
// 	}
// 	auto a = _a->body.rotate(_a->orientation) + _a->position;
// 	auto b = _b->body.rotate(_b->orientation) + _b->position;
// 	Vector<nite::Vec2> aAxes = getAxes(a);
// 	Vector<nite::Vec2> bAxes = getAxes(b);

// 	float overlap = std::numeric_limits<float>::max();
// 	nite::Vec2 smallest = nite::Vec2();
// 	for (int i = 0; i < aAxes.size(); i++) {
// 		nite::Vec2 &axis = aAxes[i];
// 		Projection p1 = project(a, axis);
// 		Projection p2 = project(b, axis);
// 		if (!overlaps(p1, p2)){
// 			return false;
// 		}else{
// 			float o = getOverlap(p1, p2);
// 			if(o < overlap){
// 				overlap = o;
// 				smallest = axis;
// 			}
// 		}
// 	}
// 	for (int i = 0; i < bAxes.size(); i++) {
// 		nite::Vec2 &axis = bAxes[i];
// 		Projection p1 = project(a, axis);
// 		Projection p2 = project(b, axis);
// 		if (!overlaps(p1, p2)) {
// 			return false;
// 		}else{
// 			float o = getOverlap(p1, p2);
// 			if (o < overlap) {
// 				overlap = o;
// 				smallest = axis;
// 			}
// 		}
// 	}
// 	penetration.normal = smallest;
// 	penetration.depth = overlap;
// 	return true;
// }

// bool Game::NetWorld::testCollision(Game::NetObject *a, Game::NetObject *b, const nite::Vec2 &diff, nite::Vec2 &limit, nite::Vec2 &normal){
// 	bool collision = false;
// 	normal.x = 0.0f;
// 	normal.y = 0.0f;
// 	bool withinTopY = a->position.y <= b->position.y + b->size.y && a->position.y >= b->position.y;
// 	bool withinBottomY = a->position.y + a->size.y <= b->position.y + a->size.y && a->position.y + a->size.y >= b->position.y;

// 	bool withinTopX = a->position.x <= b->position.x + b->size.x && a->position.x >= b->position.x;
// 	bool withinBottomX = a->position.x + a->size.x <= b->position.x + b->size.x && a->position.x + a->size.x >= b->position.x;

// 	if(diff.x > 0){
// 		float ray = a->position.x + a->size.x + diff.x;
// 		if(ray >= b->position.x && (a->position.x + a->size.x) <= b->position.x && (withinTopY || withinBottomY)){
// 			float lim = (b->position.x - (a->position.x + a->size.x)) / diff.x;
// 			if(lim < limit.x){
// 				limit.x = lim;
// 			}
// 			normal.x = 1.0f;
// 			collision = true;
// 		}
// 	}else
// 	if(diff.x < 0){
// 		float ray = a->position.x + diff.x;
// 		if(ray <= b->position.x + b->size.x && a->position.x >= b->position.x + b->size.x && (withinTopY || withinBottomY)){
// 			float lim = (a->position.x - (b->position.x + b->size.x)) / (diff.x * -1.0f);
// 			if(lim < limit.x){
// 				limit.x = lim;
// 			}			
// 			normal.x = -1.0f;
// 			collision = true;
// 		}
// 	}

// 	if(diff.y > 0){
// 		float ray = a->position.y + a->size.y + diff.y;
// 		if(ray >= b->position.y && (a->position.y + a->size.y) <= b->position.y && (withinTopX || withinBottomX)){
// 			float lim = (b->position.y - (a->position.y + a->size.y)) / diff.y;
// 			if(lim < limit.y){
// 				limit.y = lim;
// 			}			
// 			normal.y = 1.0f;
// 			collision = true;
// 		}
// 	}else
// 	if(diff.y < 0){
// 		float ray = a->position.y + diff.y;
// 		if(ray <= b->position.y + b->size.y && a->position.y >= b->position.y + b->size.y && (withinTopX || withinBottomX)){
// 			float lim = (a->position.y - (b->position.y + b->size.y)) / (diff.y * -1.0f);
// 			if(lim < limit.y){
// 				limit.y = lim;
// 			}			
// 			normal.y = -1.0f;
// 			collision = true;
// 		}
// 	}

// 	// normalize limit
// 	if(limit.x < 0.0f) limit.x = 0.0f;
// 	if(limit.x > 1.0f) limit.x = 1.0f;	

// 	if(limit.y < 0.0f) limit.y = 0.0f;
// 	if(limit.y > 1.0f) limit.y = 1.0f;		

// 	return collision;
// }

// void Game::NetWorld::updateObject(Game::NetObject *obj){
// 	if(obj->unmovable || obj == NULL) return;

// 	nite::Vec2 origp = obj->position;

// 	// we're gonna use a fixed size to look for quadrants for now(3000x3000 from center/origin)
// 	// ideally we should use a size relative to the object's speed
// 	// TODO: this ^
	
// 	Vector<Game::NetObject*> nearObjs;
// 	getQuadrant(obj->position.x - 800,  obj->position.y - 800, 1600, 1600, nearObjs);
// 	float xdiff = nite::cos(obj->direction) * obj->speed * this->timescale * obj->relativeTimescale * currentTickRate * nite::getTimescale() * 0.067f;
// 	float ydiff = nite::sin(obj->direction) * obj->speed * this->timescale * obj->relativeTimescale * currentTickRate * nite::getTimescale() * 0.067f;
// 	auto diff = nite::Vec2(xdiff, ydiff);
// 	auto normal = nite::Vec2(0.0f);
// 	auto limit = nite::Vec2(1.0f);
// 	for(int i = 0; i < nearObjs.size(); ++i){
// 		auto other = nearObjs[i];
// 		if(obj->id == other->id) continue;		
// 		if(!other->solid && other->objType != ObjectType::Ghost) continue;		
// 		Penetration result;
// 		if(test(obj, other, result)){
// 			if(other->objType == ObjectType::Ghost){
// 				auto ghost = static_cast<Game::GhostMask*>(other);
// 				ghost->callback(obj);
// 				limit = nite::Vec2(1.0f);
// 				continue; // we do not handle collision if it's a ghost object
// 			}
// 			obj->collided = true;
// 			if(!obj->destroyed){
// 				obj->onCollision(other);
// 			}
// 			break;
// 		}
// 		// if(testCollision(obj, other, diff, limit, normal)){				
// 		// 	if(other->objType == ObjectType::Ghost){
// 		// 		auto ghost = static_cast<Game::GhostMask*>(other);
// 		// 		ghost->callback(obj);
// 		// 		limit = nite::Vec2(1.0f);
// 		// 		continue; // we do not handle collision if it's a ghost object
// 		// 	}
// 		// 	obj->collided = true;
// 		// 	if(!obj->destroyed){
// 		// 		obj->onCollision(other);
// 		// 	}
// 		// 	if(limit.x == 0.0f && limit.y == 0.0f){
// 		// 		break;
// 		// 	}
// 		// }
// 	}
// 	// push off by 1 unit of the total diff
// 	// if(limit.x == 0.0f && diff.x != 0.0f){
// 	// 	limit.x = (1.0f / diff.x) * nite::getSign(diff.x) * -1.0f;		
// 	// }
// 	// if(limit.y == 0.0f && diff.y != 0.0f){
// 	// 	limit.y = (1.0f / diff.y) * nite::getSign(diff.y) * -1.0f;
// 	// }
// 	obj->position = obj->position + diff * limit;
// 	if(origp.x != obj->position.x || origp.y != obj->position.y){
// 		obj->updateQuadrant();
// 	}
// }

// void Game::NetWorld::setVisibleQuadrant(int x, int y, int w, int h){

// }

// void Game::NetWorld::renderDbug(){
// 	// debug: shows masks
// 	if(debugPhysics){
// 		for (auto it : objects){
// 			auto current = it.second;
// 			nite::setDepth(nite::DepthTop);
// 			nite::setRenderTarget(nite::RenderTargetGame);
// 			nite::setColor(current->collided ? nite::Color(1.0f, 0, 0) : nite::Color(0, 1.0f, 0));
// 			nite::Draw::Polygon(current->body.rotate(current->orientation), current->position, nite::Vec2(0.0f), false);
// 			nite::setDepth(0);
// 			nite::resetColor();
// 		}
// 		for(int i = 0; i < wallMasks.size(); ++i){
// 			auto mask = wallMasks[i];
// 			nite::setDepth(nite::DepthTop);
// 			nite::setRenderTarget(nite::RenderTargetGame);
// 			nite::setColor(mask->collided ? nite::Color(1.0f, 0, 0) : nite::Color(0, 1.0f, 0));
// 			nite::Draw::Rectangle(mask->position, mask->size, false, nite::Vec2(0.0, 0.0), 0);
// 			nite::setDepth(0);
// 			nite::resetColor();
// 		}
// 		for(int i = 0; i < ghostMasks.size(); ++i){
// 			auto mask = ghostMasks[i];
// 			nite::setDepth(nite::DepthTop);
// 			nite::setRenderTarget(nite::RenderTargetGame);
// 			nite::setColor(nite::Color(1.0f, 0, 1.0f));
// 			nite::Draw::Rectangle(mask->position, mask->size, false, nite::Vec2(0.0, 0.0), 0);
// 			nite::setDepth(0);
// 			nite::resetColor();
// 		}
// 	}
// }

// void Game::NetWorld::update(){
// 	renderDbug();

// 	// clean up
// 	if(removeQueue.size() > 0){
// 		std::sort(removeQueue.begin(), removeQueue.end());
// 		removeQueue.erase(std::unique(removeQueue.begin(), removeQueue.end()), removeQueue.end());		
// 		for(int i = 0; i < removeQueue.size(); ++i){
// 			objects[removeQueue[i]]->onDestroy();
// 			objects[removeQueue[i]]->updateQuadrant();
//     		objects[removeQueue[i]]->clearQuadrant();			
// 			objects[removeQueue[i]]->id = 0;
// 			objects.erase(removeQueue[i]);
// 		}
// 		removeQueue.clear();
// 	}
	
// 	delta = nite::getTicks() - lastTick;
// 	lastTick = nite::getTicks();
// 	currentTickRate = delta;
//   	for (auto it : objects){
// 		auto current = it.second;
// 		nite::Vec2 lastPos = current->position;
// 		float lastspd = current->speed;
// 		current->step();
// 		current->onStep();
// 		if(current->objType == ObjectType::Entity){
// 			auto ent = static_cast<Game::EntityBase*>(current.get());
// 			ent->effectStat.update();
// 			ent->entityStep();  	
// 		}	
// 		current->collided = false;
// 		// movement
// 		updateObject(it.second.get());
// 		// friction
// 		for(int i = 0; i < currentTickRate / tickrate; ++i){
// 			current->speed *= 1.0f - (current->friction * nite::getTimescale() * timescale * current->relativeTimescale);
// 		}
// 		// if(lastPos.x != current->position.x || lastPos.y != current->position.y || current->speed != lastspd){
// 			current->issueDeltaUpdate(DeltaUpdateType::PHYSICS);
// 		// }
// 	}
// }

// void Game::NetWorld::render(){
//   for (auto& it : objects){
// 		auto current = it.second;
// 		current->draw();
// 		current->onDraw();
// 	}
// }


nite::Vec2 Game::World::toCoors(int i){
	return nite::Vec2(i %  width, i / height);
}

int Game::World::toIndex(int x, int y){
	return x + y * width;
}

int Game::World::toIndex(const nite::Vec2 &p){
	return toIndex(p.x, p.y);
}

Game::World::World(){
	debug = false;
	lastId = nite::randomInt(101, 301);
	cells = NULL;
}

void Game::World::init(int w, int h, int cellsize){
	if(w < 0 || h < 0 || cellsize < 0){
		return;
	}
	if(this->cells != NULL){
		delete cells;
	}
	this->width = w;
	this->height = h;
	this->total = w * h;
	this->cellsize = cellsize;
	this->size = nite::Vec2(w, h) * nite::Vec2(cellsize);
	this->cells = new int[total];
	for(int i = 0; i < this->total; ++i){
		cells[i] = 0;
	}
	nite::print("set world size "+nite::toStr(this->width)+"x"+nite::toStr(this->height)+" units. unit size "+nite::toStr(cellsize)+" | cells "+nite::toStr(this->total));
}

void Game::World::update(){

	if(removeQueue.size() > 0){
		std::sort(removeQueue.begin(), removeQueue.end());
		removeQueue.erase(std::unique(removeQueue.begin(), removeQueue.end()), removeQueue.end());		
		for(int i = 0; i < removeQueue.size(); ++i){
			auto obj = get(removeQueue[i]);
			if(obj == NULL){
				continue;
			}
			int index = toIndex(obj->position);
			obj->onDestroy();			
			obj->id = 0;
			if(isValid(index)){
				cells[index] = 0;
			}			
			objects.erase(removeQueue[i]);
		}
		removeQueue.clear();
	}

	for(auto &it : objects){
		auto obj = it.second.get();
		obj->step();
		obj->onStep();		
		if(obj->objType == ObjectType::Entity){
			auto ent = static_cast<Game::EntityBase*>(obj);
			ent->effectStat.update();
			ent->entityStep();  	
		}
	}
}

Game::NetObject *Game::World::get(UInt16 id){
	auto it = objects.find(id);
	if(it == objects.end()){
		return NULL;
	}
	return it->second.get();
}

bool Game::World::isValid(int i){
	return i >= 0 && i < total;
}

bool Game::World::isFree(int i){
	return isValid(i) && cells[i] == 0;
}

int Game::World::findCell(int index){
	if(index < 0 || index >= total){
		return -1;
	}
	if(cells[index] == 0){
		return index;
	}
	nite::Vec2 coors = toCoors(index);
	// try up
	int c = toIndex(coors.x, coors.y - 1);
	if(c != -1 && isFree(c)){
		return c;
	}
	// try down
	c = toIndex(coors.x, coors.y + 1);
	if(c != -1 && isFree(c)){
		return c;
	}	
	// try right
	c = toIndex(coors.x + 1, coors.y);
	if(c != -1 && isFree(c)){
		return c;
	}	
	// try left
	c = toIndex(coors.x - 1, coors.y);
	if(c != -1 && isFree(c)){
		return c;
	}		
	return -1;
}

bool Game::World::add(const Shared<Game::NetObject> &obj){
	return add(obj, obj->position.x, obj->position.y, true);
}

nite::Vec2 Game::World::snap(const nite::Vec2 &p){
	return nite::Vec2(nite::round(p.x / (float)cellsize) * cellsize, nite::round(p.y / (float)cellsize) * cellsize);
}

nite::Vec2 Game::World::snap(int x, int y){
	return snap(nite::Vec2(x, y));
}

// for client-side debugging
void Game::World::renderDebug(){
	if(!debug){
		return;
	}
	auto pos = nite::getView(nite::RenderTargetGame);
	auto sizeAdj = nite::getAdjustedSize();
	auto size = nite::getSize();
	auto sizeDiff = (sizeAdj - size);
	auto sizeDiffHalf = sizeDiff * nite::Vec2(0.5f);
	int wt = nite::round((size.x + sizeDiff.x) / cellsize);
	int ht = nite::round((size.y + sizeDiff.y) / cellsize);
	nite::setRenderTarget(nite::RenderTargetGame);
	nite::setDepth(nite::DepthTop);
	static nite::Texture empty("data/texture/empty.png");
	nite::Vec2 mp = nite::getView(nite::RenderTargetGame) + nite::mousePositionAdj(nite::RenderTargetGame);
	for(int x = 0; x < width; ++x){
		for(int y = 0; y < height; ++y){
			float _x = x * cellsize;
			float _y = y * cellsize;
			int i = x + y * width;
			bool fill = false;
			if(!isValid(i)){
				continue;
			}
			switch(cells[i]){
				case 0: {
					nite::setColor(0.0f, 1.0f, 0.0f, 1.0f);
				} break;
				case -1: {
					nite::setColor(0.9f, 0.05f, 0.09f, 1.0f);
				} break;
				default: {
					nite::setColor(0.9f, 0.9f, 0.0f, 0.5f);
					fill = true;
				} break;
			}			
			if(nite::isPointInRegion(mp, nite::Vec2(_x,_y), nite::Vec2(_x,_y) + nite::Vec2(cellsize))){
				nite::setColor(0.0f, 0.5f, 0.1f, 0.8f);
				fill = true;
			}			
			nite::Draw::Rectangle(_x, _y, cellsize, cellsize, fill, 0.0f, 0.0f, 0.0f);
		}
	}
}

bool Game::World::add(const Shared<Game::NetObject> &obj, int x, int y, bool accomodate){
	int i = toIndex(x, y);
	int p = accomodate ? findCell(i) : i;
	obj->id = obj->id == 0 ? ++lastId : obj->id;
	obj->position = nite::Vec2(x, y);
	obj->rPosition = nite::Vec2(x, y) * nite::Vec2(cellsize);
	obj->container = this;
	obj->onCreate();	
	if(!isFree(i)){
		obj->onCollision(get(cells[i]));
		obj->onDestroy();
		obj->container = NULL;
		obj->id = 0;
		nite::print("[world] on add collision - destroyed");
		return false;
	}
	if(accomodate && p == -1){
		nite::print("[world] failed to add object type '"+ObjectType::name(obj->objType)+"' at '"+nite::toStr(i)+"': no empty space");
		return false;
	}
	cells[i] = obj->id;
	objects[obj->id] = obj;
	return true;
}

void Game::World::remove(UInt16 id){
	auto it = objects.find(id);
	if(it == objects.end()) {
		nite::print("[NetWorld] unable to remove object id '"+nite::toStr(id)+"': it doesn't exist.");
	}
	removeQueue.push_back(id);
}

void Game::World::clear(){
	if(cells != NULL){
		delete cells;
		cells = NULL;
	}
}


Vector<int> Game::World::getNeighbors(Dict<int, int> &map, int x, int y){
    Vector<int> neighbors;
    // top
    if(y > 0){
        int index = toIndex(x, y - 1);
        if(map.count(index)){
            neighbors.push_back(index);
        }
    }
    // right
    if(x < width){
        int index = toIndex(x + 1, y);
        if(map.count(index)){
            neighbors.push_back(index);
        }
    }
    // bottom
    if(y < height){
        int index = toIndex(x, y + 1);
        if(map.count(index)){
            neighbors.push_back(index);
        }
    }
    //left 
    if(x > 0){
        int index = toIndex(x - 1, y);
        if(map.count(index)){
            neighbors.push_back(index);
        }
    }
    return neighbors;
}

static float heuristicmd(const nite::Vec2 &a, const nite::Vec2 &b){
  static const float D = 1.0f;
  return D * (nite::abs(a.x - b.x) + nite::abs(a.y - b.y));
}

nite::MapRoute Game::World::astar(int start, int target){

	nite::MapRoute route;

	if(start == target){
		return route;	
	}

    Dict<int, int> closed;
    Dict<int, int> open;
    Dict<int, int> cameFrom;
    Dict<int, float> fScore;
    Dict<int, float> gScore;
	Dict<int, int> map;

	nite::Vec2 startc = toCoors(start);
	nite::Vec2 targetc = toCoors(target);
	int mw = std::abs(targetc.x - startc.x) + 10;
	int mh = std::abs(targetc.y - startc.y) + 10;
	if(mw > this->width) mw = this->width;
	if(mh > this->height) mh = this->height;
	int minx = std::min(startc.x, targetc.x) - 5;
	int miny = std::min(startc.y, targetc.y) - 5;
	if(minx < 0) minx = 0;
	if(miny < 0) miny = 0;

	// nite::print("min "+nite::Vec2(minx, miny).str()+" "+startc.str()+" "+targetc.str()+" start "+nite::toStr(start)+" target "+nite::toStr(target));

	for(int y = 0; y < mh; ++y){
		for(int x = 0; x < mw; ++x){
			// int i = x + y * mw;
			int c = x + minx + ((miny + y) * this->width);
			if(!isValid(c) || cells[c] == WORLD_WALL){
				continue;
			}
			map[c] = cells[c];
		}
	}

	int mtotal = mh * mw;

    for(int i = 0; i < mtotal; ++i){
        int c = cells[i];
        open[c] = i;
        fScore[c] = (float)((unsigned)-1);
        gScore[c] = (float)((unsigned)-1);
    }   

    gScore[start] = 0.0f;
    fScore[start] = heuristicmd(toCoors(start), toCoors(target));
    open[start] = start;

    auto lowestIndex = [&](){
        int i = -1;
        for (auto const& x : open){
            if(i == -1 || fScore[x.first] < fScore[i]){
            	i = x.first;
            }
        }
        return i;
    };    

    // auto reconstruct = [&](Dict<int, int> &cameFrom, int index){
    //     Vector<int> seq;
    //     seq.insert(seq.begin(), index);
    //     while(cameFrom.count(index) > 0){
    //         index = cameFrom[index];
    //         seq.push_back(index);
    //     }

    //     for(int i = 0; i < seq.size(); ++i){
    //         map[seq[i]].value = 4.0f;
    //         route.push_back(map[seq[i]].rp);
    //     }       
    // };

    // while(open.size() > 0){
    //     int index = lowestIndex();

    //     if(index == target){
    //         reconstruct(cameFrom, index);
    //         break;
    //     }

    //     open.erase(index);
    //     closed[index] = index;

    //     auto neighbors = neighbors(open, map[index].inmp.x, map[index].inmp.y);

    //     for(int i = 0; i < neighbors.size(); ++i){
    //         int z = neighbors[i];
    //         if(map[z].value == 3.0) continue;
    //         if(closed.count(z) > 0) continue;
    //         if(open.count(z) == 0){
    //             open[z] = z;
    //         }
    //         float tscore = gScore[index] + nite::distance(map[index].inmp, map[z].inmp);
    //         if(tscore >= gScore[z]){
    //             continue;
    //         }
    //         cameFrom[z] = index;
    //         gScore[z] = tscore;
    //         fScore[z] = gScore[z] + heuristicmd(map[z].inmp, map[target].inmp);
    //     }
    // }  

	return route; 
}