#ifndef GAME_OBJECT_HPP
	#define GAME_OBJECT_HPP

	#include "Types.hpp"
	#include "Tools/Tools.hpp"
	namespace nite {
		struct World;

		struct Hitbox {
			nite::Vec2 position;
			nite::Vec2 size;
			String name;
			int type;
			Hitbox(const Jzon::Node &node);
			Hitbox();
			void load(const Jzon::Node &node);
			bool collision(const Hitbox &box);
		};

		struct BaseObject {
			virtual void onCreate(){

			}

			virtual void onDestroy(){

			}

			virtual void onStep(){

			}

			virtual void onDraw(){

			}

			virtual void step(){

			}

			virtual void draw(){

			}
		};
	}

#endif
