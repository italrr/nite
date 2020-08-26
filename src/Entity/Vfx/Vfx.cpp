#include "Vfx.hpp"
#include "../../Engine/Shapes.hpp"
#include "../../Engine/Tools/Tools.hpp"

// common
static Dict<String, nite::Texture> common;

void Game::VfxBang::init(){
    // static const String fname = "data/texture/overworld/bang_ef.png";
    lifetime = 600;
    int n = nite::randomInt(8, 10);
    // if(common.find(fname) == common.end()){
    //     nite::Texture bang;
    //     bang.load(fname);
    //     common[fname] = bang;
    // }
    spread = 8.0f;
    // pattern = common[fname];
    int s = nite::randomInt(0, 360);
    float k = 360 / n;
    for(int i = 0; i < n; ++i){
        Game::OvwVfxFrag frag;
        float ang = s + k * i;
        frag.angle = ang;
        frag.mod = 0.0f;
        frag.ray = 0.0f;
        frag.position.x = 0.0f;
        frag.position.y = 0.0f;
        frag.size.x = nite::cos(nite::toRadians(ang)) * spread;
        frag.size.y = nite::sin(nite::toRadians(ang)) * spread;
        frag.color = nite::Color(1.0f, 1.0f, 1.0f, 1.0f);
        frag.alpha = 100.0f;
        frags.push_back(frag);
    }
    spread *= 2.0f;
    step();
}

void Game::VfxBang::step(){
    for(int i = 0; i < frags.size(); ++i){
        auto &frag = frags[i];


        UInt64 diff = nite::getTicks()-initTime;

        if(diff > lifetime / 2){
            nite::lerpDiscrete(frag.ray, spread, 0.15f);    
            if(nite::lerpDiscrete(frag.alpha, 0.0f, 0.35f)){
                destroy = true;
            }
        }
        nite::lerpDiscrete(frag.mod, spread * 6, 0.15f);
        frag.position.x = nite::cos(nite::toRadians(frag.angle)) * (frag.ray + frag.mod);
        frag.position.y = nite::sin(nite::toRadians(frag.angle)) * (frag.ray + frag.mod);
        frag.size.x = nite::cos(nite::toRadians(frag.angle)) * (frag.mod + spread);
        frag.size.y = nite::sin(nite::toRadians(frag.angle)) * (frag.mod + spread);
    } 
}

void Game::VfxBang::draw(){
    for(int i = 0; i < frags.size(); ++i){

        // frags[i].draw(this->pattern, this->position + nite::Vec2(nite::randomInt(-6, 6), nite::randomInt(-6, 6)));
        auto &frag = frags[i];
        nite::setColor(frag.color.r, frag.color.g, frag.color.b, frag.alpha / 100.0f);
        auto *ref = nite::Draw::Line(position.x + frag.position.x, position.y + frag.position.y, position.x + frag.size.x, position.y + frag.size.y);
        if(ref != NULL){
            ref->thickness = 4.0f;
        }
    }
}

void Game::VfxDevice::add(const Shared<OvwVfx> &eff, Int64 lifetime){
    eff->initTime = nite::getTicks();
    eff->lifetime = lifetime == -1 ? eff->lifetime : lifetime;
    effects.push_back(eff);
    eff->init();
}

void Game::VfxDevice::step(){
    for(int i = 0; i < effects.size(); ++i){
        auto ef = effects[i].get();
        ef->step();
        if(nite::getTicks()-ef->initTime > ef->lifetime || ef->destroy){
            effects.erase(effects.begin() + i);
            --i;
            continue;
        }
    }
}

void Game::VfxDevice::draw(){
    // remember to set the rendertarget when drawing this
    for(int i = 0; i < effects.size(); ++i){
        auto ef = effects[i].get();
        ef->draw();
    }    
}

void Game::VfxDevice::clear(){
    effects.clear(); // simply enough huh
}