#include "Blueprint.hpp"
#include "../Engine/Texture.hpp"
#include <algorithm>
#include <random>

Game::RING::Cell::Cell(int x, int y){
    set(x, y);
}

Game::RING::Cell::Cell(){
    type = Game::RING::CellType::Wall;
    set(0, 0);
}

int Game::RING::Cell::index(int width){
    return x + y * width;
}

void Game::RING::Cell::set(int x, int y){
    this->x = x;
    this->y = y;
}

Game::RING::Blueprint::Blueprint(){
    grid = NULL;
    clear();
}

Game::RING::Blueprint::~Blueprint(){
    if(grid == NULL) return;
    delete grid;
}

void Game::RING::Blueprint::generate(int w, int h){
    width = nite::odd(w);
    height = nite::odd(h);
    total = width * height;    
    generate();
}

void Game::RING::Blueprint::generate(){
    UInt64 init = nite::getTicks();
    nite::print("generating RING blueprint, size "+nite::toStr(width)+"x"+nite::toStr(height)+" total: "+nite::toStr(total)+" blocks ...");
    this->grid = new Game::RING::Cell[total];
    for(int i = 0; i < this->total; ++i){
        Game::RING::Cell cell;
        cell.i = i;
        cell.type = Game::RING::CellType::Wall;
        cell.x = i %  width;
        cell.y = i / height;
        grid[i] = cell;
    }
    auto randodd = [&](int from, int to){
        return nite::odd(nite::randomInt(from, to));
    };
    // build maze
    // depth-first-search
    auto depthFirstSearch = [&](){
        // choose a random start position
        Game::RING::Cell spawn(randodd(3, width - 4), randodd(3, height - 4));    
        auto makeRoom = [&](int minWidth, int minHeight, int maxWidth, int maxHeight){
            // make room
            Game::RING::Cell size(nite::randomInt(minWidth, maxWidth), nite::randomInt(minHeight, maxHeight));
            Game::RING::Cell start;
            do {
                start = Game::RING::Cell(nite::randomInt(2, width - 2), nite::randomInt(2, height - 2));
            } while((start.x + size.x >= width - 1 || start.y + size.y >= height - 1));
            fillRectangle(start.x, start.y, size.x, size.y, Game::RING::CellType::Path);
            // // set up walls
            // for(int i = 0; i < size.x; ++i){
            //     for(int j = 0; j < size.y; ++j){
            //         if(i > 0 && i < size.x - 1 && j > 0 && j < size.y - 1) continue;
            //         Game::RING::Cell p = Game::RING::Cell(start.x + i, start.y + j);
                    

            //         set(p.x, p.y, Game::RING::CellType::Wall);
            //     }
            // }
            // make doors            
        };        
        start = spawn.index(width);
        set(spawn.x, spawn.y, Game::RING::CellType::Path);
        auto randdir = [&](){
            Vector<int> dirs;
            for(int i = 0; i < 4; ++i){
                dirs.push_back(i);
            }
            std::random_shuffle(dirs.begin(), dirs.end());           
            return dirs;
        };
        std::function<void(Game::RING::Cell)> recursion = [&](Game::RING::Cell cell){
            auto dirs = randdir();
            auto index = [&](int x, int y){
                return x + y * height;
            };
            auto ptype = Game::RING::CellType::Path;
            for(int i = 0; i < dirs.size(); ++i){
                switch(dirs[i]){
                    case 0: { // up
                        if(cell.y - 2 < 0) continue;
                        if(grid[index(cell.x, cell.y - 2)].type != ptype){
                            grid[index(cell.x, cell.y - 2)].type = ptype;
                            grid[index(cell.x, cell.y - 1)].type = ptype;
                            recursion(Game::RING::Cell(cell.x, cell.y - 2));
                        }
                    } break;
                    case 1: { // right
                        if(cell.x + 2 >= width) continue;
                        if(grid[index(cell.x + 2, cell.y)].type != ptype){
                            grid[index(cell.x + 2, cell.y)].type = ptype;
                            grid[index(cell.x + 1, cell.y)].type = ptype;
                            recursion(Game::RING::Cell(cell.x + 2, cell.y));
                        }
                    } break;
                    case 2: { // down
                        if(cell.y + 2 >= height) continue;
                        if(grid[index(cell.x, cell.y + 2)].type != ptype){
                            grid[index(cell.x, cell.y + 2)].type = ptype;
                            grid[index(cell.x, cell.y + 1)].type = ptype;
                            recursion(Game::RING::Cell(cell.x, cell.y + 2));
                        }
                    } break;
                    case 3: { // left
                        if(cell.x - 2 < 0) continue;
                        if(grid[index(cell.x - 2, cell.y)].type != ptype){
                            grid[index(cell.x - 2, cell.y)].type = ptype;
                            grid[index(cell.x - 1, cell.y)].type = ptype;
                            recursion(Game::RING::Cell(cell.x - 2, cell.y));
                        }
                    } break;                                                            
                }
            }
        };
        recursion(spawn);
        // for(int i = 0; i < 3; ++i){
        //     makeRoom(4, 4, 8, 8); // make a random room
        // }

    };
    

    depthFirstSearch();
    generateMinimap();

    for(int i = 0; i < total; ++i){
        if(grid[i].type == Game::RING::CellType::Path){
            ++this->totalPath;
        }
        if(grid[i].type == Game::RING::CellType::Wall){
            ++this->totalWall;
        }        
    }
    
    nite::print("generated RING blueprint. time: "+nite::toStr(nite::getTicks() - init)+" msecs");
}

void Game::RING::Blueprint::generateMinimap(){
    static nite::Texture tex("data/sprite/empty.png");
    auto offset = nite::Vec2(0.0f);
    float scale = 16.0f;    
    minimap.init(width * scale, height * scale);
    minimap.begin();
    for(int i = 0; i < total; ++i){
        auto &cell = grid[i];
        switch(cell.type){
            case Game::RING::CellType::Path: {
                    if(i == start){ 
                        nite::setColor(nite::Color(0.0f, 1.0f, 0.0f, 1.0f));
                    }else
                    if(i == end){
                        nite::setColor(nite::Color(1.0f, 1.0f, 0.0f, 1.0f));
                    }else{
                        nite::setColor(nite::Color(1.0f, 0.0f, 0.0f, 1.0f));
                    }
                } break;
            case Game::RING::CellType::Wall: {
                    nite::setColor(nite::Color(0.0f, 0.0f, 1.0f, 1.0f));
                } break;
        }
        nite::Vec2 p(offset + nite::Vec2(cell.x, cell.y) * scale);
        tex.draw(p.x, p.y, scale, scale, 0.0f, 0.0f, 0.0f);
    }
    minimap.end();
    minimap.flush();
}

void Game::RING::Blueprint::fill(int type){
    if(grid == NULL){
        nite::print("blueprint: cannot fill. grid is not initialized");
        return;
    }
    for(int i = 0; i < total; ++i){
        grid[i].type = type;
    }
}

void Game::RING::Blueprint::fillRectangle(int x, int y, int w, int h, int type){
    if(grid == NULL){
        nite::print("blueprint: cannot fill. grid is not initialized");
        return;
    }
    for(int i = 0; i < w; ++i){
        for(int j = 0; j < h; ++j){
            set(x + i, y + j, type);
        }
    }
}

void Game::RING::Blueprint::fillBorder(int type){
    if(grid == NULL){
        nite::print("blueprint: cannot fill. grid is not initialized");
        return;
    }
    for(int i = 0; i < this->total; ++i){
        auto &cell = grid[i];
        bool lb = cell.x == 0 && cell.y >= 0 && cell.y < height;
        bool rb = cell.x == width - 1 && cell.y >= 0 && cell.y < height;
        bool tb = cell.y == 0 && cell.x >= 0 && cell.x < width;
        bool bb = cell.y == height - 1 && cell.x >= 0 && cell.x < width;
        if(lb || rb || tb || bb){
            cell.type = type;
        }
    }
}

void Game::RING::Blueprint::set(int x, int y, int type){
    if(grid == NULL){
        nite::print("blueprint: cannot set. grid is not initialized");
        return;
    }
    int index = x + y * width;
    grid[index].type = type;
}

void Game::RING::Blueprint::clear(){
    if(grid != NULL) delete grid;
    nrooms = 4;
    width = nite::odd(35);
    height = nite::odd(35);
    total = width * height;
    order = 0;
    grid = NULL;
    String hash = nite::toStr(nite::getTicks() + 1);
    String name = nite::hashMemory((char*)hash.c_str(), hash.length());
}