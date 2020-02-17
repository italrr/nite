#ifndef GAME_DAMAGE_HPP
    #define GAME_DAMAGE_HPP

    #include "../Engine/Object.hpp"

    namespace Game {

		struct MeleeHitInformation {
			bool hit;
			nite::Hitbox hitbox;
			Int16 target;
			MeleeHitInformation(){
				hit = false;
			}
		};

		struct DamageInfo {
			Int16 owner;
			Int16 target;
			Int32 amount;
			UInt8 element;
			bool knockback;
		};        
    }

#endif