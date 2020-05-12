nite Map
--------


Map system allows for maps to be fragmented in different files and/or JSON nodes and then joined together on runtime to form a bigger complex map (I call this fragment-mapping).
Another feature of this map system is the ability to stream chunks of a map at a time, allowing for massive maps. We might consider storing the data in a binary format for easy look ups while
streaming.

We'll be targeting to be able to host maps of up to 10000x10000 (1,000,000 cells)

Some of these are not mandatory allowing full flexibility
SPEC:
{
    "title": "",
    "author": "", // FORMAT: "Name:Email"
    "version": "1.0.0",
    "tileSize": {
      "width": 64, // in pixels
      "height" 64
    },
    "spacing": {
        "x": 0, // in pixels
        "y": 0,
    },
    "margin": {
        "x": 0, // pixels
        "y": 0
    },
    "size": {
        "x": 64, // in units. 1 unit = 1 tileSize
        "y": 64
    },
    "start": {
        "i": 0,
        "x": 0,
        "y": 0
    },
    "end": {
        "i": 0,
        "x": 0,
        "y": 0
    },    
    "source": [ // source defines the different tilesets used to build/     render the map. also the lowest level will be the default one unless specified otherwise by the layer
        "hash": { // md5 hash of the actual tileset
            "filename": "", // relative to the binary or absolute
            "size": {
                "x": 1000, // in pixels
                "y": 1000
            }
        }
    ],
    "layer": [ // in render other, [0] means the first one
        {
            "dynamicY": true, // if not present, it's false
            "source": "", // hash of the source to use
            "depth": 0, // should be aligned the position in the array, redundancy basically
            "type": "floor", // more like a comment or human-readable-categorization
            "cells":[
                {
                    "x": 0,
                    "y": 0,
                    "i": 0,
                    "v": 20
                }                
            ]            
        }
    ],
    "masks": [ // solids/collisions
        {
            "x": 0,
            "y": 1,
            "w": 32,
            "h": 32,
            "i": 0            
        }
    ],
    // TODO:
    // Basically, nav will be a precomputed tree for navegation used by the AI (and maybe other things)
    // it is not decided how this will work yet. we have yet to consider how to store and fetch map chunks
    "nav": [
        {
            "place":"holder"
        }
    ],
    "fragments": [
        {
            "source": "", // other map file (md5 + .json)
            "depth": 0, // it refers to the layer to be rendered at
            "pos": {
                "x": 0, // exact unit to place the fragment at
                "y": 0
            }
        }
    ]
}

TODO:
- Conversion from Tiled JSON maps to nite's on the fly (allowing for fragment-mapping)