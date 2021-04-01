#ifndef DP_DICE_HPP
    #define DP_DICE_HPP

    #include "Engine/Texture.hpp"
    #include "GameType.hpp"

    struct Dice {
        nite::Vec2 frameSize;
        nite::Texture texture;
        unsigned n;
        unsigned from;
        unsigned to;
        Vector<int> values;
        bool shuffling;
        Shared<GameType> gType;
        Dice();
        UInt64 lastShuffle;
        float rotation;
        float preRotation;
        void init(const Shared<GameType> &gType);
        void set(const Vector<int> &values);
        void shuffle();
        void update();
        void draw(float x, float y, bool vertical); 
    };

#endif