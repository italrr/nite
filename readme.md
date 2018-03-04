# nite Engine

nite Engine is a 2D game engine written by Italo Russo. Originally as toy/learning project. 
Since the last year I've been using it to make a simply RPG Game. This repo contains
the engine itself + the game, both somewhat seperated from eacher (Engine + UI are nite).

nite is made using C++ and OpenGL. nite is supposed to be completely portable, but right
now it only works on Windows.

## Modules / Features

### Basic Font Rendering
![alt text](https://www.dropbox.com/s/l4ggzkdqzrcaaoe/8.png?dl=1)
nite uses FreeTyp2 internally to render fonts. It loads from ttf and
render them into textures.

### Physics Engine & Hitbox system
![alt text](https://www.dropbox.com/s/dk5h5nth4ut7a08/1.png?dl=1)
nite uses a very simple AAxBB collision and physics system. 
Hitboxes also work this way. 
![alt text](https://www.dropbox.com/s/fl8g5mophymwodo/3.png?dl=1)
In the image, orage rectangle are hitboxes either for receiving damage
or dealing it. 
Green rectangle represents the object's collision mask.

### Console
![alt text](https://www.dropbox.com/s/vnbzpws8vobdmcs/2.png?dl=1)
nite features an in-game command line to manipulate the world of the
game without having to recompile. It supports autocomplete, too.

### Basic AI and path finding system
![alt text](https://www.dropbox.com/s/npcmjvw1cgiti8f/4.png?dl=1)
nite includes a very basic AI and path finding system. It is still in
development. The idea is that entities could have multiple behaviours that
could work in synergy to make enemies actions organic and somewhat
impredictable.

### UI System
![alt text](https://www.dropbox.com/s/ecuuynmii6vhao1/5.png?dl=1)
nite also features an UI System. It has buttons, checkboxes,
labels, windows and panels. Input text boxes are coming soon.
![alt text](https://www.dropbox.com/s/ecuuynmii6vhao1/5.png?dl=1)
![alt text](https://www.dropbox.com/s/ifhzj2a6ihqz9my/6.png?dl=1)

Panels have a cohesive layout system, which uses the flex box concept 
to accomodate components.
![alt text](https://j.gifs.com/Q0XzB7.gif)
![alt text](https://www.dropbox.com/s/658yulrtiejkmhn/7.png?dl=1)

Also, UI is loaded from JSON files dynamically. Meaning the engine
constantly listens to changes on the file and reloads as soon as it
detects the file changed.