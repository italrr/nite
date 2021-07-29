#ifndef GAME_DIALOG_HPP
    #define GAME_DIALOG_HPP

    #include "Engine/UI/UI.hpp"


    namespace Game {



        struct DialogLine {
            String emitter;
            String message;
            nite::Color color;
            int order;
            DialogLine(){
                emitter = "[UNDEFINED]";
                message = "Undefined Dialog Text Line";
                color.set(0.0f, 0.0f, 0.0f, 1.0f);
                order = 0;
            }
            DialogLine(const String &emt, const String &text, const nite::Color &color = nite::Black){
                this->emitter = emt;
                this->message = text;
                this->color = color;
            }            
        };

        struct DialogInstance {
            nite::Font font;
            nite::Font subFont;
            Shared<nite::BaseUIComponent> emtWin;
            Shared<nite::BaseUIComponent> textWin;

            int currentDiag;
            int currentChar;
            UInt64 lastChar;
            
            String targetText;
            String currenText;

            bool done;
            Vector<Shared<DialogLine>> lines;
            DialogInstance();
            void updWinValue(Shared<nite::BaseUIComponent> &win, const String &newval);
            void add(const String &emt, const String &text, const nite::Color &color = nite::Black);
            void start(const nite::Vec2 &pos, int width, int nlines);
            void reset();
            void step();
            void render();
        };



    }


#endif