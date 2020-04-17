#ifndef GAME_ENTITY_EFFECT_HPP
    #define GAME_ENTITY_EFFECT_HPP

    #include "../Engine/Tools/Tools.hpp"
    #include "../Engine/Packets.hpp"

    namespace Game {
        struct EntityBase;

		namespace EffectList {
			enum EffectList : UInt16 {
				NONE = 0,

                EXPBONUS,
				
				REL_HEAL,
				INST_HEAL,
                INSTEF_HEAL,
                EF_HEAL,

				PARALYZED,
				BURNED,
				POISONED,

                IMMUNITY_E, // elements
                IMMUNITY_C, // class
                IMMUNITY_T, // type (race)

				// Increments
				STR_INC,
				AGI_INC,
				DEX_INC,
				LUCK_INC,
				INT_INC,
				CHA_INC,

				CARRY_INC,
				ATK_INC,
				MATK_INC,
				DEF_INC,
                MDEF_INC,
                CRITR_INC,
                ATKR_INC,
                WALKR_INC,
                PRECR_INC,
                PERSR_INC,
                CHARR_INC,

                HEALTH_INC,
                MANA_INC,
                STAMINA_INC


			};
		}

        struct NetWorld;
        struct Effect {
            String name;
            String desc;
            String status;
            UInt64 duration;
            UInt64 started;
            UInt64 lastStep;
            UInt16 type;
            UInt16 insId;
            int iconId;
            Game::EntityBase *owner;
            nite::Color color;
            UInt16 amnt;
            UInt16 amntpersecond;            

            Effect(){
                this->name = "None";
                this->owner = NULL;
                this->duration = 1000 * 30;
                this->color.set("#E24E75");
                this->status = "";
                iconId = 0;
            }

            void start(EntityBase *owner){
                this->lastStep = nite::getTicks();
                this->started = nite::getTicks();
                onStart(owner);
            }

            virtual void setup(){
                
            }

            virtual String getStatus(Game::EntityBase *owner){
                return nite::toStr((nite::getTicks()-(started+this->duration))/1000)+ "s";
            }
            
            virtual void onStart(Game::EntityBase *owner){

            }

            virtual void onEnd(Game::EntityBase *owner){

            }

            virtual void readState(nite::Packet &in){

            }

            virtual void writeState(nite::Packet &out){

            }

            virtual void readInitialState(nite::Packet &in){
                in.read(&started, sizeof(started));
                in.read(&duration, sizeof(started));
            }

            virtual void writeInitialState(nite::Packet &out){
                out.write(&started, sizeof(started));
                out.write(&duration, sizeof(started));
            }            

            virtual bool step(Game::EntityBase *owner){
                return false;
            }

            virtual void onRecalculateStat(Game::EntityBase *owner){

            }

			void parse(Jzon::Node &obj);

			virtual void parseSpecial(Jzon::Node &obj){
				
			}            
		};

        struct EffectStat {
            EntityBase *owner;
            bool hasChanged;
            Dict<UInt16, Shared<Game::Effect>>  effects;
            bool add(Shared<Game::Effect> eff);
            bool add(Shared<Game::Effect> eff, UInt16 insId);
            bool remove(UInt16 insId);
            bool remove(const String &name);
            bool removeByType(UInt16 type);
            bool isOn(UInt16 insId);
            bool isOnByType(UInt16 type);
            void removeAll();
            bool update();
            EffectStat(){
                hasChanged = false;
            }
        };

        /*
            EFFECTS
        */
        namespace Effects {
            struct EffHeal : Game::Effect {
                EffHeal(){
                    this->type = Game::EffectList::EF_HEAL;
                }
                String getStatus(Game::EntityBase *owner);
                void setup(UInt16 amnt, UInt64 time);
                bool step(Game::EntityBase *owner);
            };             
        }


        Shared<Game::Effect> getEffect(UInt16 type);  
        void DBLoadEffect(const String &path);

    }

#endif