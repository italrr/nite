#ifndef NITE_BUTTON_UI_HPP
  #define NITE_BUTTON_UI_HPP

  #include "BaseUIComponent.hpp"
  #include "../Font.hpp"
  #include "../Texture.hpp"

  namespace nite {
    class ButtonUI : public nite::BaseUIComponent {
      private:
        String text;
        int fontSize;
        nite::Font font;
        void onClick(); 
        void onHover(); 
        void onUnhover();
        int buttonState; 
        UInt64 stateTimeout;   
        nite::Color textColor;
        nite::Color secondColor;   
        nite::Color baseColor;
        void defaultInit();
        void calculateSize();
        float pressOffset;
		float currentPressOffset;
		nite::Vec2 borderThickness;
        nite::Vec2 realSize;
        nite::ListenerLambda onClickMethod; 
        nite::ListenerLambda onHoverMethod; 
        nite::ListenerLambda onUnhoverMethod;                                      
        nite::Texture buttonImage;
      public:
        void setBackgroundImage(const nite::Texture &image);
        void setBaseColor(const nite::Color &baseColor);
        void setSecondColor(const nite::Color &baseColor);
        void setFontColor(const nite::Color &baseColor);
        nite::Color getBaseColor();
        nite::Color getSecondColor();
        nite::Color getFontColor();
        ButtonUI();
        ButtonUI(const String &text, nite::ListenerLambda onClick);
        void onCreate();
        void resetColor();
        void setOnClick(nite::ListenerLambda onClick);
        void setOnHover(nite::ListenerLambda onHover);
        void setOnUnhover(nite::ListenerLambda onUnhover);        
        void setFont(const nite::Font &font);
        void setText(const String &text);
        void setFontSize(int size);
		void setSize(const nite::Vec2 &size);
		nite::Vec2 getBorderThickness();
		void setBorderThickness(const nite::Vec2 &bt);
        nite::Vec2 computeSize();
        void render(const nite::Vec2 &offset);
        void update();
        int getFontSize();
        String getFontFilename();
    };
  }

#endif