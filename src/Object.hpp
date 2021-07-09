#ifndef GAME_OBJECT_HPP
    #define GAME_OBJECT_HPP

    #include "Engine/Tools/Tools.hpp"

    namespace Game {

        struct Object {
            UInt16 id;
            nite::Vec2 position;
            nite::Vec2 vel;
            nite::Vec2 accel;
            nite::Vec2 size;
            nite::Vec2 lerpPos;
            float mass;
            float invMass;
            float friction;
            float orientation;
            
            UInt8 moveState;

            nite::Polygon mask; // collision mask

            Object();

            virtual void create(){

            }

            virtual void step(){

            }

            virtual void render(){

            }

            virtual void onDestroy(){

            }

            void setMass(float mass);
            void reshape(const nite::Polygon &mask, const nite::Vec2 &size);
            void push(float force, float angle);
            void move(float x = 0.0f, float y = 0.0f);
            void setPosition(const nite::Vec2 &p);
        };


        namespace SAT {

            struct Projection {
                float min;
                float max;
                Projection(){
                    min = 0; max = 0;
                }
                Projection(float min, float max){
                    this->min = min;
                    this->max = max;
                }

                bool overlap(const Projection &other){
                    return max > other.min || min > other.max;                    
                }

                float getOverlap(const Projection &other){
                    if(!overlap(other)) {
                        return 0.0f;
                    }
                    if(max > other.min){
                        return nite::abs(max-other.min);
                    }else
                    if(min > other.max){
                        return nite::abs(min-other.max);
                    }else {
                        return 0.0f;
                    }                    
                }
            };   

            struct MTV {
                nite::Vec2 smallest;
                float overlap;
                MTV(){

                }
                MTV(const nite::Vec2 &smallest, float overlap){
                    set(smallest, overlap);
                }
                void set(const nite::Vec2 &smallest, float overlap){
                    this->smallest = smallest; 
                    this->overlap = overlap;
                }
            };    

            Projection project(Object *obj, const nite::Vec2 &axis);
            Vector<nite::Vec2> getAxes(Object *obj);

        }        

    }


#endif