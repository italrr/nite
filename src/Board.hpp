#ifndef DP_BOARD_HPP
    #define DP_BOARD_HPP
    
    #include "Engine/Tools/Tools.hpp"
    #include "Engine/Graphics.hpp"
    #include "Engine/Font.hpp"
    #include "Engine/Texture.hpp"
    
    #include "GameType.hpp"

    struct UnitSize {
        float width;
        float height;
    };

    struct Board {
        nite::Batch batch;
        nite::Font font;
        nite::Font subFont;
        int width;
        int height;
        Shared<GameType> gType;
        UnitSize unitSize;
        void init(const nite::Color &bg, int width, int height, const Shared<GameType> &gType);
        void draw(float x, float y);
    };

#endif
