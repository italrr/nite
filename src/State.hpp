#ifndef DP_STATE_HPP
    #define DP_STATE_HPP

    #include "GameType.hpp"
    #include "Board.hpp"
    #include "Dice.hpp"
    #include "Engine/UI/UI.hpp"

    struct NetHandle;
    struct ClientProxy;


    struct StatusBar {
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
        StatusBar statusBar;
        unsigned sessionId;
        unsigned totalTurns;
        unsigned expPlayers;
        bool clientSide;
        NetHandle *net;
        Shared<GameType> gType;
        Shared<Board> board;
        Shared<Dice> dice;
        void setupPlayer(Shared<ClientProxy> &client);
        void update();
        void game();
        void init(const unsigned sessionId, const unsigned expPlayers, const Shared<GameType> &gType, bool clientSide);

    };

#endif