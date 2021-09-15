#ifndef GAME_UI_THEMING
    #define GAME_UI_THEMING

    #include "../Engine/Font.hpp"
    #include "../Engine/Texture.hpp"
    #include "../Engine/Tools/Tools.hpp"

    namespace Game {

        struct UITheming {
            nite::Color fontColor;
            nite::Color fontSubColor;

            nite::Color backgroundColor;
            nite::Color borderColor;

            nite::Color solidColor;
            nite::Color subColor;

            nite::Font bigFont;
            int bigFontSize;
            nite::Font regularFont;
            int regularFontSize;
            nite::Font smallFont;
            int smallFontSize;

            nite::Texture base;

            void load(const String &path){
                if(!nite::fileExists(path)){
                    return;
                }
                Jzon::Parser parser;
                auto root = parser.parseFile(path);
                fontColor.set(nite::Color(root.get("fontColor").toString()));
                fontSubColor.set(nite::Color(root.get("fontSubColor").toString()));
                backgroundColor.set(nite::Color(root.get("backgroundColor").toString()));
                borderColor.set(nite::Color(root.get("borderColor").toString()));
                solidColor.set(nite::Color(root.get("solidColor").toString()));
                subColor.set(nite::Color(root.get("subColor").toString()));
                bigFontSize = root.get("bigFontSize").toInt();
                bigFont.load(root.get("bigFont").toString(), bigFontSize, 2.0f);
                regularFontSize = root.get("regularFontSize").toInt();
                regularFont.load(root.get("regularFont").toString(), regularFontSize, 2.0f);
                smallFontSize = root.get("smallFontSize").toInt();
                smallFont.load(root.get("smallFont").toString(), smallFontSize, 2.0f);
                base.load("data/texture/empty.png");
            }
        };


        struct UIObject {
            Shared<UITheming> theme;
            nite::Vec2 position;
            nite::Vec2 size;
            bool visible;
            UIObject(){
                theme = std::make_shared<UITheming>(UITheming());
                visible = false;
            }
            virtual void onCreate(){}
            virtual void onDestroy(){}
            virtual void step(){}
            virtual void render(){}
        };

    }

#endif