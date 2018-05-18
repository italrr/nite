nite
---

nite is a portable 2D RPG/Topdown game engine written in C++. Originally, it was a toy/learning project.

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
- Multi-layered tileset system
- Input support (Keyboard and mouse for now)
- Object / world simulation (Non-Networked)
- Shader support
- Network support
- Integrated basic tools (File reading, Hashing, Math, AsyncTask, etc)
- JSON support
- Camera support (Entity follow, shake-screen, tilt/rotations, Zoom-in and out)
- Entity system
- Basic AI for entities
- UI System based on flex-boxes

How to build?
------------

### Requirements 

- SDL2 (2.0.1 or later)
- pthread
- CMake 2.8 or later

### Building

1. Run
> cmake .

2. Done
