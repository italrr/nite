#ifndef GAME_DIALOG_HPP
    #define GAME_DIALOG_HPP

    // #include "Engine/UI/UI.hpp"
    #include "Engine/Tools/Tools.hpp"
    #include "UI/Theming.hpp"
    #include "UI/DialogBox.hpp"
    #include "UI/ListMenu.hpp"


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

        static inline Shared<DialogLine> buildLine(const String &emt, const String &text, const nite::Color &color = nite::White){
            return std::make_shared<DialogLine>(emt, text, color);
        }

        struct DialogHook {
            int currentDiag;
            int currentChar;
            UInt64 lastChar;
            bool delay;
            UInt64 delayTick;
            UInt64 delayTimeout;
            String targetText;
            String currenText;
            UInt64 lastReady;
            bool done;
            bool ready;
            bool proceed;

            Vector<Shared<DialogLine>> lines;
            

            bool autocont;
            UInt64 autoContTick;
            UInt64 autoContTimeout;
            void setAutoCont(UInt64 timeout);

            bool canCont();
            bool isReady();
            void cont();
            DialogHook();
            UInt64 getLastReady();
            bool canNext();
            void add(const String &emt, const String &text, const nite::Color &color = nite::White);
            void start();
            void reset();
            void step();

            // virtual void onUpdate() {}
            // virtual void onReset(){}
            // virtual void onUpdateText(){}
            // virtual void onNextLine(const Shared<DialogLine> &line){}

            void setImmediateText(const String &text);
            std::function<void()> onEndCallback;
            std::function<void()> onCont;
            std::function<void()> onReset;
            std::function<void()> onUpdateText;
            std::function<void(const Shared<DialogLine> &line)> onNextLine;

        };

        struct DialogBox : DialogHook {
            Shared<UIDialogBox> dialogBox;
            Shared<Game::UITheming> theme;        
            void update();
            bool isShowing();
            bool visible;
            void start(const nite::Vec2 &pos, int width, int nlines, bool useTitle = true);
            // void setBgColor(const nite::Color &color);
            // void setStaticBorderColor(bool v, const nite::Color &color = nite::Color(0.0f, 0.0f, 0.0f, 1.0f));
            DialogBox();
            void render();
        };

    }


#endif