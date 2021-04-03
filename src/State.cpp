#include <memory>
#include <algorithm>
#include "Network.hpp"
#include "Server.hpp"
#include "Client.hpp"
#include "State.hpp"
#include "GameType.hpp"

void GameState::update(){
    dice->update();
    board->update();
    if(clientSide){
        return; 
    }
    auto net = static_cast<Server*>(this->net);
    
    switch(this->net->state){
        case ST_WAITING_FOR_PLAYERS: {
            if(net->clients.size() == expPlayers && nite::getTicks()-net->lastState > 2000){
                net->broadcastNotification("Let's begin...");
                net->setState(ST_PRE_GAME);
            }
            // TODO: handle wait for too long...
        } break;
        case ST_PRE_GAME: {
            if(nite::getTicks()-net->lastState > 2500){
                net->broadcastNotification("Let's see who goes first...");
                net->setState(ST_CHOOSE_ORDERS);
                Vector<unsigned> list;
                for(auto &it : net->clients){
                    list.push_back(it.second->id);
                }
                std::sort(list.begin(), list.end(), [&](UInt8 a, UInt8 b){
                    return net->clients[a]->order < net->clients[b]->order;
                });
                this->pickList = list;                
            }
        } break;
        case ST_CHOOSE_ORDERS: {
            if(nite::getTicks()-net->lastState > 2500){
                net->setState(ST_PLAYER_THROW_DICE, this->pickList[0], 0);
                this->pAllowedShuffle = this->pickList[0];
                this->lastDiceTotalValue = 0;
                auto &client = net->clients[this->pickList[0]];
                net->broadcastNotification(client->nickname+", throw the dice");
            }            
        } break;
        case ST_PLAYER_THROW_DICE: {
            if(pAllowedShuffle == 0 && nite::getTicks()-net->lastState > 100){
                this->valueList[this->pickList[0]] = lastDiceTotalValue;
                this->pickList.erase(this->pickList.begin());
                if(this->pickList.size() > 0){
                    net->setState(ST_PLAYER_THROW_DICE, this->pickList[0], 0);
                    this->pAllowedShuffle = this->pickList[0];
                    auto &client = net->clients[this->pickList[0]];
                    net->broadcastNotification(client->nickname+", throw the dice");
                    this->lastDiceTotalValue = 0;
                }else{
                    net->setState(ST_GAME_PRESTART, 0, 0);
                    this->turnsOrder.clear();
                    for(auto &it : net->clients){
                        auto &client = it.second;
                        this->turnsOrder.push_back(client->id);
                    }
                    std::sort(this->turnsOrder.begin(), this->turnsOrder.end(), [&](UInt8 a, UInt8 b){
                        return this->valueList[a] > this->valueList[b];
                    });  
                    nite::print("Turns Order");
                    for(int i = 0; i < this->turnsOrder.size(); ++i){
                        nite::print(nite::toStr(i+1)+". "+net->clients[this->turnsOrder[i]]->nickname);
                    }

                    net->broadcastNotification(net->clients[this->turnsOrder[0]]->nickname+" goes first");
                }
                sendPlayerTurnOrderList();
            }
        } break;
        case ST_GAME_PRESTART: {
            if(nite::getTicks()-net->lastState > 2500){
                net->setState(ST_GAME_START, 0, 0);
                net->broadcastNotification("Game Start!");
            }            
        } break;          
        case ST_GAME_START: {
            if(nite::getTicks()-net->lastState > 2500){
                net->setState(ST_PLAYER_PRETURN, 0, 0);
            }            
        } break;          
        case ST_PLAYER_PRETURN: {
            if(nite::getTicks()-net->lastState < 100){
                break;
            }
            if(currentTurn >= turnsOrder.size()){
                currentTurn = 0;
            }
            auto &player = net->clients[turnsOrder[currentTurn]];
            pAllowedShuffle = player->id;
            currentPlaying = player->id;
            doubleCount = 0;
            net->broadcastNotification(player->nickname+"'s turn");
            net->setState(ST_PLAYER_TURN, player->id, 0);            
        } break;      
        case ST_PLAYER_PRETURN_DOUBLE: {
            if(nite::getTicks()-net->lastState < 100){
                break;
            } 
            auto &player = net->clients[turnsOrder[currentTurn]]; 
            pAllowedShuffle = player->id;
            currentPlaying = player->id;                      
            net->broadcastNotification(player->nickname+" got doubles. Plays again!");
            net->setState(ST_PLAYER_TURN, player->id, 0);  
        } break;
        case ST_PLAYER_TURN: {      
            if(nite::getTicks()-net->lastState < 100){
                break;
            }              
            if(currentPlaying == 0){
                break;
            }
            auto &client = net->clients[currentPlaying];
            if(isMoving && client->route.size() == 0){
                bool doubles = false;
                UInt8 firstVal = lastDiceValues[0];
                for(int i = 1; i < lastDiceValues.size(); ++i){
                    if(firstVal == lastDiceValues[i]){
                        doubles = true;
                    }else{
                        break;
                    }
                    firstVal = lastDiceValues[i];
                }
                if(doubles){
                    ++doubleCount;
                }
                if(!doubles){
                    ++currentTurn;
                }
                isMoving = false;
                net->setState((doubles ? ST_PLAYER_PRETURN_DOUBLE : ST_PLAYER_PRETURN), 0, 0);
            }
            if(!isMoving && pAllowedShuffle == 0 && nite::getTicks()-net->lastState > 100 && client->route.size() == 0){
                auto value = lastDiceTotalValue;
                lastDiceTotalValue = 0;
                Vector<UInt8> route;
                net->broadcastNotification(client->nickname+" got "+nite::toStr(value));
                UInt8 last = client->currentSpace; 
                for(int i = 0; i < value; ++i){
                    last = this->gType->spaces[last].next;
                    route.push_back(last);
                }
                
                isMoving = true;
                advancePlayerTo(client->id, last, route);
            }

        } break;


    }
}

void GameState::advancePlayerTo(UInt8 id, UInt8 space, const Vector<UInt8> &route){
    if(!net->isServer){
        return;
    }
    auto &client = net->clients[id];
    client->route = route;
    nite::Packet routep;
    routep.setHeader(SV_ADVANCE_PLAYER_TO);
    routep.write(&id, sizeof(id));
    routep.write(&space, sizeof(space));
    UInt8 n = route.size();
    routep.write(&n, sizeof(n));
    for(int i = 0; i < route.size(); ++i){
        routep.write(&route[i], sizeof(route[i]));
    }
    net->sendPersPacketForMany(net->getAllClientsIps(), routep, net->getAllClientsAcks());
}

void GameState::jumpPlayerTo(UInt8 id, UInt8 space){

}

void GameState::game(){
    if(clientSide){
        this->stateBar.update();
        this->turnOrderList.update(this->net, valueList);
    }
}

void GameState::setupPlayer(Shared<ClientProxy> &client){
    client->money = gType->rules.moneyStart;
    client->currentSpace = gType->rules.startSpace;
}

void GameState::sendPlayerTurnOrderList(){
    if(!net->isServer){
        return;
    }    
    nite::Packet orderList;
    orderList.setHeader(SV_PLAYER_TURN_ORDER_LIST);
    UInt8 n = this->valueList.size();
    orderList.write(&n, sizeof(n));
    for(auto &it : this->valueList){
        UInt8 id = it.first;
        UInt8 value = it.second;
        orderList.write(&id, sizeof(id));
        orderList.write(&value, sizeof(value));
    }
    net->sendPersPacketForMany(net->getAllClientsIps(), orderList, net->getAllClientsAcks());
}

void GameState::init(const unsigned sessionId, const unsigned expPlayers, const Shared<GameType> &gType, bool clientSide){
    this->gType = gType;
    this->sessionId = sessionId;
	this->board = std::make_shared<Board>(Board(this));
	this->dice = std::make_shared<Dice>(Dice(this));
    this->clientSide = clientSide;
    this->expPlayers = expPlayers;
	auto size = nite::Vec2(1080, 1080);    
	board->init(nite::Color("#CDE6D0"), size.x, size.y, gType, !this->clientSide);
	dice->init(gType);    
    board->position.set(nite::getWidth()-size.x, 0.05f);
    totalTurns = 0;
    currentTurn = 0;
    currentPlaying = 0;
    pAllowedShuffle = 0;
    allowThrowDice = false;
    isMoving = false;
    doubleCount = 0;
    
    // CLIENT SIDE
    if(clientSide){

        // Update state bar
        turnOrderList.init();
        stateBar.init();
        stateBar.setText("Waiting for players to join...");

        this->net->onChangeCallback.push_back([&](UInt8 nstate, UInt8 who, Int32 arg){
            switch(nstate){
                case ST_CHOOSE_ORDERS:
                case ST_GAME_PRESTART:
                case ST_PLAYER_THROW_DICE: {
                    this->turnOrderList.main->setVisible(true);
                } break;
                default: {
                    this->turnOrderList.main->setVisible(false);
                } break;
            }
        });

        this->net->addBindAckCallback(BindAckCallback([&](const UInt8 _id, nite::Packet &packet){
            packet.reset();

            String text, color;
            packet.read(text);
            packet.read(color);
            this->stateBar.setText(text);


        }, 0, SV_BROADCAST_NOTIFICATION));          

        this->net->addBindAckCallback(BindAckCallback([&](const UInt8 _id, nite::Packet &packet){
            packet.reset();
            UInt8 id;
            UInt8 target;
            UInt8 nsteps;
            Vector<UInt8> route;

            packet.read(&id, sizeof(id));
            packet.read(&target, sizeof(target));
            packet.read(&nsteps, sizeof(nsteps));


            for(int i = 0; i < nsteps; ++i){
                UInt8 n;
                packet.read(&n, sizeof(n));
                route.push_back(n);
            }

            this->net->clients[id]->setRoute(target, route);


        }, 0, SV_ADVANCE_PLAYER_TO));  

        // SV_ADVANCE_PLAYER_TO
        this->net->addBindAckCallback(BindAckCallback([&](const UInt8 _id, nite::Packet &packet){
            packet.reset();
            UInt8 n;
            packet.read(&n, sizeof(n));
            this->valueList.clear();
            for(int i = 0; i < n; ++i){
                UInt8 id, value;
                packet.read(&id, sizeof(id));
                packet.read(&value, sizeof(value));
                this->valueList[id] = value;
            }
        }, 0, SV_PLAYER_TURN_ORDER_LIST));        

        // SHUFFLE
        this->net->addBindAckCallback(BindAckCallback([&](const UInt8 id, nite::Packet &packet){
            packet.reset();
            UInt8 shuf;
            Vector<UInt8> values;
            packet.read(&shuf, sizeof(shuf));
            dice->setShuffling(shuf);
            UInt8 t;
            packet.read(&t, sizeof(t));
            for(int i = 0; i < t; ++i){
                UInt8 v;
                packet.read(&v, sizeof(v));
                values.push_back(v);
            }
            dice->set(values);
        }, 0, SV_SHUFFLE_DICE));        
    }else{
    // SERVER SIDE
        this->net->addBindAckCallback(BindAckCallback([&](const UInt8 id, nite::Packet &packet){
            if(pAllowedShuffle == id){
                if(dice->shuffling){
                    dice->setShuffling(false);
                    pAllowedShuffle = 0;
                    lastDiceTotalValue = dice->getTotal();
                    lastDiceValues = dice->values;
                    nite::print(net->clients[id]->nickname+" got "+nite::toStr(lastDiceTotalValue));
                }else{
                    dice->setShuffling(true);
                }
            }
        }, 0, SV_SHUFFLE_DICE));
    }
}

void GameState::requestThrowDice(){
    nite::Packet reqThrowDice;
    reqThrowDice.setHeader(SV_SHUFFLE_DICE);
    auto net = static_cast<Client*>(this->net);
    net->sendPersPacketFor(net->sv, reqThrowDice, ++net->svAck);
}

void TurnOrderList::init(){
    this->main = nite::UI::build("data/ui/game/turn_order.json");
    auto win = std::static_pointer_cast<nite::WindowUI>(main);
    win->setShadow(true);
    win->setVisible(false);
    this->targetPosition = nite::Vec2(nite::getWidth() * 0.20f - win->size.x * 0.5f, nite::getHeight() * 0.5f - win->size.y * 0.5f);
    win->setPosition(targetPosition);
}

void TurnOrderList::update(NetHandle *net, Dict<UInt8, unsigned> &valueList){
    auto win = std::static_pointer_cast<nite::WindowUI>(main);
    if(!win->isVisible()){
        return;
    }
    auto panel = win->getComponentById("list");
    if(panel.get() != NULL){
        if(panel->children.size() != valueList.size()){
            panel->clearChildren();
            Vector<UInt8> turnsOrder;

            for(auto &it : valueList){
                turnsOrder.push_back(it.first);
            }

            std::sort(turnsOrder.begin(), turnsOrder.end(), [&](UInt8 a, UInt8 b){
                return valueList[a] > valueList[b];
            });  

            for(int i = 0; i < turnsOrder.size(); ++i){
                auto &client = net->clients[turnsOrder[i]];
                // row
                Jzon::Node rowPanel = Jzon::object();
                rowPanel.add("import", "data/ui/game/templates/name_list_row.json");
                auto row = panel->add(rowPanel);
                auto &orderField = row->children[0];
                auto &nameField = row->children[1];
                auto &diceField = row->children[2];
                
                Jzon::Node jsonNumber = Jzon::object();
                jsonNumber.add("type", "text");        
                jsonNumber.add("font", "data/font/BebasNeueRegular-X34j2.otf");        
                jsonNumber.add("fontColor", "#000000");
                jsonNumber.add("fontSize", 16);        
                jsonNumber.add("text", nite::toStr(i+1));  

                Jzon::Node jsonName = Jzon::object();
                jsonName.add("type", "text");        
                jsonName.add("font", "data/font/BebasNeueRegular-X34j2.otf");        
                jsonName.add("fontColor", "#000000");
                jsonName.add("fontSize", 16);        
                jsonName.add("text", client->nickname);  

                Jzon::Node jsonDice = Jzon::object();
                jsonDice.add("type", "text");        
                jsonDice.add("font", "data/font/BebasNeueRegular-X34j2.otf");        
                jsonDice.add("fontColor", "#000000");
                jsonDice.add("fontSize", 16);        
                jsonDice.add("text", nite::toStr(valueList[turnsOrder[i]]));                                  

                orderField->add(jsonNumber);
                nameField->add(jsonName);
                diceField->add(jsonDice);
            }
        }
    }
}

void StateBar::setText(const String &text){
    this->newText = text;
    this->isNewText = true;
    this->goDown = false;
    this->animTimeout = nite::getTicks();
}

void StateBar::init(){
    this->main = nite::UI::build("data/ui/game/status_bar.json");
    auto win = std::static_pointer_cast<nite::WindowUI>(main);
    win->setShadow(true);
    this->targetPosition = nite::Vec2(nite::getWidth() * 0.20f - win->size.x * 0.5f, 16.0f);
    this->idle = true;
    this->goDown = false;
    this->animTimeout = nite::getTicks();
    this->isNewText = false;
}

void StateBar::update(){
    auto win = std::static_pointer_cast<nite::WindowUI>(main);    
    nite::Vec2 tPosition = targetPosition;


    if(isNewText){
        tPosition.y -= win->size.y * 2.0f;
        if(nite::getTicks() - this->animTimeout > 100){
            isNewText = false;
            this->text = this->newText;
            auto text = std::static_pointer_cast<nite::TextUI>(win->getComponentById("text"));
            text->setText(this->text);

        }
    }


    this->position.lerpDiscrete(tPosition, 0.25f);
    win->setPosition(position);
}

