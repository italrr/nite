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
            UIListMenu();
            nite::Vec2 margin;
            bool interact;
            bool visible;
            int selected;
            float diagArrowOffset;
            UInt64 diagArrowTick;
            bool diagArrowFlip;                
            nite::Texture selArrowWhite;
            nite::Vec2 selArrowPos;
            nite::Vec2 selArrowSize;
            void setPosition(const nite::Vec2 &position);
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

        struct UIYesNoMenu : UIListMenu {
          void setup(const std::function<void(UIListMenuOption *opt)> &yescb, const std::function<void(UIListMenuOption *opt)> &nocb);
        };

    }


#endif