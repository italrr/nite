#ifndef GAME_INPUT_HPP
    #define GAME_INPUT_HPP

    #include "../Engine/Tools/Tools.hpp"
    #include "../Engine/Input.hpp"
    #include "../Engine/Packets.hpp"

    namespace Game {

        static const UInt8 InputMaxFrameBuffer = 7; // record 8 frames(ticks) worth of inputs (starts from 0)
    
        namespace Key {
            enum Key : UInt8 {
                UP = 0,
                DOWN,
                RIGHT,
                LEFT,
                Z,
                X,
                C,
                A,
                S,
                SPACE
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
            { nite::keyZ, Key::Z },
            { nite::keyX, Key::X },
            { nite::keyC, Key::C },
            { nite::keyA, Key::A },
            { nite::keyS, Key::S },
            { nite::keySPACE, Key::SPACE }
        };

        struct Input {
            Dict<unsigned, UInt8> mapping; // nite key -> game key
            Vector<InputFrameBuffer> buffer;
            int lastFrame;
            UInt64 lastStroke;
            bool capturing;
            Vector<InputFrameBuffer> getBuffer();
            Input();
            void update();
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
                    {Key::LEFT, PressType::Released },  {Key::Z, PressType::Released },     {Key::X, PressType::Released },
                    {Key::C, PressType::Released },     {Key::A, PressType::Released },     {Key::S, PressType::Released },
                    {Key::SPACE, PressType::Released }
                };
                nextKey = nite::getTicks();
            }
        };

    }

#endif