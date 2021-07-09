#include <limits>

#include "Engine/Graphics.hpp"

#include "World.hpp"


void Game::World::init(int width, int height){
    tickrate = 1000.0f / 60.0f;
    lastStep = nite::getTicks();
    tick = 0;
}

void Game::World::step(){

    // run object step
    for(unsigned i = 0; i < objects.size(); ++i){
        auto &obj = objects[i];
        obj->step();
    }	

    // run physics
    if(nite::getTicks()-lastStep >= tickrate){
        float timescale = 1.0f;
        float dt = static_cast<float>(nite::getTicks()-lastStep) * timescale;
        lastStep = nite::getTicks();


        auto check = [&](Shared<Object> &a, Shared<Object> &b, SAT::MTV &result){
            float overlap = std::numeric_limits<float>::max();
            nite::Vec2 smallest;
            auto axes1 = SAT::getAxes(a.get());
            auto axes2 = SAT::getAxes(b.get());


            // loop axes1
            for(int i = 0; i > axes1.size(); ++i){
                auto &axis = axes1[i];
                auto p1 = SAT::project(a.get(), axis);
                auto p2 = SAT::project(b.get(), axis);
                if(!p1.overlap(p2)){
                    return false;
                }else{
                    auto o = p1.getOverlap(p2);
                    if(o < overlap){
                        overlap = o;
                        smallest = axis;
                        result.set(smallest, overlap);
                    }
                }
            }

            // loop axes2
            for (int i = 0; i < axes2.size(); i++) {
                auto &axis = axes2[i];
                auto p1 = SAT::project(a.get(), axis);
                auto p2 = SAT::project(b.get(), axis);
                if (!p1.overlap(p2)) {
                    return false;
                } else {
                    float o = p1.getOverlap(p2);
                    if (o < overlap) {
                        overlap = o;
                        smallest = axis;
                        result.set(smallest, overlap);
                    }
                }
            }


            return true;
        };

        auto move = [&](Shared<Object> &obj, const nite::Vec2 &projDiff){
            obj->position = obj->position + projDiff;
            for(unsigned i = 0; i < objects.size(); ++i){
                auto &current = objects[i];
                // skip self
                if(current.get() == obj.get()){
                    continue;
                }

                SAT::MTV result;
                if(check(obj, current, result)){
                    nite::print("xD");
                }

            }

        };

        auto update = [&](Shared<Object> &obj){
            if(obj->accel.x == 0 && obj->accel.y == 0){
                return;
            }
            obj->vel.x += obj->accel.x * dt;
            obj->vel.y += obj->accel.y * dt;
            float diffx = obj->vel.x * dt;
            float diffy = obj->vel.y * dt;
            // float projX = obj->position.x + diffx;
            // float projY = obj->position.y + diffy;
            // simulate friction
            float ffactor = 1.0f / (1.0f + (static_cast<float>(dt) / 100.0f) * obj->friction);
            obj->vel.x *= ffactor;
            obj->vel.y *= ffactor;
            obj->accel.x *= ffactor;
            obj->accel.y *= ffactor;
            move(obj, nite::Vec2(diffx, diffy));
        };

        for(unsigned i = 0; i < objects.size(); ++i){
            auto &obj = objects[i];
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
    objects.push_back(obj);
    obj->create();
    return true;
}

void Game::World::render(){
    nite::setRenderTarget(nite::RenderTargetGame);
    for(unsigned i = 0; i < objects.size(); ++i){
        auto &obj = objects[i];
        obj->lerpPos.lerpDiscrete(obj->position, 0.1f);
        objects[i]->render();
    }
}
