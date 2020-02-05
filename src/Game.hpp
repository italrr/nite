#ifndef GAME_HPP
	#define GAME_HPP

	#include "Engine/Tools/Tools.hpp"

	namespace Game {

		struct GameCore {
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
