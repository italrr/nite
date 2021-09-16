#ifndef GAME_UI_LIST_MENU
    #define GAME_UI_LIST_MENU

    #include "Theming.hpp"

    namespace Game {

        struct UIListMenuOption {
            int option;
            String label;
            nite::Color color;
            std::function<void(UIListMenuOption *opt)> callback;

            UIListMenuOption(){
                callback = [](UIListMenuOption *opt){

                };
            }
        };

        struct UIListMenu : Game::UIObject {
            Vector<Shared<UIListMenuOption>> options;
            nite::Vec2 margin;
            bool visible;
            int selected;
            nite::Texture selArrowWhite;
            nite::Vec2 selArrowPos;
            nite::Vec2 selArrowSize;
            void setSize(const nite::Vec2 &size);
            void setVisible(bool v);
            void recalculate();
            Shared<UIListMenuOption> addOption(const String &label,  const std::function<void(UIListMenuOption *opt)> &callback);
            void clear();
            void onCreate();
            void onDestroy();
            void step();
            void render();

        };

    }


#endif