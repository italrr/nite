#ifndef NITE_SIMPLE_WINDOW_UI_HPP
  #define NITE_SIMPLE_WINDOW_UI_HPP

    #include "BaseUIComponent.hpp"
    #include "../Tools/Tools.hpp"
    #include "../Shader.hpp"
    #include "../Texture.hpp"
    #include "../Font.hpp"
    #include <memory>

    namespace nite {

        namespace WindowResizeOrientation {
            enum WindowResizeOrientation : unsigned {
                Left = 0,
                Up,
                Right,
                Down,
            };
        }

    class WindowUI : public BaseUIComponent {
        private:
        nite::Shader uiShader;
        nite::Texture uiBackgroundImage;
        nite::Texture uiBasicTexture;
        nite::Font basicFont;
        nite::Batch batch;
        nite::Batch decoration;
        nite::Vec2 grabbedPoint;
        nite::Color leftBorderColor;
        nite::Texture borderCorner;
        nite::Color borderColor;
        nite::Color titleColor;
        String title;
        bool modal;
        bool toRerender;
        bool grabbed;
        bool resizeGrab;
        nite::Vec2 resizeGrabPoint;
        nite::Vec2 beforeGrabSize;
        int resizeOrientation;
        float headerHeight;
        nite::ListenerLambda onRerender;
        void rerender();
        void recalculate();
        void defaultInit();
        bool enableTitle;
        float borderThickness;
        nite::Vec2 origPosition;          
        bool toDestroy;
        float generalAlpha;
        void rerenderDecoration();
        public:
        void setOnRerenderListener(nite::ListenerLambda listener);
        void setBackgroundImage(const nite::Texture &bgi);
        nite::Texture getBackgroundImage();
        void setCornerPattern(const nite::Texture &tex);
        void updateListeners();
        void setTitleColor(const nite::Color &color);
        void setBorderColor(const nite::Color &color);
        void setBackgroundColor(const nite::Color &color);
        nite::Color getBackgroundColor();
        nite::Color getBorderColor();
        nite::Color getTitleColor();
        void setPosition(const nite::Vec2 &pos);
        String getTitle();
        void setModal(bool m);
        nite::Vec2 computeSize();
        void close();
        bool unmovable;
        bool resizeable;
        void setShowTitle(bool v);
        void setBorderThickness(float tn);
        void onCreate();
        void update();
        void render(const nite::Vec2 &offset);
        WindowUI(const String &title);                  
        WindowUI();                  
        void setTitle(const String &tile);
        void setSize(const nite::Vec2 &size);
    };	

    }

#endif