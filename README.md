A very simple and incomplete space shooter game based on the classic arcade game of the same name.

The reason for this project was to see if I could create a working game with a minimal amount of dependencies. So there is no engine, SDL, no glm. I use portaudio for the sound, and Vulkan for the visuals. Those two libraries and Linux OS running X11 should be all you need. The full list of linker flags are:

`-lvulkan -lxcb -lxcb-keysyms -lm -lportaudio`

You should be able to grab anything missing from your system package manager.

The game features a single space ship, a proceduarally generated asteroid field, a rudimentary space physics engine, and infinite ammo.

Once I got past the proof of concept stage my interest drifted to other project, but one day I would like to finish this one off.

Remaining things to do are:
    - handle collisions with asterioids / make space ship damageable
    - add a score / scoreboard

To build:
If you have the dependencies, you should be able to just run

```
git clone https://github.com/mogjira/asteroids
cd asteroids
make
./build/as
```
