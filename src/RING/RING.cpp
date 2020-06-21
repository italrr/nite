#include <iostream>
#include <stdlib.h>
#include <string.h>

#include "../Engine/Shapes.hpp"
#include "../Engine/Console.hpp"
#include "../Engine/Input.hpp"
#include "../Engine/View.hpp"

#include "RING.hpp"

#define MAPPING_CRITERIA_WIDTH 5
#define MAPPING_CRITERIA_HEIGHT 5
#define MAPPING_CRITERIA_SAMPLE MAPPING_CRITERIA_WIDTH * MAPPING_CRITERIA_HEIGHT
#define MAPPING_CRITERIA_ANY -1
#define MAPPING_CRITERIA_OUT_OF_BOUNDS -2
#define MAPPING_CRITERIA_OFFSET_START_X 2
#define MAPPING_CRITERIA_OFFSET_START_Y 2

struct MappingCriteria {
    String key;
    int region[MAPPING_CRITERIA_SAMPLE]; // center plus surroundings (center is always x 2 y 2)
    void pull(int index, int *grid, int width, int height){ // copy cells 
        for(int i = 0; i < MAPPING_CRITERIA_SAMPLE; ++i){
            int xrel = i % MAPPING_CRITERIA_WIDTH;
            int yrel = i / MAPPING_CRITERIA_WIDTH;
            int gridx = index % width;
            int gridy = index / width;
            int xg = gridx - MAPPING_CRITERIA_OFFSET_START_X;
            int yg = gridy - MAPPING_CRITERIA_OFFSET_START_Y;
            int xi = (xg + xrel);
            int yi = (yg + yrel);
            if(xi < 0 || yi < 0 || xi >= width || yi >= height){  // out of bouds
                this->region[i] = MAPPING_CRITERIA_OUT_OF_BOUNDS;
                continue;
            }            
            this->region[i] = grid[xi + yi * width];
        }
    }
    void set(int region[MAPPING_CRITERIA_SAMPLE], const String &key){
        this->key = key;
        for(int i = 0; i < MAPPING_CRITERIA_SAMPLE; ++i){
            this->region[i] = region[i];
        }
    }
    bool match(const MappingCriteria &other){
        for(int i = 0; i < MAPPING_CRITERIA_SAMPLE; ++i){
            if(this->region[i] == MAPPING_CRITERIA_ANY){
                continue;
            }            
            if(this->region[i] != other.region[i]){
                return false;
            }
        }
        return true;
    }
    MappingCriteria(int region[MAPPING_CRITERIA_SAMPLE], const String &key){
        set(region, key);
    }
    MappingCriteria(int index, int *grid, int width, int height){
        pull(index, grid, width, height);
    }    
    MappingCriteria(){

    }
    operator std::string() const{
        String output;
        for(int i = 0; i < MAPPING_CRITERIA_SAMPLE; ++i){
            String lett = "";
            if(this->region[i] == Game::RING::CellType::Path){
                lett = "P";
            }else
            if(this->region[i] == Game::RING::CellType::Wall){
                lett = "W";
            }else
            if(this->region[i] == MAPPING_CRITERIA_ANY){
                lett = "A";
            }else
            if(this->region[i] == MAPPING_CRITERIA_OUT_OF_BOUNDS){
                lett = "O";
            }
            output += lett+" ";
            if(i % MAPPING_CRITERIA_WIDTH == MAPPING_CRITERIA_WIDTH-1){
                output += "\n";
            }
        }
        if(this->key.length() > 0){
            output += "\n'"+this->key+"'";
        }
        return  output;
    }
};

// TODO: Convert these to JSONs
namespace DEFAULT_MAPPING {
    #define P Game::RING::CellType::Path
    #define W Game::RING::CellType::Wall
    #define A MAPPING_CRITERIA_ANY
    static const MappingCriteria criteria[] = {

        // wall_corner_bbottom_left

        MappingCriteria((int[]){    W, W, W, P, P,  
                                    W, W, W, P, P,
                                    W, W, W, W, W,
                                    W, W, W, W, W,
                                    W, W, W, W, W }, "wall_corner_bbottom_left"),


        // wall_corner_bbottom_right

        MappingCriteria((int[]){    P, P, W, W, W,  
                                    P, P, W, W, W,
                                    W, W, W, W, W,
                                    W, W, W, W, W,
                                    W, W, W, W, W }, "wall_corner_bbottom_right"),


        // wall_horizontal_bmiddle
        
        MappingCriteria((int[]){    P, P, P, P, P,  
                                    P, P, P, P, P,
                                    W, W, W, W, W,
                                    W, W, W, W, W,
                                    W, W, W, W, W }, "wall_horizontal_bmiddle"),   

        MappingCriteria((int[]){    P, P, P, P, P,  
                                    P, P, P, P, P,
                                    P, W, W, W, P,
                                    P, W, W, W, P,
                                    P, W, W, W, P }, "wall_horizontal_bmiddle"),                                      

        MappingCriteria((int[]){    W, W, P, P, P,  
                                    W, W, P, P, P,
                                    W, W, W, W, W,
                                    W, W, W, W, W,
                                    W, W, W, W, W }, "wall_horizontal_bmiddle"),   

        MappingCriteria((int[]){    W, P, P, P, P,  
                                    W, P, P, P, P,
                                    W, W, W, W, W,
                                    W, W, W, W, W,
                                    W, W, W, W, W }, "wall_horizontal_bmiddle"), 

        MappingCriteria((int[]){    P, P, P, P, W,  
                                    P, P, P, P, W,
                                    W, W, W, W, W,
                                    W, W, W, W, W,
                                    W, W, W, W, W }, "wall_horizontal_bmiddle"), 

        MappingCriteria((int[]){    P, P, P, W, W,  
                                    P, P, P, W, W,
                                    W, W, W, W, W,
                                    W, W, W, W, W,
                                    W, W, W, W, W }, "wall_horizontal_bmiddle"),                                                                                                                                                   



        // wall_corner_btop_left

        MappingCriteria((int[]){    W, W, W, W, W,  
                                    W, W, W, W, W,
                                    W, W, W, W, W,
                                    W, W, W, P, P,
                                    W, W, W, P, P }, "wall_corner_btop_left"),
        // wall_corner_ttop_left

        MappingCriteria((int[]){    A, A, A, A, A,  
                                    W, W, W, W, W,
                                    W, W, W, W, W,
                                    W, W, W, W, W,
                                    W, W, W, P, P }, "wall_corner_ttop_left"),                                                                       

        // wall_corner_btop_right

        MappingCriteria((int[]){    W, W, W, W, W,  
                                    W, W, W, W, W,
                                    W, W, W, W, W,
                                    P, P, W, W, W,
                                    P, P, W, W, W }, "wall_corner_btop_right"),      

        // wall_corner_ttop_right

        MappingCriteria((int[]){    A, A, A, A, A,  
                                    W, W, W, W, W,
                                    W, W, W, W, W,
                                    W, W, W, W, W,
                                    P, P, W, W, W }, "wall_corner_ttop_right"), 


        // wall_horizontal_tmiddle

        MappingCriteria((int[]){    A, A, A, A, A,  
                                    W, W, W, W, W,
                                    W, W, W, W, W,
                                    W, W, W, W, W,
                                    P, P, P, P, P }, "wall_horizontal_tmiddle"), 

        MappingCriteria((int[]){    A, A, A, A, A,  
                                    W, W, W, W, W,
                                    W, W, W, W, W,
                                    W, W, W, W, W,
                                    W, P, P, P, W }, "wall_horizontal_tmiddle"),  

        MappingCriteria((int[]){    A, A, A, A, A,  
                                    W, W, W, W, W,
                                    W, W, W, W, W,
                                    W, W, W, W, W,
                                    W, W, P, P, P }, "wall_horizontal_tmiddle"), 

        MappingCriteria((int[]){    A, A, A, A, A,  
                                    W, W, W, W, W,
                                    W, W, W, W, W,
                                    W, W, W, W, W,
                                    W, P, P, P, P }, "wall_horizontal_tmiddle"), 

        MappingCriteria((int[]){    A, A, A, A, A,  
                                    W, W, W, W, W,
                                    W, W, W, W, W,
                                    W, W, W, W, W,
                                    P, P, P, W, W }, "wall_horizontal_tmiddle"),                                                                       

        MappingCriteria((int[]){    A, A, A, A, A,  
                                    W, W, W, W, W,
                                    W, W, W, W, W,
                                    W, W, W, W, W,
                                    P, P, P, P, W }, "wall_horizontal_tmiddle"),  

        MappingCriteria((int[]){    W, P, P, P, W,  
                                    W, P, P, P, W,
                                    W, P, P, P, W,
                                    W, W, W, W, W,
                                    W, W, W, W, W }, "wall_horizontal_tmiddle"), 

        MappingCriteria((int[]){    P, P, P, P, P,  
                                    P, P, P, P, P,
                                    P, P, P, P, P,
                                    W, W, W, W, W,
                                    W, W, W, W, W }, "wall_horizontal_tmiddle"),   

        MappingCriteria((int[]){    P, P, P, P, P,  
                                    P, P, P, P, P,
                                    P, P, P, P, P,
                                    P, W, W, W, P,
                                    P, W, W, W, P }, "wall_horizontal_tmiddle"),                                      

        MappingCriteria((int[]){    W, P, P, P, P,  
                                    W, P, P, P, P,
                                    W, P, P, P, P,
                                    W, W, W, W, W,
                                    W, W, W, W, W }, "wall_horizontal_tmiddle"), 

        MappingCriteria((int[]){    W, W, P, P, P,  
                                    W, W, P, P, P,
                                    W, W, P, P, P,
                                    W, W, W, W, W,
                                    W, W, W, W, W }, "wall_horizontal_tmiddle"),   

        MappingCriteria((int[]){    P, W, W, W, P,  
                                    W, W, W, W, P,
                                    W, W, W, W, P,
                                    W, W, W, W, P,
                                    P, P, P, P, P }, "wall_horizontal_tmiddle"),        

        MappingCriteria((int[]){    P, W, W, W, P,  
                                    P, W, W, W, W,
                                    P, W, W, W, W,
                                    P, W, W, W, W,
                                    P, P, P, P, P }, "wall_horizontal_tmiddle"),                                                                  

        MappingCriteria((int[]){    P, P, P, P, W,  
                                    P, P, P, P, W,
                                    P, P, P, P, W,
                                    W, W, W, W, W,
                                    W, W, W, W, W }, "wall_horizontal_tmiddle"),  

        MappingCriteria((int[]){    P, P, P, W, W,  
                                    P, P, P, W, W,
                                    P, P, P, W, W,
                                    W, W, W, W, W,
                                    W, W, W, W, W }, "wall_horizontal_tmiddle"),                                                                                                                                                                                          

        MappingCriteria((int[]){    P, P, P, P, P,  
                                    P, P, P, P, P,
                                    P, P, P, P, P,
                                    P, W, W, W, W,
                                    P, W, W, W, W }, "wall_horizontal_tmiddle"),

        MappingCriteria((int[]){    P, P, P, P, P,  
                                    P, W, W, W, W,
                                    P, W, W, W, W,
                                    P, W, W, W, W,
                                    P, P, P, P, P }, "wall_horizontal_tmiddle"),  


        MappingCriteria((int[]){    P, P, P, P, P,  
                                    P, P, P, P, P,
                                    P, P, P, P, P,
                                    W, W, W, W, P,
                                    W, W, W, W, P }, "wall_horizontal_tmiddle"), 


        MappingCriteria((int[]){    P, W, W, W, P,  
                                    P, W, W, W, P,
                                    P, W, W, W, P,
                                    P, W, W, W, P,
                                    P, P, P, P, P }, "wall_horizontal_tmiddle"),    

        MappingCriteria((int[]){    P, P, P, P, P,  
                                    W, W, W, W, P,
                                    W, W, W, W, P,
                                    W, W, W, W, P,
                                    P, P, P, P, P }, "wall_horizontal_tmiddle"),                                                                                                                                             

        // wall_horizontal_bmiddle


        MappingCriteria((int[]){    W, W, W, W, W,  
                                    W, W, W, W, W,
                                    W, W, W, W, W,
                                    W, P, P, P, W,
                                    W, P, P, P, W }, "wall_horizontal_bmiddle"), 

        MappingCriteria((int[]){    W, W, W, W, W,  
                                    W, W, W, W, W,
                                    W, W, W, W, W,
                                    P, P, P, P, P,
                                    P, P, P, P, P }, "wall_horizontal_bmiddle"), 


        MappingCriteria((int[]){    W, W, W, W, P,  
                                    W, W, W, W, P,
                                    W, W, W, W, P,
                                    P, P, P, P, P,
                                    P, P, P, P, P }, "wall_horizontal_bmiddle"),  

        MappingCriteria((int[]){    P, W, W, W, P,  
                                    P, W, W, W, P,
                                    P, W, W, W, P,
                                    P, P, P, P, P,
                                    P, P, P, P, P }, "wall_horizontal_bmiddle"),                                             


        MappingCriteria((int[]){    P, W, W, W, W,  
                                    P, W, W, W, W,
                                    P, W, W, W, W,
                                    P, P, P, P, P,
                                    P, P, P, P, P }, "wall_horizontal_bmiddle"),                                                                          

        MappingCriteria((int[]){    W, W, W, W, W,  
                                    W, W, W, W, W,
                                    W, W, W, W, W,
                                    W, W, P, P, P,
                                    W, W, P, P, P }, "wall_horizontal_bmiddle"),   

        MappingCriteria((int[]){    W, W, W, W, W,
                                    W, W, W, W, W, 
                                    W, W, W, W, W,
                                    W, P, P, P, P,
                                    W, P, P, P, P }, "wall_horizontal_bmiddle"),     


        MappingCriteria((int[]){    W, W, W, W, W,  
                                    W, W, W, W, W,
                                    W, W, W, W, W,
                                    P, P, P, P, W,
                                    P, P, P, P, W }, "wall_horizontal_bmiddle"),                                                                                                        


        MappingCriteria((int[]){    W, W, W, W, W,  
                                    W, W, W, W, W,
                                    W, W, W, W, W,
                                    P, P, P, W, W,
                                    P, P, P, W, W }, "wall_horizontal_bmiddle"),  

        MappingCriteria((int[]){    W, P, P, P, W,  
                                    W, P, P, P, W,
                                    W, W, W, W, W,
                                    W, W, W, W, W,
                                    W, W, W, W, W }, "wall_horizontal_bmiddle"),  

        MappingCriteria((int[]){    P, P, P, P, P,  
                                    P, P, P, P, P,
                                    P, W, W, W, W,
                                    P, W, W, W, W,
                                    P, W, W, W, W }, "wall_horizontal_bmiddle"), 


        MappingCriteria((int[]){    P, P, P, P, P,  
                                    P, P, P, P, P,
                                    W, W, W, W, P,
                                    W, W, W, W, P,
                                    W, W, W, W, P }, "wall_horizontal_bmiddle"),                                                                               
                                                                    


        // wall_vertical_lmiddle
        MappingCriteria((int[]){    W, W, W, P, P,  
                                    W, W, W, P, P,
                                    W, W, W, P, P,
                                    W, W, W, P, P,
                                    W, W, W, P, P }, "wall_vertical_lmiddle"),  


        MappingCriteria((int[]){    W, W, W, W, W,  
                                    W, W, W, P, P,
                                    W, W, W, P, P,
                                    W, W, W, P, P,
                                    W, W, W, P, P }, "wall_vertical_lmiddle"), 

        MappingCriteria((int[]){    W, W, W, W, W,  
                                    W, W, W, P, P,
                                    W, W, W, P, P,
                                    W, W, W, P, P,
                                    W, W, W, W, W }, "wall_vertical_lmiddle"),                                     

        MappingCriteria((int[]){    P, P, P, P, P,  
                                    W, W, W, P, P,
                                    W, W, W, P, P,
                                    W, W, W, P, P,
                                    W, W, W, P, P }, "wall_vertical_lmiddle"),   

        MappingCriteria((int[]){    P, P, P, P, P,  
                                    P, P, P, P, P,
                                    W, W, W, P, P,
                                    W, W, W, P, P,
                                    W, W, W, P, P }, "wall_vertical_lmiddle"),                                                                          

        MappingCriteria((int[]){    W, W, W, W, W,  
                                    W, W, W, W, W,
                                    W, W, W, P, P,
                                    W, W, W, P, P,
                                    W, W, W, P, P }, "wall_vertical_lmiddle"), 

        MappingCriteria((int[]){    W, W, W, P, P,  
                                    W, W, W, P, P,
                                    W, W, W, P, P,
                                    W, W, W, P, P,
                                    W, W, W, W, W }, "wall_vertical_lmiddle"),                                   


        MappingCriteria((int[]){    W, W, W, P, P,  
                                    W, W, W, P, P,
                                    W, W, W, P, P,
                                    W, W, W, P, P,
                                    P, P, P, P, P }, "wall_vertical_lmiddle"),  

        MappingCriteria((int[]){    P, P, P, P, P,  
                                    W, W, W, P, P,
                                    W, W, W, P, P,
                                    W, W, W, P, P,
                                    P, P, P, P, P }, "wall_vertical_lmiddle"), 


        // wall_vertical_rmiddle

        MappingCriteria((int[]){    P, P, W, W, W,  
                                    P, P, W, W, W,
                                    P, P, W, W, W,
                                    P, P, W, W, W,
                                    P, P, W, W, W }, "wall_vertical_rmiddle"),  

        MappingCriteria((int[]){    W, W, W, W, W,  
                                    P, P, W, W, W,
                                    P, P, W, W, W,
                                    P, P, W, W, W,
                                    W, W, W, W, W }, "wall_vertical_rmiddle"),                                     


        MappingCriteria((int[]){    P, P, P, P, P,  
                                    P, P, W, W, W,
                                    P, P, W, W, W,
                                    P, P, W, W, W,
                                    P, P, W, W, W }, "wall_vertical_rmiddle"),


        MappingCriteria((int[]){    P, P, P, P, P,  
                                    P, P, W, W, W,
                                    P, P, W, W, W,
                                    P, P, W, W, W,
                                    P, P, P, P, P }, "wall_vertical_rmiddle"),                                    


        MappingCriteria((int[]){    P, P, P, P, P,  
                                    P, P, P, P, P,
                                    P, P, W, W, W,
                                    P, P, W, W, W,
                                    P, P, W, W, W }, "wall_vertical_rmiddle"),                                    

        MappingCriteria((int[]){    W, W, W, W, W,  
                                    P, P, W, W, W,
                                    P, P, W, W, W,
                                    P, P, W, W, W,
                                    P, P, W, W, W }, "wall_vertical_rmiddle"),  

        MappingCriteria((int[]){    W, W, W, W, W,  
                                    W, W, W, W, W,
                                    P, P, W, W, W,
                                    P, P, W, W, W,
                                    P, P, W, W, W }, "wall_vertical_rmiddle"), 

        MappingCriteria((int[]){    P, P, W, W, W,  
                                    P, P, W, W, W,
                                    P, P, W, W, W,
                                    P, P, W, W, W,
                                    W, W, W, W, W }, "wall_vertical_rmiddle"),

        MappingCriteria((int[]){    P, P, W, W, W,  
                                    P, P, W, W, W,
                                    P, P, W, W, W,
                                    P, P, W, W, W,
                                    P, P, P, P, P }, "wall_vertical_rmiddle"),                                                                            

        // wall_corner_tbottom_left

        MappingCriteria((int[]){    W, W, W, P, P,  
                                    W, W, W, P, P,
                                    W, W, W, P, P,
                                    W, W, W, W, W,
                                    W, W, W, W, W }, "wall_corner_tbottom_left"),                                                                                                                                             
        
        // wall_corner_tbottom_right

        MappingCriteria((int[]){    P, P, W, W, W,  
                                    P, P, W, W, W,
                                    P, P, W, W, W,
                                    W, W, W, W, W,
                                    W, W, W, W, W }, "wall_corner_tbottom_right"), 


        // wall_vertical_lbottom    

        MappingCriteria((int[]){    W, W, W, P, P,  
                                    W, W, W, P, P,
                                    W, W, W, P, P,
                                    P, P, P, P, P,
                                    P, P, P, P, P }, "wall_vertical_lbottom"),         

        // wall_vertical_rbottom  
        MappingCriteria((int[]){    P, P, W, W, W,  
                                    P, P, W, W, W,
                                    P, P, W, W, W,
                                    P, P, P, P, P,
                                    P, P, P, P, P }, "wall_vertical_rbottom"),  

        // wall_vertical_rtop 
        MappingCriteria((int[]){    P, P, P, P, P,  
                                    P, P, P, P, P,
                                    P, P, P, P, P,
                                    P, P, W, W, W,
                                    P, P, W, W, W }, "wall_vertical_rtop"),     


        // wall_vertical_ltop 
        MappingCriteria((int[]){    P, P, P, P, P,  
                                    P, P, P, P, P,
                                    P, P, P, P, P,
                                    W, W, W, P, P,
                                    W, W, W, P, P }, "wall_vertical_ltop"),                                                                                                            
                                                                                             
    };
                                                                        
    static const MappingCriteria DEFAULT = MappingCriteria((int[]){ -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 }, "");
    static const int total = sizeof(criteria) / sizeof(MappingCriteria);
    MappingCriteria match(const MappingCriteria &src){
       for(int i = 0; i < total; ++i){
           auto crit = criteria[i];
           if(crit.match(src)){
               return crit;
           }
       }
       return DEFAULT;
    }
    #undef P
    #undef W
    #undef A
};

Game::RING::TileSource::TileSource(const String &path){
    load(path);
}

Game::RING::TileSource::TileSource(){

}

void Game::RING::TileSource::load(const String &path){
    Jzon::Parser parser;
    Jzon::Node node = parser.parseFile(path);
    if (!node.isValid()){
        nite::print("failed to load json "+path+": '"+parser.getError()+"'");
        return;
    }
    this->name = node.get("name").toString();
    this->description = node.get("description").toString();
    this->sourcePath = node.get("source").toString();
    this->transparency.set(nite::Color(node.get("transparency")));
    this->source.load(this->sourcePath, this->transparency);
    this->sourceSize.set(node.get("sourceWidth").toFloat(), node.get("sourceHeight").toFloat());
    this->margin = node.get("margin").toFloat();
    this->spacing = node.get("spacing").toFloat();
    this->tileSize.set(node.get("tileWidth").toFloat(), node.get("tileHeight").toFloat());
    this->floorDefault = node.get("floorDefault").toString();
    this->floorVarianceFactor = node.get("floorVarianceFactor").toFloat();
    this->lastFloorVariant = floorDefault;
    auto mappings = node.get("mapping");
    for(auto mapp : mappings){
        this->mapping[mapp.first] = mapp.second.toInt();
    }
    // TODO: add checks key against mappings    
    if(node.has("ignoresForFloor") && node.get("ignoresForFloor").isArray()){
        for(int i = 0; i < node.get("ignoresForFloor").getCount(); ++i){
            this->ignoresForFloor.push_back(node.get("ignoresForFloor").get(i).toString());
        }
    }
    if(node.has("dynamicYDepth") && node.get("dynamicYDepth").isArray()){
        for(int i = 0; i < node.get("dynamicYDepth").getCount(); ++i){
            this->dynamicYDepth.push_back(node.get("dynamicYDepth").get(i).toString());
        }
    }    
    if(node.has("floorVariants") && node.get("floorVariants").isArray()){
        for(int i = 0; i < node.get("floorVariants").getCount(); ++i){
            this->floorVariants.push_back(node.get("floorVariants").get(i).toString());
        }
    }
    if(node.has("specialWallMasks") && node.get("specialWallMasks").isObject()){
        auto objs = node.get("specialWallMasks");
        for(auto obj : objs){
            Game::RING::TileSourceWalkMask mask;
            mask.key = obj.first;
            mask.size.x = obj.second.get("width").toFloat();
            mask.size.y = obj.second.get("height").toFloat();
            mask.offset.x = obj.second.get("xOffset").toFloat();
            mask.offset.y = obj.second.get("yOffset").toFloat();            
            this->specialWallMasks.push_back(mask);
        }        
    }
    nite::print("loaded builder template json '"+path+"'");
}


Game::RING::TileSourceWalkMask Game::RING::TileSource::getSpecialWallMask(int mappVal){
    Game::RING::TileSourceWalkMask empty;
    for(auto mapp : this->mapping){
        if(mapp.second == mappVal){
            for(int i = 0; i < this->specialWallMasks.size(); ++i){
                if(this->specialWallMasks[i].key == mapp.first){
                    return this->specialWallMasks[i];
                }
            }
        }
    }
    return empty;
}

String Game::RING::TileSource::getFloorVariant(Game::RING::Blueprint &bp){
    bool jackpot = nite::randomInt(1, 100) < 100 * this->floorVarianceFactor;
    if(floorVariants.size() == 0 || !jackpot){
        return floorDefault;
    }
    String current = floorDefault;
    while(current.length() == 0 || current == lastFloorVariant){
        current = floorVariants[nite::randomInt(0, floorVariants.size() - 1)];
    }
    lastFloorVariant = current;
    return current;
}

bool Game::RING::TileSource::isDynamicYDepth(const String &key){
    for(int i = 0; i < dynamicYDepth.size(); ++i){
        if(dynamicYDepth[i] == key){
            return true;
        }
    }
    return false;
}

bool Game::RING::TileSource::isIgnoredForFLoors(const String &key){
    for(int i = 0; i < ignoresForFloor.size(); ++i){
        if(ignoresForFloor[i] == key){
            return true;
        }
    }
    return false;
}

Shared<nite::Map> Game::RING::generateMap(Shared<Game::RING::Blueprint> bp, Game::RING::TileSource &temp, int scale){
    UInt64 initTime = nite::getTicks();
    auto width = bp->width * scale;
    auto height = bp->height * scale;
    auto size = width * height;
    auto map = Shared<nite::Map>(new nite::Map());
    nite::print("generating map | size "+nite::Vec2(width, height).str()+" | total "+nite::toStr(size)+" cells");

    // create layers
    int floor[size];
    int walls[size];
    int dynamicY[size];
    // int specials[size];
    int mirror[size]; // temp
    int allWalls[size]; //temp

    for(int i = 0; i < size; ++i){
        floor[i] = -1;
        walls[i] = -1;
        dynamicY[i] = -1;
        // specials[i] = -1;
        mirror[i] = -1;
        allWalls[i] = -1;        
    }

    // fill grid using blueprint 
    for(int i = 0; i < size; ++i){
        int pind = (i % width) / scale + ((i / width) / scale) * bp->width; // index using projected x,y
        mirror[i] = bp->grid[pind].type;
    }

    // put walls using template
    for(int i = 0; i < size; ++i){
        auto crit = MappingCriteria(i, mirror, width, height);
        auto isMatch = DEFAULT_MAPPING::match(crit);
        // stored[i] = crit;
        if(isMatch.key != ""){
            if(temp.isDynamicYDepth(isMatch.key)){
                dynamicY[i] = temp.mapping[isMatch.key];
            }else{
                walls[i] = temp.mapping[isMatch.key];
            }
            allWalls[i] = temp.mapping[isMatch.key];
        }
        // anything that is a "path" or an inner wall gets to have a floor using ignoresForFloor for exceptions
        if(mirror[i] == Game::RING::CellType::Path || (isMatch.key.length() > 0 && !temp.isIgnoredForFLoors(isMatch.key))){ 
            floor[i] = temp.mapping[temp.getFloorVariant(*bp)];
        }        
    }
    
    // generate map
    String createdDate = nite::getTime(nite::TM::ALL);
    for(int i = 0; i < createdDate.length(); ++i){
        if(createdDate[i] == ' '){
            createdDate[i] = '-';
        }
        if(createdDate[i] == ':'){
            createdDate[i] = '_';
        }
    }
    String uuid = nite::hashString(nite::toStr(nite::getTicks()));
    map->title = "RING_"+createdDate+"_"+uuid;
    map->hash = uuid;
    map->author = "RING:none@none.com";
    map->version = "0.0.0";
    map->startCell.index = (bp->start % bp->width) * scale + (bp->start / bp->width) * scale;
    map->startCell.x = (bp->start % bp->width) * scale * temp.tileSize.x + temp.tileSize.x * scale * 0.5f;
    map->startCell.y = (bp->start / bp->width) * scale * temp.tileSize.y + temp.tileSize.y * scale * 0.5f;
    map->setup(3, nite::Vec2(width, height), temp.tileSize, temp.margin, temp.spacing);
    map->add(floor, 0, temp.sourcePath, "floor");
    map->add(walls, 1, temp.sourcePath, "walls");
    map->add(dynamicY, 2, temp.sourcePath, "dynamicY", true); // dynamicY is usually walls
    
    // now, create masks...
    // Vector<nite::MapWallMask> chunksTemp;
    
    // // prepare table (remember to delete at the end!)
    // auto _ptable = [](int w, int h, Vector<nite::MapWallMask> &wallMask){
    //     auto total = w*h;
    //     size_t tsize = sizeof(nite::MapWallMask);
    //     char *buffer = (char*)malloc(total * tsize);
    //     memset(buffer, 0, total * tsize);
    //     for(int i = 0; i < wallMask.size(); ++i){
    //         memcpy(buffer + wallMask[i].index * tsize, &wallMask[i], tsize);
    //     }
    //     return buffer;
    // };


    for(int i = 0; i < size; ++i){
        if(mirror[i] != Game::RING::CellType::Wall){
            continue;
        }
        auto mask = nite::MapWallMask();
        mask.size.set(temp.tileSize);
        mask.index = i;
        mask.position.x = (i % width) * temp.tileSize.x + temp.tileSize.x * 0.5f;
        mask.position.y = (i / width) * temp.tileSize.y + temp.tileSize.y * 0.5f;
        Game::RING::TileSourceWalkMask specMask = temp.getSpecialWallMask(allWalls[i]);
        if(specMask.key != ""){
            mask.size.set(specMask.size);
            mask.position.x += specMask.offset.x;        
            mask.position.y += specMask.offset.y;
        }
        // chunks[i % width][i / width] = mask;
        map->masks.push_back(mask);
        // chunksTemp.push_back(mask);
    }

    // auto table = _ptable(width, height, chunksTemp);

    // auto _xy2ind = [](int index, int w){
    //     return (index % w) + (index / w);
    // };

    // // optimize masks by joining them together
    // auto _lrj = [](int index, int w, char* chunks){
    //     int sindex = index;
    //     do {
    //         nite::MapWallMask* main = (nite::MapWallMask*)(chunks + index * sizeof(nite::MapWallMask));
    //         if(main == NULL){
    //             break;
    //         }
    //         ++sindex;
    //     } while (sindex % w != 0);
    // };

    // _lrj(0, width, table);

    // delete table;
    
    nite::print("built map: size "+nite::toStr(width)+"x"+nite::toStr(height)+"("+nite::toStr(size)+") | masks: "+nite::toStr(map->masks.size())+" | time: "+nite::toStr(nite::getTicks() - initTime)+" msecs");
    return map;
}