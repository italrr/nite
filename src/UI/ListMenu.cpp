#include "ListMenu.hpp"

Shared<Game::UIListMenuOption> Game::UIListMenu::addOption(const String &label,  const std::function<void(UIListMenuOption *opt)> &callback){
    
    auto option = std::make_shared<UIListMenuOption>(UIListMenuOption());
    option->label = label;
    option->callback = callback;
    this->options.push_back(option);

    recalculate();

    return option;
}

void Game::UIListMenu::recalculate(){
    if(theme.get() == NULL){
        return;
    }
    float h = 0.0f;
    for(int i = 0; i < options.size(); ++i){
        h += theme->bigFont.getHeight() + 4;
    }
    size.y = h + margin.y;
}

void Game::UIListMenu::clear(){
    size.y = 0.0f;
    options.clear();
    recalculate();
}

void Game::UIListMenu::onCreate(){
    this->margin.set(15.0f);
    this->selArrowSize.set(16.0f);
    this->selected = 0;
    if(!selArrowWhite.isLoaded()){
        selArrowWhite.load("data/texture/ui/arrow_white.png", nite::Color(1.0f, 0.0f, 0.0f, 1.0f));
    }     
}

void Game::UIListMenu::onDestroy(){

}

void Game::UIListMenu::step(){

}

void Game::UIListMenu::setVisible(bool v){
    visible = v;
}

void Game::UIListMenu::setSize(const nite::Vec2 &size){
    this->size.x = size.x;
    recalculate();
}

void Game::UIListMenu::render(){
   if(theme.get() == NULL || !visible){
        return;
    }
    nite::Vec2 p = position;
    nite::setColor(theme->borderColor);
    theme->base.draw(p.x + 4, p.y + 4, size.x, size.y, 0.0f, 0.0f, 0.0f);
    nite::setColor(theme->backgroundColor);
    theme->base.draw(p.x, p.y, size.x, size.y, 0.0f, 0.0f, 0.0f);

    float ycursor = margin.y * 0.5f;
    nite::setColor(theme->fontColor);
    for(int i = 0; i < options.size(); ++i){
        nite::setColor(theme->fontColor);
        auto texp = nite::Vec2(p.x + margin.x * 0.5f + selArrowSize.x + 2, p.y + ycursor + 2);
        theme->bigFont.draw(options[i]->label, texp.x, texp.y, 0.0f, 0.0f, 0.0f);
        ycursor += theme->bigFont.getHeight() + 4;
        if(selected == i){
            nite::setColor(theme->backgroundColor);
            auto arrefsha = selArrowWhite.draw(p.x + 4 + margin.x * 0.5f + 2, texp.y + 2 + theme->bigFont.getHeight() * 0.5f, selArrowSize.x, selArrowSize.y, 0.5f, 0.5f, 270.0f);
            nite::setColor(theme->solidColor);
            auto arref = selArrowWhite.draw(p.x + 4 + margin.x * 0.5f, texp.y + theme->bigFont.getHeight() * 0.5f, selArrowSize.x, selArrowSize.y, 0.5f, 0.5f, 270.0f);
        }
    }





}