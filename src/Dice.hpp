#ifndef DP_DICE_HPP
    #define DP_DICE_HPP

    #include "Engine/Texture.hpp"

    struct Dice {
        nite::Vec2 frameSize;
        nite::Texture texture;
        Vector<int> values;
        void init(int n, int from, int to);
        void set(const Vector<int> &values);
        void shuffle();
        void update();
        void draw(float x, float y, bool vertical); 
    };

#endif