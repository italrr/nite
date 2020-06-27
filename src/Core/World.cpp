#include <string.h>

#include "../Engine/Shapes.hpp"
#include "../Engine/Tools/Tools.hpp"
#include "../Engine/Graphics.hpp"
#include "World.hpp"
#include "Network.hpp"


static UInt16 generateId(){
	static UInt16 seed = nite::randomInt(25, 50);
	return ++seed; // networld ids are never to be 0. so a response with 0 means error
}

void Game::NetWorld::setSize(int w, int h, int unitsize){
	if(w <= 0 || h <= 0 || unitsize <= 0){
		return;
	}
	if(cells != NULL){
		delete cells;
	}
	this->cusize = unitsize;
	this->cwidth = w / this->cusize;
	this->cheight = h / this->cusize;
	this->ctotal = this->cwidth * this->cheight;
	this->cells = new Game::NetObject*[this->cwidth * this->cheight];
	for(int i = 0; i < this->ctotal; ++i){
		this->cells[i] = NULL;
	}
	nite::print("set world size "+nite::toStr(w)+"x"+nite::toStr(h)+"("+nite::toStr(unitsize)+") | cells "+nite::toStr(this->ctotal));
}
#include "../Engine/Texture.hpp"
Vector<Game::NetObject*> Game::NetWorld::getQuadrant(int x, int y, int w, int h){
	Vector<Game::NetObject*> quadrant;
	if(cells == NULL){
		return quadrant;
	}
	x = x / cusize;
	y = y / cusize;
	w = w / cusize;
	h = h / cusize;
	int tx = x + w;
	if(tx >= cwidth){
		tx = tx % cwidth;
	}
	int ty = y + h;
	if(ty >= cheight){
		ty = cheight - 1;
	}
	// static nite::Texture empty("data/texture/empty.png");
	for(int _x = x; _x < tx; ++_x){
		for(int _y = y; _y < ty; ++_y){
			auto c = cells[_x + _y * cwidth];
			// nite::setRenderTarget(nite::RenderTargetGame);
			// nite::setDepth(nite::DepthTop);
			// nite::setColor(1.0f, 0.0f, 0.0f, 1.0f);
			if(c != NULL){
				// continue;
				// nite::setColor(1.0f, 1.0f, 0.0f, 1.0f);
				quadrant.push_back(c);
			}
			// empty.draw(_x * 16, _y * 16, 16, 16, 0.0f, 0.0f, 0.0f);			
			
		}
	}
	return quadrant;
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
	objects.reserve(5000);
}

bool Game::NetWorld::exists(UInt16 id){
	return id != -1 ? objects.find(id) != objects.end() : false;
}

UInt16 Game::NetWorld::add(Shared<Game::NetObject> &obj){
	UInt16 id = generateId();
	objects[id] = obj;
	obj->id = id;
	obj->container = this;
	obj->onCreate();
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

Shared<Game::NetObject> Game::NetWorld::get(UInt16 id){
	auto it = objects.find(id);
	if(it == objects.end()){
		return Shared<Game::NetObject>(NULL);
	}
	return it->second;
}

void Game::NetWorld::step(){
  for (auto& it : objects){
		auto current = it.second;
		current->step();
		current->onStep();
	}
}

void Game::NetWorld::updateObjectPhysics(Game::NetObject *obj, float x, float y){
	if(obj->unmovable) return;
	nite::Vec2 origp = obj->position;
	
	Vector<Game::NetObject*> nearObjs;
	if(obj->sv != NULL)
		nearObjs = getQuadrant(obj->position.x - 1024 * 0.5f,  obj->position.y - 1024 * 0.5f, 1024, 1024);
	if(x != 0){
		float xdiff = x * this->timescale * obj->relativeTimescale * nite::getDelta() * nite::getTimescale() * 0.067f;
		obj->position.x += xdiff;	
		if(obj->solid){
			for(int i = 0; i < nearObjs.size(); ++i){
				auto _obj = nearObjs[i];
				if(obj->id == _obj->id) continue;				
				if(!_obj->solid) continue;			
				if(_obj->isCollidingWith(obj)){
					obj->onCollision(obj);
					obj->collided = true;
					if(x > 0.0f){
						obj->position.x = _obj->position.x - _obj->size.x * 0.5f - obj->size.x * 0.5f - 1.0f;
					}
					if(x < 0.0f){
						obj->position.x = _obj->position.x + _obj->size.x * 0.5f + obj->size.x * 0.5f + 1.0f;
					}
					x = 0.0f;				
					break;
				}
			}
		}
	}
	if(y != 0){
		float ydiff = y * this->timescale * obj->relativeTimescale * nite::getDelta() * nite::getTimescale() * 0.067f;
		obj->position.y += y * this->timescale * obj->relativeTimescale * nite::getDelta() * nite::getTimescale() * 0.067f;
		if(obj->solid){
			for(int i = 0; i < nearObjs.size(); ++i){
				auto _obj = nearObjs[i];
				if(obj->id == _obj->id) continue;
				if(!_obj->solid) continue;
				if(_obj->isCollidingWith(obj)){
					obj->onCollision(obj);
					obj->collided = true;
					if(y > 0.0f){
						obj->position.y = _obj->position.y - _obj->size.y/2.0f - obj->size.y/2.0f - 1.0f;
					}
					if(y < 0.0f){
						obj->position.y = _obj->position.y + _obj->size.y/2.0f + obj->size.y/2.0f + 1.0f;
					}
					y = 0.0f;
					break;
				}
			}
		}
	}
	if(origp.x != obj->position.x || origp.y != obj->position.y){
		obj->updateQuadrant();
	}
		
}

void Game::NetWorld::update(){
	updateQueue.clear();
  	for (auto it : objects){
		auto current = it.second;
		nite::Vec2 p = current->position;
		auto obj = it.second.get();
		if(this->debugPhysics){
			nite::setDepth(nite::DepthTop);
			nite::setRenderTarget(nite::RenderTargetGame);
			nite::setColor(current->collided ? nite::Color(1.0f, 0, 0) : nite::Color(0, 1.0f, 0));
			nite::Draw::Rectangle(current->position, current->size, false, nite::Vec2(0.5, 0.5), 0);
			nite::setDepth(0);
			nite::resetColor();
		}
		current->collided = false;
		updateObjectPhysics(obj, current->speed.x, current->speed.y);
		if(obj->net != NULL){
			current->speed.lerpDiscrete(nite::Vec2(0.0f), nite::getDelta() * 0.067f * current->friction * current->relativeTimescale * this->timescale);
		}
		if(current->position.x != p.x || current->position.y != p.y){
			this->updateQueue.push_back(current->id);
		}		
	}
	// clean up
	for(int i = 0; i < removeQueue.size(); ++i){
		objects[removeQueue[i]]->onDestroy();
		objects[removeQueue[i]]->id = 0;
		objects.erase(removeQueue[i]);
	}
	removeQueue.clear();
}

void Game::NetWorld::render(){
  for (auto& it : objects){
		auto current = it.second;
		current->draw();
		current->onDraw();
	}
}
