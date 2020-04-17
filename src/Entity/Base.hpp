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

        static const UInt64 RecalculateStatTimeout = 150; 

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
                this->lastUpdateStats = nite::getTicks();
                this->objType = ObjectType::Entity;

            }
            Game::Actionable actionables[5]; // z x c a s keys
            UInt8 faceDirection;
            String name;
            float walkPushRate;
            bool dead;
			void kill();
            void onCreate();
            void draw();
            void entityMove(float angle, float mod, bool holdStance);
            void entityStep();
            void printInfo();
            bool damage(const Game::DamageInfo &dmg);
            UInt64 lastUpdateStats;
            void recalculateStats();
			virtual void onDeath(){
				
			}   

            // stats         
            virtual void writeInitialState(nite::Packet &packet);
            virtual void readInitialState(nite::Packet &packet);     
            virtual void writeAllStatState(nite::Packet &packet);       
            virtual void readAllStatState(nite::Packet &packet); 

            virtual void writeHealthStatState(nite::Packet &packet);       
            virtual void readHealthStatState(nite::Packet &packet);           
        };
    }

#endif 