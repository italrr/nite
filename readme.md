nite Engine
----------

nite Engine is a 2D game engine purely in C++. This has been an on-going project which I started some years ago, slowly adding new features and
mechanics. Originally as a toy & learning project. One key feature of nite is making most of the engine completely customizable through JSON files and using its own scripting system (nite Script).

About the Game
--------------
The game being developed with this engine is an RPG heavily oriented to the Standard RPG System. The core feature will be the use of RNG for generating the campaigns from
the very beginning to the end. The end goal is to be able to create engaging RPG campaigns with high complexity and unpredictability, but fairness above both. We'll be experimenting with different
maze generation algorithms to make this possible. Might consider using Neural Networks.

Another very important aspect of the game, and a big goal for nite, is the to be able to create challenging enemy AI. As of right now, AI is bare bones at best. But it's
something we'll be heavily focused on, too.

This game is inspired by another older project of the original creator: [https://play.google.com/store/apps/details?id=com.nite.rushindungeons](https://play.google.com/store/apps/details?id=com.nite.rushindungeons)


**Attention: Documentation is work in progress.**

### Features
- Texture Rendering (Image loading using SOIL)
- Basic Batch Animation
- AAxBB physics simulation
- AAxBB Hitbox system
- Audio (Using RaptorAudio. Windows only for now)
- Font rendering (Using FreeType2) 
- Depth/Layered Rendering (Simulated Z-index)
- Render buffers
- Util types (nite::Color, nite::Vec2, nite::Rect, nite::Polygon)
- Basic primitive shape rendering
- Integrated console
- Multi-layered and complex tileset system
- Input support (Keyboard and mouse for now)
- Object / world simulation (Non-Networked)
- Shader support
- Network support (Game logic is not networked as of now)
- Integrated basic tools (File reading, Hashing, Math, AsyncTask, etc)
- JSON support
- Camera support (Entity follow, shake-screen, tilt/rotations, Zoom-in and out)
- Entity system
- Basic AI for entities
- UI System based on flex-boxes
- Basic scripting language (nite Script)

### Coming Soon
- General clean up and more bug fixing
- RING (A combination of maze generation algorithms and other technologies to create a highly complex and engaging campaign)
- Better AI templates for enemies ('Hunting', 'Gather', and 'Wandering' personalities will be the firsts)
- More features for the UI System (Text boxes, Scrolling, Focus)
- Proper Audio Support


How to build?
------------

### Requirements 
- CMake 2.8 or later
- SDL2 (2.0.1 or later)
- pthread

### Building

1. Prepare
> `cmake .`

2. Compile
> `make`

2. Run
> `./Game`