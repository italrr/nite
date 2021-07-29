#include "Dialog.hpp"
#include <memory>


Game::DialogInstance::DialogInstance(){
    reset();
}

void Game::DialogInstance::add(const String &emt, const String &text, const nite::Color &color){
    auto line = std::make_shared<Game::DialogLine>(Game::DialogLine(emt, text, color));
    this->lines.push_back(line);
}

void Game::DialogInstance::start(const nite::Vec2 &pos, const nite::Vec2 &size){
    if(done){
        return;
    }

    textWin = nite::UI::build("data/ui/dialog_window.json");
    emtWin = nite::UI::build("data/ui/dialog_window.json");

    auto twin = std::dynamic_pointer_cast<nite::WindowUI>(textWin);
    auto emwin = std::dynamic_pointer_cast<nite::WindowUI>(emtWin);
    twin->setSize(size);
    twin->setPosition(nite::Vec2(0.0f, 42.0f) + pos);

    emwin->setSize(nite::Vec2(size.x * 0.25f, size.y * 0.36f));
    emwin->setPosition(pos);
    
    
    updWinValue(textWin, "");
    updWinValue(emtWin, "Tester");
    targetText = "aaaa aaaa aaaa aaaa aaaa aaaa aaaa aaaa aaaa aaaa aaaa aaaa aaaa aaaa aaaa aaaa aaaa aaaa";

    nite::print("[debug] started dialog");
}

void Game::DialogInstance::reset(){
    done = false;
    targetText = "";
    currenText = "";
    currentDiag = 0;
    currentChar = 0;
    lastChar = nite::getTicks();
    if(textWin.get() != NULL){
        auto casted = std::dynamic_pointer_cast<nite::WindowUI>(textWin);
        casted->close();
    }
}

void Game::DialogInstance::updWinValue(Shared<nite::BaseUIComponent> &win, const String &newval){
    if(win.get() != NULL && win->children.size() > 0){
        if(win->children[0]->type == "text"){
            auto txt = std::dynamic_pointer_cast<nite::TextUI>(win->children[0]);
            txt->setText(newval);
        }else{
            nite::print("DialogInstance::updWinValue: no text was found on the window");
        }
    }else{
        nite::print("DialogInstance::updWinValue: window doesn't exist");
    }
}

void Game::DialogInstance::step(){
    if(nite::getTicks()-lastChar < 20 || currenText.size() >= targetText.size()){
        return;
    }
    
    ++currentChar;

    currenText = targetText.substr(0, currentChar);

    updWinValue(textWin, currenText);

    
    lastChar = nite::getTicks();
}

void Game::DialogInstance::render(){

}