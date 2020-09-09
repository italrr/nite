#ifndef GAME_ICON_MANAGER_HPP
    #define GAME_ICON_MANAGER_HPP

    #include "../Engine/Texture.hpp"

    namespace Game {

        struct Icon {
            nite::Texture source;
            int id; // index
            nite::Vec2 iconSize;
            nite::Vec2 inTexPosition;
            void draw(float x, float y, float w, float h, float origx, float origy, float angle);
        };

        struct IconManager {
            nite::Vec2 iconSize;
            nite::Texture source;
            int total;
            Dict<int, Game::Icon> icons;    
            void load(const nite::Texture &source, nite::Vec2 iconSize);
            Game::Icon getIcon(int id);
            Game::Icon getIcon(int x, int y);
        };

    }

#endif