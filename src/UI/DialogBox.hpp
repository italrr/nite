#ifndef GAME_UI_DIALOG_BOX
    #define GAME_UI_DIALOG_BOX

    #include "Theming.hpp"

    namespace Game {

        struct UIDialogBox : Game::UIObject {
            nite::Vec2 margin;
            String targetText;
            String bufferText;
            UIDialogBox();
            bool visible;
            nite::Vec2 size;
            void setVisible(bool v);
            void clear();
            void setText(const String &text);
            void refresh();
            void setSize(const nite::Vec2 &size);
            void onCreate();
            void onDestroy();
            void step();
            void render();

        };

    }


#endif