#include "Dialog.hpp"
#include <memory>

Game::DialogInstance::DialogInstance(){
    reset();
}

void Game::DialogInstance::add(const String &emt, const String &text, const nite::Color &color){
    auto line = std::make_shared<Game::DialogLine>(Game::DialogLine(emt, text, color));
    this->lines.push_back(line);
}

void Game::DialogInstance::start(const nite::Vec2 &pos, int width, int nlines){
    if(done){
        return;
    }

    static const float borderWdith = 5.0f;

    if(!font.isLoaded()){
        font.load("data/font/BebasNeueRegular-X34j2.otf", 36, 2);
    }
    if(!subFont.isLoaded()){
        subFont.load("data/font/BebasNeueRegular-X34j2.otf", 20, 1.0f);
    }


    

    textWin = nite::UI::build("data/ui/dialog_window.json");
    emtWin = nite::UI::build("data/ui/dialog_window.json");

    auto twin = std::dynamic_pointer_cast<nite::WindowUI>(textWin);
    auto emwin = std::dynamic_pointer_cast<nite::WindowUI>(emtWin);

    float bwidth = 4;
    float padd = 4;
    float h = font.getHeight() * nlines + bwidth * 2 + padd * 2;

    twin->setBorderThickness(borderWdith);
    twin->setPosition(nite::Vec2(0.0f, 42.0f) + pos);
    twin->setSize(nite::Vec2(width + padd * 2, h));
    twin->removeCornerPattern();
    if(twin->children.size() > 0 && twin->children[0]->type == "text"){
        auto text = std::dynamic_pointer_cast<nite::TextUI>(twin->children[0]);
        text->setPadding(nite::Vec2(padd * 2 + bwidth * 2, padd * 2));
        text->setFont(font);
    }else{
        nite::print("DialogInstance::start: missing text child");
    }


    bwidth = 3;
    padd = 3;

    emwin->setBorderThickness(borderWdith);
    emwin->setPosition(pos);
    emwin->setSize(nite::Vec2(width * 0.33f, subFont.getHeight() + bwidth * 2 + padd * 2));
    emwin->removeCornerPattern();

    if(emwin->children.size() > 0 && emwin->children[0]->type == "text"){
        auto text = std::dynamic_pointer_cast<nite::TextUI>(emwin->children[0]);
        text->setPadding(nite::Vec2(padd * 2 + bwidth * 2, padd * 2));
        text->setFont(subFont);
    }else{
        nite::print("DialogInstance::start: missing text child");
    }    


    
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