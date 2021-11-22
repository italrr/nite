#include <limits>

#include "Engine/Graphics.hpp"
#include "Engine/Console.hpp"
#include "Engine/Shapes.hpp"
#include "World.hpp"

static bool showCMasks = false;
static nite::Console::CreateProxy cpShowCMasks("cl_show_cmasks", nite::Console::ProxyType::Bool, sizeof(bool), &showCMasks);

static bool showSMasks = false;
static nite::Console::CreateProxy cpShowSMasks("cl_show_smasks", nite::Console::ProxyType::Bool, sizeof(bool), &showSMasks);


void Game::World::init(int width, int height){
    tickrate = 1000.0f / 60.0f;
    lastStep = nite::getTicks();
    tick = 0;
    useCameraFollow = false;
}

void Game::World::setFollowObject(bool v, const String &followObject){
    this->useCameraFollow = v;
    this->followObject = followObject;
}

static bool testCollision(Game::Object *a, Game::Object *b, const nite::Vec2 &diff, nite::Vec2 &limit, nite::Vec2 &normal){
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
	if(limit.x < 0.0f) limit.x = 0.0f;
	if(limit.x > 1.0f) limit.x = 1.0f;	

	if(limit.y < 0.0f) limit.y = 0.0f;
	if(limit.y > 1.0f) limit.y = 1.0f;		

	return collision;
}


void Game::World::step(){

    auto checkNearEntity = [&](Shared<Object> &obj){
        if(obj->objType != ObjectType::ENTITY){
            return;
        }
        for(auto &it : objects){
            auto &obj2 = it.second;
            if(nite::distance(obj->position + obj->size * 0.5f, obj2->position + obj2->size * 0.5f) < 64.0f){
                nite::print("xD");
            }
        }    
    };

    // run object step
    for(auto &it : objects){
        auto &obj = it.second;
        obj->step();  
        // follow camera
        if(useCameraFollow && obj->symRefId == followObject){
            static const float mu = 0.25f;
            nite::Vec2 &v = obj->position;
            nite::setView(true, nite::RenderTargetGame);
            nite::Vec2 p = nite::getView(nite::RenderTargetGame);
            cameraFollowNewPos = v - nite::getSize() * 0.5f;

	        p.lerpDiscrete(cameraFollowNewPos, mu);
	        setViewPosition(p, nite::RenderTargetGame);            
        }
        // checkNearEntity(obj);
        
    }


    // run physics
    if(nite::getTicks()-lastStep >= tickrate){
        float timescale = 1.0f;
        float dt = static_cast<float>(nite::getTicks()-lastStep) * timescale;
        lastStep = nite::getTicks();

        auto update = [&](Shared<Object> &obj){
            // intertia
            if(obj->accel.x == 0 && obj->accel.y == 0){
                return;
            }
            obj->vel.x += obj->accel.x * dt;
            obj->vel.y += obj->accel.y * dt;
            nite::Vec2 diff(obj->vel.x * dt, obj->vel.y * dt);
            nite::Vec2 limit(1.0f);
            nite::Vec2 normal(0.0f);
            // simulate friction
            float ffactor = 1.0f / (1.0f + (static_cast<float>(dt) / 100.0f) * obj->friction);
            obj->vel.x *= ffactor;
            obj->vel.y *= ffactor;
            obj->accel.x *= ffactor;
            obj->accel.y *= ffactor;
            for(auto &it : objects){
                auto &obj2 = it.second;     
                if(obj2.get() == obj.get()){
                    continue;
                }
                if(testCollision(obj.get(), obj2.get(), diff, limit, normal)){				
                    if(limit.x == 0.0f && limit.y == 0.0f){
                        break;
                    }
                }    
            }       
            obj->position = obj->position + diff * limit;
        };

        for(auto &it : objects){
            auto &obj = it.second;
            auto origPos = obj->position;
            update(obj);
            if(obj->position.x != origPos.x || obj->position.y != origPos.y){
                // TODO
            }
        }

        ++tick;
    }

    // clean up detroyed obects
    if(removeQueue.size() > 0){
        for(int i = 0; i < removeQueue.size(); ++i){
            auto &symId = removeQueue[i];
            objects[symId]->onDestroy();
            objects.erase(symId);
        }  
        removeQueue.clear();
    }
}

bool Game::World::add(const Shared<Object> &obj){
    objects[obj->symRefId] = obj;
    obj->create();
    return true;
}

bool Game::World::remove(const String &symId){

    auto it = objects.find(symId);
    if(it == objects.end()){
        return false;
    }
    if(isToRemove(symId)){
        return true;
    }

    removeQueue.push_back(symId);
    it->second->destroyed = true;

    return true;
}

bool Game::World::remove(int refId){
    for(auto &it : objects){
        if(it.second->refId == refId){
            if(!isToRemove(it.second->symRefId)){
                removeQueue.push_back(it.second->symRefId);
            }
            it.second->destroyed = true;
            return true;
        }
    }
    return false;
}

bool Game::World::isToRemove(const String &symId){
    for(int i = 0; i < removeQueue.size(); ++i){
        if(removeQueue[i] == symId){
            return true;
        }
    }
    return false;
}

void Game::World::useMap(const Shared<nite::Map> &map){
    currentMap = map;
    for(int i = 0 ; i < map->masks.size(); ++i){
        // setup collisions
        if(map->masks[i]->type != nite::MapMaskType::COLLISION){
            continue;
        }
        auto mask = std::static_pointer_cast<nite::MapWallMask>(map->masks[i]);
        auto wmask = Shared<Game::WallMask>(new Game::WallMask());
        wmask->reshape(mask->size);
        wmask->setPosition(mask->position);
        this->wallMasks.push_back(wmask);
        this->add(wmask);
    }
    nite::print("[MAP] placed "+nite::toStr(this->wallMasks.size())+" mask(s)");
}

void Game::World::resetMap(){
    currentMap = Shared<nite::Map>(NULL);
    for(int i = 0; i < wallMasks.size(); ++i){
        remove(wallMasks[i]->symRefId);
    }
}

void Game::World::render(){
    nite::setRenderTarget(nite::RenderTargetGame);

    if(showCMasks){
        for(auto &it : objects){
            nite::setColor(1.0f, 0.0f, 0.0f, 0.08f);
            auto ref = nite::Draw::Rectangle(it.second->position.x, it.second->position.y, it.second->size.x, it.second->size.y, true, 0.0f, 0.0f, 0.0f);
            nite::setColor(1.0f, 0.0f, 0.0f, 0.75f);
            auto refline = nite::Draw::Rectangle(it.second->position.x, it.second->position.y, it.second->size.x, it.second->size.y, false, 0.0f, 0.0f, 0.0f);
            if(refline != NULL){
                 refline->thickness = 4.0f;
            }
        }
    }

    if(showSMasks && currentMap.get() != NULL){
        for(int i = 0; i < currentMap->masks.size(); ++i){
            auto &currentMask = currentMap->masks[i];
            switch(currentMask->type){
                case nite::MapMaskType::SIGN: {
                    static nite::Color useColor("#2460e1");
                    nite::setColor(useColor.r, useColor.g, useColor.b, 0.08f);
                    auto ref = nite::Draw::Rectangle(currentMask->position.x, currentMask->position.y, currentMask->size.x, currentMask->size.y, true, 0.0f, 0.0f, 0.0f);
                    nite::setColor(useColor.r, useColor.g, useColor.b, 0.75f);
                    auto refline = nite::Draw::Rectangle(currentMask->position.x, currentMask->position.y, currentMask->size.x, currentMask->size.y, false, 0.0f, 0.0f, 0.0f);
                    // nite::print(currentMask->position);
                    if(refline != NULL){
                        refline->thickness = 4.0f;
                    }
                } break;
                case nite::MapMaskType::TELEPORTER: {
                    static nite::Color useColor("#54ff05");
                    nite::setColor(useColor.r, useColor.g, useColor.b, 0.08f);
                    auto ref = nite::Draw::Rectangle(currentMask->position.x, currentMask->position.y, currentMask->size.x, currentMask->size.y, true, 0.0f, 0.0f, 0.0f);
                    nite::setColor(useColor.r, useColor.g, useColor.b, 0.75f);
                    auto refline = nite::Draw::Rectangle(currentMask->position.x, currentMask->position.y, currentMask->size.x, currentMask->size.y, false, 0.0f, 0.0f, 0.0f);
                    if(refline != NULL){
                        refline->thickness = 4.0f;
                    }
                } break;
                case nite::MapMaskType::EVENT: {

                } break;
            }
        }   
    }

    for(auto &it : objects){
        auto &obj = it.second;
        obj->lerpPos.lerpDiscrete(obj->position, 0.25f);
        obj->render();        
    }
}
