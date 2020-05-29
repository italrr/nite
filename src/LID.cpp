#include <memory>

#include "Engine/nScript.hpp"
#include "Engine/View.hpp"

#include "Game.hpp"

#include "RING/RING.hpp"

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

	// if(runserver){
	// 	game.localSv.preinit();
	// 	game.localSv.setupGame("Pacifier's corner", 4, 1);
	// }	
	// game.client.setup("pepper"+nite::toStr(nite::randomInt(100, 999)));
	// nite::nScript debug("./debug_init.ns");
	// debug.execute();

	nite::FileTransfer::UDPClient sender;
	sender.listen(nite::NetworkDefaultFileTransferPort);
	sender.indexDir("data/map/");
	nite::FileTransfer::UDPClient receiver;
	receiver.listen(nite::NetworkDefaultFileTransferPort + 1);
	while(game.isRunning){	
		if(nite::keyboardPressed(nite::keyF1)){
			receiver.request(nite::IP_Port("127.0.0.1", nite::NetworkDefaultFileTransferPort), "4aec2d931e5c57f763ebbcbd9ca9f793", "./yes.json", [](const nite::FileTransfer::IndexedFile &file){
				nite::print("finished '"+file.path+"'");
			});	
		}
		game.update();
		game.render();
	}
	game.onEnd();

	return 0;
}