#include "Game.hpp"
#include "Engine/View.hpp"

static Game::GameCore *instance = NULL;

void Game::GameCore::start(){
	instance = this;
	nite::graphicsInit();
	nite::inputInit();
	isRunning = true;
	stepGeneralTimer.name = "STEP GENERAL";
	drawGeneralTimer.name = "DRAW GENERAL";
}

void Game::GameCore::end(){
	isRunning = false;
}

void Game::GameCore::onEnd(){
	nite::graphicsEnd();
	this->localSv.close();	
	this->client.disconnect();
	nite::print("bye!");
}

void Game::GameCore::update(){
	nite::setZoom(nite::RenderTargetGame, 0.75f);
	nite::viewUpdate();
	nite::inputUpdate();
	this->client.update();
	this->localSv.update();
}

void Game::GameCore::render(){	
	this->client.render();
	nite::graphicsRender();
}

Game::GameCore *Game::getGameCoreInstance(){
	return instance;
}