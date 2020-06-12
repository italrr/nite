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
        win->setVisible(true);         
        win->setOnRerenderListener([&](const Shared<nite::ListenerInfo> &info, nite::BaseUIComponent *component){
           this->resetValues();
        });
    }
    actShader.load("./data/shaders/ui_cooldown_effect_f.glsl", "./data/shaders/ui_cooldown_effect_v.glsl");
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

    if(nite::getTicks()-lastCheck > 32){
        updateValues();
        lastCheck = nite::getTicks();
    }
}

void Game::HUD::updateValues(){
    if(this->followId == 0){ // an entity id can never be 0
        return;
    }
    auto ent = client->getEntity(followId);
    if(ent == NULL){
        this->main->setVisible(false);
        return;
    }
    
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

    auto updateActionable = [&](Game::ActionableHUDObject &actObj, int indx, const String &letter){
        auto &cmp = actObj.cmp;
        auto textcmp = cmp->getComponentByType("text");        
        if(auto text = dynamic_cast<nite::TextUI*>(textcmp.get())){
            text->setText(letter);
        }        
        if(ent->actionables[indx].type == Game::ActionableType::None){
            return;
        }
        auto iconcmp = cmp->getComponentByType("icon");
        if(auto icon = dynamic_cast<nite::IconUI*>(iconcmp.get())){
            auto &act = ent->actionables[indx];
            switch(act.type){
                case Game::ActionableType::Skill: {
                    int skId = ent->actionables[indx].id;
                    auto sk = ent->skillStat.get(skId);
                    if(sk != NULL && ent->net){
                        icon->setIndex(sk->iconId);
                        float &p = actObj.recharge;
                        float &a = actObj.alpha;
                        float k = 0.0f;
                        if(sk->getCooldown(ent) > 0.0f){
                            double a = (double)(ent->net->clock.time - sk->lastUse);
                            double b = (double)sk->getCooldown(ent);
                            k = a >= b ? 100.0f : (a / b) * 100.0f;
                            // nite::lerpDiscrete(p, k, 0.30f);                   
                        }
                        if(nite::lerpDiscrete(a, k > 99.0f ? 100.0f : 92.0f, 0.45f) && p == k){
                            break;
                        }
                        p = k;                         
                        nite::Uniform uni;
                        uni.add("p_total", p / 100.0f);
                        uni.add("p_alpha", a / 100.0f);
                        cmp->apply(actShader, uni);  
                        cmp->recalculate();                    
                    }
                } break ;
                case Game::ActionableType::Item: {
                    // TODO
                } break ;          
            }
        }

    };
    
    bool populate = actionables.size() == 0;
    for(int i = 0; i < Game::EntityActionables; ++i){
        auto actPanel = this->main->getComponentById("actionable_k"+nite::toStr(i+1));
        if(actPanel.get() == NULL){
            continue;
        }
        if(populate){
            Game::ActionableHUDObject obj;
            obj.cmp = actPanel;
            actionables.push_back(obj);
        }        
        updateActionable(actionables[i], i, nite::toStr(i + 1));
    }

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

    // add
    for(auto &ef : effs){
        auto it = lastEffectList.find(ef.second->insId);
        if(it == lastEffectList.end()){

            Jzon::Node json = Jzon::object();
            json.add("import", "data/ui/hud/effectcol_object.json");

            auto efpnl = statusPanel->add(json);


            auto panel = static_cast<nite::PanelUI*>(efpnl.get());
            panel->setBackgroundColor(ef.second->color);


            auto iconcmp = efpnl->getComponentByType("icon");
            auto textcmp = efpnl->getComponentByType("text");                    
            if(auto icon = dynamic_cast<nite::IconUI*>(iconcmp.get())){
                icon->setIndex(ef.second->iconId);
            }
            if(auto text = dynamic_cast<nite::TextUI*>(textcmp.get())){
                text->setText(ef.second->getStatus(ent));
            }             
            lastEffectList[ef.second->insId] = efpnl;
        }
    }

    // update
    for(auto &ef : effs){
        auto it = lastEffectList.find(ef.second->insId);
        if(it != lastEffectList.end() && it->second->children.size() > 0){
            auto iconcmp = it->second->getComponentByType("icon");
            auto textcmp = it->second->getComponentByType("text");
            if(auto icon = dynamic_cast<nite::IconUI*>(iconcmp.get())){
                icon->setIndex(ef.second->iconId);
            }
            if(auto text = dynamic_cast<nite::TextUI*>(textcmp.get())){
                text->setText(ef.second->getStatus(ent));
            }
        }
    }    

    
    

}

void Game::HUD::stop(){
    if(auto win = dynamic_cast<nite::WindowUI*>(this->main.get())){
        win->close();
    }
}