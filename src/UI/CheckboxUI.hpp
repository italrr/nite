#ifndef NITE_CHECKBOX_UI
  #define NITE_CHECKBOX_UI

  #include "BaseUIComponent.hpp"
  #include "../Engine/Tools/Tools.hpp"
  #include "../Engine/Shader.hpp"
  #include "../Engine/Texture.hpp"
  #include "../Engine/Font.hpp"

  namespace nite {

    struct ListenerInfoCheckbox : public nite::ListenerInfo {
      bool value;
      ListenerInfoCheckbox(bool v){
        this->value = v;
      }
      ListenerInfoCheckbox(){
        this->value = false;
      }
    };

    class CheckboxUI : public nite::BaseUIComponent {
      private:
        String text;
        int fontSize;
        nite::Font font;
        void onClick(); 
        void onHover(); 
        void onUnhover();
        nite::Color textColor;
        nite::Color baseColor;
        nite::Color selectorColor;
        bool value;
        void defaultInit();
        nite::ListenerLambda onChangeMethod;   
        nite::ListenerLambda onHoverMethod;
        nite::ListenerLambda onUnhoverMethod;                                              
        void calculateSize();        
        nite::Vec2 realSize;
      public:
        nite::Color getBoxColor();
        nite::Color getFontColor();
        nite::Color getSelectorColor();
        void setBoxColor(const nite::Color &color);
        void setFontColor(const nite::Color &color);
        void setSelectorColor(const nite::Color &color);
        void onCreate();      
        void resetColor();        
        CheckboxUI();
        CheckboxUI(const String &text, nite::ListenerLambda onChangeMethod);
        void setOnChange(nite::ListenerLambda onChangeMethod);
        void setOnHover(nite::ListenerLambda onHoverMethod);    
        void setOnUnhover(nite::ListenerLambda onUnhover);            
        void setFont(const nite::Font &font);
        void setText(const String &text);
        void setFontSize(int size);
        void setSize(const nite::Vec2 &size);
        nite::Vec2 computeSize();
        void render();
        void update();
        int getFontSize();
        String getFontFilename();        
    };
  }

#endif