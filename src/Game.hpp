#ifndef GAME_HPP
	#define GAME_HPP

	#include "Engine/Tools/Tools.hpp"
	#include "Core/Server.hpp"
	#include "Core/Client.hpp"
	namespace Game {

		struct GameCore {
			Game::Client client;
			Game::Server localSv;
			bool isRunning;
			void start();
			void update();
			void render();
			void end();
			void onEnd();
			nite::StepTimer stepGeneralTimer;
			nite::StepTimer drawGeneralTimer;
		};

		Game::GameCore *getGameCoreInstance();

	}  

#endif
