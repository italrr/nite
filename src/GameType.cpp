#include "GameType.hpp"

 Shared<GameType> loadGameType(const String &filename){
    auto type = std::make_shared<GameType>(GameType());
    if(!nite::fileExists(filename)){
        nite::print("failed to load GameType '"+filename+"': it doesn't exist");
        return Shared<GameType>(NULL);
    }
    Jzon::Parser parser;
    auto node = parser.parseFile(filename);
    if(!node.isValid()){
        nite::print("failed to load GameType '"+filename+"': "+parser.getError());
        return Shared<GameType>(NULL);
    }
    type->name = node.get("name").toString();
    type->totalSpaces = node.get("spaces").toInt();
    type->horSpaces = node.get("horSpaces").toInt();
    type->verSpaces = node.get("verSpaces").toInt();
    type->width = node.get("width").toInt();
    type->height = node.get("height").toInt();
    type->cornerSpacesize.set(node.get("cornerSpaceSize").get("width").toInt(), node.get("cornerSpaceSize").get("height").toInt());
    type->singleSpaceSize.set(node.get("singleSpaceSize").get("width").toInt(), node.get("singleSpaceSize").get("height").toInt());
    // rules
    auto rules = node.get("rules");
    type->rules.moneyScale = rules.get("moneyScale").toFloat();
    type->rules.moneyStart = rules.get("moneyPlayerStart").toInt();
    type->rules.moneyBankStart = rules.get("moneyBankStart").toInt();
    type->rules.startSpace = rules.get("start").toInt();
    type->rules.endSpace = rules.get("end").toInt();
    // pieces
    auto pieces = node.get("pieces");
    for(auto &it : pieces){
        auto piece = PieceType();
        piece.name = it.second.get("name").toString();
        piece.filename = it.second.get("file").toString();
        type->pieces[it.first] = piece;
    }
    // dice
    type->diceType.from = node.get("dice").get("from").toInt();
    type->diceType.to = node.get("dice").get("to").toInt();
    type->diceType.filename = node.get("dice").get("file").toString();
    // layout
    auto layout = node.get("layout");
    for(auto &it : layout){
        auto space = SpaceLayout();
        space.x = it.second.get("x").toInt();
        space.y = it.second.get("y").toInt();
        space.width = it.second.get("width").toInt();
        space.height = it.second.get("height").toInt();
        space.orientation = it.second.get("orientation").toFloat();
        type->layout[nite::toInt(it.first)] = space;
    }
    // spaces
    auto spaces = node.get("spaces");
    for(auto &it : spaces){
        auto space = Space();
        space.id = it.second.get("id").toInt();
        space.next = it.second.get("next").toInt();
        space.color = nite::Color(it.second.get("color").toString());
        space.name = it.second.get("name").toString();
        space.type = SPACE_TYPE::type(it.second.get("type").toString());
        space.price = it.second.get("price").toInt();
        type->spaces[nite::toInt(it.first)] = space;
    }  
    return type;
}