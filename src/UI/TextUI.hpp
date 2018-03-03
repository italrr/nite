#ifndef NITE_TEXT_UI
  #define NITE_TEXT_UI

  #include "BaseUIComponent.hpp"
  #include "../Engine/Font.hpp"
  namespace nite {
    class TextUI : public nite::BaseUIComponent {
      private:
        String originalText;
        String text;
        int fontSize;
        nite::Font font;
        nite::Vec2 actualSize;
        nite::Color shadowColor;
        nite::Vec2 shadowOffset;
        nite::Vec2 realSize;        
        void defaultInit();
        void calculateSize();
        nite::ListenerLambda onClickMethod;
        nite::ListenerLambda onHoverMethod;
        nite::ListenerLambda onUnhoverMethod;                                              
        void onUnhover();        
        void onClick();
        void onHover();
      public:
        void setOnClick(nite::ListenerLambda onClick);
        void setOnHover(nite::ListenerLambda onHover);
        void setOnUnhover(nite::ListenerLambda onUnhover);        
        TextUI();
        TextUI(const String &text);
        void onCreate();
        void setFont(const nite::Font &font);
        void setText(const String &text);
        void setFontSize(int size);
        void accommodate(float w, float h);
        void setShadowColor(const nite::Color &color);
        void setShadowOffset(const nite::Vec2 &offset);
        void setFontColor(const nite::Color &color);
        nite::Color getShadowColor();
        nite::Vec2 getShadowOffset();
        nite::Color getFontColor();
        void setSize(const nite::Vec2 &size);
        nite::Vec2 computeSize();
        void render();
        void update();
        int getFontSize();
        String getFontFilename();
    };
  }

#endif