#include "../Engine/Input.hpp"
#include "ListMenu.hpp"

Shared<Game::UIListMenuOption> Game::UIListMenu::addOption(const String &label,  const std::function<void(UIListMenuOption *opt)> &callback){
    
    auto option = std::make_shared<UIListMenuOption>(UIListMenuOption());
    option->label = label;
    option->callback = callback;
    this->options.push_back(option);

    recalculate();

    return option;
}


void Game::UIListMenu::setTitle(bool v, const String &title){
    this->title = title;
    this->useTitle = v;
    recalculate();
}

void Game::UIListMenu::recalculate(){
    if(theme.get() == NULL){
        return;
    }
    float h = 0.0f;
    for(int i = 0; i < options.size(); ++i){
        h += theme->regularFont.getHeight() + 4;
    }
    if(useTitle){
        h +=  theme->bigFont.getHeight() + 4;
    }
    size.y = h + margin.y;
    if(useTitle){
        auto maxWidth = theme->bigFont.getWidth(title);
        if(size.x < maxWidth*1.25f){
            size.x = maxWidth*1.25f;
        }
    }
}

void Game::UIListMenu::clear(){
    size.y = 0.0f;
    selected = 0;
    options.clear();
    selArrowPos = nite::Vec2(position.x + 4 + margin.x * 0.5f, position.y + margin.y * 0.5f + selected * (theme->bigFont.getHeight() + 4) + theme->bigFont.getHeight() * 0.5f + (useTitle ? theme->bigFont.getHeight() + 4 : 0));
    recalculate();
}

Game::UIListMenu::UIListMenu(){
    diagArrowFlip = false;
    useTitle = false;
    allowLineBreaking = true;    
    diagArrowTick = nite::getTicks();
    visible = false;
    interact = false;
}

void Game::UIListMenu::onCreate(){
    clear();
    this->margin.set(15.0f);
    this->selArrowSize.set(16.0f);
    this->visible = false;
    this->interact = true;
    if(!selArrowWhite.isLoaded()){
        selArrowWhite.load("data/texture/ui/arrow_white.png", nite::Color(1.0f, 0.0f, 0.0f, 1.0f));
    }     
}

void Game::UIListMenu::onDestroy(){

}

void Game::UIListMenu::step(){

    if(!visible){
        return;
    }

    if(nite::getTicks()-diagArrowTick > 320){
        diagArrowTick = nite::getTicks();
        diagArrowFlip = !diagArrowFlip;
    }                

    nite::lerpDiscrete(diagArrowOffset, diagArrowFlip ? 0.0f : 100.0f, diagArrowFlip ? 0.12f : 0.04f);
    nite::Vec2 selArrPosTarget = nite::Vec2(position.x + 4 + margin.x * 0.5f + 8.0f * (diagArrowOffset / 100.0f), position.y + margin.y * 0.5f + selected * (theme->regularFont.getHeight() + 4) + theme->regularFont.getHeight() * 0.5f + (useTitle ? theme->bigFont.getHeight() + 4 : 0));
    selArrowPos.lerpDiscrete(selArrPosTarget, 0.10f);

    if(!interact){
        return;
    }

    if(nite::keyboardPress(nite::keyUP)){
        --selected;
        if(selected < 0){
            selected = options.size()-1;
        }
    }else
    if(nite::keyboardPress(nite::keyDOWN)){
        ++selected;
        if(selected >= options.size()){
            selected = 0;
        }
    }    

    if(nite::keyboardPressed(nite::keyZ)){
        interact = false;
        options[selected]->callback(options[selected].get());
    }
}

void Game::UIListMenu::setVisible(bool v){
    if(v){
        interact = true;
    }
    visible = v;
}

void Game::UIListMenu::setPosition(const nite::Vec2 &position){
    this->position.set(position);
    nite::Vec2 selArrPosTarget = nite::Vec2(position.x + 4 + margin.x * 0.5f, position.y + margin.y * 0.5f + selected * (theme->bigFont.getHeight() + 4) + theme->bigFont.getHeight() * 0.5f + (useTitle ? theme->bigFont.getHeight() + 4 : 0));
    selArrowPos.lerpDiscrete(selArrPosTarget, 1.0f);
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
    if(useTitle){
        nite::setColor(theme->solidColor);
        String string = "\""+title+"\"";
        theme->bigFont.draw(string, p.x + size.x * 0.5f - theme->bigFont.getWidth(string) * 0.50f, p.y + margin.y * 0.5f, 0.0f, 0.0f, 0.0f);
        ycursor += theme->bigFont.getHeight() + 4;
    }
    for(int i = 0; i < options.size(); ++i){
        nite::setColor(theme->fontColor);
        auto texp = nite::Vec2(p.x + margin.x * 0.5f + selArrowSize.x + 2, p.y + ycursor + 2);
        theme->regularFont.draw(options[i]->label, texp.x, texp.y, 0.0f, 0.0f, 0.0f);
        ycursor += theme->regularFont.getHeight() + 4;
    }

    nite::setColor(theme->borderColor);
    auto arrefsha = selArrowWhite.draw(selArrowPos.x + 2, selArrowPos.y + 2, selArrowSize.x, selArrowSize.y, 0.5f, 0.5f, 270.0f);
    nite::setColor(theme->solidColor);
    auto arref = selArrowWhite.draw(selArrowPos.x, selArrowPos.y, selArrowSize.x, selArrowSize.y, 0.5f, 0.5f, 270.0f);



}

void Game::UIYesNoMenu::setup(const std::function<void(UIListMenuOption *opt)> &yescb, const std::function<void(UIListMenuOption *opt)> &nocb){
    clear();
    addOption("Yes", yescb);
    addOption("No", nocb);
}

