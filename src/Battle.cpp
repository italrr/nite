#include "Battle.hpp"
#include "Engine/Input.hpp"

Game::Battle::Battle(){
    this->status = Game::BattleStatus::BATTLE_READY;
    
    this->dialog = Shared<Game::DialogHook>(new Game::DialogHook());

    this->dialog->onReset = [&](){
        // if(textWin.get() != NULL){
        //     std::dynamic_pointer_cast<nite::WindowUI>(textWin)->close();
        // }
        // if(emtWin.get() != NULL){
        //     std::dynamic_pointer_cast<nite::WindowUI>(emtWin)->close();
        // } 
    };

    this->dialog->onUpdateText = [&](){
        // updWinValue(textWin, currenText);

        auto ftbox = batWin->getComponentById("text-box");
        if(ftbox.get() != NULL && ftbox->type == "text"){
            auto text = std::dynamic_pointer_cast<nite::TextUI>(ftbox);
            text->setText(dialog->currenText);
        }else{
            nite::print("Battle::Battle::onUpdateText: cannot find 'text-box'");
        }        

    };

    this->dialog->onNextLine = [&](const Shared<DialogLine> &line){
        // updTextColor(emtWin, line->color);
        // updWinValue(emtWin, line->emitter);

        // updWinBorderColor(textWin, line->color);
        // updWinBorderColor(emtWin, line->color);
    };

}

void Game::Battle::start(const Vector<Shared<Game::Entity>> &groupA, const Vector<Shared<Game::Entity>> &groupB){
    if(!empty.isLoaded()){
        empty.load("data/texture/empty.png");
    }

    dialog->add("betsy", "this is a line of text", nite::Color("#d20021"));
    dialog->add("betsy", "i thought what id do was id pretend i was one of those death mutes", nite::Color("#d20021"));
    dialog->add("betsy", "this is a line of text", nite::Color("#d20021"));
    dialog->add("betsy", "i thought what id do was id pretend i was one of those death mutes", nite::Color("#d20021"));
    dialog->add("betsy", "this is a line of text", nite::Color("#d20021"));
    dialog->add("betsy", "i thought what id do was id pretend i was one of those death mutes", nite::Color("#d20021"));

    // actBarPos.set(0.0f, nite::getHeight() * (1.0f - 0.33f));
    dialog->start();
    // dialog->setStaticBorderColor(true, nite::Color("#6f3997"));
    // dialog->setBgColor(nite::Color("#08021c"));
    // // default dialog bg #0f0f0f

    // if(optWin.get() != NULL){
    //     std::dynamic_pointer_cast<nite::WindowUI>(optWin)->close();
    // }

    // auto optPos = (nite::Vec2(32.0f, 16.0f) + actBarPos) + nite::Vec2(720.0f, 0.0f);
    


    if(!font.isLoaded()){
        font.load("data/font/BebasNeueRegular-X34j2.otf", 36, 2);
    }
    if(!subFont.isLoaded()){
        subFont.load("data/font/BebasNeueRegular-X34j2.otf", 20, 1.0f);
    }
    if(!empty.isLoaded()){
        empty.load("data/texture/empty.png");
    }
    
    if(batWin.get() != NULL){
        std::dynamic_pointer_cast<nite::WindowUI>(batWin)->close();
    }
    batWin = nite::UI::build("data/ui/battle_menu.json");
    if(batWin.get() != NULL && batWin->type == "window"){
        // setup window
        auto win =  std::dynamic_pointer_cast<nite::WindowUI>(batWin);
        float x = 0.0f;
        float y = nite::getHeight() * (1.0f - 0.30f);
        float w = nite::getWidth();
        float h = nite::getHeight()-y;
        win->setPosition(nite::Vec2(x, y));
        win->setSize(nite::Vec2(w, h));     

        auto ftbox = win->getComponentById("text-box");
        if(ftbox.get() != NULL && ftbox->type == "text"){
            auto text = std::dynamic_pointer_cast<nite::TextUI>(ftbox);
            text->setFont(font);
        }else{
            nite::print("Battle::start: cannot find 'text-box'");
        }

    }

    // dialog->reset();
    // if(optWin.get() != NULL && optWin->type == "window"){
    //     auto win =  std::dynamic_pointer_cast<nite::WindowUI>(optWin);
    //     float w = nite::getWidth() - optPos.x;
    //     float h = nite::getHeight() - optPos.y;
    //     win->setPosition(optPos);
    //     win->setSize(nite::Vec2(w - 16.0f, h - 10.0f));
    // }
}

void Game::Battle::end(){

}

void Game::Battle::step(){
    dialog->step();

    if(nite::keyboardPressed(nite::keySPACE)){
    	dialog->cont();
    }
}

void Game::Battle::render(){
    // action bar
	// nite::setRenderTarget(nite::RenderTargetUI);
	// nite::setDepth(nite::DepthBottom);
    // nite::setColor(0.0f, 0.0f, 0.0f, 0.65f);
    // empty.draw(actBarPos.x, actBarPos.y, nite::getWidth(), nite::getHeight()-actBarPos.y, 0.0f, 0.0f, 0.0f);
    // 
    // dialog->render();
}