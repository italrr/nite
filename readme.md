nite Engine
----------

`nite` Engine is a 2D game engine in C++11. This has been an on-going project which the author started some years ago, slowly adding new features and mechanics.

About the Game
--------------
The game being developed with this engine is a precedurally generated co-op RPG. The end goal is to be able to create engaging RPG campaigns with high complexity and unpredictability, but fairness above both. We'll be experimenting with different maze generation algorithms to make this possible.

Another very important aspect of the game, and a big goal for `nite`, is the to be able to create challenging enemy AI. As of right now, AI is bare bones at best. But it's
something we'll be heavily focused on.

This game is inspired by another older project of the original author: [https://play.google.com/store/apps/details?id=com.nite.rushindungeons](https://play.google.com/store/apps/details?id=com.nite.rushindungeons)


### Features in `nite`
- Texture Rendering
- Basic batch type animation
- AAxBB physics simulation
- AAxBB Hitbox system
- Audio (Using RaptorAudio. Windows only for now)
- Font rendering (Embedded FreeType2) 
- Depth/Layered Rendering (Simulated Z-index)
- Render buffers
- Util types (`nite::Color`, `nite::Vec2`, `nite::Rect`, `nite::Polygon`)
- Basic primitive shape rendering
- Integrated in-game console
- Multi-layered and complex tileset system
- Input support (Keyboard and mouse for now)
- Object / world simulation (Networked simulation in progress!)
- Shader support
- Network support (UDP socket support, plus some abstraction for packets)
- Integrated basic tools (File reading, Hashing, Math, AsyncTask, etc)
- JSON support
- Camera support (Entity follow, shake-screen, tilt/rotations, Zoom-in and out)
- Entity system (Networked in progress!)
- Basic AI for entities
- UI System based on flex-boxes and sourcing from JSONs
- Basic scripting language (`nite Script`)

### In Progress

- UI
    - [!] Relative sizes (i.e. `"width": "100%"`)
    - [ ] Focus
    - [ ] Scrolling
    - [ ] Textboxes
    - [ ] Flow control (freeze windows to wait for other to finish)
- Client/Server
    - [ ] World synchronization
        - [ ] Animations
        - [ ] Physics
    - [ ] Control packets (add, remove, update skills, effects, inventory)
    - [ ] Data transfer (send maps, and other dynamic content)
- Stats/Entity
    - [ ] Inventory system
    - [ ] Skills
    - [ ] Effects
    - [ ] Add new class (Stalker/Assassin type)
- RING
    - [ ] Search algorithms
    - [ ] Implement new maze generation algorithms
    - [ ] Implement rooms
- AI
    - [ ] Implement search algorithms
    - [ ] Add optimizations for such algorithms
- nite Script (canvas)
    - [ ] TODO
- Misc
    - [ ] Visility shadow
    - [ ] Grooming, minor bugs fixing, API redesign
    - [ ] Improve audio to be OS agnostic (Most likely will be using another library)

How to build?
------------

Most dependencies are already included inside of the engine. **This might change in the future**.

### Requirements 
- CMake 2.8 or later
- SDL2 (2.0.1 or later)
- pthreads
- OpenGL 3.0 or later
- Little endian CPU (x86, x86_64, ARM, etc)
- At least 1GB RAM
- At least 128 MB VRAM

### Building

1. Prepare
> `cmake .`

2. Compile
> `make`

3. Run
> `./Game`