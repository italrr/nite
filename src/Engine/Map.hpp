#ifndef NITE_MAP_HPP
    #define NITE_MAP_HPP

    #include "Types.hpp"
    #include "Texture.hpp"
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
            // some pre-computed values
            Int32 vx; // (valuex,valuey) in-texture coors (from source)
            Int32 vy;
            Int32 imx; // (in-map-x, in-map-y) in-map coors
            Int32 imy;
            MapCell(){
                vx = 0;
                vy = 0;
            }
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
            nite::MapCell *cells; // w * h
            nite::Texture src; // proxy to the local copy for actually rendering
            String srcHash;
            UInt32 total;
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

            Dict<String, nite::MapTilesetSource> sources;
            // TODO: in the future we most likely will adapt this to use trees/chunks to improve performance
            Vector<nite::MapLayer*> layers; 
            Vector<nite::MapWallMask> masks;
            Map();
            ~Map();
            // load will also load the fragments 
            // and compose them with the bool compose() function
            bool load(const String &path);

            // for compose to work, the fragment needs to have the exact same tileSize,
            // margin and spacing as the composed, otherwise bool returns false (failure)
            bool compose(const nite::Map &fragment); // for composing fragments


            bool setup(int n, const nite::Vec2 &size, const nite::Vec2 &tileSize, const nite::Vec2 &margin, const nite::Vec2 &spacing);
            bool add(int *cells, int depth, const String &src, const String &type = "generic", bool dynamicY = false);
            // viewport and xy are recommended to be at least 25% bigger than the actual view size
            // to avoid black spots when zooming out
            void render(int x, int y, float wviewport, float hviewport);
            void unload();
            bool exportToJson(const String &path, bool allowOverwrite = true);
        };

    } 

#endif