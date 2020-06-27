#ifndef NITE_PARTICLES_HPP
	#define NITE_PARTICLES_HPP

    #include "Graphics.hpp"
    #include "Texture.hpp"
    #include "Shader.hpp"
    #include "Tools/Tools.hpp"

    namespace nite {


        namespace ParticlePatternType {
            enum ParticlePatternType : int {
                NONE = 0,
                SIMPLE_CIRCULAR,
                VARIANT_RADIUS,
                SIMPLE_SINE,
                RAISING,
            };
        }

        namespace ParticleTransType {
            enum ParticleTransType : int {
                NONE = 0,
                SHINY,
            };
        }        

        struct ParticleObject {
            nite::Vec2 pos;
            nite::Vec2 size;
            nite::Color color;
            int index;
            float alpha;
            float angle;
            int tick;
            bool dead;

            UInt64 initTime;
            UInt64 lifetime;
            ParticleObject(){
                tick = 0;
                index = 0;
                alpha = 0.0f;
                angle = 0.0f;
                dead = false;
                initTime = nite::getTicks();
            }
        };

        struct ParticleTemplate {
            nite::Texture texture;
            nite::Shader shader;
            nite::Color color;
            nite::Vec2 size;
            float alpha;
            float angle;
            bool dynamicY;
            UInt64 lifetime;
            bool varLifetime;
            int startTick;
            Vector<int> transforms;
            Vector<int> patterns;
            Dict<String, float> props;
            UInt64 tickrate; // every x msecs
            float getProp(const String &label, float fallback){
                auto it = props.find(label);
                return it == props.end() ? fallback : it->second;
            }
            ParticleTemplate(){
                this->lifetime = 0;
                this->varLifetime = true;
                this->dynamicY = true;
                this->transforms = {
                    nite::ParticleTransType::NONE
                };
                this->patterns = {
                    nite::ParticlePatternType::NONE
                };        
                tickrate = 4;   
            }            
        };

        struct ParticleEmitter {
            nite::ParticleTemplate templ;
            float timescale;
            UInt64 initTime;
            nite::Vec2 region;
            UInt64 lifetime;
            UInt64 lastTick;
            int units;
            Vector<nite::ParticleObject> objects;
            void addObject();
            void fixIndexes();
            void setup(const nite::ParticleTemplate &templ, int units, UInt64 lifetime, const nite::Vec2 &region);
            void render(float x, float y);
        };

        struct ParticleDevice {
            Dict<int, nite::ParticleEmitter> emitters;
            int emit(nite::ParticleEmitter &emitter);
            void render(float x, float y);
        };

    }

#endif