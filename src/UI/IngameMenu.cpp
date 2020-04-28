#include "IngameMenu.hpp"
#include "../Engine/Input.hpp"
#include "../Engine/Console.hpp"
#include "../Core/Client.hpp"
#include "../Entity/Base.hpp"

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
        invUpdateCached();
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
    if(nite::keyboardPressed(nite::keyENTER) && !nite::Console::isOpen()){
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

void Game::InGameMenu::invUpdateCached(){
    auto it = client->world.objects.find(client->entityId);
    if(it == client->world.objects.end()){
        hideAllSubs();
        main->setVisible(false);
        nite::print("failed to build inventory menu due to entity assigned to this client does not exist");
        return;
    }
    Game::EntityBase *ent = static_cast<Game::EntityBase*>(it->second.get()); 
    auto lst = subs[MenuType::Inventory]->getComponentById("inv_list");
    // add if it doesn't exist
    for(auto &item : ent->invStat.carry){
        if(lst.get() == NULL){
            nite::print("dang it");
            break;
        }        
        auto it = invCached.find(item.first);
        if(it != invCached.end()){
            continue;
        }
        // add it
        InGameMenuInvCached chd;
        chd.item = item.second;
        Jzon::Node json = Jzon::object();
        json.add("import", "data/ui/ingame_menu/objs/inventory_object.json");        
        chd.cmp = lst->add(json);
        if(chd.cmp.get() == NULL || chd.cmp->type != "button"){
            continue;
        }
        auto but = static_cast<nite::ButtonUI*>(chd.cmp.get());
        but->setText("x"+nite::toStr(chd.item->qty)+" "+chd.item->name);
        invCached[item.first] = chd;
        lst->fixNavIndexes(); // nav indexes get mess up when you add new comps on the fly        
    }


    // for(auto &it : ent->invStat.carry){
    //     nite::print(it.second->name+" "+nite::toStr(it.second->qty));
    // }

    // remove it if it's no longer in the inv
    for(auto &cached : invCached){
        auto it = ent->invStat.carry.find(cached.first);
        // remove it
        if(it == ent->invStat.carry.end()){
            lst->remove(cached.second.cmp->id);
            invCached.erase(cached.first);
            lst->fixNavIndexes(); // nav indexes get mess up when you add new comps on the fly
        }
    }

    // update
    for(auto &cached : invCached){
        auto it = ent->invStat.carry.find(cached.first);
        auto but = static_cast<nite::ButtonUI*>(cached.second.cmp.get());
        but->setText("x"+nite::toStr(cached.second.item->qty)+" "+cached.second.item->name);        
    }

}