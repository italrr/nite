#ifndef GAME_CAMERA_HPP
    #define GAME_CAMERA_HPP

    #include "Engine/Tools/Tools.hpp"

    namespace Game {

		struct Camera {
			int followId;
			void update();
			void update(nite::Vec2 &v, float mu);
			void update(nite::Vec2 &v);
			void follow(int id);
			Camera();
		};

    }
#endif