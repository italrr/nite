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

			virtual void entityStep(){
				
			}

			virtual void update(){
				
			}

			virtual void step(){

			}

			virtual void entityDraw(){

			}

			virtual void draw(){

			}
			virtual void updatePhysics(){

			}
		};

		struct PhysicsObject : public BaseObject {
			int id;
			World *container;						
			nite::Vec2 position;
			nite::Vec2 speed;
			nite::Vec2 size;
			
			float friction;
			float mass;
			void destroy();
			float relativeTimescale;
			bool solid;
			bool unmovable;
			bool collided;
			PhysicsObject();
			void push(float angle, float force);
			void move(float angle, float mod);
			bool isCollidingWith(PhysicsObject *other);
			bool isCollidingWithSomething();
			bool isCollidingWithExcept(const Vector<nite::PhysicsObject*> &ignores);
			float getDistance(PhysicsObject *other);
			void physicsObjectUpdate(float x, float y);
			void updatePhysics();
			virtual void onCollision(PhysicsObject *obj){}
		};
	}

#endif
