#include <string.h>
#include <limits>

#include "../Engine/Shapes.hpp"
#include "../Engine/Tools/Tools.hpp"
#include "../Engine/Graphics.hpp"
#include "World.hpp"
#include "Network.hpp"


static UInt16 generateId(){
	static UInt16 seed = nite::randomInt(25, 50);
	return ++seed; // networld ids are never to be 0. so a response with 0 means error
}

void Game::NetWorld::setSize(int w, int h, int unitsize, float gridSpec){
	if(w <= 0.0f || h <= 0.0f || unitsize <= 0){
		return;
	}
	if(cells != NULL){
		delete cells;
	}
	this->gridSpec = nite::Vec2(gridSpec);
	this->size.set(w, h);
	this->cusize = unitsize;
	this->cwidth = w / this->cusize;
	this->cheight = h / this->cusize;
	this->ctotal = this->cwidth * this->cheight;
	this->cells = new Game::NetObject*[this->ctotal];
	for(int i = 0; i < this->ctotal; ++i){
		this->cells[i] = NULL;
	}
	nite::print("set world size "+nite::toStr(this->cwidth)+"x"+nite::toStr(this->cheight)+"("+nite::toStr(unitsize)+") | cells "+nite::toStr(this->ctotal));
}

void Game::NetWorld::getQuadrant(int x, int y, int w, int h, Vector<Game::NetObject*> &quadrant){
	if(cells == NULL){
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
			if(c != NULL){
				quadrant.push_back(c);
			}			
			
		}
	}
	return;
}

Game::NetWorld::~NetWorld(){
	if(cells != NULL){
		delete cells;
	}
}

Game::NetWorld::NetWorld(){
	timescale = 1.0f;
	debugPhysics = false;
	cells = NULL;
	snapshotOrder = 0;
	tickrate = 1000 / 60;
	objects.reserve(5000);
}

bool Game::NetWorld::exists(UInt16 id){
	return id != -1 ? objects.find(id) != objects.end() : false;
}

UInt16 Game::NetWorld::add(Shared<Game::NetObject> &obj, int useId){
	UInt16 id = useId == -1 ? generateId() : useId; 
	objects[id] = obj;
	obj->id = id;
	obj->container = this;
	obj->onCreate();
	obj->updateQuadrant();
	return id;
}

void Game::NetWorld::clear(){
	objects.clear();
}

void Game::NetWorld::remove(UInt16 objectId){
	auto search = objects.find(objectId);
	if(search != objects.end()) {
		for(int i = 0; i < removeQueue.size(); ++i){
			if(removeQueue[i] == objectId) return;
		}
		removeQueue.push_back(objectId);
	}else{
		nite::print("unable to remove object id '"+nite::toStr(objectId)+"': it doesn't exist.");
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

float Game::NetWorld::testSweptAABB(Game::NetObject *a, Game::NetObject *b, const nite::Vec2 &diff, nite::Vec2 &normal){
	nite::Vec2 apos = a->position - a->size * 0.5f;
	nite::Vec2 bpos = b->position - b->size * 0.5f;
	float dxEntry, dxExit;
	float dyEntry, dyExit;
	float txEntry, txExit;
	float tyEntry, tyExit;
	normal.x = 0.0f;
	normal.y = 0.0f;
	if(diff.x > 0.0f){
		dxEntry = bpos.x - (apos.x + a->size.x);
		dxExit = (bpos.x + b->size.x) - apos.x;
	}else{
		dxEntry = (bpos.x + b->size.x) - apos.x;
		dxExit = bpos.x - (apos.x + a->size.x);
	}
	if(diff.y > 0.0f){
		dyEntry = bpos.y - (apos.y + a->size.y);
		dyExit = (bpos.y + b->size.y) - apos.y;
	}else{
		dyEntry = (bpos.y + b->size.y) - apos.y;
		dyExit = bpos.y - (apos.y + a->size.y);
	}	
	if(diff.x == 0.0f){
		txEntry = -std::numeric_limits<float>::infinity();
		txExit = std::numeric_limits<float>::infinity();
	}else{
		txEntry = dxEntry / diff.x;
		txExit = dxExit / diff.x;
	}
	if(diff.y == 0.0f){
		tyEntry = -std::numeric_limits<float>::infinity();
		tyExit = std::numeric_limits<float>::infinity();
	}else{
		tyEntry = dyEntry / diff.y;
		tyExit = dyExit / diff.y;
	}
	float entryTime = std::max(txEntry, tyEntry);
	float exitTime = std::min(txExit, tyExit);
	if(	entryTime > exitTime || txEntry < 0.0f && tyEntry < 0.0f || txEntry > 1.0f || tyEntry > 1.0f ||
		txEntry < 0.0f && (apos.x + a->size.x < bpos.x || apos.x > bpos.x + b->size.x) || // this could be error-prone at high speeds.
		tyEntry < 0.0f && (apos.y + a->size.y < bpos.y || apos.y > bpos.y + b->size.y) ){ // right now i don't have any other solution
		return 1.0f;
	}
	if (txEntry > tyEntry){ 
		if (dxEntry < 0.0f){ 
			normal.x = 1.0f; 
			normal.y = 0.0f; 
		} else{ 
			normal.x = -1.0f; 
			normal.y = 0.0f;
		} 
	}else{ 
		if (dyEntry < 0.0f){ 
			normal.x = 0.0f; 
			normal.y = 1.0f; 
		}else{ 
			normal.x = 0.0f; 
			normal.y = -1.0f; 
		} 
	}
	return entryTime;
}


void Game::NetWorld::updateObject(Game::NetObject *obj){
	if(obj->unmovable || obj == NULL) return;
	nite::Vec2 origp = obj->position;
	
	// we're gonna use a fixed size to look for quadrants for now(1024x1024 from center/origin)
	// ideally we should use a size relative to the object's speed
	// TODO: this ^
	
	Vector<Game::NetObject*> nearObjs;
	nite::Vec2 diff = nite::Vec2(nite::getSign(obj->speed.x) * gridSpec.x, nite::getSign(obj->speed.y) * gridSpec.y);
	bool predicting = false;

	if(obj->sv == NULL){
		auto &snaps = obj->snapshots;
		auto find = [&](const nite::Vec2 &p){
			for(int i = 0; i < snaps.size(); ++i){
				if(p.x == snaps[i].pos.x && p.y == snaps[i].pos.y){
					return i;
				}
			}
			return -1;
		};
		auto orig = find(origp);
		auto next = find(obj->position + diff);		
		if(orig != -1 && next != -1 && nite::abs((int)snaps[orig].order - (int)snaps[next].order) == 1){
			predicting = true;
			obj->snapshots.erase(obj->snapshots.begin(), obj->snapshots.begin() + next);
			obj->position = obj->position + diff;
			nite::print(":^)");
		}else{
			obj->position.set(obj->nextPosition);
		}
	}

	if(!predicting){ // would always be false on the server
		getQuadrant(obj->position.x - 1024 * 0.5f,  obj->position.y - 1024 * 0.5f, 1024, 1024, nearObjs);		
		obj->position = obj->position + diff;	
		for(int i = 0; i < nearObjs.size(); ++i){
			auto other = nearObjs[i];
			if(obj->id == other->id) continue;				
			if(!other->solid) continue;			
			if(obj->isCollidingWith(other)){
				nite::print("lol");
				obj->position = obj->position - diff;
				break;
			}
		}
		// auto test = [&](const nite::Vec2 &test){
		// 	bool r = true;
		// 	obj->position = obj->position + test;
		// 	for(int i = 0; i < nearObjs.size(); ++i){
		// 		auto other = nearObjs[i];
		// 		if(obj->id == other->id) continue;				
		// 		if(!other->solid) continue;			
		// 		if(obj->isCollidingWith(other)){
		// 			r = false;
		// 			break;
		// 		}
		// 	}			
		// 	obj->position = obj->position - test;
		// 	return r;
		// };
		obj->speed.set(0.0f);	
		// build predictions
		if(obj->sv != NULL){
			obj->snapshots.clear();
			nite::Vec2 start = origp;
			for(int i = 0; i < 3; ++i){
				Game::PredictFragment frg;
				frg.order = ++snapshotOrder;
				frg.pos = start;
				start = start + diff;
				// if(test(frg.pos)){
					obj->snapshots.push_back(frg);
				// }
			}
		}
	}

	obj->snapPosition();	
	obj->lerpPosition.lerpDiscrete(obj->position, 0.15f);
	if(origp.x != obj->position.x || origp.y != obj->position.y){
		obj->updateQuadrant();
	}	
}

void Game::NetWorld::update(){
	delta = nite::getTicks() - lastTick;
	lastTick = nite::getTicks();
	currentTickRate = ((float)delta / (float)tickrate) * 16.6666667f;
  	for (auto it : objects){
		auto current = it.second;
		nite::Vec2 p = current->position;
		auto obj = it.second.get();
		if(obj->sv == NULL && debugPhysics){
			nite::setDepth(nite::DepthTop);
			nite::setRenderTarget(nite::RenderTargetGame);
			nite::setColor(current->collided ? nite::Color(1.0f, 0, 0) : nite::Color(0, 1.0f, 0));
			nite::Draw::Rectangle(current->position, current->size, false, nite::Vec2(0.0, 0.0), 0);
			nite::setDepth(0);
			nite::resetColor();
		}
		current->collided = false;
		updateObject(obj);
		if(current->position.x != p.x || current->position.y != p.y){
			this->updateQueue[current->id] = current.get();
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
