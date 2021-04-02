#ifndef DP_STATE_HPP
    #define DP_STATE_HPP

    #include "GameType.hpp"
    #include "Board.hpp"
    #include "Dice.hpp"
    #include "Engine/UI/UI.hpp"

    struct NetHandle;
    struct ClientProxy;


    struct StateBar {
        Shared<nite::BaseUIComponent> main;
        String text;
        String newText;
        bool isNewText;
        nite::Vec2 position;
        nite::Vec2 nextPosition;
        nite::Vec2 targetPosition;
        bool idle;
        bool goDown;
        UInt64 animTimeout;
        void setText(const String &text);
        void init();
        void update();
    };

    struct GameState {
        StateBar stateBar;
        unsigned sessionId;
        unsigned totalTurns;
        unsigned expPlayers;
        bool clientSide;
        bool allowThrowDice;
        NetHandle *net;
        UInt8 currentTurn;
        UInt8 currentPlaying;
        Shared<GameType> gType;
        Shared<Board> board;
        Shared<Dice> dice;
        UInt8 pAllowedShuffle;
        UInt8 lastDiceTotalValue;
        Vector<UInt8> lastDiceValues;
        Vector<UInt8> turnsOrder;
        bool isMoving;
        void advancePlayerTo(UInt8 id, UInt8 space, const Vector<UInt8> &route);
        void jumpPlayerTo(UInt8 id, UInt8 space);
        void setupPlayer(Shared<ClientProxy> &client);
        void update();
        void game();
        void requestThrowDice();
        void init(const unsigned sessionId, const unsigned expPlayers, const Shared<GameType> &gType, bool clientSide);

        //In-game
        unsigned useNumber;
        Vector<unsigned> pickList;
        Dict<UInt8, unsigned> valueList;

    };

#endif