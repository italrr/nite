#ifndef DP_DICE_HPP
    #define DP_DICE_HPP

    #include "Engine/Texture.hpp"
    #include "GameType.hpp"

    struct GameState;
    struct Dice {
        GameState *game;
        nite::Vec2 frameSize;
        nite::Texture texture;
        unsigned n;
        unsigned from;
        unsigned to;
        Vector<int> values;
        bool shuffling;
        Shared<GameType> gType;
        Dice(GameState *state);
        UInt64 lastShuffle;
        float rotation;
        float preRotation;
        unsigned getTotal();
        void init(const Shared<GameType> &gType);
        void set(const Vector<int> &values);
        void setShuffling(bool v);
        void update();
        void draw(float x, float y, bool vertical);
    };

#endif