#ifndef GAME_UI_BATTLE_PLAYER_INFO
    #define GAME_UI_BATTLE_PLAYER_INFO

    #include "Theming.hpp"

    namespace Game {

        struct Entity;
        struct UIBattlePlayerInfo : Game::UIObject {
            UIBattlePlayerInfo();
            bool small;
            Shared<Entity> entity;
            float secondBar;
            bool affected;
            UInt64 affectedTick;
            void setAffected();
            void onCreate();
            void onDestroy();
            void step();
            void render();

        };

    }


#endif