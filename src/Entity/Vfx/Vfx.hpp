#ifndef GAME_OVERWORLD_EFFECTS_HPP
    #define GAME_OVERWORLD_EFFECTS_HPP

    #include "../../Engine/Texture.hpp"

    namespace Game {
        
        struct OvwVfx {
            nite::Vec2 position;
            UInt64 initTime;
            UInt64 lifetime;
            nite::Texture pattern;
            bool destroy;
            OvwVfx(){
                destroy = false;
                lifetime = 650;
            }
            virtual void init(){}
            virtual void step(){}
            virtual void draw(){}
        };


        struct OvwVfxFrag {
            nite::Vec2 position;
            float angle;
            float alpha;
            nite::Color color;
            nite::Vec2 size;
            float mod;
            float ray;
            float reduct;
            OvwVfxFrag(){
                alpha = 100.0f;
            }
            void draw(nite::Texture &pattern, const nite::Vec2 &rel) {
                nite::setColor(nite::Color(color.r, color.g, color.b, alpha / 100.0f));
                pattern.draw(rel.x + position.x, rel.y + position.y, size.x, size.y, 0.5f, 0.5f, angle);
            }
        };  

        struct VfxBang : OvwVfx {
            float spread;
            Vector<OvwVfxFrag> frags;
            void init();
            void step();
            void draw();
        };

        struct VfxDash : OvwVfx {
            float height;
            float density;
            float length;
            Vector<OvwVfxFrag> frags;
            void init();
            void step();
            void draw();
            VfxDash(){
                density = 0.2f;
            }
        };        

        struct VfxTeleport : OvwVfx {
            float spread;
            bool start;
            float refrad;
            float radius;
            float target;
            float vert;
            float alpha;
            float lastAng;
            nite::Color color;
            int n;
            Vector<OvwVfxFrag> frags;
            void init();
            void step();
            void draw();
            void addRay(float ang);
            VfxTeleport(){
                refrad = 64.9f;
                start = false;
            }
        };

        struct VfxDevice {
            Vector<Shared<OvwVfx>> effects;
            void add(const Shared<OvwVfx> &eff, Int64 lifetime = -1);
            void step();
            void draw();
            void clear();
        };
    }


#endif