#include "Particles.hpp"

static int lastId = 0;

void nite::ParticleEmitter::fixIndexes(){
    for(int i = 0; i < objects.size(); ++i){
        objects[i].index = i;
    }
}

void nite::ParticleEmitter::addObject(){
    nite::ParticleObject obj;
    obj.pos.set(0.0f);
    if(objects.size() > 0){
        auto &last = objects[objects.size() - 1];
        float an = nite::arctan(obj.pos.y - last.pos.y, obj.pos.x - last.pos.x);
        float r = templ.size.x * templ.size.y;
        obj.pos.set(last.pos.x + nite::cos(an) * r,  last.pos.y + nite::sin(an) * r);
    }
    obj.size = templ.size;
    obj.color = templ.color;
    obj.alpha = templ.alpha;
    obj.angle = templ.angle;
    obj.index = 0;
    obj.tick = templ.startTick;
    obj.initTime = nite::getTicks();
    obj.lifetime = templ.lifetime == 0 ? 0 : (templ.varLifetime ? nite::randomInt(templ.lifetime * 0.85f, templ.lifetime * 1.15f) : templ.lifetime);
    this->objects.push_back(obj);
}

void nite::ParticleEmitter::setup(const nite::ParticleTemplate &templ, int units, UInt64 lifetime, const nite::Vec2 &region){
    if(units <= 0){
        return;
    }
    this->templ = templ;
    this->lifetime = lifetime;
    this->initTime = nite::getTicks();
    this->units = units;
    for(int i = 0; i < units; ++i){
        addObject();
    }
    fixIndexes();
    this->region = region;  
}

void nite::ParticleEmitter::render(float x, float y){
    auto pattern = [&](int p, nite::ParticleObject &obj, int tick){
        switch(p){
            case nite::ParticlePatternType::SIMPLE_CIRCULAR: {
                float offset = 0.0f;
                float crad = nite::distance(nite::Vec2(x, y), obj.pos);
                float _x = x + nite::cos(nite::toRadians(tick + offset)) * crad;
                float _y = y + nite::sin(nite::toRadians(tick + offset)) * crad;
                obj.pos.cInterpDiscrete(nite::Vec2(_x, _y), 0.10f);
            } break;
            case nite::ParticlePatternType::VARIANT_RADIUS: {
                float an = nite::arctan(obj.pos.y - y, obj.pos.x - x);
                float crad = nite::distance(nite::Vec2(x, y), obj.pos);
                float mrad = (region.x + region.y) * 0.5f;
                float trad = templ.getProp("P_VARIANT_RADIUS_END", mrad);
                nite::cInterpDiscrete(crad, (tick % 100) > 50 ? mrad : trad, 0.10f); // switches on half of the ticks
                float _x = x + nite::cos(an) * crad;
                float _y = y + nite::sin(an) * crad;
                obj.pos.cInterpDiscrete(nite::Vec2(_x, _y), 0.10f);    
            } break;
            case nite::ParticlePatternType::SIMPLE_SINE: {
                float offset = (360.0f / this->units) * obj.index;
                float _x = x - region.x + (tick % (int)(region.x)) + offset;
                float _y = y + nite::cos(nite::toRadians(_x)) * region.y * 0.5f;
                obj.pos.cInterpDiscrete(nite::Vec2(_x, _y), 0.10f);                
                if(tick > 0 && tick % (360 * 16) == 0){
                    nite::print(tick);
                    obj.dead = true;
                }
            } break;
            case nite::ParticlePatternType::RAISING: {
                int _x = x;
                // int _y = y + region.y * 0.5f - tick %;
            } break;            
            default:
                return;
        }
    };
    
    auto transform = [&](int t, nite::ParticleObject &obj, int tick){
        switch(t){
            case nite::ParticleTransType::SHINY: {
                
            } break;
            default:
                return;
        }
    };
    auto renderObj = [&](int z, nite::ParticleObject &obj){
        nite::setDepth(z);
        nite::setColor(obj.color.r, obj.color.r, obj.color.r, obj.alpha / 100.0f);
        templ.texture.draw(obj.pos.x, obj.pos.y, obj.size.x, obj.size.y, 0.5f, 0.5f, obj.angle);
    };
    bool update = false;
    if(nite::getTicks()-lastTick > templ.tickrate){
        update = true;
        lastTick = nite::getTicks();
    }    
    if(objects.size() < this->units){
        addObject();
        fixIndexes();
    }
    Vector<int> remove;
    bool erased = false;
    for(int j = 0; j < objects.size(); ++j){
        auto &obj = objects[j];
        for(int i = 0; i < templ.patterns.size(); ++i){
            pattern(templ.patterns[i], obj, obj.tick);
            transform(templ.patterns[i], obj, obj.tick);
            renderObj(templ.dynamicY ? -y : nite::DepthMiddle, obj);
        }
        if(update){
            ++obj.tick;
        }
        if(obj.dead){
            erased = true;
            objects.erase(objects.begin() + j);
            --j;
        }
        if(obj.lifetime > 0 && nite::getTicks()-obj.initTime > obj.lifetime){
            obj.dead = true;
        }
    }
    if(erased){
        fixIndexes();
    }
    
}


int nite::ParticleDevice::emit(nite::ParticleEmitter &emitter){
    emitter.initTime = nite::getTicks();
    emitters[++lastId] = emitter;
    return lastId;
}

void nite::ParticleDevice::render(float x, float y){
    Vector<int> remove;
    for(auto &emitter : emitters){
        emitter.second.render(x, y);
        if(nite::getTicks()-emitter.second.initTime > emitter.second.lifetime){
            remove.push_back(emitter.first);
        }
    }
    for(int i = 0; i < remove.size(); ++i){
        emitters.erase(remove[i]);
    }
}