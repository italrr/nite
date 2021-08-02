#include "Battle.hpp"

Game::Battle::Battle(){
    this->status = Game::BattleStatus::BATTLE_READY;
}

void Game::Battle::setDialog(const Shared<Game::DialogInstance> &dialog){
    this->dialog = dialog;
}

void Game::Battle::start(const Vector<Shared<Game::Entity>> &groupA, const Vector<Shared<Game::Entity>> &groupB){

}

void Game::Battle::end(){

}

void Game::Battle::step(){

}

void Game::Battle::render(){

}