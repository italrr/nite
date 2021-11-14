#ifndef NITE_MAP_HPP
    #define NITE_MAP_HPP

    #include "Types.hpp"
    #include "Texture.hpp"
    #include "Graphics.hpp"
    #include "Object.hpp"
    #include "Indexer.hpp"

    namespace nite {


        /*
            The server is always going to send a fully composed map with a copy
            of the title, author and version in the metadata. The server never sends
            fragments.

            The server also sends all the sources (although not composed into a single image file).
            It's the client's job to fetch these sources and use them when rendering the composed map
            sent by the server.

            However, a client can request the usage of a custom map (provided by the client itself)
            the request must provide the map itself and its fragments(dependencies) for the server to
            build the composed one and send it to all the clients (including the client who requested it)
        */

        struct MapCell {
            Int32 x; // these are in units, not pixels
            Int32 y;
            Int32 index;
            Int32 val;
            MapCell(){
            }
            MapCell(int i, int v, int w){
                this->index = i;
                this->val = v;
                this->x = i % w;
                this->y = i / w;
            }
            MapCell(const nite::Vec2 &p, int v, int w){
                this->index = p.x + p.y * w;
                this->x = p.x;
                this->y = p.y;
                this->val = v;
            }
        };

        struct MapRoute {
            Vector<nite::MapCell> route;
            UInt32 start;
            UInt32 end;
        };

        struct MapTilesetSource {
            String hash; // also the id
            String filename; // relative to the client/server
            size_t size; // in bytes;
            nite::Vec2 bsize; //in pixels
            nite::Vec2 units;
            nite::Texture src; // local copy used for actually rendering
        };

        struct MapLayer {
            nite::Texture src; // proxy to the local copy for actually rendering
            nite::TextureCellBatch batch;
            Int32 *cells;
            String srcHash;
            UInt32 total;
            nite::Vec2 position;    
            bool depthTest;
            String name;
            bool visible;
            bool ySort;
            bool floorDepth;
            UInt16 w;
            UInt16 h;
            Int8 depth;
            bool dynamicY;
            String type;
            MapLayer();
            void generate(int w, int h, int dvalue, int depth, const String &srcHash, const nite::Texture &src);
            void clear();
        };      

        struct MapWallMask {
            nite::Vec2 position;
            nite::Vec2 size;
            Int32 index;
        };

        struct MapDynamicTileGroup {
            Dict<int, int> stateToIndex;
            Vector<int> tiles;
            int state;
            int layer;
            int id;
        };

        struct MapSpecialDescriptor {
            int id;
            int ref;
            String tag;
            String type;
            nite::Vec2 position;
            nite::Vec2 size;            
        };

        struct Map {

            String title;
            String author;
            String version;
            String hash; // hash is also id. we'll be using MD5 to make the hashes

            nite::Vec2 size; // in units
            UInt32 total;
            nite::Vec2 tileSize; // in "pixels"
            nite::Vec2 margin;
            nite::Vec2 spacing;
            nite::Vec2 position; // from which the cells start (relative to the one being composed, it would x->0,y->0 if none)
            nite::MapCell startCell;
            nite::MapCell endCell;
            nite::Texture empty;
            nite::Batch batch;

            Dict<String, nite::MapTilesetSource> sources;
            Vector<nite::MapLayer*> layers; 
            Vector<nite::MapWallMask> masks;
            
            // Vector
            Map();
            ~Map();
            // load will also load the fragments 
            // and compose them with the bool compose() function
            bool load(const String &path);

            // for compose to work, the fragment needs to have the exact same tileSize,
            // margin and spacing as the composed, otherwise bool returns false (failure)
            bool compose(const nite::Map &fragment); // for composing fragments

            int lastDTId;
            Dict<int, MapSpecialDescriptor> specials;
            Dict<int, MapDynamicTileGroup> dynamicTiles;
            int generateDescriptor(const nite::Vec2 &p, const nite::Vec2 &s, const String &type, const String &tag, int ref = 0, int setId = 0);
            int generateDynamicTile(const Dict<int, int> &stateToIndex, const Vector<int> &tiles, int layer, int setId = 0);
            void setDynamicTileState(int id, int state);
            void resetDynamicTile(int id);

            bool setup(int n, const nite::Vec2 &size, const nite::Vec2 &tileSize, const nite::Vec2 &margin, const nite::Vec2 &spacing);
            bool add(int *cells, int depth, const String &src, const String &type = "generic", bool dynamicY = false);
            void render(const nite::Vec2 &pos, const nite::Rect &vwp);
            void unload();
            bool exportToJson(const String &path, bool allowOverwrite = true);
        };

    } 

#endif