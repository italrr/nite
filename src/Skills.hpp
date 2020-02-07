#ifndef GAME_SKILLS_HPP
    #define GAME_SKILLS_HPP

    #include "Engine/Tools/Tools.hpp"
    #include "Entity.hpp"

    namespace Game {

        namespace MasteryType {
            struct Type {
                String name;
                String shortName;
                String desc;
                UInt8 id;
                Type(){

                }
                Type(UInt8 id, const String &name, const String &shortName, const String &desc){
                    this->id = id;
                    this->shortName = shortName;
                    this->name = name;
                    this->desc = desc;
                }
            };
            static const MasteryType::Type Knight        = Type(1, "Art of the Blade", "Knight", "Tree of skills for sword masters and endurants");
            static const MasteryType::Type Cleric        = Type(2, "Path for the Divine", "Cleric", "Tree of skills for holy enchanters and curaters");
            static const MasteryType::Type Ranger        = Type(3, "Walk of the Wild", "Ranger", "Tree of skills for the huntsman and nagivators of nature");
            static const MasteryType::Type Bard          = Type(4, "Speciality of the Charm", "Bard", "Tree of skills for the witty and smooth talkers");
            static const MasteryType::Type Necromancer   = Type(5, "Path for the Unholy", "Necromancer", "Tree of skills for the summoners of death");
            static const MasteryType::Type Sorcerer      = Type(6, "Master of the Elements", "Sorcerer", "Tree of skills for the channelers of chaos");
            static const Dict<UInt8, Game::MasteryType::Type> Types = {
                {Knight.id, Knight}, {Cleric.id, Cleric}, {Ranger.id, Ranger},
                {Bard.id, Bard}, {Necromancer.id, Necromancer}, {Sorcerer.id, Sorcerer}
            };
        }

        enum SkillType : UInt8 {
            ACTIVE = 0,
            PASSIVE,
            AOE // area of effect
        };

        struct Skill {
            String name;
            String desc;
            UInt16 id;
            Vector<UInt16> reqs; // skills required to unlock this one
            UInt8 lv;
            UInt8 type;
            UInt8 maxLv;
            UInt16 manaCost;
            UInt16 staminaCost;
            UInt8 family; // mastery type id
            UInt16 range; // 1 unit is "32 radial pixels"
            UInt64 delay; // cast delay (milliseconds)
            UInt64 lastUse;
            UInt64 adelay; // after cast delay
            UInt16 minUseLv;
            UInt16 skPointsPerLv; // how many skill points cost one level
            Skill();
            virtual void beforeUse(Shared<Game::Entity> who);
            virtual void afterUse(Shared<Game::Entity> who);
            void use(Shared<Game::Entity> who);
            void use(Shared<Game::Entity> who, Shared<Game::Entity> to); // to is allowed to be null
            virtual void use(Shared<Game::Entity> who, Shared<Game::Entity> to, const nite::Vec2 &at);
        };

        enum SkillList : UInt16 {
            NONE = 0,
            
            // SPECIALS [SP] 
            
            // BASE [BA] 
            BA_ATTACK,          // basic attack (arrow shoot, sword swing, bare hands punch)
            BA_BASH,            // stronger 'attack', only close range
            BA_DASH,
            BA_PARRY,
            BA_FIRST_AID,       // theorically heals 25% of a lv 1 entity

            // KNIGHT [KN] 
            KN_MASTERY_KNIGHT,
            KN_SWORD_MASTERY,
            KN_FURY_BASH,
            
            // CLERIC [CL] 
            CL_MASTERY_CLERIC,
            CL_HEAL,
            
            // RANGER [RE] 
            RE_MASTERY_RANGER,
            
            // BARD [BD] 
            BD_MASTERY_BARD,
            
            // NECROMANCER [NE] 
            NE_MASTERY_NECROMANCER,
            
            // SORCERER [SO]
            SO_MASTERY_SORCERER
        };

        Game::Skill getSkill(UInt16 id, UInt8 lv);

        /* SKILLS */
        namespace Skills {
            // SPECIALS [SP] 
            
            // BASE [BA] 

            // KNIGHT [KN] 

            // CLERIC [CL] 

            // RANGER [RE] 

            // BARD [BD] 

            // NECROMANCER [NE] 

            // SORCERER [SO]
 
        }

    }

#endif