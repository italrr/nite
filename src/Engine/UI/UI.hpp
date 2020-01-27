#ifndef NITE_UI_HPP
    #define NITE_UI_HPP

    #include "BaseUIComponent.hpp"
    #include "WindowUI.hpp"
    #include "TextUI.hpp"
    #include "ButtonUI.hpp"
    #include "CheckboxUI.hpp"  
    #include "PanelUI.hpp"
    #include <memory>

    namespace nite {

        namespace UI {
            void add(std::shared_ptr<nite::BaseUIComponent> comp);
            void update();
            void render();
            Vector<nite::BaseUIComponent*> removeQueue;
            void remove(nite::BaseUIComponent *comp);

            // builds a UI -anything- from a JSON file      
            std::shared_ptr<nite::BaseUIComponent> build(const String &path, const Dict<String, nite::ListenerLambda> &listeners);    
            std::shared_ptr<nite::BaseUIComponent> build(const String &path); 
        }; 
    } 

#endif