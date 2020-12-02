#ifndef GAME_INPUT_HPP
    #define GAME_INPUT_HPP

    #include "../Engine/Tools/Tools.hpp"
    #include "../Engine/Input.hpp"
    #include "../Engine/Packets.hpp"

    namespace Game {
    
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
                k7,
                SPACE
            };
            static const UInt8 total = 12;
        }

        namespace PressType {
            enum PressType : UInt8 {
                None = 0,
                Pressed,
                Released
            };
        }

        static const Dict<unsigned, UInt8> InputDefaultMapping = { 
            { nite::keyW, Key::UP },
            { nite::keyS, Key::DOWN },
            { nite::keyA, Key::LEFT },
            { nite::keyD, Key::RIGHT },
            { nite::key1, Key::k1 },
            { nite::key2, Key::k2 },
            { nite::key3, Key::k3 },
            { nite::key4, Key::k4 },
            { nite::key5, Key::k5 },
            { nite::butLEFT, Key::k6 },
            { nite::butRIGHT, Key::k7 },
            { nite::keySPACE, Key::SPACE }
        };

        struct InputCompacter {
            nite::Vec2 mpos;
            UInt8 states[Key::total];
            bool changed;
            bool set(UInt8 key, UInt16 v);
            void setAll(UInt16 v);
            InputCompacter();
            bool isKeyPress(UInt8 key);
            UInt16 getCompat();
            void loadCompat(UInt16 v);
        };

        struct Input : InputCompacter {
            nite::Vec2 mpos;
            UInt64 lastChange;
            UInt64 lastCheck;
            Dict<unsigned, UInt8> mapping; // nite key -> game key
            Input();
            void update(bool update);
        };

    }

#endif