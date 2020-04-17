#include "IngameMenu.hpp"
#include "../Engine/Input.hpp"

static String IGMMainPath = "data/ui/ingame_menu/main.json";

Game::InGameMenu::InGameMenu(){
    open = false;
    client = NULL;
    main = Shared<nite::BaseUIComponent>(NULL);
}

void Game::InGameMenu::start(Game::Client *client){
    this->client = client;
    this->main = nite::UI::build(IGMMainPath);
    if(auto win = dynamic_cast<nite::WindowUI*>(this->main.get())){
        win->setVisible(open);
    }
}

void Game::InGameMenu::update(){
    if(nite::keyboardPressed(nite::keyENTER)){
        open = !open;
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