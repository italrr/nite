#ifndef GAME_DAMAGE_HPP
    #define GAME_DAMAGE_HPP

    #include "../Engine/Object.hpp"

    namespace Game {

		struct EntityBase;
		struct EquipItem;

		struct MeleeHitInformation {
			bool hit;
			nite::Hitbox hitbox;
			Int16 target;
			MeleeHitInformation(){
				hit = false;
			}
		};

        namespace DamageType {
            enum DamageType : UInt8 {
                Physical = 0,
                Magical,
                Ranged,
            };
        }  		

		struct DamageInfo {
			UInt16 emitter;
			UInt16 receiver;
			UInt16 weap; // slotId
			UInt8 dmgtype;
			Int32 amnt;
			bool truedmg;
			UInt8 elmnt;
			bool isCrit;
			nite::Vec2 position;
			DamageInfo(){
				this->isCrit = false;
				this->truedmg = false;
				this->position.set(-1.0f);
			}
		};     

		namespace Element {
			enum Element : UInt8 {
				None = 0,
				Neutral,
				Fire,
				Electric,
				Steel,
				Wind,
				Umbra,
				Celestial,
				Emerium
			};
			static String name(int id){
				switch(id){
					case Neutral:
						return "Neutral";
					case Fire:
						return "Fire";
					case Electric:
						return "Electric";
					case Umbra:
						return "Umbra";	
					case Celestial:
						return "Wind";						
					case Wind:
						return "Celestial";							
					case Steel:
						return "Steel";			
					case Emerium:
						return "Emerium";			
					default:
						return "Undefined";																										
				}
			}
			// Effectiveness or uneffectiveness
			// TODO: load this from a file
			static const float EffectiveTable[8][8] = {
				//neutral   	fire  		electric steel   		wind  		umbra 		celestial 	emerium
				{1.0f,			1.0f, 		1.0f, 		1.0f, 		1.0f, 		1.0f, 		1.0f, 		1.0f},   // neutral
				{1.0f,			1.0f, 		1.0f, 		1.0f, 		1.0f, 		1.0f, 		1.0f, 		1.0f},   // fire
				{1.0f,			1.0f, 		1.0f, 		1.0f, 		1.0f, 		1.0f, 		1.0f, 		1.0f},   // electric
				{1.0f,			1.0f, 		1.0f, 		1.0f, 		1.0f, 		1.0f, 		1.0f, 		1.0f},   // steel
				{1.0f,			1.0f, 		1.0f, 		1.0f, 		1.0f, 		1.0f, 		1.0f, 		1.0f},   // wind
				{1.0f,			1.0f, 		1.0f, 		1.0f, 		1.0f, 		1.0f, 		1.0f, 		1.0f},   // umbra
				{1.0f,			1.0f, 		1.0f, 		1.0f, 		1.0f, 		1.0f, 		1.0f, 		1.0f},   // celestial
				{1.0f,			1.0f, 		1.0f, 		1.0f, 		1.0f, 		1.0f, 		1.0f, 		1.0f},   // emerium
				
			};
			static float isEffective(UInt8 from, UInt8 to){
				return from > 8 || to > 8 ? 0.0f : EffectiveTable[from][to];
			}
		}		 
    }

#endif