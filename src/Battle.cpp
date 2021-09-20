#include <algorithm>

#include "Object.hpp"
#include "Battle.hpp"
#include "Engine/Input.hpp"


Game::Battle::Battle(){
    playerStatPos.set(8);
    this->state = Game::BattleState::BATTLE_READY;
    
    this->dialog = Shared<Game::DialogHook>(new Game::DialogHook());
    this->dialogBox = std::make_shared<Game::UIDialogBox>(Game::UIDialogBox());
    this->optionsMenu = std::make_shared<Game::UIListMenu>(Game::UIListMenu());

    this->startTurn = 0;

    this->onBattleEnd = [](int winGroup){

    };

    this->lastStChange = nite::getTicks();

    this->dialog->onReset = [&](){
        dialogBox->setText("");  
    };

    this->dialog->onUpdateText = [&](){
        dialogBox->setText(dialog->currenText);      
    };

    this->dialog->onNextLine = [&](const Shared<DialogLine> &line){

    };

    this->selTargetTick = nite::getTicks();
    this->selTargetFlip = false;

    theme = std::make_shared<Game::UITheming>(Game::UITheming());
    themeEnemy = std::make_shared<Game::UITheming>(Game::UITheming());
    themeDialogBox = std::make_shared<Game::UITheming>(Game::UITheming());

    theme->load("data/ui/base_theme.json");
    themeEnemy->load("data/ui/base_enemy_theme.json");
    themeDialogBox->load("data/ui/dialogbox_theme.json");

    this->dialogBox->theme = themeDialogBox;
    this->optionsMenu->theme = theme;
}

bool Game::Battle::isShowing(){
    return this->state != BattleState::BATTLE_READY;
}

void Game::Battle::setState(int nstate){
    this->lastStChange = nite::getTicks();
    this->state = nstate;
    nite::print("[debug] battle set new state "+nite::toStr(nstate));
}

void Game::Battle::end(int winGroup){
    onBattleEnd(winGroup);
    reset();
}

void Game::Battle::reset(){
    setState(BATTLE_READY);
    groupA.clear();
    groupB.clear();
    dialog->reset();
    dialogBox->setVisible(false);
    this->selTargetTick = nite::getTicks();
    this->selTargetFlip = false;    
    this->startTurn = 0;
}


static inline String bttTextColorEnemyName(const String &input){
    return "$[cs:#3273ef]"+input+"$[cr]";
}

static inline String bttTextColorPlayerName(const String &input){
    return "$[cs:#ff9e00]"+input+"$[cr]";
}

static inline String bttTextColorReducedDamage(const String &input){
    return "$[cs:#e8ff00]"+input+"$[cr]";
}

static inline String bttTextColorDamageNumber(const String &input){
    return "$[cs:#ff3900]"+input+"$[cr]";
}



void Game::Battle::start(const Vector<Shared<Game::Entity>> &groupA, const Vector<Shared<Game::Entity>> &groupB){

    // TODO: handle if groupB is empty

    // prepare first line
    String names = bttTextColorEnemyName(groupB[0]->nickname);
    for(int i = 1; i < groupB.size(); ++i){
        names += i < groupB.size()-1 ? ", " : " and " + bttTextColorEnemyName(groupB[i]->nickname);
    }
    static const Vector<String> randomEnterPhrase = {"Oh! ", "Quick! ", "Look! "};
    String pickedPhrase = randomEnterPhrase[nite::randomInt(1, randomEnterPhrase.size()+1)-1];
    dialog->add("", pickedPhrase+"@500!"+names+(groupB.size() > 1 ? " are " : " is ")+"approaching...", nite::Color("#d20021"));
    dialog->start();
    dialogBox->setVisible(true);
    dialogBox->setSize(nite::Vec2(nite::getWidth() * 0.75f, dialogBox->theme->bigFont.getHeight() * 4.0f));
    dialogBox->position = nite::Vec2(nite::getWidth() * 0.5f - dialogBox->size.x * 0.5f, nite::getHeight() - dialogBox->size.y - 16);

    optionsMenu->setSize(nite::Vec2(200.0f, 0.0f));
    optionsMenu->setPosition(nite::Vec2(nite::getWidth() * 0.5f - optionsMenu->size.x * 0.5f, nite::getHeight() -  optionsMenu->size.y - optionsMenu->margin.y));

    if(!selArrowBlack.isLoaded()){
        selArrowBlack.load("data/texture/ui/arrow_black.png", nite::Color(1.0f, 1.0f, 1.0f, 1.0f));
    }   

    // group a
    for(int i = 0; i < groupA.size(); ++i){
        auto ent = std::make_shared<BattleEntity>(BattleEntity());
        ent->theme = theme;
        ent->entity = groupA[i];
        ent->group = BattleGroup::GROUP_A;
        ent->onJoin(nite::Vec2(16.0f));
        this->groupA.push_back(ent);
    }
    for(int i = 0; i < groupB.size(); ++i){
        auto ent = std::make_shared<BattleEntity>(BattleEntity());
        ent->theme = themeEnemy;
        ent->entity = groupB[i];
        ent->statusWindow->small = true;    
        ent->group = BattleGroup::GROUP_B;
        ent->onJoin(nite::Vec2(0.0f));
        this->groupB.push_back(ent);
    } 

    currentTurn = startTurn;
    
    dialogBox->onCreate();
    optionsMenu->onCreate();

    setState(BattleState::BATTLE_START);

    nite::print("[debug] battle start");

}

Shared<Game::BattleEntity> Game::Battle::getCurrentTurnSubject(){
    if(cdecision <= groupA.size()-1){
        return groupA[cdecision];
    }else{
        int adjcdecision = cdecision - groupA.size();
        return groupB[adjcdecision];
    }
}

Shared<Game::BattleEntity> Game::Battle::getCurrentSelTarget(){
    if(selTarget <= groupA.size()-1){
        return groupA[selTarget];
    }else{
        int adjselTarget = selTarget - groupA.size();
        return groupB[adjselTarget];
    }

}

void Game::Battle::onSwitchSelTarget(){
    auto sel = getCurrentSelTarget();
    dialog->setImmediateText("Attack "+bttTextColorEnemyName(sel->entity->nickname)+"?");
}

void Game::Battle::step(){
    
    if(state == BattleState::BATTLE_READY){
        return;
    }

    playerStatPos.lerpDiscrete(nite::Vec2(8) + shakeOffPos, 0.15f);

    Vector<int> dmgNumToBeRemoved;
    for(int i = 0; i < dmgNumbers.size(); ++i){
        dmgNumbers[i]->step();
        if(dmgNumbers[i]->destroyed){
            dmgNumToBeRemoved.push_back(dmgNumbers[i]->id);   
        }
    }
    for(int i = 0; i < dmgNumToBeRemoved.size(); ++i){
        for(int j = 0; j < dmgNumbers.size(); ++j){
            if(dmgNumbers[j]->id == dmgNumToBeRemoved[i]){
                dmgNumbers.erase(dmgNumbers.begin() + j);
                break;
            }
        }
    }

    optionsMenu->step();
    dialog->step();
    vfxDev.step();

    const static UInt64 avgTransitionWait = 80;


    for(int i = 0; i < groupA.size(); ++i){
        groupA[i]->statusWindow->step();
        groupA[i]->statusWindow->position = nite::Vec2(16.0f) + shakeOffPos;
        groupA[i]->statusWindow->render();
        
    }  

    /*
        ENGAGE
    */  
    auto generateEngageOptions = [&](){
        optionsMenu->clear();
        optionsMenu->setVisible(true);

        optionsMenu->addOption("ATTACK", [&](Game::UIListMenuOption *opt){
            selAction.type = Game::ActionType::ATTACK;
            selAction.owner = getCurrentTurnSubject();
            setState(PRE_PICK_TARGET);
        });  

        optionsMenu->addOption("SAY...", [&](Game::UIListMenuOption *opt){
            optionsMenu->interact = true;
            return;
        });  

        optionsMenu->addOption("OFFER...", [&](Game::UIListMenuOption *opt){
            optionsMenu->interact = true;
            return;
        }); 

        optionsMenu->addOption("BACK", [&](Game::UIListMenuOption *opt){
            setState(PRE_PICK_ACTION);
        });        

        optionsMenu->setPosition(nite::Vec2(nite::getWidth() * 0.5f - optionsMenu->size.x * 0.5f, nite::getHeight() -  optionsMenu->size.y - optionsMenu->margin.y));

        menuState = BattleMenuState::IN_ENGAGE;

    };


    /*
        EVADE OPTIONS
    */    
    auto generateEvadeOptions = [&](){
        optionsMenu->clear();
        optionsMenu->setVisible(true);
        optionsMenu->addOption("BLOCK", [&](Game::UIListMenuOption *opt){
            selAction.type = Game::ActionType::BLOCK;
            selAction.owner = getCurrentTurnSubject();
            setState(BattleState::TURN_START);
            ++cdecision;
            decisions.push_back(selAction); 
        });        

        optionsMenu->addOption("DODGE", [&](Game::UIListMenuOption *opt){
            selAction.type = Game::ActionType::DODGE;
            selAction.owner = getCurrentTurnSubject();
            setState(BattleState::TURN_START);
            ++cdecision;
            decisions.push_back(selAction); 
        });   

        optionsMenu->addOption("TANK IT", [&](Game::UIListMenuOption *opt){
            selAction.type = Game::ActionType::TANK;
            selAction.owner = getCurrentTurnSubject();
            setState(BattleState::TURN_START);
            ++cdecision;
            // decisions.push_back(selAction); 
        });    

        optionsMenu->addOption("BACK", [&](Game::UIListMenuOption *opt){
            setState(PRE_PICK_ACTION);
        });                    

        optionsMenu->setPosition(nite::Vec2(nite::getWidth() * 0.5f - optionsMenu->size.x * 0.5f, nite::getHeight() -  optionsMenu->size.y - optionsMenu->margin.y));
        menuState = BattleMenuState::IN_EVADE;

    };

    /*
        MAIN OPTIONS
    */

    auto generateMainOptions = [&](){
        optionsMenu->clear();
        optionsMenu->setVisible(true);
        optionsMenu->addOption("ENGAGE", [&](Game::UIListMenuOption *opt){
            nite::print("SWITCH STATE");
            setState(Game::BattleState::PICK_ENGAGE_OPT);
        });
        optionsMenu->addOption("EVADE", [&](Game::UIListMenuOption *opt){
            setState(Game::BattleState::PICK_EVADE_OPT);
        });
        optionsMenu->addOption("BAG", [&](Game::UIListMenuOption *opt){
            nite::print("BAG");
            optionsMenu->interact = true;
        });
        optionsMenu->addOption("ESCAPE", [&](Game::UIListMenuOption *opt){
            nite::print("ESCAPE");
            optionsMenu->interact = true;
        });     
        optionsMenu->setPosition(nite::Vec2(nite::getWidth() * 0.5f - optionsMenu->size.x * 0.5f, nite::getHeight() -  optionsMenu->size.y - optionsMenu->margin.y));                   
        menuState = BattleMenuState::IN_MAIN;
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
        optionsMenu->setVisible(true);
        dialogBox->setVisible(false);        
    };

    auto getFirstDeadOne = [&](){
        Vector<Shared<BattleEntity>> dead;
        for(int i = 0; i < groupA.size(); ++i){
            if(groupA[i]->entity->healthStat.dead){
                dead.push_back(groupA[i]);
            }
        }
        for(int i = 0; i < groupB.size(); ++i){
            if(groupB[i]->entity->healthStat.dead){
                dead.push_back(groupB[i]);
            }
        }                    

        return dead;
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
                for(int i = 0; i < groupA.size(); ++i){
                    groupA[i]->tryBlocking = false;
                    groupA[i]->tryDodging = false;
                }                
                for(int i = 0; i < groupB.size(); ++i){
                    groupB[i]->tryBlocking = false;
                    groupB[i]->tryDodging = false;
                }
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
                    optionsMenu->setVisible(false);
                    dialogBox->setVisible(true);  
                    dialog->reset();
                    if(groupA.size() == 1){
                        dialog->add("", "What will "+bttTextColorPlayerName("you")+" do?", nite::Color("#d20021"));
                    }else{
                        dialog->add("", "What will "+bttTextColorPlayerName(groupA[cdecision]->entity->nickname)+" do?", nite::Color("#d20021"));
                    }
                    dialog->start();           
                    dialog->setAutoCont(1000);
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
                    nite::print("[AI] '"+who->entity->nickname+"' decided to "+ActionType::name(act.type)+" '"+act.target->entity->nickname+"'");
                    ++cdecision;
                }
            }
        } break;     

        case PRE_PICK_ACTION: {
            if(dialog->canCont() && nite::getTicks()-lastStChange > 0){
                setState(PICK_ACTION);
                generateMainOptions();
                optionsMenu->setVisible(true);
                dialogBox->setVisible(false);
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

                optionsMenu->setVisible(false);
                dialogBox->setVisible(true);  

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

                optionsMenu->setVisible(false);
                dialogBox->setVisible(true);  

                dialog->reset();
                dialog->add("", "Attack who?", nite::Color("#d20021"));
                dialog->start();                  
                setState(PICK_TARGET);
            }            
        } break;      

        case PLAY_ACTIONS_DECIDE_ORDER: {
            // TODO: implement algorithm to decide the order based on agility or some other stat

            // BLOCK & DODGE are always last
            std::sort(decisions.begin(), decisions.end(), [](ActionTurn &a, ActionTurn &b){
                if(b.type == ActionType::BLOCK || b.type == ActionType::DODGE){
                    return true;
                }else{
                    return false;
                }
            });      
            for(int i = 0; i < decisions.size(); ++i){
                nite::print(ActionType::name(decisions[i].type));
            }
            setState(PRE_PLAY_ACTIONS);
        } break;

        case PRE_PLAY_ACTIONS: {
            if(groupB.size() == 0){
                // TODO: handle exp gain, and level up
                dialog->reset();
                dialog->add("", "Phew, you made it.", nite::Color("#d20021"));
                dialog->start(); 
                setState(BATTLE_END);
                break;
            }
            if(decisions.size() == 0){
                // dialog->cont();
                setState(PRE_TURN);
                break;
            }
            dialogBox->setVisible(true);
            optionsMenu->setVisible(false);

            auto &current = decisions[0];

            switch(current.type){
                case ActionType::ATTACK: {

                    bool triedToBlock = false;
                    bool triedToDodge = false;
                    // did target try to block/dodge it?
                    for(int i = 0; i < decisions.size(); ++i){
                        if(decisions[i].owner.get() == current.target.get()){
                            if(decisions[i].type == ActionType::DODGE){
                                decisions[i].owner->tryDodging = true;
                                triedToDodge = true;
                                decisions.erase(decisions.begin() + i);
                                break;                                
                            }else                            
                            if(decisions[i].type == ActionType::BLOCK){
                                decisions[i].owner->tryBlocking = true;
                                triedToBlock = true;
                                decisions.erase(decisions.begin() + i);
                                break;
                            }
                        }
                    }
                    
                    onActionTimeout = 750;
                    onActionTick = nite::getTicks();

                    // float dmg = 100.0f;
                    // float def = 50.0f;

                    // float after = (dmg - def) * (triedToBlock ? 0.90f : 1.0f);
                    // current.fDmg = after;

                    optionsMenu->setVisible(false);
                    dialogBox->setVisible(true);                      

                    String ownName = current.owner->entity->nickname;
                    ownName = current.owner->entity->entityType == EntityType::PLAYER ? bttTextColorPlayerName(ownName) : bttTextColorEnemyName(ownName);
                    String tarName = current.target->entity->nickname;
                    tarName = current.target->entity->entityType == EntityType::PLAYER ? bttTextColorPlayerName(tarName) : bttTextColorEnemyName(tarName);

                    dialog->reset();
                    if(triedToBlock || triedToDodge){
                        String verb = triedToBlock ? "block" : "dodge";
                        dialog->add("", ownName+" attacks "+tarName+"@100!.@100!.@100!.@100! "+tarName+" prepares to "+verb+" it!", nite::Color("#d20021"));
                    }else{                       
                        dialog->add("", ownName+" attacks "+tarName+"!", nite::Color("#d20021"));
                    }
                    dialog->start();  
                    dialog->setAutoCont(triedToBlock || triedToDodge ? 2300 : 1600);
                    setState(PLAY_ACTION_ATTACK);
                } break;
                case ActionType::DODGE: {
                    String ownName = current.owner->entity->nickname;
                    ownName = current.owner->entity->entityType == EntityType::PLAYER ? bttTextColorPlayerName(ownName) : bttTextColorEnemyName(ownName);                    
                    dialog->reset();
                    dialog->add("",  ownName+" tried to dodge nothing...", nite::Color("#d20021"));
                    dialog->start();  
                    dialog->setAutoCont(1600);
                    setState(POST_PLAY_ACTIONS);
                } break;
                case ActionType::BLOCK: {
                    String ownName = current.owner->entity->nickname;
                    ownName = current.owner->entity->entityType == EntityType::PLAYER ? bttTextColorPlayerName(ownName) : bttTextColorEnemyName(ownName);                      
                    dialog->reset();
                    dialog->add("", ownName+" tried to block nothing...", nite::Color("#d20021"));
                    dialog->start();  
                    dialog->setAutoCont(1600);
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
            if(current.owner->isBattleAnim() && dialog->canCont()){
 
                auto dmginfo = DamageInfo();
                bool targetMissed = false;
                dmginfo.owner = current.owner->entity;
                dmginfo.target = current.target->entity;
			    dmginfo.tryingBlock = current.target->tryBlocking;
			    dmginfo.tryingDodge = current.target->tryDodging;;                   
                dmginfo.target->damage(dmginfo);
                targetMissed = dmginfo.dodged;
                current.owner->onAffect();
                current.target->onAffect();

                if(!targetMissed){
                    current.owner->setBattleAnim(EntityBattleAnim::ATTACK, onActionTimeout);
                    current.target->setBattleAnim(EntityBattleAnim::STUTTER, 500);                       
                }

                if(!targetMissed && (current.target->group != BattleGroup::GROUP_A)){
                    addDmgNumber(current.target->position, dmginfo.dmg);

                    auto ef = Shared<Game::VfxBang>(new Game::VfxBang());
                    ef->position = current.target->position;
                    this->vfxDev.add(ef);     

                }else
                if(!targetMissed){
                    shakeEff();
                    addDmgNumber(playerStatPos + 100, dmginfo.dmg, DamageNumberPattern::DROP_RIGHT);

                    auto ef = Shared<Game::VfxBang>(new Game::VfxBang());
                    ef->position = playerStatPos + 100;
                    this->vfxDev.add(ef);                       
                }
                String ownName = current.owner->entity->nickname;
                ownName = current.owner->entity->entityType == EntityType::PLAYER ? bttTextColorPlayerName(ownName) : bttTextColorEnemyName(ownName);   
                String tarName = current.target->entity->nickname;
                tarName = current.target->entity->entityType == EntityType::PLAYER ? bttTextColorPlayerName(tarName) : bttTextColorEnemyName(tarName);   
                dialog->reset();
                if(targetMissed){
                    dialog->add("", ownName+" missed!", nite::Color("#d20021"));
                }else
                if(dmginfo.tryingBlock){
                    dialog->add("", tarName+" received "+bttTextColorDamageNumber(nite::toStr(dmginfo.dmg)+ " damage, "+bttTextColorReducedDamage(nite::toStr(dmginfo.blockDmg))+" was blocked")+".", nite::Color("#d20021"));
                }else{
                    dialog->add("", tarName+" received "+bttTextColorDamageNumber(nite::toStr(dmginfo.dmg)+ " damage")+".", nite::Color("#d20021"));
                }
                dialog->start(); 
                dialog->setAutoCont(2000);                
                setState(POST_PLAY_ACTIONS);
                // TODO: animation handling
                // TODO: check if target died
            }
        } break;
        case POST_PLAY_ACTIONS: {        
            if(dialog->canCont()){
                dialog->cont();

                auto died = getFirstDeadOne();

                // if no one died, carry on
                if(died.size() == 0){
                    auto &current = decisions[0];
                    if(current.owner.get() != NULL){
                        current.owner->setBattleAnim(EntityBattleAnim::IDLE, 0);
                    }
                    if(current.target.get() != NULL){
                        current.target->setBattleAnim(EntityBattleAnim::IDLE, 0);                
                    }
                    decisions.erase(decisions.begin());
                    setState(BattleState::PRE_PLAY_ACTIONS);
                }else{
                // someone died, do the animation
                    setState(BattleState::ENTITY_DEATH);
                }
            } 
        } break;
        case ENTITY_DEATH: {  
            if(dialog->canCont() && nite::getTicks()-lastStChange > 0){
                auto died = getFirstDeadOne();

                if(died.size() == 0){
                    setState(BattleState::POST_PLAY_ACTIONS);
                    break;
                }

                std::sort(died.begin(), died.end(), [](Shared<BattleEntity> &a, Shared<BattleEntity> &b){
                    return a->entity->entityType != EntityType::PLAYER || b->entity->entityType != EntityType::PLAYER;
                });      

                if(died[0]->entity->entityType == EntityType::PLAYER){
                    dialog->reset();
                    dialog->add("", "You died.", nite::Color("#d20021"));
                    dialog->start();                     
                    setState(BattleState::GAME_OVER);
                }

                if(!died[0]->died){
                    String tarName = died[0]->entity->nickname;
                    tarName = died[0]->entity->entityType == EntityType::PLAYER ? bttTextColorPlayerName(tarName) : bttTextColorEnemyName(tarName);                      
                    dialog->reset();
                    dialog->add("", tarName+" died.", nite::Color("#d20021"));
                    dialog->start();                     
                    died[0]->died = true;
                    died[0]->setBattleAnim(EntityBattleAnim::IDLE, 0); 
                }

                if(died[0]->isOut()){
                    if(died[0]->group == BattleGroup::GROUP_A){
                        for(int i = 0; i < groupA.size(); ++i){
                            if(died[0].get() == groupA[i].get()){
                                died[0]->onDeath();
                                groupA.erase(groupA.begin() + i);
                            }
                        }
                    }else
                    if(died[0]->group == BattleGroup::GROUP_B){                
                        for(int i = 0; i < groupB.size(); ++i){
                            if(died[0].get() == groupB[i].get()){
                                died[0]->onDeath();
                                groupB.erase(groupB.begin() + i);
                            }
                        }
                    }                    
                }

            }
        } break;
        case GAME_OVER: {
            if(dialog->canCont() && nite::getTicks()-lastStChange > 0){
                end(BattleGroup::GROUP_B);
            }
        } break;
        case BATTLE_END: {
            if(dialog->canCont() && nite::getTicks()-lastStChange > 0){
                end(BattleGroup::GROUP_A);
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
    if(nite::keyboardPressed(nite::keyZ) && nite::getTicks()-lastStChange > 100){
    	dialog->cont();
        switch(state){
            case BattleState::PICK_TARGET:{
                // confirm target
                selAction.target = getCurrentSelTarget();
                ++cdecision;
                decisions.push_back(selAction);  
                dialogBox->setVisible(false);
                optionsMenu->setVisible(false);
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
    if(shakeDamageEff){
        if(nite::getTicks()-shakeDmgEffTick > 500){
            shakeDamageEff = false;
            shakeOffPos.set(0.0f);
        }else
        if(nite::getTicks()-shakeDmgEffAppTick > 30){
            shakeDmgEffAppTick = nite::getTicks();
            shakeOffPos.x = nite::randomInt(-2, 2);
            shakeOffPos.y = nite::randomInt(-2, 2);
        }
    }
}

void Game::Battle::shakeEff(){

    shakeDamageEff = true;
    shakeDmgEffTick = nite::getTicks();
    shakeDmgEffAppTick = nite::getTicks();
    shakeOffPos.x = nite::randomInt(-2, 2);
    shakeOffPos.y = nite::randomInt(-2, 2);    

}

void Game::Battle::addDmgNumber(const nite::Vec2 &stPos, int amnt, int pattern){
    auto num = std::make_shared<Game::DamageNumber>(Game::DamageNumber());
    num->setPosition(stPos);
    num->startTime = nite::getTicks();
    num->lifetime = 1800;
    num->theme = themeDialogBox;
    num->amount = amnt;
    num->pattern = pattern;
    this->dmgNumbers.push_back(num);
}

void Game::Battle::render(){
    
    if(state == BattleState::BATTLE_READY){
        return;
    }

	nite::setRenderTarget(nite::RenderTargetUI);
	nite::setDepth(nite::DepthBottom);
    nite::setColor(nite::Color("#594d8f", 1.0f));
    if(theme.get() != NULL){
        theme->base.draw(0, 0, nite::getWidth(), nite::getHeight(), 0.0f, 0.0f, 0.0f);
    }
    

    float parts = nite::getWidth() * (1.0f / (float)(groupB.size()+1));

    for(int i = 0; i < groupB.size(); ++i){
        float x = parts * (i+1);
        float y = nite::getHeight() * 0.4f;
        groupB[i]->renderBattleAnim(x, y, state == PICK_TARGET && selTarget == i + groupA.size());
        if(state == PICK_TARGET && selTarget == i + groupA.size()){
            if(nite::getTicks()-selTargetTick > 150){
                selTargetTick = nite::getTicks();
                selTargetFlip = !selTargetFlip;
            }
            nite::setColor(0.1f, 0.1f, 0.1f, 1.0f);            
            auto arrefsha = selArrowBlack.draw(x + 1, y - nite::getHeight() * 0.25f + 32.0f * (selTargetOffset/100.0f) + 1, 32, 32, 0.5f, 0.5f, 0.0f);
            if(arrefsha != NULL){
                arrefsha->smooth = true;
            }
            nite::setColor(1.0f, 1.0f, 1.0f, 1.0f);            
            nite::lerpDiscrete(selTargetOffset, selTargetFlip ? 0.0f : 100.0f, 0.08f);
            auto arref = selArrowBlack.draw(x, y - nite::getHeight() * 0.25f + 32.0f * (selTargetOffset/100.0f), 32, 32, 0.5f, 0.5f, 0.0f);
            if(arref != NULL){
                arref->smooth = true;
            }
        }        
    }

    for(int i = 0; i < dmgNumbers.size(); ++i){
        dmgNumbers[i]->render();
    }

    dialogBox->showArrow = dialog->canNext() && !dialog->autocont;

    nite::setDepth(nite::DepthTop);
    nite::setRenderTarget(nite::RenderTargetUI);  
    
    dialogBox->render();
    optionsMenu->render();
    vfxDev.draw();

    // shakeOffPos.

    // dialog->render();
}

Game::DamageNumber::DamageNumber(){
    color.set("#ff3500");
    targetAngle = 0.0f;
    angle = 0.0f;
    pattern = DamageNumberPattern::FALL_LEFT;
}

void Game::DamageNumber::step(){
    UInt64 elapsed = nite::getTicks()-startTime;
    double timeDiff = (double)(elapsed) / (double)startTime;
    if(elapsed > lifetime){
        destroyed = true;
    }
    nite::Vec2 patternPos;
    float patternAngle;
    switch(pattern){
        case DamageNumberPattern::DROP_RIGHT: {
            patternPos = nite::Vec2(80.0f, 0.0f);
            patternAngle = 10.0f;
        } break;
        case DamageNumberPattern::FALL_LEFT: {
            patternPos = nite::Vec2(-80.0f, -80.0f);
            patternAngle = -10.0f;
        } break;
    }
    if(elapsed < lifetime/3){
        this->targetPosition = startPosition + patternPos;
        this->targetAngle = patternAngle;
    }else
    if(elapsed > lifetime/3){
        this->targetPosition = nite::Vec2(startPosition.x + patternPos.x, this->targetPosition.y + 1);
        this->targetAngle = 0.0f;
    }
    nite::lerpDiscrete(angle, targetAngle * 100.0f, 0.20f);
    this->position.lerpDiscrete(this->targetPosition, 0.1f);
    if(nite::getTicks()-lastShakeTick > 200){
        this->offPosition.x = nite::randomInt(-3, 3);
        this->offPosition.y = nite::randomInt(-3, 3);
    }
    lerpPos.lerpDiscrete(this->position + this->offPosition, 0.10f);
}

void Game::DamageNumber::render(){
    if(theme.get() == NULL){
        return;
    }
    if(!batch.isDirty()){
        String amount = "-"+nite::toStr(this->amount);
        batch.init(theme->bigFont.getWidth(amount) + 2, theme->bigFont.getHeight(amount) + 2); 
        batch.begin();
        nite::setColor(color);
        theme->bigFont.draw(amount, 1.0f, 1.0f);
        batch.end();
        batch.flush();
    }
    nite::setDepth(nite::DepthTop);
    nite::setRenderTarget(nite::RenderTargetUI);
    nite::setColor(1.0f, 1.0f, 1.0f, 1.0f);
    auto obj = batch.draw(lerpPos.x, lerpPos.y, batch.getSize().x, batch.getSize().y, 0.5f, 0.5f, angle / 100.0f);
    if(obj != NULL){
        obj->smooth = true;
    }
}

Game::BattleEntity::BattleEntity(){
    died = false;
    posOffset = nite::Vec2(0.0f);
	battlAnimBlinkFlip = false;
	battleAnimStatus = EntityBattleAnim::IDLE;
	lastBattleAnimBlinkTick = nite::getTicks();    
    statusWindow = std::make_shared<UIBattlePlayerInfo>(UIBattlePlayerInfo());
}

void Game::BattleEntity::onJoin(const nite::Vec2 &winPos){

    this->statusWindow->entity = entity;
    this->statusWindow->theme = theme;
    this->statusWindow->position = winPos;
    this->statusWindow->onCreate();
    // TODO: handle enemy windows
}

void Game::BattleEntity::onDeath(){
}

void Game::BattleEntity::onAffect(){
    this->statusWindow->setAffected();
}

void Game::BattleEntity::setBattleAnim(int anim, UInt64 animTargetTime){
	this->battleAnimTargetTime = animTargetTime;
	this->battleAnimStatus = anim;
	this->lastBattleAnimTick = nite::getTicks();
	this->lastBattleAnimInnerTick = nite::getTicks();
	this->battleAnimStep = 0;
	this->battleAnimTargetExp = 0.0f;
	this->battlAnimPosOff.set(0.0f);
}

bool Game::BattleEntity::isBattleAnim(){
	return nite::getTicks()-this->lastBattleAnimTick  > battleAnimTargetTime;
}

bool Game::BattleEntity::isOut(){
    return posOffset.y > nite::getHeight()-1;
}

void Game::BattleEntity::renderBattleAnim(float x, float y, bool blink){
    auto &battleAnim = entity->battleAnim;

	float rateExp = 0.0f;
	float maxExp = 0.0f;

	float xoff = 0.0f;
	float yoff = 0.0f;
	float targetAngle = 0.0f;
	nite::Vec2 origin(0.5f);

    if(died){
        posOffset.lerpDiscrete(nite::Vec2(0.0f, nite::getHeight()), 0.10f);
    }

	switch(battleAnimStatus){
        case EntityBattleAnim::ATTACK_MISS: 
		case EntityBattleAnim::ATTACK: {
			UInt64 splitTime = this->battleAnimTargetTime / 3;
			bool npassed = nite::getTicks()-lastBattleAnimInnerTick < splitTime;
			float xFinOffset = -64.0f;
			
			if(nite::getTicks()-lastBattleAnimInnerTick > splitTime){
				if(this->battleAnimStep < 2){
					++battleAnimStep;
				}
				this->lastBattleAnimInnerTick = nite::getTicks();
			}
			double timeDiff = (double)(nite::getTicks()-lastBattleAnimInnerTick) / (double)splitTime;
			if(npassed && this->battleAnimStep == 0){
				battlAnimPosOff.lerpDiscrete(xFinOffset, 0.25f);
				if(battleAnimStatus == EntityBattleAnim::ATTACK_MISS){
                    targetAngle = -270.f;
                }
			}else
			if(npassed && this->battleAnimStep == 1){
				battlAnimPosOff.lerpDiscrete(xFinOffset, 0.25f);
				xoff = battlAnimPosOff.x;
				xoff += nite::randomInt(-16, 16);
				yoff += nite::randomInt(-16, 16);
				nite::lerpDiscrete(battleAnimTargetExp, 8 * 100.0f, 0.25f);
				rateExp = battleAnimTargetExp / 100.0f;
				maxExp = 4.0f;
				targetAngle = battleAnimStatus == EntityBattleAnim::ATTACK_MISS ? 270 : -2.5f;
			}else			
			if(npassed && this->battleAnimStep == 2){
				battlAnimPosOff.lerpDiscrete(0.0f, 0.10f);
				xoff = battlAnimPosOff.x;	
				nite::lerpDiscrete(battleAnimTargetExp, 0.0f, 0.10f);
				rateExp = battleAnimTargetExp / 100.0f;
				maxExp = 4.0f;	
				if(battleAnimStatus == EntityBattleAnim::ATTACK_MISS){
                    // targetAngle = 180.f;
                }                			
			}
			xoff = battlAnimPosOff.x;	
			// // origin.set(0.45f);
			// battlAnimPosOff.lerpDiscrete(xFinOffset, 0.05f);
			// xoff = battlAnimPosOff.x;
			// if(battlAnimPosOff.x / xFinOffset > 0.5f){
			// 	float n = battleAnimTargetExp * 100.0f;
			// 	rateExp = 1.0f;
			// 	nite::lerpDiscrete(n, 3000.0f, 0.25f);
			// 	battleAnimTargetExp = n / 100.0f;
			// 	maxExp = battleAnimTargetExp;
			// 	xoff += nite::randomInt(1, 2);
			// 	yoff += nite::randomInt(1, 2);
			// }
			// if(battlAnimPosOff.x / xFinOffset >= 0.95f){
			// 	battleAnimTargetTime = 0;
			// }
			// if(nite::getTicks()-this->lastBattleAnimTick  > battleAnimTargetTime){
			// 	setBattleAnim(EntityBattleAnim::IDLE, 0);
			// }
			
		} break;
		case EntityBattleAnim::STUTTER: {
			UInt64 splitTime = this->battleAnimTargetTime / 2;
			bool npassed = nite::getTicks()-lastBattleAnimInnerTick < splitTime;
			float xFinOffset = 32.0f;

			if(nite::getTicks()-lastBattleAnimInnerTick > splitTime){
				if(this->battleAnimStep < 2){
					++battleAnimStep;
				}
				this->lastBattleAnimInnerTick = nite::getTicks();
			}
			// double timeDiff = (double)(nite::getTicks()-lastBattleAnimInnerTick) / (double)splitTime;
			if(npassed && this->battleAnimStep == 0){
				battlAnimPosOff.lerpDiscrete(xFinOffset, 0.25f);
				xoff = battlAnimPosOff.x;	
				xoff += nite::randomInt(-128, 128);
				yoff += nite::randomInt(-128, 128);	
				angle = 2.5f;						
			}else
			if(npassed && this->battleAnimStep == 1){
				battlAnimPosOff.lerpDiscrete(0.0f, 0.10f);
				xoff = battlAnimPosOff.x;	
			}


		} break;			
		case EntityBattleAnim::IDLE: {
			if(nite::getTicks()-lastBattleAnimBlinkTick > 600){
				battlAnimBlinkFlip = !battlAnimBlinkFlip;
				lastBattleAnimBlinkTick = nite::getTicks();
			}			
			nite::lerpDiscrete(battleAnimBlink, battlAnimBlinkFlip ? 100.0f : 0.0f, 0.05f);
			rateExp = battleAnimBlink / 100.0f;
			maxExp = 8.0f;
		} break;
	}


    nite::setColor(0.1f, 0.1f, 0.1f, 1.0f);
	float ratio = battleAnim.getWidth() / battleAnim.getHeight();
    nite::lerpDiscrete(this->angle, targetAngle, 0.08f);
    posLerp.set(xoff + x + posOffset.x, yoff + y + posOffset.y);
    position.lerpDiscrete(posLerp, 0.10f);
	auto fshad = battleAnim.draw(position.x - 5, position.y + 5, battleAnim.getWidth() * 2.0f + maxExp * rateExp * ratio, battleAnim.getHeight() * 2.0f + maxExp * rateExp, origin.x, origin.y, this->angle);
	nite::setColor(1.0f, 1.0f, 1.0f, 1.0f);
	if(blink){
		nite::setColor(1.0f -  0.5f * rateExp, 1.0f -  0.5f * rateExp, 1.0f - 0.5f * rateExp, 1.0f);
	}
	auto f = battleAnim.draw(position.x, position.y, battleAnim.getWidth() * 2.0f + maxExp * rateExp * ratio, battleAnim.getHeight() * 2.0f + maxExp * rateExp, origin.x, origin.y, this->angle);

    auto winSize = statusWindow->size * nite::Vec2(0.5f, 0.25f);
    
    statusWindow->step();
    statusWindow->position = nite::Vec2(position.x - winSize.x, (position.y  + battleAnim.getHeight()) + winSize.y);
    statusWindow->render();
    

	if(f != NULL){
		f->smooth = true;
	}
	if(fshad != NULL){
		fshad->smooth = true;
	}
}