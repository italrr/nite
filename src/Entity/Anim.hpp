#ifndef GAME_ANIM_CORE_HPP
    #define GAME_ANIM_CORE_HPP

    #include "../Engine/Types.hpp"
    #include "../Engine/Animation.hpp"
    #include "../Engine/Indexer.hpp"

    namespace Game {

        namespace AnimType {
            enum AnimType : UInt8 {
                UNDEFINED,
                // BOT
                BOT_WALKING = 1,
                BOT_STANDING,
                BOT_JUMPING,
                // MID
                MID_MELEE_NOWEAP,
                MID_PARRY_NOWEAP,
                MID_STANDING,
                MID_SHOOTING_HANDGUN,
                MID_SHOOTING_BOW,
                MID_HOLDING_BOW,
                MID_HOLDING_HANDGUN,
                MID_HOLDING_SWORD,
                MID_HOLDING_KNIFE, // works for kunai too
                MID_WAVING_KNIFE,
                MID_CASTING,
                MID_WAVING_SWORD,
                // TOP
                TOP_NEUTRAL
            };
            static const UInt8 total = 15;
            static UInt8 getType(const String &name){
                if(name == "bot_walking"){
                    return AnimType::BOT_WALKING;
                }else
                if(name == "bot_standing"){
                    return AnimType::BOT_STANDING;
                }else
                if(name == "mid_melee_noweap"){
                    return AnimType::MID_MELEE_NOWEAP;
                }else
                if(name == "mid_standing"){
                    return AnimType::MID_STANDING;
                }else
                if(name == "mid_parry_noweap"){
                    return AnimType::MID_PARRY_NOWEAP;
                }else
                if(name == "top_neutral"){
                    return AnimType::TOP_NEUTRAL;
                }else{
                    return AnimType::UNDEFINED;
                }                    
            }
        }        

        struct AnimFrame {
            UInt8 id; // nite::Animation id
            UInt8 n;
            UInt16 x;
            UInt16 y;
            UInt8 type;
            UInt64 spd;
            UInt8 keyframe;
        };

        namespace AnimPart {
            enum AnimPart : UInt8 {
                BOTTOM = 0,
                MIDDLE,
                TOP
            };
            static const UInt8 total = 3;
        }

        struct Anim {
            UInt8 parts[AnimPart::total];
            nite::Vec2 frameSize;
            nite::Animation anim;
            nite::Batch batch;
            Dict<UInt8, Game::AnimFrame> frames;
            Game::AnimFrame *getAnim(UInt8 anim);
            bool load(const String &path);
            nite::IndexedFile source;
            nite::Color transparency;
            Anim();
            void setState(UInt8 anims[AnimPart::total], UInt8 sframes[AnimPart::total]);
        };

    }

#endif