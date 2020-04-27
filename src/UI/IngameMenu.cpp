#include "IngameMenu.hpp"
#include "../Engine/Input.hpp"

static String IGMMainPath = "data/ui/ingame_menu/main.json";
static String IGMSubsPath[Game::MenuType::total] = {
    "data/ui/ingame_menu/status_window.json.json",
    "data/ui/ingame_menu/equip_window.json",
    "data/ui/ingame_menu/inventory_window.json"
};

Game::InGameMenu::InGameMenu(){
    open = false;
    client = NULL;
    main = Shared<nite::BaseUIComponent>(NULL);
    for(int i = 0; i < MenuType::total; ++i){
        subs[i] = Shared<nite::BaseUIComponent>(NULL);
    }
}

void Game::InGameMenu::hideAllSubs(){
    for(int i = 0; i < 3; ++i){
        subs[i]->setVisible(false);
    }    
}

void Game::InGameMenu::start(Game::Client *client){

    Dict<String, Lambda<void(const Shared<nite::ListenerInfo> &info, nite::BaseUIComponent *component)>> listeners;

    listeners["onclick_status"] = [&](const Shared<nite::ListenerInfo> &info, nite::BaseUIComponent *component){
        subs[MenuType::Status]->setVisible(true);
        main->setVisible(false);
    };

    listeners["onclick_equip"] = [&](const Shared<nite::ListenerInfo> &info, nite::BaseUIComponent *component){
        subs[MenuType::Equip]->setVisible(true);
        main->setVisible(false);
    };   

    listeners["onclick_inventory"] = [&](const Shared<nite::ListenerInfo> &info, nite::BaseUIComponent *component){
        subs[MenuType::Inventory]->setVisible(true);
        main->setVisible(false);
    }; 

    listeners["onclick_goback"] = [&](const Shared<nite::ListenerInfo> &info, nite::BaseUIComponent *component){
        component->setVisible(false);
        main->setVisible(true);
    };     

    listeners["onclick_close"] = [&](const Shared<nite::ListenerInfo> &info, nite::BaseUIComponent *component){
        main->setVisible(false);
        open = false;
    };  
        
    this->client = client;
    this->main = nite::UI::build(IGMMainPath, listeners);
    if(auto win = dynamic_cast<nite::WindowUI*>(this->main.get())){
        win->setVisible(open);
    }
    // start subs
    for(int i = 0; i < 3; ++i){
        auto win = nite::UI::build(IGMSubsPath[i], listeners);
        subs[i] = win;
        if(auto ref = dynamic_cast<nite::WindowUI*>(win.get())){
            ref->setVisible(false);
        }        
    }
}

void Game::InGameMenu::update(){
    if(nite::keyboardPressed(nite::keyENTER)){
        open = !open;
        if(!open){
            hideAllSubs();
        }
        if(auto win = dynamic_cast<nite::WindowUI*>(this->main.get())){
            win->setVisible(open);
        }        
    }
}

void Game::InGameMenu::stop(){
    // if(auto win = dynamic_cast<nite::WindowUI*>(this->main.get())){
    //     win->close();
    // }
}