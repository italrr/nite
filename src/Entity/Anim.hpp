#ifndef GAME_ANIM_CORE_HPP
    #define GAME_ANIM_CORE_HPP

    #include "../Engine/Object.hpp"
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
                if(name == "mid_standing"){
                    return AnimType::MID_STANDING;
                }else            
                if(name == "mid_casting"){
                    return AnimType::MID_CASTING;
                }else                      
                if(name == "mid_parry_noweap"){
                    return AnimType::MID_PARRY_NOWEAP;
                }else
                if(name == "mid_weap_bow_idle"){
                    return AnimType::MID_HOLDING_BOW;
                }else                      
                if(name == "mid_weap_bow_shoot"){
                    return AnimType::MID_SHOOTING_BOW;
                }else
                if(name == "mid_weap_sword_idle"){
                    return AnimType::MID_HOLDING_SWORD;
                }else       
                if(name == "mid_weap_sword_waving"){
                    return AnimType::MID_WAVING_SWORD;
                }else                             
                if(name == "top_neutral"){
                    return AnimType::TOP_NEUTRAL;
                }else{
                    return AnimType::UNDEFINED;
                }                    
            }
        }        

        struct AnimLimbFrame {
            String type;
            String limb;
            nite::Vec2 p;
            float an;
            bool xflip;
            bool yflip;
            float shakeMag;
            String weapKeyFrame;
            String hold;
            AnimLimbFrame(){
                yflip = false;
                xflip = false;
                hold = "";
                shakeMag = 0.0f;
                weapKeyFrame = "";
                an = 90;
                p.set(0.0f);
            }
        };

        struct AnimLimbFrameOverride {
            UInt64 spd;
            UInt64 espd;
            UInt8 n;
            UInt8 st; 
            UInt64 lastThick;
            AnimLimbFrameOverride(){
                st = 0;
            }
        };

        struct AnimFrame {
            UInt8 id; // nite::Animation id
            UInt8 n;
            UInt16 x;
            UInt16 y;
            UInt8 type;
            UInt64 spd;
            UInt8 keyframe;
            bool useLimbOverride;
            Game::AnimLimbFrameOverride limbOverride;
            Dict<String, Vector<Game::AnimLimbFrame>> limbs; 
            AnimFrame(){
                useLimbOverride = false;
            }
        };

        struct AnimTypeLimb {
            String name;
            nite::Vec2 inTexCoors;
            nite::Vec2 inTexSize;
        };

        struct AnimLimb {
            String name;
            nite::Vec2 pos;
            nite::Vec2 npos;
            float angle;
            float nangle;
            String type;
            int depth;
            bool xflip;
            bool yflip;
            float shakeMag;
            bool active = false;
            String weapKeyFrame;
            String hold;
            Int64 spd;
            AnimLimb(){
                spd = -1;
                active = true;
                nangle = 0;
                angle = 0;
            }
        };

        namespace AnimPart {
            enum AnimPart : UInt8 {
                BOTTOM = 0,
                MIDDLE,
                TOP
            };
            static const UInt8 total = 3;
        }

        struct EntityBase;
        struct Anim {
            float lookingAtAngle;
            float lerpLookingAtAngle;
            Game::EntityBase *owner;
            UInt8 lastSFrame[AnimPart::total];
            Game::AnimFrame *lastAnim[AnimPart::total];
            Vector<nite::Hitbox> hitboxes;
            Dict<String, Game::AnimTypeLimb> limbTypes;
            Dict<String, Game::AnimLimb> limbs;
            nite::Vec2 limbSize;
            nite::Hitbox meleeNoWeapHb;
            nite::Vec2 maskSize;
            nite::Vec2 arrowShootPos;
            UInt8 parts[AnimPart::total];
            nite::Vec2 frameSize;
            nite::Animation anim;
            nite::Batch batch;
            nite::Batch looseLimbsBatch;
            nite::Texture shadow;
            float shadowYOffset;
            bool useLooseLimbs;
            float bodyDepthOffset;
            String path;
            void reload();
            Dict<UInt8, Game::AnimFrame> frames;
            Game::AnimFrame *getAnim(UInt8 anim);
            bool load(const String &path);
            nite::IndexedFile source;
            nite::Color transparency;
            Anim();
            void rerender();
            void update();
            void setState(UInt8 anims[AnimPart::total], UInt8 sframes[AnimPart::total], UInt64 exTime[AnimPart::total]);
        };

    }

#endif