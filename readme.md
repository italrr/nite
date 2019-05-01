nite Engine
---

nite Engine is a 2D game engine written by Italo Russo. This has been and on-going project of mine which I started some years ago, slowly adding features and
mechanics. The engine is intended for RPG-top down games.

nite is written in C++ and using OpenGL to render. nite is portable, although right now it only works well on Windows.

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
- Network support
- Integrated basic tools (File reading, Hashing, Math, AsyncTask, etc)
- JSON support
- Camera support (Entity follow, shake-screen, tilt/rotations, Zoom-in and out)
- Entity system
- Basic AI for entities
- UI System based on flex-boxes
- Basic scripting language (nite Script)


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

### Screenshots

- Basic Font Rendering
![alt text](http://198.199.91.93/imgs/8.png)
nite uses FreeType2 internally to render fonts. It loads them from a ttf and
render them into textures.

- Physics Engine & Hitbox system
![alt text](http://198.199.91.93/imgs/1.png)
nite uses a very simple AAxBB collision and physics system. 
Hitboxes also work this way. 
![alt text](http://198.199.91.93/imgs/3.png)
In the image, orange rectangle are hitboxes either for receiving damage
or dealing it. 
Green rectangle represents the object's collision mask.

- Console
![alt text](http://198.199.91.93/imgs/2.png)
nite features an in-game command line to manipulate the world of the
game without having to recompile. It supports autocomplete, too.

- Basic AI and path finding system
![alt text](http://198.199.91.93/imgs/4.png)
nite includes a very basic AI and path finding system. It is still in
development. The idea is that entities could have multiple behaviours that
could work in synergy to make enemies actions organic and somewhat
unpredectible.

- UI System
![alt text](http://198.199.91.93/imgs/5.png)
nite also features an UI System. It has buttons, checkboxes,
labels, windows and panels. Input text boxes are coming soon.
![alt text](http://198.199.91.93/imgs/5.png)
![alt text](http://198.199.91.93/imgs/6.png)

- Panels have a cohesive layout system, which uses the flex box concept to accomodate components.
![alt text](https://j.gifs.com/Q0XzB7.gif)
![alt text](http://198.199.91.93/imgs/7.png)
Also, UI is loaded from JSON files dynamically. Meaning the engine
constantly listens to changes on the file and reloads as soon as it
detects the file changed.