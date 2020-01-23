#include "Network.hpp"
#include "../Engine/Tools/Tools.hpp"
#include "../Engine/Tools/hashids.hpp"

static int _uniqueId = nite::randomInt(125, 375);

Game::Net::Net(){
    setState(Game::NetState::Disconnected);
}

void Game::Net::setState(unsigned state){
    this->lastState = nite::getTicks();
    this->state = state;
}

String Game::Net::generateId(){
    // Does this satisfy our needs?
    hashidsxx::Hashids hash("nite Engine salt");
    return hash.encode({nite::getTicks() + ++_uniqueId});
}

void Game::Net::step(){

}