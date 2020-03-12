#ifndef GAME_ENTITY_STATUS_HPP
    #define GAME_ENTITY_STATUS_HPP

    #include "../Engine/Tools/Tools.hpp"
    #include "../Entity/Base.hpp"

    namespace Game {

		namespace EffectType {
			enum EffectType : UInt16 {
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
            UInt64 duration;
            UInt64 started;
            UInt64 lastStep;
            UInt16 type;
            UInt16 target;
            UInt16 owner;

            Effect(){
                this->name = "None";
                this->owner = 0;
                this->target = 0;
                this->duration = 1000 * 30;
            }

            void start(){
                this->lastStep = nite::getTicks();
                this->started = nite::getTicks();
                buildDesc();
                onStart();
            }

            virtual void step(Game::EntityBase &ent){

            }

            virtual void buildDesc(){
                this->desc = "None";
            }
            
            virtual void onStart(){

            }

            virtual void onEnd(){

            }
		};

        struct EffectStat {
            Vector<Game::Effect> effects;
            bool add(Game::Effect status);
            bool remove(UInt16 type);
            bool remove(const String &name);
            bool isOn(UInt16 type);
            void removeAll();
            void update(Game::EntityBase &ent);
        };

        /*
            EFFECTS
        */
        struct Eff_EffHeal : Effect {
            UInt16 amnt;
            Eff_EffHeal(UInt16 amnt){
                this->type = Game::EffectType::EF_HEAL;
                this->amnt = amnt;
                this->duration = 1000 * 10;
                this->name = "Effective Heal";
            }
            void buildDesc(){
                this->desc = "Healing "+nite::toStr(amnt)+" HP per second";
            }
            void step(Game::EntityBase &ent);
        };      

    }

#endif