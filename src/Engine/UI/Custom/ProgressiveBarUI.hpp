#ifndef NITE_PROGRESSIVE_BAR_UI_HPP
  #define NITE_PROGRESSIVE_BAR_UI_HPP

  #include "../BaseUIComponent.hpp"
  #include "../../Tools/Tools.hpp"
  #include "../../Shader.hpp"
  #include "../../Texture.hpp"
  #include "../../Font.hpp"  

  namespace nite {  

    struct ProgressiveBarUI : public nite::BaseUIComponent {
        float borderThickness;
        nite::Color borderColor;
        nite::Shader uiShader;
        nite::Texture uiBasicTexture;
        nite::Texture uiBackgroundImage;

        nite::Color barColorFull;
        nite::Color barColorReduce;

        bool useReduce;
        bool showMax;
        bool showValue;

        float maxValue;
        float targetValue;
        float currentValue;
        UInt64 hitTimeout;

        void setValue(float tvalue);
        void setValue(float tvalue, float maxValue);

        void setBarColor(const nite::Color &color);
        void setReduceColor(const nite::Color &color);

        void setUseReduce(bool v);
        void setUseMax(bool v);
        void setUseValue(bool v);

        nite::Font font;
        int fontSize;

        nite::Batch batch;
        void setFont(const nite::Font &font);
        void setFontSize(int size);
        void rerender();
        void recalculate();
        void setBorderThickness(float tn);
        void setBorderColor(const nite::Color &color);
        void defaultInit();
        bool toRerender;
        void onHover();
        void onUnhover();        
        void onClick();
        nite::ListenerLambda onClickMethod;
        nite::ListenerLambda onHoverMethod;
        nite::ListenerLambda onUnhoverMethod;                                              
        void recalculateFromHead(); 
        void setBackgroundImage(const nite::Texture &bgi);
        nite::Texture getBackgroundImage();      
        void setOnUnhover(nite::ListenerLambda onUnhover);                  
        void setOnClick(nite::ListenerLambda onClick);
        void setOnHover(nite::ListenerLambda onHover);
        ProgressiveBarUI(const nite::Vec2 &size);
        ProgressiveBarUI();
        void accommodate(float w, float h);
        void setSize(const nite::Vec2 &size);
        nite::Vec2 computeSize();
        void onCreate();
        void update();
        void render(const nite::Vec2 &offset);
        void setBackgroundColor(const nite::Color &color);
        nite::Color getBackgroundColor();
    };

  }

#endif