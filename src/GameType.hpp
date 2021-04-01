#ifndef DP_GAMETYPE_HPP
    #define DP_GAMETYPE_HPP

    #include <map>
    #include "Engine/Tools/Tools.hpp"


    namespace SPACE_TYPE {
        enum SPACE_TYPE : unsigned {
            SPECIAL,
            PROPERTY,
            UTILITY,
            RAILROAD,
            UNDEFINED
        };
        static String name(unsigned type){
            switch(type){
                case SPECIAL:
                    return "SPECIAL";
                case PROPERTY:
                    return "PROPERTY";
                case UTILITY:
                    return "UTILITY";
                case RAILROAD:
                    return "RAILOAD";
                default:
                    return "UNDEFINED";
            }
        }
        static unsigned type(const String &name){
            if(name == "SPECIAL"){
                return SPECIAL;
            }else
            if(name == "PROPERTY"){
                return PROPERTY;
            }else           
            if(name == "UTILITY"){
                return UTILITY;
            }else           
            if(name == "RAILROAD"){
                return RAILROAD;
            }else{
                return UNDEFINED;
            }             
        }
    }

    struct GameRules {
        float moneyScale;
        unsigned moneyStart;
        unsigned moneyBankStart;
        unsigned startSpace;
        unsigned endSpace;
    };


    struct PieceType {
        String name;
        String hash;
        String filename;
    };

    struct DiceType {
        unsigned n;
        unsigned from;
        unsigned to;
        String hash;
        String filename;
    };

    struct SpaceLayout {
        unsigned id;
        unsigned x;
        unsigned y;
        unsigned width;
        unsigned height;
        float orientation;
    };

    struct Space {
        unsigned id;
        unsigned next;
        nite::Color color;
        String name;
        unsigned type;
        unsigned price;
    };

    struct GameType {
        String name;
        unsigned totalSpaces;
        unsigned horSpaces;
        unsigned verSpaces;
        unsigned width;
        unsigned height;
        nite::Vec2 cornerSpacesize;
        nite::Vec2 singleSpaceSize;
        GameRules rules;
        Dict<String, PieceType> pieces;
        DiceType diceType;
        std::map<unsigned, SpaceLayout> layout; 
        std::map<unsigned, Space> spaces;
    };

    Shared<GameType> loadGameType(const String &filename);

#endif