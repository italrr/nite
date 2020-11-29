#ifndef GAME_CAMERA_HPP
    #define GAME_CAMERA_HPP

    #include "../Engine/Tools/Tools.hpp"

    namespace Game {

		struct Client;
		struct Camera {
			UInt16 followId;
			nite::Vec2 np;
			Game::Client *client;
			float dir;
			float spd;
			void update();
			void update(nite::Vec2 &v, float mu);
			void update(nite::Vec2 &v);
			void follow(UInt16 id);
			void start(Game::Client *client);
			Camera();
		};

    }
#endif