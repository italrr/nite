#ifndef GAME_SKILL_HPP
    #define GAME_SKILL_HPP

    #include "../Engine/Tools/Tools.hpp"
    #include "../UI/Icon.hpp"

    namespace Game {

        struct EntityBase;

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
            static const MasteryType::Type Cleric        = Type(2, "Path for the Divine", "Cleric", "Tree of skills for holy enchanters and curators");
            static const MasteryType::Type Ranger        = Type(3, "Walk of the Wild", "Ranger", "Tree of skills for the huntsman and adventurers");
            static const MasteryType::Type Bard          = Type(4, "Speciality of the Charm", "Bard", "Tree of skills for the witty and smooth talkers");
            static const MasteryType::Type Necromancer   = Type(5, "Path for the Unholy", "Necromancer", "Tree of skills for the summoners of death");
            static const MasteryType::Type Sorcerer      = Type(6, "Master of the Elements", "Sorcerer", "Tree of skills for the channelers of chaos");
            static const Dict<UInt8, Game::MasteryType::Type> Types = {
                {Knight.id, Knight}, {Cleric.id, Cleric}, {Ranger.id, Ranger},
                {Bard.id, Bard}, {Necromancer.id, Necromancer}, {Sorcerer.id, Sorcerer}
            };
        }

        enum SkillType : UInt8 {
            Active = 0,
            Passive,
            AoE // area of effect
        };

        enum SkillUsageType : UInt8 {
            Self = 0,
            Target
        };

        struct EntityBase;
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
            UInt8 usageType;
            UInt8 family; // mastery type id
            UInt16 range; // 1 unit is "32 radial pixels"
            // UInt64 delay; // cast delay (milliseconds)
            UInt64 lastUse;
            UInt64 castDelay;
            UInt64 cooldown; // after cast delay
            UInt16 minUseLv;
            UInt16 skPointsPerLv; // how many skill points cost one level
            int iconId;
            UInt16 baseDmg;
            Skill(){
                id = 0;
                lastUse = 0;
            }
			virtual void onRecalculateStat(EntityBase *owner){}             
            virtual void passive(Game::EntityBase *who) {}
            virtual void beforeUse(EntityBase *who, Game::EntityBase *to, const nite::Vec2 &at) {}
            virtual void afterUse(EntityBase *who, Game::EntityBase *to, const nite::Vec2 &at) {}
            virtual bool use(EntityBase *who, Game::EntityBase *to, const nite::Vec2 &at) {}
            virtual bool isReady(EntityBase *who);
            virtual UInt64 getCooldown(EntityBase *who);
            virtual void writeUpdate(nite::Packet &packet);
            virtual void readUpdate(nite::Packet &packet);
            virtual void buildDesc(EntityBase *owner){
                this->desc = "None";
            }           
			void parse(Jzon::Node &obj);

			virtual void parseSpecial(Jzon::Node &obj){
				
			}             
        };

        enum SkillList : UInt16 {
            NONE = 0,
            
            // SPECIALS [SP] 
            
            // BASE [BA] 
            BA_ATTACK = 2,              // basic attack (arrow shoot, sword swing, bare hands punch)
            BA_BASH,                    // stronger 'attack', only close range
            BA_DODGE,                   // dash movement to `dodge` attacks
            BA_PARRY,                   // block % of next physical attack received
            BA_FIRST_AID,               // heals 20% of a lv 1 entity *with less than 50% hp* over 30 seconds

            // KNIGHT [KN] 
            KN_MASTERY_KNIGHT,          // 10% dmg inc with swords | 8% inc def, 12% inc mdef | 5 points for endu and atk
            KN_SWORD_MASTERY,           // 4% dmg inc with swords (per lv, max lv 5)
            KN_FURY_BASH,               // same as bash, but higher range (makes char jump into target), 10% chance of causing stun on target
            
            // CLERIC [CL] 
            CL_MASTERY_CLERIC,          // 10% heal amnt inc for "healing" skills | 10% inc for def & mdef | 60% damage resist from demons and undead
            CL_HEAL,                    // 20% heal to target over 30 seconds
            CL_ENCHANT_VISION,          // enchants vision for 8 secs to target's weap
            CL_ENCHANT_BLEEDING,        // enchants bleeding for 10 secs with 40% success to target's weap
            CL_SOLEMN_BLESS,            // inc target's agi & dex by 10% for 3 mins | 8% dmg resist and 15% resist against demons
            CL_EQUINOX_BLESS,           // inc target's def, mdef by 90%, make it non-knockable for 8 secs
            CL_CREATORS_WRATH,          // makes a magic attack of a single blow on target. the attack is always type light
            
            // RANGER [RE] 
            RE_MASTERY_RANGER,          // 10% dmg inc with bows | ability to craft complex items | 5 points + 12% agi inc
            RE_BOW_MASTERY,             // 4% dmg inc with bows (per lv, max lv 5)
            RE_DIRECT_BLOW,             // shoots 3 arros with 0.5 msecs delay. 1st normal dmg, 2nd double (*2), 3rd double of the triple (2 * 2) | 5% chance of causing stun on target
            RE_VISION_TORCH,            // spawns torch on the floor (only). everything around a 40px radius is visible. lasts 3 mins
            
            // BARD [BD] 
            BD_MASTERY_BARD,            // 8% dmg inc with bows | 75% charm rate inc | ability to spell by typing in chat | ability of dealing dmg by playing instruments
            BD_ENCORE,                  // 5% dmg inc with next "music attack" | if there's another player using encore, it's 10%
            BD_SING,                    // does not require an instrument | does a sing attack
            BD_WAR_OVERTUDE,            // requires guitar, flute or bagpipes. 10% dmg inc, 8% movement spd inc, complete immunity to magic for 4 secs for all party members in 40px radius
            BD_SONG_OF_MEDUSA,          // sings a song, entities within 90px radius proximity of the user have a  25% chance of being stunt (bosses and minibosses are immune)
            BD_WILLIAM_TELL,            // sings a song, party members within a 45px of the user are granted 50% movement increment and 5 point inc for agi for 12 secs
            BD_TAROT,                   // users pick a random card and applies its effect on target
            
            // NECROMANCER [NE] 
            NE_MASTERY_NECROMANCER,     // 80% dmg reduction from undead | 4% movement spd | 4 points inc agi | inability to be buffed from cleric's powers
            NE_SUMMON,                  // summons a dead entity with 15% of its original stats as slave. doesn't work on bosses (max 4 slaves). cannot resummon a dead summoned
            NE_AFTERLIFE,               // grants the user an extra life when it dies
            NE_CORPSE_ABSORD,           // consume a dead body (within 45px radius of user) and gain 5% hp and sp
            NE_CORPSE_EXPLODE,          // makes a dead body (within 45px radius of user) explode dealing damage to surrouding entities
            NE_DEVILS_CALL,             // spawns a mini demon as slave (max 3 slave)
            
            // SORCERER [SO]
            SO_MASTERY_SORCERER,        // 15% magic dmg inc |  10% magic def inc
            SO_FIREBALL,                // creates a fire ball instantly(no cast) | 15% chance of casuing burning on target
            SO_PORTAL,                  // ability to create 1 entrance and 1 exit but only areas visible
            SO_TELEPORT,                // ability to teleport itself to any visible to only visible areas
            SO_LIGHTNING                // creates a lighting attack from the 
        };

        Shared<Game::Skill> getSkill(UInt16 id, UInt8 lv);
        void DBLoadSkill(const String &path);

        struct SkillStat {
            EntityBase *owner;
            Dict<UInt16, Shared<Game::Skill>> skills;
            bool add(Shared<Game::Skill> &sk);
            Game::Skill *get(UInt16 id);
            bool remove(UInt16 id);
            bool contains(UInt16 id);
            bool lvUp(UInt16 id);
            // TODO: update for passives
            // TODO: add use
        };

        /* SKILLS */
        namespace Skills {
            // SPECIALS [SP] 

            // BASE [BA] 
            struct BA_Attack : Skill {
                BA_Attack(){ this->id = Game::SkillList::BA_ATTACK; }
                bool use(EntityBase *who, Game::EntityBase *to, const nite::Vec2 &at);
            };
            struct BA_Bash : Skill {
                BA_Bash(){ this->id = Game::SkillList::BA_BASH; }
                bool use(EntityBase *who, Game::EntityBase *to, const nite::Vec2 &at);
            };
            struct BA_Dodge : Skill {
                BA_Dodge(){ this->id = Game::SkillList::BA_DODGE; }
                bool use(EntityBase *who, Game::EntityBase *to, const nite::Vec2 &at);
            };             
            struct BA_Parry : Skill {
                BA_Parry(){ this->id = Game::SkillList::BA_PARRY; }
                bool use(EntityBase *who, Game::EntityBase *to, const nite::Vec2 &at);
            };         
            struct BA_FIRST_AID : Skill {
                BA_FIRST_AID(){ this->id = Game::SkillList::BA_FIRST_AID; }
                bool use(EntityBase *who, Game::EntityBase *to, const nite::Vec2 &at);
            };               


            // KNIGHT [KN] 

            // CLERIC [CL] 

            // RANGER [RE] 

            // BARD [BD] 

            // NECROMANCER [NE] 

            // SORCERER [SO]
 
        }

    }

#endif