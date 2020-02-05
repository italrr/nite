#include "Game.hpp"
#include "Engine/View.hpp"
#include "Engine/Graphics.hpp"
#include "Engine/Input.hpp"
#include "Engine/nScript.hpp"
#include "Engine/UI/UI.hpp"

static Game::GameCore *instance = NULL;

void Game::GameCore::start(){
	instance = this;
	nite::graphicsInit();
	isRunning = true;
	stepGeneralTimer.name = "STEP GENERAL";
	drawGeneralTimer.name = "DRAW GENERAL";
}

void Game::GameCore::end(){
	isRunning = false;
}

void Game::GameCore::onEnd(){
}

void Game::GameCore::update(){
	nite::setZoom(nite::RenderTargetGame, 0.75f);
	nite::viewUpdate();
	nite::inputUpdate();
}

void Game::GameCore::render(){	
	nite::graphicsRender();
}

Game::GameCore *Game::getGameCoreInstance(){
	return instance;
}

#include "Network/Server.hpp"
#include "Network/Client.hpp"

int main(int argc, char* argv[]){

	Vector<String> params;
	for(int i = 0; i < argc; ++i){
		params.push_back(String(argv[i]));
	}
	nite::setParameters(params);

	Game::GameCore game;
	game.start();

	Game::Server sv;
	
	sv.preinit();
	sv.setupGame(4, 1);

	Game::Client cl;
	cl.setup("pepper");
	nite::AsyncTask::spawn(nite::AsyncTask::ALambda([&](nite::AsyncTask::Context &ct){
		cl.connect("127.0.0.1", nite::NetworkDefaultPort);
		ct.stop();
	}), 1000);


	Game::Client cl2;
	cl2.setup("churro");
	nite::AsyncTask::spawn(nite::AsyncTask::ALambda([&](nite::AsyncTask::Context &ct){
		cl2.connect("127.0.0.1", nite::NetworkDefaultPort);
		ct.stop();
	}), 3000);	

	UInt64 t = nite::getTicks();

	while(game.isRunning){
		sv.update();
		cl.update();
		if(nite::getTicks()-t < 10000)
		cl2.update();
		game.update();
		game.render();
	}
	game.onEnd();

	return 0;
}
