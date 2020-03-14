#include "HUD.hpp"
#include "../Engine/Console.hpp"
#include "../Engine/Graphics.hpp"
#include "../Engine/UI/TextUI.hpp"
#include "../Entity/Base.hpp"
#include "../Core/Client.hpp"

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
    updateValues();
}

void Game::HUD::updateValues(){
    if(this->followId == 0){ // an entity id can never be 0
        return;
    }
    auto it = client->world.objects.find(followId) ;
    if(it == client->world.objects.end()){
        this->main->setVisible(false);
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

    auto updateActionable = [&](Shared<nite::BaseUIComponent> cmp, int indx){
        if(cmp.get() != NULL && ent->actionables[0].type != Game::ActionableType::None){
            if(auto icon = dynamic_cast<nite::IconUI*>(cmp->children[0].get())){
                auto act = ent->actionables[indx];
                switch(act.type){
                    case Game::ActionableType::Skill: {
                        auto sk = getSkill(ent->actionables[indx].id, 0);
                        icon->setIndex(sk->iconId);
                    } break ;
                    case Game::ActionableType::Item: {
                        // TODO
                    } break ;          
                }
            }
        }  
    };

 
    auto actZPanel = this->main->getComponentById("actionable_z");
    updateActionable(actZPanel, 0);
    auto actXPanel = this->main->getComponentById("actionable_x");
    updateActionable(actXPanel, 1);
    auto actCPanel = this->main->getComponentById("actionable_c");
    updateActionable(actCPanel, 2);
    auto actAPanel = this->main->getComponentById("actionable_a");
    updateActionable(actAPanel, 3);
    auto actSPanel = this->main->getComponentById("actionable_s");
    updateActionable(actSPanel, 4);

    // stats
    auto statusPanel = this->main->getComponentById("hud_eff_column");
    statusPanel->clear();
    auto &effs = ent->effectStat.effects;
    //we're gonna create the same number of effects as panels
    for(int i = 0; i < effs.size(); ++i){
        auto efpnl = Shared<nite::PanelUI>(new nite::PanelUI());
        // TODO: use effect color. and then icon
        // statusPanel
        statusPanel->add(efpnl);
    }
    for(int i = 0; i < effs.size(); ++i){
        auto &ef = effs[i];
        
    }
}

void Game::HUD::stop(){
    if(auto win = dynamic_cast<nite::WindowUI*>(this->main.get())){
        win->close();
    }
}