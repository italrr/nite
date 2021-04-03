#include "Engine/Graphics.hpp"
#include "Engine/Input.hpp"
#include "Engine/View.hpp"
#include "Engine/UI/UI.hpp"
#include "Engine/Network.hpp"

#include "GameType.hpp"
#include "Board.hpp"
#include "Dice.hpp"
#include "Client.hpp"
#include "Server.hpp"
#include "Network.hpp"

int main(int argc, char* argv[]){
	Vector<String> params;
	for(int i = 0; i < argc; ++i){
		params.push_back(String(argv[i]));
	}
	nite::setParameters(params);
	bool onlyClient = false;
	for(int i = 0; i < params.size(); ++i){
		if(params[i] == "cl"){
			onlyClient = true;
		}
	}

	nite::graphicsInit();
	nite::inputInit();

	
	Client client;
	Server server;
	auto gType = loadGameType("data/games/traditional.json");
	auto gameServer = std::make_shared<GameState>(GameState());
	auto gameClient = std::make_shared<GameState>(GameState());
	unsigned sessionId = nite::randomInt(1000, 2500);

	if(!onlyClient)
	server.init(gameServer);
	client.init(gameClient, onlyClient ? "johan" : "pepper", onlyClient ? nite::Color(1.0f, 0.0f, 0.0f) : nite::Color(1.0f, 1.0f, 0.5f));
	
	if(!onlyClient)
	gameServer->init(sessionId, 2, gType, false);

	gameClient->init(sessionId, 2, gType, true);
	
	if(!onlyClient)
	server.listen(nite::IP_Port("127.0.0.1", DEFAULT_PORT));
	client.connect(nite::IP_Port("127.0.0.1", DEFAULT_PORT));

	while(nite::isRunning()){	
		nite::viewUpdate();
		nite::inputUpdate();
		if(!onlyClient)
		server.update();	
		client.update();
		client.draw();
		nite::graphicsRender();
	}
	nite::graphicsEnd();


	return 0;
}