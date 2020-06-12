#ifndef GAME_INPUT_HPP
    #define GAME_INPUT_HPP

    #include "../Engine/Tools/Tools.hpp"
    #include "../Engine/Input.hpp"
    #include "../Engine/Packets.hpp"

    namespace Game {

        static const UInt8 InputMaxFrameBuffer = 1;
    
        namespace Key {
            enum Key : UInt8 {
                UP = 0,
                DOWN,
                RIGHT,
                LEFT,
                k1,
                k2,
                k3,
                k4,
                k5,
                k6,
                k7
            };
        }

        namespace PressType {
            enum PressType : UInt8 {
                None = 0,
                Pressed,
                Released
            };
        }

        struct InputFrameBuffer {
            UInt8 key;
            UInt8 type;
            UInt64 lastStroke; // it should never be more than 255 msecs, thats why using int8
            InputFrameBuffer(){

            }
            InputFrameBuffer(UInt8 key, UInt8 type, UInt8 lastStroke){
                this->key = key;
                this->type = type;
                this->lastStroke = lastStroke;
            }
        };

        static const Dict<unsigned, UInt8> InputDefaultMapping = { 
            { nite::keyUP, Key::UP },
            { nite::keyDOWN, Key::DOWN },
            { nite::keyLEFT, Key::LEFT },
            { nite::keyRIGHT, Key::RIGHT },
            { nite::key1, Key::k1 },
            { nite::key2, Key::k2 },
            { nite::key3, Key::k3 },
            { nite::key4, Key::k4 },
            { nite::key5, Key::k5 },
            { nite::key6, Key::k6 },
            { nite::key7, Key::k7 }
        };

        struct Input {
            Dict<unsigned, UInt8> mapping; // nite key -> game key
            Vector<InputFrameBuffer> buffer;
            int lastFrame;
            UInt64 lastStroke;
            bool capturing;
            Vector<InputFrameBuffer> getBuffer();
            Input();
            void update(bool update);
        };

        struct InputSimulator {
            Vector<InputFrameBuffer> queue;
            UInt64 nextKey;
            Dict<UInt8, UInt8> keys;
            bool isKeyPress(UInt8 key);
            void update();
            InputSimulator(){
                this->keys = {
                    {Key::UP, PressType::Released },    {Key::DOWN, PressType::Released },  {Key::RIGHT, PressType::Released },
                    {Key::LEFT, PressType::Released },  {Key::k1, PressType::Released },     {Key::k2, PressType::Released },
                    {Key::k3, PressType::Released },     {Key::k4, PressType::Released },     {Key::k5, PressType::Released },
                    {Key::k6, PressType::Released }, {Key::k7, PressType::Released }
                };
                nextKey = nite::getTicks();
            }
        };

    }

#endif