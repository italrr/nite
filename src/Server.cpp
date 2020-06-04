#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "Game.hpp"
#include "Engine/Network.hpp"
#include "Core/Server.hpp"
#include "Core/Client.hpp"

#include "Entity/Skill.hpp"
#include "Entity/Effect.hpp"
#include "Entity/Inventory.hpp"

int main(int argc, char* argv[]){
	Game::GameCore game;

	Vector<String> params;
	for(int i = 0; i < argc; ++i){
		params.push_back(String(argv[i]));
	}

	Game::DBLoadSkill("data/db/skills.json");
	Game::DBLoadEffect("data/db/effects.json");
	Game::DBLoadInventory("data/db/items.json");

    auto indexer = nite::getIndexer();
    indexer->indexDir("./data/map/");
    indexer->indexDir("./data/tileset/");
    indexer->indexDir("./data/texture/tileset/");
	indexer->indexDir("./data/texture/overworld/");

    int opt;
    UInt16 maxPlayers = 4;
    UInt16 port = nite::NetworkDefaultPort;
    bool running = true;
    UInt8 difficulty = 4;
    UInt8 mapNumber = 1; // TODO: this is 4, in production
    String name = "LID-Dedi-"+nite::getTimestamp()+"-"+nite::toStr(nite::randomInt(25000, 75000));
    UInt64 timeout = nite::getTicks();
    static UInt64 ticktime = 1000 / 30; 
    auto &sv = game.localSv;
    sv.preinit();

    if(params.size() > 1){
        for(int i = 1; params.size(); ++i){
            if((params[i] == "--max-players" || params[i] == "-mp") && i < params.size()-1){
                if(nite::isNumber(params[i+1])){
                    maxPlayers = nite::toInt(params[i+1]);
                }else{
                    nite::print(params[i]+" must be a number");
                }
            }
            if((params[i] == "--port" || params[i] == "-p") && i < params.size()-1){
                if(nite::isNumber(params[i+1])){
                    port = nite::toInt(params[i+1]);
                }else{
                    nite::print(params[i]+" must be a number");
                }
            }     
            if((params[i] == "--tick-rate" || params[i] == "-tr") && i < params.size()-1){
                if(nite::isNumber(params[i+1])){
                    ticktime = nite::toInt(params[i+1]);
                }else{
                    nite::print(params[i]+" must be a number");
                }
            }  
            if((params[i] == "--name" || params[i] == "-tr") && i < params.size()-1){
                if(params[i+1].length() > 2){
                    name = params[i+1];
                }else{
                    nite::print(params[i]+" must be at least 2 characters long");
                }
            }    
            if((params[i] == "--map-number" || params[i] == "-mn") && i < params.size()-1){
                if(nite::isNumber(params[i+1])){
                    mapNumber = nite::toInt(params[i+1]);
                }else{
                    nite::print(params[i]+" must be a number");
                }
            }                                
        }        
    }

    nite::socketInit();
    sv.setupGame(name, maxPlayers, mapNumber);

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
