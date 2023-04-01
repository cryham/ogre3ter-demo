## Screenshot

![](https://github.com/cryham/ogre3ter-demo/blob/main/screen1.jpg?raw=true)


## Description

Ogre-Next demo. Based on Tutorial_Terrain.
Showing:
* **Terrain** with 4 layers
* **Vegetation**, 9 layers: tree, palm, 4 rocks, 2 ferns, plant
* **Atmosphere** component with adjustable parameters
* **Car** 3 models, with dynamic reflections, **particles**

No real PBR textures yet just values.  
Specular light is somewhat broken, too much still.  
Models and textures from [StuntRally](https://github.com/stuntrally/stuntrally), details in: [_Licenses.txt](https://github.com/cryham/ogre3ter-demo/blob/main/Media/models/_Licenses.txt), [_terrain.txt](https://github.com/cryham/ogre3ter-demo/blob/main/Media/2.0/scripts/materials/Tutorial_Terrain/_terrain.txt) and [_sky_readme.txt](https://github.com/cryham/ogre3ter-demo/blob/main/Media/textures/_sky_readme.txt).

----
## Keys

F1 - Toggle Help text  

W,S, A,D, Q,E - move camera: forward,backward, left,right, down,up

Hold: Shift - for smaller chages, Ctrl - for bigger chages  (for all)

V - **Add** vegetation (can be used more times)  
C - Clear all vegetation  

G - Add car in front, next model.  
F - Add fire with smoke particles  

T - toggle Terrain / flat plane. Note: terrain loads in 5 sec.  
R - terrain wireframe toggle  
K - Add / remove SkyDome, next texture.  

Up,Down - previous / next **parameter**  
Left,Right - dec / increase parameter  

Numpad /,* - rotate **sun** yaw  
Numpad +,- - change sun pitch (time of day)  

To reload *shaders* (after editing):  
Ctrl-F1 - PBS,  Ctrl-F2 - Unlit,  Ctrl-F3 - Compute,  Ctrl-F4 - Terrain  
Ctrl-F5 - Force device reelection  

## Info on screen

Top line meaning:

**Fps** (frames per second), **f** face (triangles) count [k = 1000], **d** draw count, **i** instances  
Veget - total vegetation models on scene

----
## Building from sources

See [CMakeLists.txt](/CMakeLists.txt) for details, and adjust fixes if needed.

Only tested on Linux, Debian 11.  
Guide below has setup steps for empty Debian 11:

1. Basic setup for building C++ etc:  
`sudo apt-get install g++ binutils gdb git make cmake ninja-build`

2. First install Ogre dependencies, as in [here](https://github.com/OGRECave/ogre-next#dependencies-linux)  
`sudo apt-get install libfreetype6-dev libfreeimage-dev libzzip-dev libxrandr-dev libxcb-randr0-dev libxaw7-dev freeglut3-dev libgl1-mesa-dev libglu1-mesa-dev libx11-xcb-dev libxcb-keysyms1-dev doxygen graphviz python-clang libsdl2-dev`

3. Build **Ogre-Next** from sources, using [scripts](https://github.com/OGRECave/ogre-next/tree/master/Scripts/BuildScripts/output).  

- Save the file [build_ogre_linux_c++latest.sh](https://raw.githubusercontent.com/OGRECave/ogre-next/master/Scripts/BuildScripts/output/build_ogre_linux_c%2B%2Blatest.sh) and put it inside our root folder, called here e.g. `dev/`

- Go into `dev/` and start it:  
```
cd dev/
chmod +x ./build_ogre_linux_c++latest.sh
./build_ogre_linux_c++latest.sh
```

- This should succeed after a longer while and build Ogre-Next with its dependencies.

- If so you can start and check Ogre demos and samples inside:  
`dev/Ogre/ogre-next/build/Release/bin/`

4. Go into `dev/`, clone **this repo**, and build:  
```
git clone https://github.com/cryham/ogre3ter-demo.git demo
cd demo
mkdir build
cd build
cmake ../ -DCMAKE_BUILD_TYPE="Release"
make -j5
(or: ninja - if not using makefile)
```

----
## My folder tree
```
dev/
    demo - this repo inside
    Ogre
        ogre-next - Ogre-Next build inside
        ogre-next-deps
```

Note: CMake default is `Dependencies/Ogre` inside demo. On Windows, it can be as link, if this doesn't work.
