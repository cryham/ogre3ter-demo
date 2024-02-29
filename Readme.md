## Screenshots

![](https://github.com/cryham/ogre3ter-demo/blob/main/screens/2.jpg?raw=true)

![](https://github.com/cryham/ogre3ter-demo/blob/main/screens/1.jpg?raw=true)


## Description

Ogre-Next demo. Based on Tutorial_Terrain.
Showing:
* **Terrain** with 4 layers and triplanar
* **Water** flat, no animation, with reflection or refraction and depth color shading
* **Vegetation**, 9 layers: tree, palm, 4 rocks, 2 ferns, plant
* **Atmosphere** component with fog and adjustable parameters
* **Car** 3 models, with dynamic reflections, **particles**

No real PBR textures yet just values.  
Models and textures from [StuntRally](https://github.com/stuntrally/stuntrally), details in: [_Licenses.txt](https://github.com/cryham/ogre3ter-demo/blob/main/Media/models/_Licenses.txt), [_terrain.txt](https://github.com/cryham/ogre3ter-demo/blob/main/Media/2.0/scripts/materials/Tutorial_Terrain/_terrain.txt) and [_sky_readme.txt](https://github.com/cryham/ogre3ter-demo/blob/main/Media/textures/_sky_readme.txt).


----
## Keys

F1 - Toggle Help text  

W,S, A,D, Q,E - move camera: forward,backward, left,right, down,up

Hold:  
Shift - for smaller chages,  
Ctrl - for bigger chages  (for all)

V - **Add** vegetation (can be used more times)  
C - remove all vegetation  

G - Add **Car** in front, next model  
H - remove all cars

T - toggle **Terrain** / flat plane. _Note:_ terrain loads in 5 sec.  
P - terrain triplanar toggle  
R - terrain wireframe toggle  

N - Add water, at set height (param -1)  
M - remove water  

K - Add / remove **Sky** dome, next texture  
F - Add fire with smoke particles  

Up,Down - previous / next **parameter** e.g. Fog (at param 0)  
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

### Linux

See [CMakeLists.txt](/CMakeLists.txt) for details, and adjust fixes if needed.

Guide below has setup steps for empty **Debian** 11 or 12 (works also on Debian based, like Ubuntu):

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

#### My folder tree
```
dev/
    demo - this repo inside
    Ogre
        ogre-next - Ogre-Next build inside
        ogre-next-deps
```

----
### Windows

Tested on Windows 10.

Ogre-Next approach is different. It needs a `Dependencies` subdir inside this project.  
So create a Dependencies dir after pulling this repo.
Then you can just link to Ogre dir. So: create a symbolic link to Ogre dir, and put inside Dependencies.

Run CMake-Gui, pick this project path for sources, and the same with subir `build\` for build output.  
Press `Configure``.  
There can be an error that I just fixed by copying that:  
`OgreBuildSettings.h` (found some file inside Ogre, for Release build if many) into:
`Ogre\build\ogre-next\build\Release\include\`

After that `Configure` has succeeded for me.  
Pressing `Generate`` creates `sln` file inside `demo\build\`.  
Open it with Visual Studio (same version as chosen in CMake).  
Try building.  
For me it failed few times, and I fixed it by adding in VS paths that were wrong or missing.  
(this isn't the best way, as you can't press Generate in CMake anymore, it would loose any editing in VS).

Open properties for TerrainDemo project.  
fopr /Wswitch

#### My folder tree
```
dev/
    demo - this repo inside
    Dependencies
        Ogre  - this can be a symbolic link
            ogre-next - Ogre-Next build inside
                build
                    Release
                        include
                            OgreBuildSettings.h
            ogre-next-deps
```

----
## Sources

Some key locations in code are marked with `//**`

Materials are in:  
`/Media/materials/ground.material.json`  
`/Media/2.0/scripts/materials/Tutorial_Terrain/Terra.material.json`  

and in (simpler syntax):  
`/Media/materials/vegetation.material`  
`/Media/materials/cars.material`  

Used compositor workspaces are defined in:  
`/Media/2.0/scripts/Compositors/Tutorial_Terrain.compositor`


## Docs

Quick links for Ogre-Next documentation:  
- [Manual](https://ogrecave.github.io/ogre-next/api/latest/manual.html) - need to read it when beginning.
- [Compositor](https://ogrecave.github.io/ogre-next/api/latest/compositor.html) - for effects, RTTs, reflections, etc.
- [HLMS doc](https://ogrecave.github.io/ogre-next/api/latest/hlms.html) - long read, explains all.
- [HLMS shaders](https://ogrecave.github.io/ogre-next/api/latest/hlms.html#HlmsCreationOfShaders).
- [Terrain](https://ogrecave.github.io/ogre-next/api/latest/_terra_system.html)
