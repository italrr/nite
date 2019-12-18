#ifndef GAME_RING_HPP
    #define GAME_RING_HPP
    /*
        [R]USH [I]N DU[N]GEONS [G]ENERATOR

        RING is the game generator and coordinator used to
        create the campaigns of this game.

        We'll be using different types of Maze generators at the
        beginning, but our ultimate goal is to combine the technologies.

        Our first experimentation will be using Prim.

    */
    #define RING_DEFAULT_UNIT_SIZE nite::Vec2(16.0f, 16.0f)
    namespace Game {
        
        namespace RINGEnvType {
            unsigned Dungeon = 0;
        }

        struct RINGBase {
            nite::Vec2 envSize; // unit size
            float scale;
            float difficulty;
            float complexity;
            unsigned type;
            
            void init(); // default
        };

        struct RING {
            bool build(int width, int height, Game::RINGBase env);
        }; 

   }

#endif