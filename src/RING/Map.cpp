#include "Map.hpp"
#include "../Engine/Input.hpp"
#include "../Engine/View.hpp"
#include <iostream>
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

Game::RING::TileTemplate::TileTemplate(const String &path){
    load(path);
}

Game::RING::TileTemplate::TileTemplate(){

}

void Game::RING::TileTemplate::load(const String &path){
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
    if(node.has("floorVariants") && node.get("floorVariants").isArray()){
        for(int i = 0; i < node.get("floorVariants").getCount(); ++i){
            this->floorVariants.push_back(node.get("floorVariants").get(i).toString());
        }
    }
    nite::print("loaded builder template json '"+path+"'");
}

String Game::RING::TileTemplate::getFloorVariant(Game::RING::Blueprint &bp){
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

bool Game::RING::TileTemplate::isIgnoredForFLoors(const String &key){
    for(int i = 0; i < ignoresForFloor.size(); ++i){
        if(ignoresForFloor[i] == key){
            return true;
        }
    }
    return false;
}

Game::RING::Layer::Layer(int width, int height, int depth){
    this->width = width;
    this->height = height;
    this->total = this->width * this->height;
    this->grid = new int[this->total];
    this->depth = depth;
}

Game::RING::Layer::~Layer(){

}

void Game::RING::Layer::clear(){
    if(this->grid == NULL){
        return;
    }
    delete this->grid;
    this->total = 0;
    this->width = 0;
    this->height = 0;
    this->depth = 0;
}

void Game::RING::Layer::fill(int val){
    if(this->grid == NULL){
        return;
    }
    for(int i = 0; i < this->total; ++i){
        this->grid[i] = val;
    }
}

// for debugging (To be removed)
static MappingCriteria *stored = NULL;

void Game::RING::Map::build(Shared<Game::RING::Blueprint> bp, const Game::RING::TileTemplate &temp, int scale){
    UInt64 initTime = nite::getTicks();
    nite::print("about to build RING map using RING blueprint...");
    this->temp = temp;
    this->bp = bp;
    this->scale = scale;
    this->width = bp->width * scale;
    this->height = bp->height * scale;
    this->size = this->width * this->height;
    stored = new MappingCriteria[this->size]; // TODO: Remove

    // create layers
    Game::RING::Layer *floor = new Game::RING::Layer(this->width, this->height, 0);
    Game::RING::Layer *walls = new Game::RING::Layer(this->width, this->height, 1);
    Game::RING::Layer *specials = new Game::RING::Layer(this->width, this->height, 2);
    this->layers.push_back(floor);
    this->layers.push_back(walls);
    this->layers.push_back(specials);

    Game::RING::Layer mirror(this->width, this->height, 0);

    // fill out grid 
    walls->fill(-1);
    floor->fill(-1);
    specials->fill(-1);
    mirror.fill(-1);

    // fill grid using blueprint 
    for(int i = 0; i < this->size; ++i){
        int pind = (i % this->width) / scale + ((i / this->width) / scale) * bp->width; // index using projected x,y
        mirror.grid[i] = bp->grid[pind].type;
    }
    // put walls using template
    for(int i = 0; i < this->size; ++i){
        auto crit = MappingCriteria(i, mirror.grid, this->width, this->height);
        auto isMatch = DEFAULT_MAPPING::match(crit);
        stored[i] = crit;
        if(isMatch.key != ""){
            walls->grid[i] = this->temp.mapping[isMatch.key];
        }
        // everything that is a "path" or an inner wall gets to have a floor using ignoresForFloor for exceptions
        if(mirror.grid[i] == Game::RING::CellType::Path || (isMatch.key.length() > 0 && !this->temp.isIgnoredForFLoors(isMatch.key))){ 
            floor->grid[i] = this->temp.mapping[this->temp.getFloorVariant(*bp)];
        }        
    }
    // set start
    startPosition.x = (bp->start % bp->width) * scale * temp.tileSize.x + temp.tileSize.x * scale * 0.5f;
    startPosition.y = (bp->start / bp->width) * scale * temp.tileSize.y + temp.tileSize.y * scale * 0.5f;

    mirror.clear();
    nite::print("built RING map. time: "+nite::toStr(nite::getTicks() - initTime)+" msecs");
}

void Game::RING::Map::draw(){
    nite::setDepth(0.0f);
    nite::setRenderTarget(nite::RenderTargetGame);   
    nite::setColor(1.0f, 1.0f, 1.0f, 1.0f); 
    static nite::Texture emp("data/sprite/empty.png"); 
    int scellwidth = 4096 / 128;
    int scellheight = 4096 / 128;

    // for(int i = 0; i < this->size; ++i){
    //     int inx = (i %  this->width) * temp.tileSize.x;
    //     int iny = (i / this->width) * temp.tileSize.y;        
    //     int v = 130 - 1;
    //     float tileY = v / scellwidth;
    //     float tileX = v - tileY * scellwidth;
    //     temp.source.setRegion(tileX * 128, tileY * 128, 128, 128);
    //     auto obj = temp.source.draw(inx, iny);
    // }
    for(int j = 0; j < layers.size(); ++j){
        auto *grid = layers[j]->grid;
        for(int i = 0; i < this->size; ++i){
            nite::Vec2 mrp = nite::mousePosition() + nite::getView(nite::RenderTargetGame);
            int inx = (i %  this->width) * temp.tileSize.x;
            int iny = (i / this->width) * temp.tileSize.y;
            bool inRegion = nite::isPointInRegion(mrp, nite::Vec2(inx, iny), nite::Vec2(inx, iny) + nite::Vec2(128.0f));

            if(inRegion && nite::mousePressed(nite::butLEFT)){
                std::cout << (String)stored[i] << std::endl;
            }

            // if(grid[i] <= 0){
            //     nite::setColor(1.0f, 0.0f, 0.0f, inRegion ? 0.5f : 1.0f);
            //     emp.draw(nite::Vec2(inx, iny), nite::Vec2(128, 128), nite::Vec2(0.0f), 0.0f);
            //     continue;
            // }

            nite::setColor(1.0f, 1.0f, 1.0f, inRegion ? 0.5f : 1.0f); 
            int v = grid[i] - 1;
            float tileY = v / scellwidth;
            float tileX = v - tileY * scellwidth;
            temp.source.setRegion(tileX * 128, tileY * 128, 128, 128);
            auto obj = temp.source.draw(inx, iny);
        }
    }
}

Game::RING::Map::Map(){
}

Game::RING::Map::~Map(){
    for(int i = 0; i < layers.size(); ++i){
        layers[i]->clear();
        delete layers[i];
    }
    layers.clear();
}