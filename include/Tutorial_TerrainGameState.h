/*
-----------------------------------------------------------------------------
This source file is part of OGRE-Next
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2021 Torus Knot Software Ltd

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
-----------------------------------------------------------------------------
*/

#ifndef _Demo_Tutorial_TerrainGameState_H_
#define _Demo_Tutorial_TerrainGameState_H_

#include "OgreHlmsDatablock.h"
#include "OgrePrerequisites.h"
#include "TutorialGameState.h"

#define LogO(s)  LogManager::getSingleton().logMessage(s)


namespace Ogre
{
    class Terra;
    class HlmsPbsTerraShadows;
}

namespace Demo
{

    struct VegetLayer
    {
        std::string mesh;
        float scaleMin, scaleMax, density;
        float visFar, down;
        bool rotAll;
        int count;  // auto
        // range ter angle, height..
        
        VegetLayer(std::string mesh1, float scMin, float scMax,
                float dens, float dn, float vis, bool rot)
            : mesh(mesh1), scaleMin(scMin), scaleMax(scMax)
            , density(dens), visFar(vis), down(dn), rotAll(rot), count(0)
        {   }
    };


    class Tutorial_TerrainGameState : public TutorialGameState
    {
        float mPitch;  // sun dir
        float mYaw;
        Ogre::Vector3 camPos;

        int mKeys[4] = {0,0,0,0};  // sun keys
        int param = 0;  // to adjust
        bool left = false, right = false;  // arrows
        bool shift = false, ctrl = false;

        Ogre::Terra *mTerra = 0;  // terrain
        Ogre::Light *mSunLight = 0;

        // Listener to make PBS objects also be affected by terrain's shadows
        Ogre::HlmsPbsTerraShadows *mHlmsPbsTerraShadows = 0;

        //  wireframe
        Ogre::HlmsMacroblock macroblockWire;
        bool wireTerrain = false;

        void generateDebugText( float timeSinceLast, Ogre::String &outText ) override;

    public:
        Tutorial_TerrainGameState( const Ogre::String &helpDescription );

        //  main
        void createScene01() override;
        void destroyScene() override;

        void update( float timeSinceLast ) override;

        //  events
        void keyPressed( const SDL_KeyboardEvent &arg ) override;
        void keyReleased( const SDL_KeyboardEvent &arg ) override;


        //  scene  ----
        Ogre::Real sizeXZ = 1000.f;
        void CreateTerrain(), DestroyTerrain();
        Ogre::SceneNode *nodeTerrain = 0;

        void CreatePlane(), DestroyPlane();
        Ogre::MeshPtr planeMesh = 0;
        Ogre::Item *planeItem = 0;
        Ogre::SceneNode *planeNode = 0;

        void CreateSkyDome(Ogre::String sMater, float yaw);
        int iSky = 0;
        Ogre::ManualObject* moSky = 0;
        Ogre::SceneNode* ndSky = 0;
        void DestroySkyDome();
    
        void SetupTrees(), CreateTrees(), DestroyTrees();

        std::vector<VegetLayer> vegetLayers;
		std::vector<Ogre::Item*> vegetItems;
    	std::vector<Ogre::SceneNode*> vegetNodes;

        void CreateManualObj(Ogre::Vector3 camPos);
        void CreateParticles();

        void CreateCar();
        int iCar = 1;  const int nCars = 3;
    };
}

#endif
