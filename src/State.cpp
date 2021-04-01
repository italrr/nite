#include <memory>
#include "Network.hpp"
#include "Server.hpp"
#include "State.hpp"


void GameState::update(){
    if(clientSide){
        return; 
    }
    auto net = static_cast<Server*>(this->net);

    switch(this->net->state){
        case ST_WAITING_FOR_PLAYERS: {
            if(net->clients.size() == expPlayers && nite::getTicks()-net->lastState > 2500){
                net->setState(ST_GAME_START);
            }
            // TODO: handle wait for too long...
        } break;
        case ST_GAME_START: {
            if(nite::getTicks()-net->lastState > 2500){
                net->setState(ST_CHOOSE_ORDERS);
            }
        } break;
    }
}

void GameState::game(){
    this->statusBar.update();
}

void GameState::setupPlayer(Shared<ClientProxy> &client){
    client->money = gType->rules.moneyStart;
}

void GameState::init(const unsigned sessionId, const unsigned expPlayers, const Shared<GameType> &gType, bool clientSide){
    this->gType = gType;
    this->sessionId = sessionId;
	this->board = std::make_shared<Board>(Board());
	this->dice = std::make_shared<Dice>(Dice());
    this->clientSide = clientSide;
    this->expPlayers = expPlayers;
	auto size = nite::Vec2(1080, 1080);    
	board->init(nite::Color("#CDE6D0"), size.x, size.y, gType, !this->clientSide);
	dice->init(gType);    
    totalTurns = 0;
    // build ui to use
    if(clientSide){
        statusBar.init();
        statusBar.setText("Waiting for players to join...");
        this->net->onChangeCallback.push_back([&](UInt8 nstate){
            switch(nstate){
                case ST_GAME_START: {
                    this->statusBar.setText("GAME STARTS!");
                } break;
                case ST_CHOOSE_ORDERS: {
                    this->statusBar.setText("TIME TO DECIDE WHO GOES FIRST...");
                } break;                
            }
        });
    }
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

