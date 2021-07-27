#ifndef GAME_OBJECT_HPP
    #define GAME_OBJECT_HPP

    #include "Engine/Tools/Tools.hpp"

    namespace Game {

        struct Object {
            UInt16 id;
            nite::Vec2 position;
            nite::Vec2 vel;
            nite::Vec2 accel;
            nite::Vec2 size;
            nite::Vec2 lerpPos;
            float mass;
            float invMass;
            float friction;
            
            UInt8 moveState;

            Object();

            virtual void create(){

            }

            virtual void step(){

            }

            virtual void render(){

            }

            virtual void onDestroy(){

            }

            void setMass(float mass);
            void reshape(const nite::Vec2 &size);
            void push(float force, float angle);
            void move(float x = 0.0f, float y = 0.0f);
            void setPosition(const nite::Vec2 &p);
        };     

    }


#endif