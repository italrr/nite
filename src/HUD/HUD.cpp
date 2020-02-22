#include "HUD.hpp"
#include "../Engine/Console.hpp"
#include "../Engine/Graphics.hpp"
#include "../Engine/UI/TextUI.hpp"
#include "../Entity/Base.hpp"
#include "../Network/Client.hpp"

static String HUDMainPath = "data/ui/hud/main.json";
static nite::Console::CreateProxy cpAnDatTo("hud_main_src", nite::Console::ProxyType::Literal, HUDMainPath.size(), (char*)HUDMainPath.c_str());

Game::HUD::HUD(){
    this->followId = 0;
    this->main = Shared<nite::BaseUIComponent>(NULL);
}

void Game::HUD::start(Game::Client *client){
    this->client = client;
    this->main = nite::UI::build(HUDMainPath);
    this->followId = 0;
    if(auto win = dynamic_cast<nite::WindowUI*>(this->main.get())){
        win->setPosition(nite::Vec2(0.0f));
        win->setSize(nite::Vec2(nite::getWidth(), nite::getHeight()));
        win->setVisible(false);         
        win->setOnRerenderListener([&](const Shared<nite::ListenerInfo> &info, nite::BaseUIComponent &component){
           this->resetValues();
        });
    }
}

void Game::HUD::resetValues(){
    auto nickText = this->main->getComponentById("hud_nickname_text");
    if(nickText.get() != NULL){
        if(auto txt = dynamic_cast<nite::TextUI*>(nickText.get())){
            txt->setText(this->client->nickname);
        }
    } 
}

void Game::HUD::setFollow(UInt16 followId){
    this->followId = followId;
    if(this->main.get() != NULL){
        if(auto win = dynamic_cast<nite::WindowUI*>(this->main.get())){ // no need to cast it. just being formal
            win->setVisible(true);        
        }        
    }
    update();
}

void Game::HUD::update(){
    if(this->followId == 0){ // an entity id can never be 0
        return;
    }
    auto it = client->world.objects.find(followId) ;
    if(it == client->world.objects.end()){
        return;
    }
    Game::EntityBase *ent = static_cast<Game::EntityBase*>(it->second.get()); 
    // HP
    auto hpText = this->main->getComponentById("hud_hp_text");
    if(hpText.get() != NULL){
        if(auto txt = dynamic_cast<nite::TextUI*>(hpText.get())){
            txt->setText("HP "+nite::toStr(ent->healthStat.health)+" / "+nite::toStr(ent->healthStat.maxHealth));
        }
    }

    // SP
    auto spText = this->main->getComponentById("hud_sp_text");
    if(spText.get() != NULL){
        if(auto txt = dynamic_cast<nite::TextUI*>(spText.get())){
            txt->setText("SP "+nite::toStr(ent->healthStat.mana)+" / "+nite::toStr(ent->healthStat.maxMana));
        }
    }    

    // STA
    auto staText = this->main->getComponentById("hud_sta_text");
    if(staText.get() != NULL){
        if(auto txt = dynamic_cast<nite::TextUI*>(staText.get())){
            txt->setText("STA "+nite::toStr(ent->healthStat.stamina)+" / "+nite::toStr(ent->healthStat.maxStamina));
        }
    }       

}

void Game::HUD::stop(){
    if(auto win = dynamic_cast<nite::WindowUI*>(this->main.get())){
        win->close();
    }
}