#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "Game.hpp"
#include "Engine/Network.hpp"
#include "Core/Server.hpp"
#include "Core/Client.hpp"


int main(int argc, char* argv[]){
	Game::GameCore game;

    int opt;
    UInt16 maxPlayers = 4;
    UInt16 port = nite::NetworkDefaultPort;
    bool running = true;
    String name = "LID-Dedi-"+nite::getTimestamp()+"-"+nite::toStr(nite::randomInt(250, 750));
    UInt64 timeout = nite::getTicks();
    static const UInt64 ticktime = 1000 / 30; 
    Game::Server sv;

    while ((opt = getopt(argc, argv, "pnm")) != -1) {
        switch (opt) {
        case 'p': // port
            port = atoi(optarg);
            break;
        case  'n': // server name
            name = String(optarg);
            break;
        case 'm': // max number of players
            maxPlayers = atoi(optarg); 
            break;
        default:
            fprintf(stderr, "undefined parameter %c", opt);
        }
    }

    nite::socketInit();
    sv.setupGame(name, maxPlayers, 1);

    while(running){
        if(nite::getTicks()-timeout > ticktime){
            sv.update();
            timeout = nite::getTicks();
        }
        running = sv.init;
    }

    nite::socketEnd();

	return 0;
}
