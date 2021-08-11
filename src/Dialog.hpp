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
            DialogLine(const String &emt, const String &text, const nite::Color &color = nite::White){
                this->emitter = emt;
                this->message = text;
                this->color = color;
            }            
        };

        struct DialogHook {
            int currentDiag;
            int currentChar;
            UInt64 lastChar;
            
            String targetText;
            String currenText;
            bool done;
            bool ready;
            bool proceed;

            Vector<Shared<DialogLine>> lines;
            
            bool canCont();
            bool isReady();
            void cont();
            DialogHook();
            void add(const String &emt, const String &text, const nite::Color &color = nite::White);
            void start();
            void reset();
            void step();

            // virtual void onUpdate() {}
            // virtual void onReset(){}
            // virtual void onUpdateText(){}
            // virtual void onNextLine(const Shared<DialogLine> &line){}

            std::function<void()> onCont;
            std::function<void()> onReset;
            std::function<void()> onUpdateText;
            std::function<void(const Shared<DialogLine> &line)> onNextLine;

        };

        struct DialogBox : DialogHook {
            nite::Font font;
            nite::Font subFont;
            nite::Texture empty;
            nite::Vec2 dialogPos;
            Shared<nite::BaseUIComponent> emtWin;
            Shared<nite::BaseUIComponent> textWin;
            bool useStBColor;
            nite::Color borderColor;
            nite::Color bgColor;            

            bool isShowing();
            void start(const nite::Vec2 &pos, int width, int nlines, bool useTitle = true);
            void setBgColor(const nite::Color &color);
            void setStaticBorderColor(bool v, const nite::Color &color = nite::Color(0.0f, 0.0f, 0.0f, 1.0f));
            DialogBox();
            void updWinValue(Shared<nite::BaseUIComponent> &win, const String &newval);
            void updTextColor(Shared<nite::BaseUIComponent> &win, const nite::Color &color);
            void updWinBorderColor(Shared<nite::BaseUIComponent> &win, const nite::Color &color);
            void render();
        };



    }


#endif