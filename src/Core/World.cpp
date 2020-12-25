#include <string.h>
#include <limits>
#include <cmath>
#include  <algorithm>

#include "../Engine/Shapes.hpp"
#include "../Engine/Tools/Tools.hpp"
#include "../Engine/Graphics.hpp"
#include "../Engine/View.hpp"
#include "../Engine/Font.hpp"

#include "World.hpp"
#include "Network.hpp"
#include "../Entity/Base.hpp"

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
	width = 0;
	height = 0;
	total = 0;
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
		if(obj->lastRouteMove > 0 && nite::getTicks()-obj->lastMove > obj->speed){
			auto nstep = obj->route.route[obj->lastRouteMove - 1];
			if(!isFree(nstep.index)){
				// cock-blocked
				obj->lastRouteMove = 0;
				obj->route.route.clear();
				obj->route.start = 0;
				obj->route.end = 0;
				// TODO notify clients
			}else{
				--obj->lastRouteMove;
				obj->lastMove = nite::getTicks();
				auto diff = nite::Vec2(nstep.x, nstep.y) - obj->position; // make it relative
				obj->move(diff.x, diff.y);
			}
		}
		obj->step();
		obj->onStep();		
		if(obj->objType == ObjectType::Entity){
			auto ent = static_cast<Game::EntityBase*>(obj);
			ent->effectStat.update();
			ent->entityStep();  	
		}
	}
}

bool Game::World::swapCells(int s, int t){
	if(!isValid(s) || !isValid(t)){
		return false;
	}
	int h = cells[t];
	cells[t] = cells[s];
	cells[s] = h;
	return true;
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
	static nite::Font font(nite::DefaultFontPath, 16, 1.0f);
	nite::Vec2 mp = nite::getView(nite::RenderTargetGame) + nite::mousePositionAdj(nite::RenderTargetGame);
	bool renderSel = false;
	int renderInd;
	nite::Vec2 renderP;
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
				nite::setColor(0.0f, 0.7f, 0.9f, 0.8f);
				fill = true;
				renderSel = true;
				renderInd = i;
				renderP = nite::Vec2(x, y);
			}			
			nite::Draw::Rectangle(_x, _y, cellsize, cellsize, fill, 0.0f, 0.0f, 0.0f);
		}
	}
	if(renderSel){
		nite::setRenderTarget(nite::RenderTargetUI);
		nite::setDepth(nite::DepthTop);
		nite::setColor(0.0f, 0.7, 0.9, 1.0f);
		font.draw(nite::toStr(renderInd)+" "+renderP.str(), nite::getWidth() * 0.5f, font.getHeight(), 0.5f, 0.5f, 0.0f);
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
	Dict<int, float> map;

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

	for(int y = 0; y < mh; ++y){
		for(int x = 0; x < mw; ++x){
			int c = x + minx + ((miny + y) * this->width);
			if(!isValid(c) || cells[c] == WORLD_WALL){
				continue;
			}
			map[c] = cells[c] == 0 ? 0.0f : 3.0f;
		}
	}
	map[start] = 1.0f;
	map[target] = 2.0f;

    for(auto &it : map){
        int c = it.first;
        open[c] = it.first;
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
    
	auto reconstruct = [&](Dict<int, int> &cameFrom, int index){
        Vector<int> seq;
        seq.insert(seq.begin(), index);
        while(cameFrom.count(index) > 0){
            index = cameFrom[index];
            seq.push_back(index);
        }
        for(int i = 0; i < seq.size(); ++i){
            map[seq[i]] = 4.0f;
            route.route.push_back(nite::MapCell(seq[i], 0, this->width));
        }       
    };

    while(open.size() > 0){
        int index = lowestIndex();
        if(index == target){
            reconstruct(cameFrom, index);
            break;
        }
        open.erase(index);
        closed[index] = index;
		auto ccoors = toCoors(index);
        auto neighbors = getNeighbors(open, ccoors.x, ccoors.y);
        for(int i = 0; i < neighbors.size(); ++i){
            int z = neighbors[i];
            if(map[z] == 3.0) continue;
            if(closed.count(z) > 0) continue;
            if(open.count(z) == 0){
                open[z] = z;
            }
			auto ccoors = toCoors(index);
			auto zcoors = toCoors(z);
			auto tcoors = toCoors(target);
            float tscore = gScore[index] + nite::distance(ccoors, zcoors);
            if(tscore >= gScore[z]){
                continue;
            }
            cameFrom[z] = index;
            gScore[z] = tscore;
            fScore[z] = gScore[z] + heuristicmd(zcoors, tcoors);
        }
    }

	// erase start
	if((route.route.end()-1)->index == start){
		route.route.erase(route.route.end()-1);
	}

	route.start = start;
	route.end = target;
	return route; 
}