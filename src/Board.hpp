#ifndef DP_BOARD_HPP
    #define DP_BOARD_HPP
    
    #include "Engine/Tools/Tools.hpp"
    #include "Engine/Graphics.hpp"
    #include "Engine/Font.hpp"
    #include "Engine/Shader.hpp"
    #include "Engine/Texture.hpp"
    
    #include "GameType.hpp"

    struct UnitSize {
        float width;
        float height;
    };

    struct GameState;
    struct ClientProxy;
    struct Piece {
        Shared<ClientProxy> owner;
        nite::Texture texture;
        nite::Color color;
        nite::Vec2 size;
        nite::Vec2 targetSize;
        nite::Vec2 originalSize;
        nite::Vec2 position;
        UInt64 lastPieceMove;
        float yShadowOffset;
        nite::Vec2 nextPosition;
        void draw();
        Piece(){
            this->lastPieceMove = nite::getTicks();
            nite::setColor(1.0f, 0.0f, 0.0f, 1.0f);
        }
    };

    struct Board {
        GameState *game;
        Dict<UInt8, nite::Vec2> spaces;
        Dict<UInt8, Vector<UInt8>> occupied;
        Dict<UInt8, Shared<Piece>> pieces;
        nite::Batch batch;
        nite::Font font;
        nite::Texture empty;
        nite::Shader shader;
        void setPieceAt(UInt8 id, UInt8);
        void swapPiece(UInt8 id, UInt8 to);
        int width;
        int height;
        Shared<GameType> gType;
        UnitSize unitSize;
        nite::Color bg;
        Board(GameState *state);
        void init(const nite::Color &bg, int width, int height, const Shared<GameType> &gType, bool headless = false);
        void rerender();
        void update();
        void draw(float x, float y);
    };

#endif
