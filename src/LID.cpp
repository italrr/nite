#include <memory>

#include "Engine/nScript.hpp"
#include "Engine/View.hpp"

#include "Game.hpp"

#include "RING/RING.hpp"
#include "Engine/Particles.hpp"
#include "Engine/Graphics.hpp"
#include "Engine/Console.hpp"


namespace DEFAULT_MAPPING {
	void __exportall(const String &path);
}

void __temp();

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


    // Game::RING::TileSource src("data/tileset/alp_ruins.json"); 
        
    // auto bp = Shared<Game::RING::Blueprint>(new Game::RING::Blueprint());
    // bp->generate(35, 35);
    // auto map = Game::RING::generateMap(bp, src, 4);

	while(game.isRunning){	
		game.update();
		// __temp();
		// if(nite::keyboardPress(nite::keySPACE)){
		// 	map->unload();
		// 	nite::Console::interpret("ren_reload_texture 0", false, false, false, false);
		// 	src = Game::RING::TileSource("data/tileset/alp_ruins.json");
		// 	map = Game::RING::generateMap(bp, src, 4);
		// }

		// nite::setZoom(nite::RenderTargetGame, 1.0f);
		// game.client.camera.update();
		// auto pos = nite::getView(nite::RenderTargetGame);
        // auto sizeAdj = nite::getAdjustedSize();
        // auto size = nite::getSize();
        // auto sizeDiff = (sizeAdj - size);
        // auto sizeDiffH = sizeDiff * nite::Vec2(0.5f);
        // // draw tiles within view scope plus targetExcess diff (getAdjustedSize)
        // map->draw(nite::Vec2(0.0f), nite::Rect(pos.x - sizeDiffH.x, pos.y - sizeDiffH.y, nite::getWidth() + sizeDiff.x, nite::getHeight() + sizeDiff.y));

		game.render();
	}
	game.onEnd();

	// DEFAULT_MAPPING::__exportall("criteria.json");


	return 0;
}