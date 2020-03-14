
#include "Game.hpp"

int main(int argc, char* argv[]){
	Vector<String> params;
	for(int i = 0; i < argc; ++i){
		params.push_back(String(argv[i]));
	}
	nite::setParameters(params);
	
	Game::GameCore game;
	game.start();

	game.localSv.preinit();
	game.localSv.setupGame("Pacifier's corner", 4, 1);

	game.client.setup("pepper"+nite::toStr(nite::randomInt(25, 50)));

	nite::AsyncTask::spawn(nite::AsyncTask::ALambda([&](nite::AsyncTask::Context &ct){
		game.client.connect("127.0.0.1", nite::NetworkDefaultPort);
		ct.stop();
	}), 1000);

	while(game.isRunning){			
		game.update();
		game.render();
	}
	game.onEnd();

	return 0;
}
