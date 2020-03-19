#ifndef GAME_ENTITY_BASE_HPP
    #define GAME_ENTITY_BASE_HPP

    #include "../Core/Object.hpp"
    #include "Stat.hpp"

    namespace Game {

        namespace ActionableType {
            enum ActionableType : UInt8 {
                None = 0,
                Skill,
                Item
            };
        }

        namespace DamageType {
            enum DamageType : UInt8 {
                Physical = 0,
                Magical,
                Ranged,
            };
        }

        struct Actionable {
            UInt8 type;
            UInt32 id; // either slotId or skill id
            Actionable(){
                this->id = 0; // an item or skill cannot be 0
                this->type = Game::ActionableType::None;
            }
        };

        struct EntityBase : Game::NetObject, Game::Stat {
            EntityBase(){
                this->effectStat.owner = this;
                this->skillStat.owner = this;
                this->invStat.owner = this;
                this->element = Game::Element::Neutral;
            }
            Game::Actionable actionables[5]; // z x c a s keys
            UInt8 element;
            UInt8 faceDirection;
            String name;
            float walkPushRate;
			void kill();
            void onCreate();
            void draw();
            void entityMove(float angle, float mod, bool holdStance);
            void printInfo();
            bool damage(Int16 amnt, UInt8 elmnt, UInt8 enttype, UInt8 dmgType, bool isCrit);
			virtual void onDeath(){
				
			}            
            void writeInitialState(nite::Packet &packet);
            void readInitialState(nite::Packet &packet);            
        };
    }

#endif 