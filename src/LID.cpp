
#include "Engine/nScript.hpp"

#include "Game.hpp"

int main(int argc, char* argv[]){
	Vector<String> params;
	bool runserver = false;
	for(int i = 0; i < argc; ++i){
		if(String(argv[i]) == "-sv"){
			runserver = true;
		}
		params.push_back(String(argv[i]));
	}
	nite::setParameters(params);
	
	Game::GameCore game;
	game.start();

	if(runserver){
		game.localSv.preinit();
		game.localSv.setupGame("Pacifier's corner", 4, 1);
	}	

	game.client.setup("pepper"+nite::toStr(nite::randomInt(25, 50)));


	nite::nScript debug("./debug_init.ns");
	debug.execute();

	// nite::UI::build("data/ui/ingame_menu/main.json");
	while(game.isRunning){			
		game.update();
		game.render();
	}
	game.onEnd();

	return 0;
}