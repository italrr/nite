#include "HUD.hpp"
#include "../Engine/Console.hpp"
#include "../Engine/Graphics.hpp"
#include "../Engine/UI/TextUI.hpp"
#include "../Entity/Base.hpp"

static String HUDMainPath = "data/ui/hud/main.json";
static nite::Console::CreateProxy cpAnDatTo("hud_main_src", nite::Console::ProxyType::Literal, HUDMainPath.size(), (char*)HUDMainPath.c_str());

void Game::HUD::start(Game::NetWorld *container){
    this->container = container;
    this->main = nite::UI::build(HUDMainPath);
    this->followId = 0;
    if(auto win = dynamic_cast<nite::WindowUI*>(this->main.get())){
        win->setPosition(nite::Vec2(0.0f));
        win->setSize(nite::Vec2(nite::getWidth(), nite::getHeight()));
        
    }   
}

void Game::HUD::setFollow(UInt16 followId){
    this->followId = followId;
    update();
}

void Game::HUD::update(){
    if(this->followId == 0){ // an entity id can never be 0
        return;
    }
    auto it = container->objects.find(followId) ;
    Game::EntityBase *ent = NULL;
    if(it == container->objects.end()){
        return;
    }
    ent = static_cast<Game::EntityBase*>(it->second.get());
    // HP
    auto hpText = this->main->getComponentById("hud_hp_text");
    if(hpText.get() != NULL){
        if(auto txt = dynamic_cast<nite::TextUI*>(hpText.get())){
            txt->setText("HP 0 / 0");
        }
    }

    // SP
    // auto spText = this->main->getComponentById("hud_sp_text");
    // if(spText.get() != NULL){
    //     if(auto txt = dynamic_cast<nite::TextUI*>(spText.get())){
    //         txt->setText("SP 0 / 0");
    //     }
    // }    

    // STA
    // auto staText = this->main->getComponentById("hud_sta_text");
    // if(staText.get() != NULL){
    //     if(auto txt = dynamic_cast<nite::TextUI*>(staText.get())){
    //         txt->setText("dasd 0 / 0");
    //     }
    // }       

}

void Game::HUD::stop(){
    if(auto win = dynamic_cast<nite::WindowUI*>(this->main.get())){
        win->close();
    }
}