## Screenshot

![](https://github.com/cryham/ogre3ter-demo/blob/main/screen1.jpg?raw=true)


## Description

Ogre-Next demo. Based on Tutorial_Terrain.
Showing:
* **Terrain** with 4 layers
* **Vegetation**, few layers: tree, palm, ferns, rocks
* **Atmosphere** component with adjustable parameters
* **Car** models, **particles**

No reflection or real PBR textures yet.  
Specular light is somewhat broken, too much still.  
Models and textures from [StuntRally](https://github.com/stuntrally/stuntrally), see _Licenses.txt  

## Keys

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

F1 - Toggle Help text  

To reload *shaders* (after editing):  
Ctrl-F1 - PBS,  Ctrl-F2 - Unlit,  Ctrl-F3 - Compute,  Ctrl-F4 - Terrain  
Ctrl-F5 - Force device reelection  

## Info on screen

Top line meaning:

**Fps** (frames per second), **f** face (triangles) count [k = 1000], **d** draw count, **i** instances  
Veget - total vegetation models on scece
