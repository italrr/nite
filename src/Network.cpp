#include "Network.hpp"

Shared<ClientProxy> NetHandle::getClientBySock(Int32 id){
    for(auto &it : clients){    
        if(it.second->socket == id){
            return it.second;
        }
    }
    return Shared<ClientProxy>(NULL);
}