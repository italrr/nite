#include "Engine/Input.hpp"
#include "Dice.hpp"

Dice::Dice(){
    shuffling = false;
}

void Dice::init(const Shared<GameType> &gType){
    this->n = gType->diceType.n;
    this->from = gType->diceType.from;
    this->to = gType->diceType.to;
    this->gType = gType;
    for(int i = 0; i < this->n; ++i){
        this->values.push_back(this->from);
    }
    this->frameSize.set(64.0f);
    this->texture.load("data/texture/dice_atlas.png");
}

void Dice::set(const Vector<int> &values){
    this->values = values;
}

void Dice::shuffle(){
    this->shuffling = !this->shuffling;
    this->lastShuffle = nite::getTicks();
    if(!this->shuffling){
        rotation = nite::fmod(rotation, 45.0f);
    }
}

void Dice::update(){
    if(shuffling && nite::getTicks()-this->lastShuffle > 100){
        preRotation += 90.0f;
        this->lastShuffle = nite::getTicks();
        for(int i = 0; i < n; ++i){
            values[i] = nite::randomInt(from, to);
        }
    }else
    if(!shuffling){
        preRotation = 0.0f;
    }
    nite::lerpDiscrete(rotation, preRotation, 0.25f);
}

void Dice::draw(float x, float y, bool vertical){
    float xorint = vertical ? 0.0f : frameSize.x*1.15f;
    float yorient = vertical ? frameSize.y*1.15f : 0.0f;
    auto inTextSize = nite::Vec2(frameSize); 
    for(int i = 0; i < n; ++i){
        nite::Vec2 P = nite::Vec2(x + xorint*i, y + yorient*i)-nite::Vec2(0.5f)*frameSize;
        if(nite::mousePressed(nite::butLEFT) && nite::isMouseInRegion(P, P + frameSize)){
            shuffle();
        }        
        auto inTextCoors = nite::Vec2((values[i]-1)*frameSize.x, 0.0f); 
        // if(!shuffling){
            nite::setColor(0.0f, 0.0f, 0.0f, 0.70f);
            auto objShadow = this->texture.draw(x + xorint*i + 4, y + yorient*i + 4, frameSize.x, frameSize.y, 0.5f, 0.5f, rotation);
            objShadow->smooth = true;
            objShadow->region.set(inTextCoors, inTextSize);            
        // }
        nite::setColor(1.0f, 1.0f, 1.0f, 1.0f);
        auto obj = this->texture.draw(x + xorint*i, y + yorient*i, frameSize.x, frameSize.y, 0.5f, 0.5f, rotation);
        obj->smooth = true;
        obj->region.set(inTextCoors, inTextSize);
    }
}