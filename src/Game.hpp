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
			pthread_t svThread; 
			UInt64 svLastTick;
			UInt64 svTickRate;
			nite::StepTimer stepGeneralTimer;
			nite::StepTimer drawGeneralTimer;

			void start();
			void update();
			void render();
			void end();
			void onEnd();
			void spawnServerThread();
			void killServerThread();
		};

		Game::GameCore *getGameCoreInstance();

	}  

#endif
