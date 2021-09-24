#include <limits>

#include "Engine/Graphics.hpp"

#include "World.hpp"


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

    // run object step
    for(auto &it : objects){
        auto &obj = it.second;
        obj->step();  
        if(useCameraFollow && obj->symRefId == followObject){
            static const float mu = 0.25f;

            nite::Vec2 &v = obj->position;
            nite::setView(true, nite::RenderTargetGame);
            nite::Vec2 p = nite::getView(nite::RenderTargetGame);
            cameraFollowNewPos = v - nite::getSize() * 0.5f;


	        p.lerpDiscrete(cameraFollowNewPos, mu);
	        setViewPosition(p, nite::RenderTargetGame);            

        }
    }


    // run physics
    if(nite::getTicks()-lastStep >= tickrate){
        float timescale = 1.0f;
        float dt = static_cast<float>(nite::getTicks()-lastStep) * timescale;
        lastStep = nite::getTicks();

        auto update = [&](Shared<Object> &obj){
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
}

bool Game::World::add(const Shared<Object> &obj){
    objects[obj->symRefId] = obj;
    obj->create();
    return true;
}

void Game::World::render(){
    nite::setRenderTarget(nite::RenderTargetGame);
    for(auto &it : objects){
        auto &obj = it.second;
        obj->lerpPos.lerpDiscrete(obj->position, 0.25f);
        obj->render();        
    }
}
