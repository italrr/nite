#ifndef GAME_ENTITY_TRAP_HPP
    #define GAME_ENTITY_TRAP_HPP

    #include "../../../Engine/Map.hpp"
    #include "../../Core/World.hpp"

    namespace Game {

        namespace TrapType {
            enum TrapType : UInt8 {
                NONE = 1,
                NEEDLES,
            };
                
        }
        struct Trap {
            int dynTile;
            int state;
            Game::GhostMask *mask;
            nite::Vec2 position;
            nite::Vec2 size;
            UInt16 id;
            UInt8 type;
            std::function<void(Game::NetObject *who)> callback;
            virtual void update(const Shared<nite::Map> &map, Game::NetWorld &world) {}
            virtual void setState(int state, const Shared<nite::Map> &map, Game::NetWorld &world) {
                if(this->dynTile > 0){
                    map->setDynamicTileState(dynTile, state);
                }                
            }
            Trap(){
                mask = NULL;
                type = TrapType::NONE;
                callback = [](Game::NetObject *who){
                    nite::print("quaker");
                    return;
                };
            }
            virtual void damage(Game::NetObject *who) {}
        };

        struct TrapNeedles : Trap {
            UInt64 initTime;
            UInt64 timeout;
            void update(const Shared<nite::Map> &map, Game::NetWorld &world);
            void setState(int state, const Shared<nite::Map> &map, Game::NetWorld &world);
            void damage(Game::NetObject *who);
            TrapNeedles();
        };

        struct Net;
        struct TrapDevice {
            UInt16 lastId;
            Dict<UInt16, Shared<Trap>> traps;
            Game::Net *host;
            void start(Game::Net *host);            
            UInt16 add(const Shared<Trap> &trap, int useId = -1);
            void remove(UInt16 id);
            Vector<UInt16> update();
            void clear();
            Trap *getTrap(UInt16 id);
        };

        Shared<Game::Trap> getTrap(int type);
        Shared<Game::Trap> getTrap(const String &type);
    }

#endif