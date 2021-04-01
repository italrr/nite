#include <memory>
#include <algorithm>
#include "Network.hpp"
#include "Server.hpp"
#include "Client.hpp"
#include "State.hpp"


void GameState::update(){
    dice->update();
    if(clientSide){
        return; 
    }
    auto net = static_cast<Server*>(this->net);
    
    switch(this->net->state){
        case ST_WAITING_FOR_PLAYERS: {
            if(net->clients.size() == expPlayers && nite::getTicks()-net->lastState > 2000){
                net->setState(ST_GAME_START);
            }
            // TODO: handle wait for too long...
        } break;
        case ST_GAME_START: {
            if(nite::getTicks()-net->lastState > 2500){
                net->setState(ST_CHOOSE_ORDERS);
            }
            Vector<unsigned> list;
            for(auto &it : net->clients){
                list.push_back(it.second->id);
            }
            std::sort(list.begin(), list.end(), [&](UInt8 a, UInt8 b){
                return net->clients[a]->order < net->clients[b]->order;
            });
            this->pickList = list;
        } break;
        case ST_CHOOSE_ORDERS: {
            if(nite::getTicks()-net->lastState > 2500){
                net->setState(ST_PLAYER_THROW_DICE, this->pickList[0], 0);
                this->pAllowedShuffle = this->pickList[0];
                this->lastDiceValue = 0;
            }            
        } break;
        case ST_PLAYER_THROW_DICE: {
            if(pAllowedShuffle == 0 && nite::getTicks()-net->lastState > 100){
                this->valueList[this->pickList[0]] = lastDiceValue;
                this->pickList.erase(this->pickList.begin());
                if(this->pickList.size() > 0){
                    net->setState(ST_PLAYER_THROW_DICE, this->pickList[0], 0);
                    this->pAllowedShuffle = this->pickList[0];
                    this->lastDiceValue = 0;
                }else{
                    net->setState(ST_PLAYER_TURN, 0, 0);
                    
                    // std::sort(net->clients.begin(), net->clients.end(), [&](Shared<ClientProxy> a, Shared<ClientProxy> b){
                    //     return this->valueList[a->id] < this->valueList[b->id];
                    // });
                    // int count = 0;
                    // for(auto &it : net->clients){
                    //     it.second->order = count;
                    //     ++count;
                    // }

                    // for(auto &it : net->clients){
                    //     nite::print(it.second->order);
                    // }
                }
            }
        } break;
        case ST_PLAYER_TURN: {
            // nite::print("got it");
        } break;


    }
}

void GameState::game(){
    if(clientSide){
        this->statusBar.update();
        // if(allowThrowDice)
    }
}

void GameState::setupPlayer(Shared<ClientProxy> &client){
    client->money = gType->rules.moneyStart;
}

void GameState::init(const unsigned sessionId, const unsigned expPlayers, const Shared<GameType> &gType, bool clientSide){
    this->gType = gType;
    this->sessionId = sessionId;
	this->board = std::make_shared<Board>(Board());
	this->dice = std::make_shared<Dice>(Dice(this));
    this->clientSide = clientSide;
    this->expPlayers = expPlayers;
	auto size = nite::Vec2(1080, 1080);    
	board->init(nite::Color("#CDE6D0"), size.x, size.y, gType, !this->clientSide);
	dice->init(gType);    
    totalTurns = 0;
    currentTurn = 0;
    pAllowedShuffle = 0;
    allowThrowDice = false;
    
    // CLIENT SIDE
    if(clientSide){
        statusBar.init();
        statusBar.setText("Waiting for players to join...");
        this->net->onChangeCallback.push_back([&](UInt8 nstate, UInt8 who, Int32 arg){
            switch(nstate){
                case ST_GAME_START: {
                    this->statusBar.setText("GAME  STARTS!");
                } break;
                case ST_CHOOSE_ORDERS: {
                    this->statusBar.setText("TIME TO DECIDE WHO GOES FIRST...");
                } break;                
                case ST_PLAYER_THROW_DICE: {
                    this->statusBar.setText(this->net->clients[who]->nickname+", throw the dice");
                } break;                 
                case ST_PLAYER_TURN: {                    
                    this->statusBar.setText("Rickiy's trn bitch :)");
                } break;                    
            }
        });
        this->net->addBindAckCallback(BindAckCallback([&](const UInt8 id, nite::Packet &packet){
            packet.reset();
            UInt8 shuf;
            Vector<int> values;
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
                    lastDiceValue = dice->getTotal();
                    // nite::print(net->clients[id]->nickname+" got "+nite::toStr(lastDiceValue));
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

void StatusBar::setText(const String &text){
    this->newText = text;
    this->isNewText = true;
    this->goDown = false;
    this->animTimeout = nite::getTicks();
}

void StatusBar::init(){
    this->main = nite::UI::build("data/ui/game/status_bar.json");
    auto win = std::static_pointer_cast<nite::WindowUI>(main);
    this->targetPosition = nite::Vec2(nite::getWidth() * 0.75f - win->size.x * 0.5f, 16.0f);
    this->idle = true;
    this->goDown = false;
    this->animTimeout = nite::getTicks();
    this->isNewText = false;
}

void StatusBar::update(){
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

