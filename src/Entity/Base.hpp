#ifndef GAME_ENTITY_BASE_HPP
    #define GAME_ENTITY_BASE_HPP

    #include "../Network/Object.hpp"
    #include "Stat.hpp"

    namespace Game {

        namespace ActionableType {
            enum ActionableType : UInt8 {
                None = 0,
                Skill,
                Item
            };
        }

        struct Actionable {
            UInt8 type;
            UInt32 id; // either item-instance-id or skill id
            Actionable(){
                this->id = 0; // an item or skill cannot be 0
                this->type = Game::ActionableType::None;
            }
        };

        struct EntityBase : Game::NetObject, Game::Stat {
            Game::Actionable actionables[5]; // z x c a s keys
            UInt8 faceDirection;
            String name;
            float walkPushRate;
			void kill();
            void onCreate();
            void draw();
            void entityMove(float angle, float mod, bool holdStance);
			virtual void onDeath(){
				
			}            
            void writeInitialStateForSync(nite::Packet &packet);
            void readInitialStateForSync(nite::Packet &packet);            
        };
    }

#endif 