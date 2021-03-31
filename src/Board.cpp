#include "Engine/Shader.hpp"
#include "Board.hpp"

void Board::init(const nite::Color &bg, int width, int height, const Shared<GameType> &gType){
    this->unitSize.width = width / gType->width;
    this->unitSize.height = height / gType->height;
    
    nite::Texture empty;
    empty.load("data/texture/empty.png");
    nite::Shader shader;
    shader.load("data/shaders/space_background_f.glsl", "data/shaders/space_background_v.glsl");

    float ratio = (float)(width*height) / (1080.0f * 1080.0f);
    this->font.load("data/font/Saniretro.ttf", 12.0f * ratio, 0.0f);
    this->subFont.load("data/font/Saniretro.ttf", 14.0f * ratio, 0.0f);

    this->width = width;
    this->height = height;
    batch.init(width, height);

    auto generateSpace = [&](const Space &space, const SpaceLayout &layout, nite::Batch &batch){
        // generate space
        nite::Batch spaceb;
        const float width = this->unitSize.width * layout.width;
        const float height = this->unitSize.height * layout.height;
        const nite::Color titleColor = space.type == SPACE_TYPE::PROPERTY ? space.color : nite::Color();
        const bool isProperty = space.type == SPACE_TYPE::PROPERTY || space.type == SPACE_TYPE::UTILITY || space.type == SPACE_TYPE::RAILROAD;        
        spaceb.init(width, height);
        spaceb.begin();
            nite::setColor(0.0f, 0.0f, 0.0f, 1.0f);
            auto body = empty.draw(0, 0, width, height, 0.0f, 0.0f, 0.0f);
            nite::Uniform uniform;
            uniform.add("size", nite::Vec2(width, height));
            uniform.add("border", 1.0f);
            uniform.add("baseColor", bg);
            uniform.add("borderColor", nite::Color(0.0f, 0.0f, 0.0f, 1.0f));
            uniform.add("useTitle", space.type == SPACE_TYPE::PROPERTY);
            uniform.add("titleHeight", height*(1.0f/6.0f));
            uniform.add("titleColor", titleColor);
            body->apply(shader, uniform);
            nite::setColor(0.0f, 0.0f, 0.0f, 1.0f);
            auto robj = this->font.draw(nite::toUpper(space.name), width * 0.5f, height * 0.5f, 0.5f, 0.5f, 0.0f);
            robj->autobreak = true;
            robj->horSpace = width-8;
            robj->align = nite::TextAlign::CENTER;
            if(isProperty){
                nite::setColor(0.1f, 0.1f, 0.1f, 1.0f);
                auto robj = this->subFont.draw("$"+nite::toStr(space.price), width * 0.5f, height * 0.85f, 0.5f, 0.5f, 0.0f);
                robj->autobreak = true;
                robj->horSpace = width-4;
                robj->align = nite::TextAlign::CENTER;
            }
        spaceb.end();
        spaceb.flush();
        // append to board's batch
        float rotw = width * nite::cos(nite::toRadians(layout.orientation)) - height * nite::sin(nite::toRadians(layout.orientation));
        float roth = width * nite::sin(nite::toRadians(layout.orientation)) + height * nite::cos(nite::toRadians(layout.orientation));
        float x = (this->unitSize.width * layout.x) + nite::abs(rotw)*0.5;
        float y = (this->unitSize.height * layout.y) + nite::abs(roth)*0.5;
        batch.begin();
            nite::setColor(1.0f, 1.0f, 1.0f, 1.0f);
            spaceb.draw(x, y, spaceb.getSize().x, spaceb.getSize().y, 0.5f, 0.5f, layout.orientation);
        batch.end();
        batch.flush(false); // false doesn't clear batch
        return spaceb;
    };


    for(auto &it : gType->layout){
        auto &space = gType->spaces[it.first];
        auto b = generateSpace(space, it.second, this->batch);
    }

}

void Board::draw(float x, float y){
    nite::setDepth(nite::DepthMiddle);
    nite::setColor(1.0f, 1.0f, 1.0f, 1.0f);
    nite::setRenderTarget(nite::RenderTargetGame);
    batch.draw(x, y, batch.getSize().x, batch.getSize().y, 0.0f, 0.0f, 0.0f);
}