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
    this->lastCheck = nite::getTicks();
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

    if(nite::getTicks()-lastCheck > 125){
        updateValues();
        lastCheck = nite::getTicks();
    }
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

    auto updateActionable = [&](Shared<nite::BaseUIComponent> cmp, int indx, String letter){
        if(cmp.get() == NULL || ent->actionables[indx].type == Game::ActionableType::None){
            return;
        }
        auto iconcmp = cmp->getComponentByType("icon");
        auto textcmp = cmp->getComponentByType("text");
        if(auto icon = dynamic_cast<nite::IconUI*>(iconcmp.get())){
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
        if(auto text = dynamic_cast<nite::TextUI*>(textcmp.get())){
            text->setText(letter);
        }

    };

 
    auto actZPanel = this->main->getComponentById("actionable_z");
    updateActionable(actZPanel, 0, "Z");
    auto actXPanel = this->main->getComponentById("actionable_x");
    updateActionable(actXPanel, 1, "X");
    auto actCPanel = this->main->getComponentById("actionable_c");
    updateActionable(actCPanel, 2, "C");
    auto actAPanel = this->main->getComponentById("actionable_a");
    updateActionable(actAPanel, 3, "A");
    auto actSPanel = this->main->getComponentById("actionable_s");
    updateActionable(actSPanel, 4, "S");

    auto &effs = ent->effectStat.effects;
    auto statusPanel = this->main->getComponentById("hud_eff_column");

    // remove
    for(auto &ef : lastEffectList){
        auto it = effs.find(ef.first);
        if(it == effs.end()){
            statusPanel->remove(ef.second->id);
            lastEffectList.erase(ef.first);
        }
    }


    auto buildEffPanel = [&](Game::Effect *ef){
            Jzon::Node color = Jzon::object();
            Jzon::Node icon = Jzon::object();
            Jzon::Node json = Jzon::object();
            Jzon::Node children = Jzon::array();
            Jzon::Node iconSize = Jzon::object();
            Jzon::Node iconTextPosition = Jzon::object();
            // color
            color.add("r", ef->color.r);
            color.add("g", ef->color.g);
            color.add("b", ef->color.b);
            color.add("a", 1.0f);
            // iconTextPosition
            iconTextPosition.add("hor", 0.5f);
            iconTextPosition.add("vert", 0.5f);            
            // icon size
            iconSize.add("hor", 32);
            iconSize.add("vert", 32);            
            // icon
            icon.add("type", "icon");
            icon.add("source", "data/ui/icons/test_icons.png");
            icon.add("width", 64);
            icon.add("height", 64);
            icon.add("iconSize", iconSize);
            icon.add("textPosition", iconTextPosition);
            icon.add("fontColor", color);
            children.add(icon);
            // panel
            json.add("width", 64);
            json.add("height", 64);
            json.add("type", "panel");
            json.add("layout", "hbox");
            json.add("backgroundColor", color);
            json.add("children", children);
            return nite::UI::build(json);
    };

    // add
    for(auto &ef : effs){
        auto it = lastEffectList.find(ef.second->insId);
        if(it == lastEffectList.end()){
            auto efpnl = buildEffPanel(ef.second.get());
            statusPanel->add(efpnl);
            if(auto icon = dynamic_cast<nite::IconUI*>(efpnl->getComponentByType("icon").get())){
                icon->setIndex(ef.second->iconId);
                icon->setFontSize(12);
                icon->setTextPosition(nite::Vec2(0.15f));
            }            
            lastEffectList[ef.second->insId] = efpnl;
        }
    }

    // update
    for(auto &ef : effs){
        auto it = lastEffectList.find(ef.second->insId);
        if(it != lastEffectList.end() && it->second->children.size() > 0){
            auto icon = static_cast<nite::IconUI*>(it->second->getComponentByType("icon").get());
            icon->setText(ef.second->getStatus(ent));
        }
    }    

    
    

}

void Game::HUD::stop(){
    if(auto win = dynamic_cast<nite::WindowUI*>(this->main.get())){
        win->close();
    }
}