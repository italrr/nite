#include <memory>

#include "Engine/nScript.hpp"
#include "Engine/View.hpp"

#include "Game.hpp"

#include "RING/RING.hpp"
#include "Engine/Particles.hpp"
#include "Engine/Graphics.hpp"

int main(int argc, char* argv[]){
	Vector<String> params;
	for(int i = 0; i < argc; ++i){
		params.push_back(String(argv[i]));
	}
	nite::setParameters(params);
	
	Game::GameCore game;
	game.start();
	game.client.setup("pepper"+nite::toStr(nite::randomInt(100, 999)));
	nite::nScript debug("./debug_init.ns");
	debug.execute();

	while(game.isRunning){	
		game.update();
		game.render();
	}
	game.onEnd();

	return 0;
}