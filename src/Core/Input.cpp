#include "Input.hpp"


Vector<Game::InputFrameBuffer> Game::Input::getBuffer(){
    if(capturing || buffer.size() == 0){
        return Vector<Game::InputFrameBuffer>();
    }
    auto copy = this->buffer;
    for(int i = 1; i < copy.size(); ++i){
        copy[i].lastStroke = copy[i].lastStroke - buffer[i - 1].lastStroke;
    }    
    copy[0].lastStroke = 0;
    buffer.clear();
    return copy;
}

Game::Input::Input(){
    this->mapping = Game::InputDefaultMapping;
    capturing = false;
    lastFrame = 0;
}

void Game::Input::update(bool ignore){
    if(capturing){
        ++lastFrame;
    }
    if(capturing && lastFrame >= InputMaxFrameBuffer){
        capturing = false;
        lastFrame = 0;        
    }
    for(auto key : this->mapping){
        auto gk = key.second;
        auto nk = key.first;
        bool pressed = !ignore && nite::keyboardPressed(nk);
        bool released = !ignore && nite::keyboardReleased(nk);
        if(!pressed && !released){
            continue;
        }
        if(!capturing){
            capturing = true;
            buffer.clear();
            lastFrame = 0;
        }
        Game::InputFrameBuffer in;
        in.lastStroke = nite::getTicks();
        in.key = gk;
        in.type = pressed ? Game::PressType::Pressed : Game::PressType::Released;
        buffer.push_back(in);
    }
}


bool Game::InputSimulator::isKeyPress(UInt8 key){
    auto it = keys.find(key);
    if(it == keys.end()){
        return false;
    }
    return keys[key] == Game::PressType::Pressed;
}

void Game::InputSimulator::update(){
    for(int i = 0; i < this->queue.size(); ++i){
        auto &key = this->queue[i];
        if(nite::getTicks()-this->nextKey > key.lastStroke){
            auto it = this->keys.find(key.key);
            if(it != this->keys.end()){
                this->keys[key.key] = key.type;
                this->queue.erase(this->queue.begin() + i);
                --i;
                continue;
            }
            this->nextKey = nite::getTicks();
        }
    }
}