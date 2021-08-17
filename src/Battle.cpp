#include <algorithm>

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

    this->selTargetTick = nite::getTicks();
    this->selTargetFlip = false;

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
    static const Vector<String> randomEnterPhrase = {"Oh! ", "Quick! ", "Look! "};
    dialog->add("", randomEnterPhrase[nite::randomInt(0, randomEnterPhrase.size())]+"@500!"+names+(groupB.size() > 1 ? " are " : " is ")+"approaching...", nite::Color("#d20021"));
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
    if(!selArrow.isLoaded()){
        selArrow.load("data/texture/arrow.png", nite::Color(1.0f, 1.0f, 1.0f, 1.0f));
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
        // auto optBox = win->getComponentById("options-box");
        // if(optBox.get() != NULL && optBox->children.size() > 0 && optBox->children[0]->type == "panel"){
        //     auto panel = optBox->children[0];
        //     for(int i = 0; i < panel->children.size(); ++i){
        //         auto button = std::dynamic_pointer_cast<nite::ButtonUI>(panel->children[i]);
        //         button->setFont(subFont);
        //     }            
        // }else{
        //     nite::print("Battle::start: cannot find option-box's inline panel");
        // }

        // set menu title font 
        auto optBoxTitle = win->getComponentById("options-box-title");
        if(optBoxTitle.get() != NULL && optBoxTitle->type == "text"){
            auto txtUI = std::dynamic_pointer_cast<nite::TextUI>(optBoxTitle);
            txtUI->setFont(subFont);
            txtUI->setFontColor(nite::Color("#fd9e16"));
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

void Game::Battle::updOptBoxTitle(const String &str){
    auto optBoxTitle = batWin->getComponentById("options-box-title");
    if(optBoxTitle.get() != NULL && optBoxTitle->type == "text"){
        auto txtUI = std::dynamic_pointer_cast<nite::TextUI>(optBoxTitle);
        txtUI->setText(str);
    }    
}

Shared<Game::Entity> Game::Battle::getCurrentTurnSubject(){
    if(cdecision <= groupA.size()-1){
        return groupA[cdecision];
    }else{
        int adjcdecision = cdecision - groupA.size();
        return groupB[adjcdecision];
    }
}

Shared<Game::Entity> Game::Battle::getCurrentSelTarget(){
    if(selTarget <= groupA.size()-1){
        return groupA[selTarget];
    }else{
        int adjselTarget = selTarget - groupA.size();
        return groupB[adjselTarget];
    }

}

void Game::Battle::onSwitchSelTarget(){
    auto sel = getCurrentSelTarget();
    dialog->setImmediateText("Attack "+sel->nickname+"?");
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

    auto clearOptionBox = [&](){
        auto list = batWin->getComponentById("options-box-list");
        list->clearChildren();        
    };

    /*
        ENGAGE
    */  
    auto generateEngageOptions = [&](){
        auto list = batWin->getComponentById("options-box-list");
        list->clearChildren();

        auto attkButton = generateButton(list, "ATTACK");
        if(attkButton.get() != NULL){
            attkButton->setBaseColor(nite::Color("#7474b4"));
            attkButton->setOnClick([&](const Shared<nite::ListenerInfo> &info, nite::BaseUIComponent *component){
                selAction.type = Game::ActionType::ATTACK;
                selAction.owner = getCurrentTurnSubject();
                setState(PRE_PICK_TARGET);
            });
        }

        auto sayButton = generateButton(list, "SAY...");
        if(sayButton.get() != NULL){
            sayButton->setBaseColor(nite::Color("#7474b4"));
            sayButton->setOnClick([&](const Shared<nite::ListenerInfo> &info, nite::BaseUIComponent *component){
                return;
            });
        }


        auto offerButton = generateButton(list, "OFFER...");
        if(offerButton.get() != NULL){
            offerButton->setBaseColor(nite::Color("#7474b4"));
            offerButton->setOnClick([&](const Shared<nite::ListenerInfo> &info, nite::BaseUIComponent *component){
                return;
            });
        }


        auto escapeButton = generateButton(list, "BACK");
        if(escapeButton.get() != NULL){
            escapeButton->setBaseColor(nite::Color("#e68e27"));
            escapeButton->setOnClick([&](const Shared<nite::ListenerInfo> &info, nite::BaseUIComponent *component){
                setState(PRE_PICK_ACTION);
            });
        }                    

        list->fixNavIndexes();
        list->nav.current = 0;
        menuState = BattleMenuState::IN_ENGAGE;
        updOptBoxTitle("ENGAGE");

    };


    /*
        EVADE OPTIONS
    */    
    auto generateEvadeOptions = [&](){
        auto list = batWin->getComponentById("options-box-list");
        list->clearChildren();

        auto parryButton = generateButton(list, "BLOCK");
        if(parryButton.get() != NULL){
            parryButton->setBaseColor(nite::Color("#7474b4"));
            parryButton->setOnClick([&](const Shared<nite::ListenerInfo> &info, nite::BaseUIComponent *component){
                selAction.type = Game::ActionType::BLOCK;
                selAction.owner = getCurrentTurnSubject();
                setState(BattleState::TURN_START);
                ++cdecision;
                decisions.push_back(selAction);                
            });
        }

        auto evadeButton = generateButton(list, "DODGE");
        if(evadeButton.get() != NULL){
            evadeButton->setBaseColor(nite::Color("#7474b4"));
            evadeButton->setOnClick([&](const Shared<nite::ListenerInfo> &info, nite::BaseUIComponent *component){
                selAction.type = Game::ActionType::DODGE;
                selAction.owner = getCurrentTurnSubject();
                setState(BattleState::TURN_START);
                ++cdecision;
                decisions.push_back(selAction);                
            });
        }


        // auto tankButton = generateButton(list, "TANK IT");
        // if(tankButton.get() != NULL){
        //     tankButton->setBaseColor(nite::Color("#4d34eb"));
        //     tankButton->setOnClick([&](const Shared<nite::ListenerInfo> &info, nite::BaseUIComponent *component){
        //         selAction.type = Game::ActionType::TANK;
        //         selAction.owner = getCurrentTurnSubject();
        //         setState(BattleState::TURN_START);
        //     });                 
        // }

        auto escapeButton = generateButton(list, "BACK");
        if(escapeButton.get() != NULL){
            escapeButton->setBaseColor(nite::Color("#e68e27"));
            escapeButton->setOnClick([&](const Shared<nite::ListenerInfo> &info, nite::BaseUIComponent *component){
                setState(PRE_PICK_ACTION);
            });
        }                    

        list->fixNavIndexes();
        list->nav.current = 0;
        menuState = BattleMenuState::IN_EVADE;
        updOptBoxTitle("EVADE");

    };

    /*
        MAIN OPTIONS
    */

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
        menuState = BattleMenuState::IN_MAIN;
        updOptBoxTitle("WHAT DO?");
    };
 
    auto switchToMenu = [&](int nstate, int menuOpt){  
        setState(nstate);
        switch(menuOpt){
            case BattleMenuState::IN_ENGAGE: {
                generateEngageOptions();
            } break;
            case BattleMenuState::IN_EVADE: {
                generateEvadeOptions();
            } break;
            case BattleMenuState::IN_MAIN: {
                generateMainOptions();
            } break;                        
        }
        setOptBoxVis(true);
        setDialogBoxVis(false);          
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
                auto total = groupA.size() + groupB.size();
                // everyone chose
                if(cdecision == total){
                    setState(PLAY_ACTIONS_DECIDE_ORDER);
                }else
                // player chooses
                if(cdecision <= groupA.size()-1){
                    setOptBoxVis(false);
                    setDialogBoxVis(true);                    
                    dialog->reset();
                    if(groupA.size() == 1){
                        dialog->add("", "What will you do?", nite::Color("#d20021"));
                    }else{
                        dialog->add("", "What will "+groupA[cdecision]->nickname+" do?", nite::Color("#d20021"));
                    }
                    dialog->start();               
                    setState(PRE_PICK_ACTION);
                // AI Chooses
                }else{
                    int adjcdecision = cdecision - (groupA.size()-1);
                    auto who = getCurrentTurnSubject();
                    ActionTurn act;
                    act.owner = who;
                    act.type = ActionType::ATTACK;
                    act.target = groupA[0];
                    decisions.push_back(act);
                    nite::print("[AI] '"+who->nickname+"' decided to "+ActionType::name(act.type)+" '"+act.target->nickname+"'");
                    ++cdecision;
                }
            }
        } break;     

        case PRE_PICK_ACTION: {
            if(dialog->canCont() && nite::getTicks()-lastStChange > 0 || dialog->isReady() && nite::getTicks()-lastStChange > 1000){
                setState(PICK_ACTION);
                generateMainOptions();
                setOptBoxVis(true);
                setDialogBoxVis(false);                
                // switchToMenu(BattleState::PICK_ACTION, lastMenuOption);
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
        case PRE_PICK_TARGET: {
            if(dialog->canCont() && nite::getTicks()-lastStChange > 0){
                setOptBoxVis(false);
                setDialogBoxVis(true);
                dialog->reset();
                dialog->add("", "", nite::Color("#d20021"));
                dialog->start();                  
                selTarget = groupA.size();
                onSwitchSelTarget();
                setState(PICK_TARGET);
            }            
        } break;
        case PICK_TARGET: {
            if(dialog->canCont() && nite::getTicks()-lastStChange > 0){
                // setOptBoxVis(false);
                // setDialogBoxVis(true);
                // dialog->reset();
                // dialog->add("", "Attack who?", nite::Color("#d20021"));
                // dialog->start();                  
                // setState(PICK_TARGET);
            }            
        } break;      

        case PLAY_ACTIONS_DECIDE_ORDER: {
            // TODO: implement algorithm to decide the order based on agility or some other stat

            // BLOCK & DODGE are always last
            std::sort(decisions.begin(), decisions.end(), [](ActionTurn &a, ActionTurn &b){
                if(b.type == ActionType::BLOCK || b.type == ActionType::DODGE){
                    return true;
                }else{
                    return true;
                }
            });      
            for(int i = 0; i < decisions.size(); ++i){
                nite::print(ActionType::name(decisions[i].type));
            }
            setState(PRE_PLAY_ACTIONS);
        } break;

        case PRE_PLAY_ACTIONS: {
            if(decisions.size() == 0){
                // dialog->cont();
                setState(PRE_TURN);
                break;
            }
            auto &current = decisions[0];

            switch(current.type){
                case ActionType::ATTACK: {

                    bool triedToBlock = false;
                    bool triedToDodge = false;
                    // did target try to block/dodge it?
                    for(int i = 0; i < decisions.size(); ++i){
                        if(decisions[i].owner.get() == current.target.get()){
                            if(decisions[i].type == ActionType::DODGE){
                                triedToDodge = true;
                                decisions.erase(decisions.begin() + i);
                                break;                                
                            }else                            
                            if(decisions[i].type == ActionType::BLOCK){
                                triedToBlock = true;
                                decisions.erase(decisions.begin() + i);
                                break;
                            }
                        }
                    }
                    
                    current.owner->setBattleAnim(EntityBattleAnim::ATTACK, 500);

                    float dmg = 100.0f;
                    float def = 50.0f;

                    float after = (dmg - def) * (triedToBlock ? 0.90f : 1.0f);
                    current.fDmg = after;

                    setOptBoxVis(false);
                    setDialogBoxVis(true);                    
                    onActionTimeout = 500;
                    onActionTick = nite::getTicks();
                    dialog->reset();
                    if(triedToBlock){
                        dialog->add("", current.owner->nickname+" attacks "+current.target->nickname+". @800!"+current.target->nickname+" tries to block it!", nite::Color("#d20021"));
                    }else{
                        dialog->add("", current.owner->nickname+" attacks "+current.target->nickname+"!", nite::Color("#d20021"));
                    }
                    dialog->start();  
                    setState(PLAY_ACTION_ATTACK);
                } break;
                case ActionType::DODGE: {
                    dialog->reset();
                    dialog->add("", current.owner->nickname+" tried to dodged nothing...", nite::Color("#d20021"));
                    dialog->start();  
                    setState(POST_PLAY_ACTIONS);
                } break;
                case ActionType::BLOCK: {
                    dialog->reset();
                    dialog->add("", current.owner->nickname+" tried to block nothing...", nite::Color("#d20021"));
                    dialog->start();  
                    setState(POST_PLAY_ACTIONS);
                } break;                
                default: {
                    nite::print("Unimplemented action: '"+ActionType::name(current.type)+"'");
                    decisions.erase(decisions.begin());
                } break;
            }

        } break;
        case PLAY_ACTION_ATTACK: {
            auto &current = decisions[0];
            if(current.owner->isBattleAnim() && dialog->isReady() && dialog->getLastReady() > 1500 && nite::getTicks()-onActionTick > onActionTimeout){
                current.owner->setBattleAnim(EntityBattleAnim::IDLE, 0);
                dialog->reset();
                dialog->add("", current.target->nickname+" received "+nite::toStr(current.fDmg)+" damage.", nite::Color("#d20021"));
                dialog->start(); 
                setState(POST_PLAY_ACTIONS);
                // TODO: animation handling
                // TODO: check if target died
            }
        } break;
        case POST_PLAY_ACTIONS: {        
            if(dialog->isReady()  && dialog->getLastReady() > 2200){
                dialog->cont();
                decisions.erase(decisions.begin());
                setState(BattleState::PRE_PLAY_ACTIONS);
            } 
        } break;


    }

    // go back
    if((state == BattleState::PICK_ACTION || state == BattleState::PICK_TARGET) && menuState != BattleMenuState::IN_MAIN && nite::keyboardPressed(nite::keyX)){
        dialog->cont(); 
        switch(state){
            case BattleState::PICK_TARGET:{
                switchToMenu(BattleState::PICK_ACTION, BattleMenuState::IN_ENGAGE);
            } break;
            default: {
                setState(PRE_PICK_ACTION);
            } break;
        }
    }

    // continue on dialog
    if(nite::keyboardPressed(nite::keyZ)){
    	dialog->cont();
        switch(state){
            case BattleState::PICK_TARGET:{
                // confirm target
                selAction.target = getCurrentSelTarget();
                ++cdecision;
                decisions.push_back(selAction);
                setOptBoxVis(false);
                setDialogBoxVis(false);     
                setState(BattleState::TURN_START);          
            } break;
        }        
    }


    if(state == BattleState::PICK_TARGET){
        if(nite::keyboardPressed(nite::keyLEFT)){
            --selTarget;
            if(selTarget < groupA.size()){
                selTarget = groupA.size();
            }
            onSwitchSelTarget();
        }
        if(nite::keyboardPressed(nite::keyRIGHT)){
            ++selTarget;
            if(selTarget > groupA.size() + groupB.size()-1){
                selTarget = groupA.size() + groupB.size()-1;
            }
            onSwitchSelTarget();
        }    
    }
}

void Game::Battle::render(){
    
	nite::setRenderTarget(nite::RenderTargetUI);
	nite::setDepth(nite::DepthBottom);
    nite::setColor(nite::Color("#b19ddd", 1.0f));
    empty.draw(0, 0, nite::getWidth(), nite::getHeight(), 0.0f, 0.0f, 0.0f);

    float parts = nite::getWidth() * (1.0f / (float)(groupB.size()+1));

    for(int i = 0; i < groupB.size(); ++i){
        float x = parts * (i+1);
        float y = nite::getHeight() * 0.5f;
        groupB[i]->renderBattleAnim(x, y, state == PICK_TARGET && selTarget == i + groupA.size());
        if(state == PICK_TARGET && selTarget == i + groupA.size()){
            if(nite::getTicks()-selTargetTick > 150){
                selTargetTick = nite::getTicks();
                selTargetFlip = !selTargetFlip;
            }
            nite::lerpDiscrete(selTargetOffset, selTargetFlip ? 0.0f : 100.0f, 0.08f);
            auto arref = selArrow.draw(x, y - nite::getHeight() * 0.25f + 32.0f * (selTargetOffset/100.0f), 32, 32, 0.5f, 0.5f, 0.0f);
            if(arref != NULL){
                arref->smooth = true;
            }
        }        
    }



    // dialog->render();
}