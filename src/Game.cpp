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
	nite::inputInit();
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

	// game.localSv.preinit();
	// game.localSv.setupGame("Pacifier's corner", 4, 1);

	// game.client.setup("pepper"+nite::toStr(nite::randomInt(25, 50)));

	// nite::AsyncTask::spawn(nite::AsyncTask::ALambda([&](nite::AsyncTask::Context &ct){
	// 	game.client.connect("127.0.0.1", nite::NetworkDefaultPort);
	// 	ct.stop();
	// }), 1000);

	nite::UI::build("data/ui/hud/main.json");
	while(game.isRunning){			
		game.update();
		game.render();
	}
	game.onEnd();

	return 0;
}
