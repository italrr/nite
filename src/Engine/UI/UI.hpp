#ifndef NITE_UI_HPP
    #define NITE_UI_HPP

    #include "BaseUIComponent.hpp"
    #include "WindowUI.hpp"
    #include "TextUI.hpp"
    #include "ButtonUI.hpp"
    #include "CheckboxUI.hpp"  
    #include "PanelUI.hpp"
    #include "IconUI.hpp"
    #include <memory>

    namespace nite {
        namespace UI {
            
            #define NITE_VALUE_CHANGE_LISTENER_LASTVAL_SIZE 32

            namespace ValueChangeListenerType {
                enum ValueChangeListenerType : UInt8 {
                    None = 0,
                    Literal, // std::string
                    UInteger,
                    Intenger,
                    Float
                };
            };

            struct ValueChangeListener {
                void *target;
                char lastVal[NITE_VALUE_CHANGE_LISTENER_LASTVAL_SIZE];
                UInt8 type;
                bool changed;
                Lambda<void(void *nv)> onChange;
                void listen(Int32 *v);
                void listen(UInt32 *v);
                void listen(String *v);
                void listen(float *v);
                void clear();
                void update();
                ValueChangeListener();
            };

            void add(std::shared_ptr<nite::BaseUIComponent> comp);
            void update();
            void render();
            void remove(nite::BaseUIComponent *comp);

            // builds a UI -anything- from a JSON file      
            std::shared_ptr<nite::BaseUIComponent> build(const String &path, const Dict<String, nite::ListenerLambda> &listeners);    
            std::shared_ptr<nite::BaseUIComponent> build(const String &path); 
        }; 
    } 

#endif