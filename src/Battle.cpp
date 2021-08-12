#include "Battle.hpp"
#include "Engine/Input.hpp"

Game::Battle::Battle(){
    this->state = Game::BattleState::BATTLE_READY;
    
    this->dialog = Shared<Game::DialogHook>(new Game::DialogHook());

    this->startTurn = 0;

    this->lastStChange = nite::getTicks();

    this->dialog->onReset = [&](){
        auto ftbox = batWin->getComponentById("text-box-object");
        if(ftbox.get() != NULL && ftbox->type == "text"){
            auto text = std::dynamic_pointer_cast<nite::TextUI>(ftbox);
            text->setText("");
        }else{
            nite::print("Battle::Battle::onReset: cannot find 'text-box-object'");
        }   
    };

    this->dialog->onUpdateText = [&](){
        // updWinValue(textWin, currenText);

        auto ftbox = batWin->getComponentById("text-box-object");
        if(ftbox.get() != NULL && ftbox->type == "text"){
            auto text = std::dynamic_pointer_cast<nite::TextUI>(ftbox);
            text->setText(dialog->currenText);
        }else{
            nite::print("Battle::Battle::onUpdateText: cannot find 'text-box-object'");
        }        

    };

    this->dialog->onNextLine = [&](const Shared<DialogLine> &line){
        // updTextColor(emtWin, line->color);
        // updWinValue(emtWin, line->emitter);

        // updWinBorderColor(textWin, line->color);
        // updWinBorderColor(emtWin, line->color);
    };

}

bool Game::Battle::isShowing(){
    return this->state != BattleState::BATTLE_READY;
}

void Game::Battle::setState(int nstate){
    this->lastStChange = nite::getTicks();
    this->state = nstate;
    nite::print("[debug] battle set new state "+nite::toStr(nstate));
}

void Game::Battle::start(const Vector<Shared<Game::Entity>> &groupA, const Vector<Shared<Game::Entity>> &groupB){
    if(!empty.isLoaded()){
        empty.load("data/texture/empty.png");
    }

    // TODO: handle if groupB is empty

    // prepare first line
    String names = groupB[0]->nickname;
    for(int i = 1; i < groupB.size(); ++i){
        names += i < groupB.size()-1 ? ", " : " and " + groupB[i]->nickname;
    }
    static const Vector<String> randomEnterPhrase = {"Oh, ", "Quick, ", "Look, "};
    dialog->add("", randomEnterPhrase[nite::randomInt(0, randomEnterPhrase.size())]+names+(groupB.size() > 1 ? " are " : " is ")+"approaching...", nite::Color("#d20021"));
    dialog->start();

    this->groupA = groupA;
    this->groupB = groupB;
    

    static const String mainFont = "MONOFONT.TTF";

    if(!font.isLoaded()){
        font.load("data/font/"+mainFont, 32, 2);
    }
    if(!subFont.isLoaded()){
        subFont.load("data/font/"+mainFont, 22, 2.0f);
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
        float y = nite::getHeight() * (1.0f - 0.25f);
        float w = nite::getWidth();
        float h = nite::getHeight()-y;
        win->setPosition(nite::Vec2(x, y));
        win->setSize(nite::Vec2(w, h));     

        // set text-box-object font
        auto ftbox = win->getComponentById("text-box-object");
        if(ftbox.get() != NULL && ftbox->type == "text"){
            auto text = std::dynamic_pointer_cast<nite::TextUI>(ftbox);
            text->setFont(font);
        }else{
            nite::print("Battle::start: cannot find 'text-box-object'");
        }

        // set buttons font
        auto optBox = win->getComponentById("options-box");
        if(optBox.get() != NULL && optBox->children.size() > 0 && optBox->children[0]->type == "panel"){
            auto panel = optBox->children[0];
            for(int i = 0; i < panel->children.size(); ++i){
                auto button = std::dynamic_pointer_cast<nite::ButtonUI>(panel->children[i]);
                button->setFont(subFont);
            }            
        }else{
            nite::print("Battle::start: cannot find option-box's inline panel");
        }



    }

    currentTurn = startTurn;

    setState(BattleState::BATTLE_START);
    setOptBoxVis(false);
    nite::print("[debug] battle start");

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

void Game::Battle::setOptBoxVis(bool v){
    if(batWin.get() == NULL){
        return;
    }
    auto optBox = batWin->getComponentById("options-box");
    if(optBox.get() == NULL || optBox->type != "panel"){
        return;
    }
    optBox->setVisible(v);
}

bool Game::Battle::isOptBoxVis(){
    if(batWin.get() == NULL){
        return false;
    }
    auto optBox = batWin->getComponentById("options-box");
    if(optBox.get() == NULL || optBox->type != "panel"){
        return false;
    }  
    return optBox->isVisible();
}

void Game::Battle::setDialogBoxVis(bool v){
    if(batWin.get() == NULL){
        return;
    }
    auto optBox = batWin->getComponentById("text-box");
    if(optBox.get() == NULL || optBox->type != "panel"){
        return;
    }
    optBox->setVisible(v);
}

bool Game::Battle::isDialogBoxVis(){
    if(batWin.get() == NULL){
        return false;
    }
    auto optBox = batWin->getComponentById("text-box");
    if(optBox.get() == NULL || optBox->type != "panel"){
        return false;
    }  
    return optBox->isVisible();
}

void Game::Battle::step(){
    dialog->step();

    const static UInt64 avgTransitionWait = 80;

    auto generateButton = [&](Shared<nite::BaseUIComponent> &container, const String &text){
        Jzon::Node json = Jzon::object();
        json.add("import", "data/ui/battle_menu_button.json");
        auto added = container->add(json);
        if(added.get() != NULL && added->type == "button"){
            auto button = std::dynamic_pointer_cast<nite::ButtonUI>(added);
            button->setText(text);
            button->setFont(subFont);
            return button;
        }else{
            nite::print("Battle::step::generateButton: no button was generated");
        }
        return Shared<nite::ButtonUI>(NULL);
    };


    auto generateEngageOptions = [&](){
        auto list = batWin->getComponentById("options-box-list");
        list->clearChildren();

        auto attkButton = generateButton(list, "ATTACK");
        if(attkButton.get() != NULL){
            attkButton->setBaseColor(nite::Color("#ff5400"));
            attkButton->setOnClick([&](const Shared<nite::ListenerInfo> &info, nite::BaseUIComponent *component){
                return;
            });
        }

        auto sayButton = generateButton(list, "SAY...");
        if(sayButton.get() != NULL){
            sayButton->setBaseColor(nite::Color("#5541aa"));
            sayButton->setOnClick([&](const Shared<nite::ListenerInfo> &info, nite::BaseUIComponent *component){
                return;
            });
        }


        auto offerButton = generateButton(list, "OFFER...");
        if(offerButton.get() != NULL){
            offerButton->setBaseColor(nite::Color("#937014"));
            offerButton->setOnClick([&](const Shared<nite::ListenerInfo> &info, nite::BaseUIComponent *component){
                return;
            });
        }


        auto escapeButton = generateButton(list, "BACK");
        if(escapeButton.get() != NULL){
            escapeButton->setBaseColor(nite::Color("#394739"));
            escapeButton->setOnClick([&](const Shared<nite::ListenerInfo> &info, nite::BaseUIComponent *component){
                setState(PRE_PICK_ACTION);
            });
        }                    

        list->fixNavIndexes();
        list->nav.current = 0;

    };

    auto generateEvadeOptions = [&](){
        auto list = batWin->getComponentById("options-box-list");
        list->clearChildren();

        auto parryButton = generateButton(list, "BLOCK");
        if(parryButton.get() != NULL){
            parryButton->setBaseColor(nite::Color("#ff5400"));
            parryButton->setOnClick([&](const Shared<nite::ListenerInfo> &info, nite::BaseUIComponent *component){
                return;
            });
        }

        auto evadeButton = generateButton(list, "DODGE");
        if(evadeButton.get() != NULL){
            evadeButton->setBaseColor(nite::Color("#5541aa"));
            evadeButton->setOnClick([&](const Shared<nite::ListenerInfo> &info, nite::BaseUIComponent *component){
                return;
            });
        }

        auto escapeButton = generateButton(list, "BACK");
        if(escapeButton.get() != NULL){
            escapeButton->setBaseColor(nite::Color("#394739"));
            escapeButton->setOnClick([&](const Shared<nite::ListenerInfo> &info, nite::BaseUIComponent *component){
                setState(PRE_PICK_ACTION);
            });
        }                    

        list->fixNavIndexes();
        list->nav.current = 0;

    };

    auto generateMainOptions = [&](){
        auto list = batWin->getComponentById("options-box-list");
        list->clearChildren();

        auto engButton = generateButton(list, "ENGAGE");
        if(engButton.get() != NULL){
            engButton->setBaseColor(nite::Color("#c92600"));
            engButton->setOnClick([&](const Shared<nite::ListenerInfo> &info, nite::BaseUIComponent *component){
                setState(Game::BattleState::PICK_ENGAGE_OPT);
                return;
            });
        }

        auto evButton = generateButton(list, "EVADE");
        if(evButton.get() != NULL){
            evButton->setBaseColor(nite::Color("#226e46"));
            evButton->setOnClick([&](const Shared<nite::ListenerInfo> &info, nite::BaseUIComponent *component){
                setState(Game::BattleState::PICK_EVADE_OPT);
                return;
            });            
        }

        auto bagButton = generateButton(list, "BAG");
        if(bagButton.get() != NULL){
            bagButton->setBaseColor(nite::Color("#4d34eb"));
            bagButton->setOnClick([&](const Shared<nite::ListenerInfo> &info, nite::BaseUIComponent *component){
                nite::print("BAG");
                return;
            });                 
        }

        auto escButton = generateButton(list, "ESCAPE");
        if(escButton.get() != NULL){
            escButton->setBaseColor(nite::Color("#fbad3c"));
            escButton->setOnClick([&](const Shared<nite::ListenerInfo> &info, nite::BaseUIComponent *component){
                nite::print("ESCAPE");
                return;
            });              
        }         

        list->fixNavIndexes();
        list->nav.current = 0;
    };


    switch(state){
        case BATTLE_START: {
            if(dialog->canCont() && nite::getTicks()-lastStChange > avgTransitionWait){
                setState(PRE_TURN);
            }
        } break;
        case PRE_TURN: {
            if(dialog->canCont() && nite::getTicks()-lastStChange > avgTransitionWait){
                decisions.clear();
                cdecision = 0;
                setState(TURN_START);
            }
        } break;   
        case TURN_START: {
            if(dialog->canCont() && nite::getTicks()-lastStChange > 0){
                // player chooses
                if(cdecision <= groupA.size()-1){
                    dialog->reset();
                    if(groupA.size() == 1){
                        dialog->add("", "What will you do?", nite::Color("#d20021"));
                    }else{
                        dialog->add("", "What will "+groupA[cdecision]->nickname+" do?", nite::Color("#d20021"));
                    }
                    dialog->start();                    
                    setState(PRE_PICK_ACTION);
                }else{
                    int adjcdecision = cdecision - (groupA.size()-1);
                    // goes through AI
                }
            }
        } break;             
        case PRE_PICK_ACTION: {
            if(dialog->canCont() && nite::getTicks()-lastStChange > 0){
                setState(PICK_ACTION);
                generateMainOptions();
                setOptBoxVis(true);
                setDialogBoxVis(false);                
            }
        } break;        
        case PICK_ACTION: {
            if(dialog->canCont() && nite::getTicks()-lastStChange > 0){

            }
        } break;
        case PICK_ENGAGE_OPT: {
            if(dialog->canCont() && nite::getTicks()-lastStChange > 0){
                generateEngageOptions();
                setState(PICK_ACTION);
            }
        } break;         
        case PICK_EVADE_OPT: {
            if(dialog->canCont() && nite::getTicks()-lastStChange > 0){
                generateEvadeOptions();
                setState(PICK_ACTION);
            }
        } break;        
    }

    if(nite::keyboardPressed(nite::keyZ)){
    	dialog->cont();
    }
}

void Game::Battle::render(){
    // 
	// nite::setRenderTarget(nite::RenderTargetUI);
	// nite::setDepth(nite::DepthBottom);
    // nite::setColor(0.0f, 0.0f, 0.0f, 1.0f);
    // empty.draw(0, 0, nite::getWidth(), nite::getHeight(), 0.0f, 0.0f, 0.0f);
    
    // dialog->render();
}