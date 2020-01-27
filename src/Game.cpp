#include "Game.hpp"
#include "Engine/View.hpp"
#include "Engine/Graphics.hpp"
#include "Engine/Input.hpp"
#include "Engine/nScript.hpp"

static Game::GameCore *instance = NULL;

void Game::GameCore::start(){
	instance = this;
	nite::graphicsInit();
	orch.start();
	isRunning = true;
	stepGeneralTimer.name = "STEP GENERAL";
	drawGeneralTimer.name = "DRAW GENERAL";
}

void Game::GameCore::end(){
	isRunning = false;
}

void Game::GameCore::onEnd(){
	orch.end();
}

void Game::GameCore::update(){
	nite::setZoom(nite::RenderTargetGame, 0.75f);
	nite::viewUpdate();
	nite::inputUpdate();
	orch.update();
}

void Game::GameCore::render(){	
	nite::graphicsUpdate();
	orch.render();
}

Game::GameCore *Game::getGameCoreInstance(){
	return instance;
}

int main(int argc, char* argv[]){

	Vector<String> params;
	for(int i = 0; i < argc; ++i){
		params.push_back(String(argv[i]));
	}
	nite::setParameters(params);

	Game::GameCore game;
	game.start();

	while(game.isRunning){
		game.update();
		game.render();
	}
	game.onEnd();

	return 0;
}
