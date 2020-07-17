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

		struct PhysicsObject : public nite::BaseObject {
			UInt16 id;
			nite::World *container;						
			nite::Vec2 position;
			nite::Vec2 speed;
			nite::Vec2 size;
			float direction;
			float friction;
			float mass;
			virtual void destroy();
			float relativeTimescale;
			bool solid;
			bool unmovable;
			bool collided;
			PhysicsObject();
			virtual bool move(const nite::Vec2 &dir);
			virtual bool push(const nite::Vec2 &dir);
			bool isCollidingWith(nite::PhysicsObject *other);
			bool isCollidingWithSomething();
			bool isCollidingWithExcept(const Vector<nite::PhysicsObject*> &ignores);
			float getDistance(nite::PhysicsObject *other);
			virtual void onCollision(nite::PhysicsObject *obj){

			}
		};
	}

#endif
