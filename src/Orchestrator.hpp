#ifndef GAME_ORCHESTRATOR_HPP
    #define GAME_ORCHESTRATOR_HPP

    #include "Network/Client.hpp"
    #include "Network/Server.hpp"

    namespace Game {

        struct Orchestrator {
            Game::Client player;
            Game::Server server;
            // Game::Camera camera;

            Orchestrator();
            ~Orchestrator();

            void setupSimpleGame(bool coop, int maxPlayers, int maps);

            void update();
            void render();

            void start();
            void end();
            void restart();
            
            void onPlayerKilled();
            void onGameOver();
            void onGameStart();


        };


    }

#endif