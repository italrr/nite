#include "Input.hpp"


Game::InputCompacter::InputCompacter(){
    for(int i = 0; i < Key::total; ++i){
        states[i] = Game::PressType::Released;
    }
}

UInt16 Game::InputCompacter::getCompat(){
    UInt16 compat = 0;
    int bit = 1;
    for(int i = 0; i < Key::total; ++i){
        compat =  compat | ((states[i] == PressType::Pressed) ? (1 << bit) : 0);
        bit *= 2;
    }
    return compat;
}

bool Game::InputCompacter::set(UInt8 key, UInt16 v){
    if(key >= Game::Key::total){
        return false;
    }
    states[key] = v;
    return true;
}

void Game::InputCompacter::setAll(UInt16 v){
    for(int i = 0; i < Key::total; ++i){
        states[i] = v;
    }
}

void Game::InputCompacter::loadCompat(UInt16 v){
    int bit = 1;
    for(int i = 0; i < Key::total; ++i){
        states[i] = v & (1 << bit) ? PressType::Pressed : PressType::Released;
        bit *= 2;
    }
}

bool Game::InputCompacter::isKeyPress(UInt8 key){
    if(key >= Game::Key::total){
        return false;
    }
    return states[key] == Game::PressType::Pressed;
}

Game::Input::Input(){
    this->mapping = Game::InputDefaultMapping;

}

void Game::Input::update(bool ignore){
    for(auto key : this->mapping){
        bool check = !ignore && (key.first > 200 ? nite::mouseCheck(key.first) : nite::keyboardCheck(key.first));
        auto nv = check ? Game::PressType::Pressed : Game::PressType::Released;
        if(states[key.second] != nv){
            states[key.second] = nv;
            lastChange = nite::getTicks();
            changed = true;
        }
    }
}
