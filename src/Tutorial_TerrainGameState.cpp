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

#include "Tutorial_TerrainGameState.h"
#include "CameraController.h"
#include "GraphicsSystem.h"
#include "SDL_scancode.h"
#include "Utils/MeshUtils.h"

#include "OgreSceneManager.h"

#include "OgreRoot.h"
#include "Vao/OgreVaoManager.h"
#include "Vao/OgreVertexArrayObject.h"

#include "OgreCamera.h"
#include "OgreWindow.h"
#include "OgreFrameStats.h"

#include "Terra/Hlms/OgreHlmsTerra.h"
#include "Terra/Hlms/PbsListener/OgreHlmsPbsTerraShadows.h"
#include "Terra/Terra.h"
#include "Terra/TerraShadowMapper.h"
#include "OgreHlmsManager.h"
#include "OgreHlms.h"
#include "Compositor/OgreCompositorManager2.h"
#include "Compositor/OgreCompositorWorkspace.h"

#include "OgreTextureGpuManager.h"

#include "OgreGpuProgramManager.h"
#include "OgreHlmsPbsDatablock.h"

#include "OgreItem.h"
#include "OgreLogManager.h"

#include "OgreMesh.h"
#include "OgreMeshManager.h"
#include "OgreMesh2.h"
#include "OgreMeshManager2.h"
#include "OgreManualObject2.h"

#include "OgreParticleSystem.h"

#ifdef OGRE_BUILD_COMPONENT_ATMOSPHERE
#    include "OgreAtmosphereNpr.h"
#endif


using namespace Demo;
using namespace Ogre;

#define LogO(s)  LogManager::getSingleton().logMessage(s)


namespace Demo
{
    Tutorial_TerrainGameState::Tutorial_TerrainGameState( const String &helpDescription ) :
        TutorialGameState( helpDescription ),
        mLockCameraToGround( false ),
        mPitch( Math::PI * 0.55f ),  // par
        mYaw( 0 ),
        mTerra( 0 ),
        mSunLight( 0 ),
        mHlmsPbsTerraShadows( 0 )
    {
        macroblockWire.mPolygonMode = PM_WIREFRAME;
        SetupTrees();
    }

    
    //  Create
    //-----------------------------------------------------------------------------------------------------------------------------
    void Tutorial_TerrainGameState::createScene01()
    {
        Root *root = mGraphicsSystem->getRoot();
        SceneManager *sceneManager = mGraphicsSystem->getSceneManager();
        
        HlmsManager *hlmsManager = root->getHlmsManager();
        HlmsDatablock *datablock = 0;
        
       
        SceneNode *rootNode = sceneManager->getRootSceneNode( SCENE_STATIC );

        LogManager::getSingleton().setLogDetail(LoggingLevel::LL_BOREME);

        LogO("---- createScene");

        RenderSystem *renderSystem = root->getRenderSystem();  //**
        renderSystem->setMetricsRecordingEnabled( true );


        LogO("---- new Terra");

        // Render terrain after most objects, to improve performance by taking advantage of early Z

    #if 1  //** 1 terrain
        mTerra = new Terra( Id::generateNewId<MovableObject>(),
                            &sceneManager->_getEntityMemoryManager( SCENE_STATIC ),
                            sceneManager, 11u, root->getCompositorManager2(),
                            mGraphicsSystem->getCamera(), false );
        mTerra->setCastShadows( false );

        LogO("---- Terra load");

        //  Heightmap  ------------------------------------------------
        //  64  flat
        //mTerra->load( "Heightmap64.png", Vector3( 64.0f, 4096.0f * 0.15f, 64.0f ), Vector3( 12096.0f, 6096.0f, 12096.0f ), false, false );
        //  1k  600 fps  4 tex
        mTerra->load( "Heightmap.png", Vector3( 64.0f, 4096.0f * 0.5f, 64.0f ), Vector3( 4096.0f, 4096.0f, 4096.0f ), false, false );
        // mTerra->load( "Heightmap.png", Vector3( 64.f, 512.f, 64.f ), Vector3( 1024.f, 1.f, 1024.f ), false, false );
        //  2k
        //mTerra->load( "Heightmap2c.png", Vector3( 64.0f, 4096.0f * 0.15f, 64.0f ), Vector3( 12096.0f, 6096.0f, 12096.0f ), false, false );
        //  4k
        //mTerra->load( "Heightmap4.png", Vector3( 64.0f, 4096.0f * 0.5f, 64.0f ), 2.f* Vector3( 4096.0f, 4096.0f, 4096.0f ), false, false );

        SceneNode *node = rootNode->createChildSceneNode( SCENE_STATIC );
        node->attachObject( mTerra );

        LogO("---- Terra attach");

        datablock = hlmsManager->getDatablock( "TerraExampleMaterial" );
        mTerra->setDatablock( datablock );

        {
            mHlmsPbsTerraShadows = new HlmsPbsTerraShadows();
            mHlmsPbsTerraShadows->setTerra( mTerra );
            //Set the PBS listener so regular objects also receive terrain shadows
            Hlms *hlmsPbs = root->getHlmsManager()->getHlms( HLMS_PBS );
            hlmsPbs->setListener( mHlmsPbsTerraShadows );
        }
    #else
        //  Plane  ------------------------------------------------
        v1::MeshPtr planeMeshV1 = v1::MeshManager::getSingleton().createPlane( "Plane v1",
            ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
            Plane( Vector3::UNIT_Y, 1.0f ), 2000.0f, 2000.0f,
            10, 10, true, 1, 40.0f, 40.0f, Vector3::UNIT_Z,
            v1::HardwareBuffer::HBU_STATIC, v1::HardwareBuffer::HBU_STATIC );

        MeshPtr planeMesh = MeshManager::getSingleton().createByImportingV1(
            "Plane", ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
            planeMeshV1.get(), true, true, true );
        {
            Item *item = sceneManager->createItem( planeMesh, SCENE_STATIC );
            item->setDatablock( "Ground" );
            SceneNode *node = rootNode->createChildSceneNode( SCENE_STATIC );
            node->setPosition( 0, 0, 0 );
            node->attachObject( item );

            //  Change the addressing mode to wrap
            /*assert( dynamic_cast<HlmsPbsDatablock*>( item->getSubItem(0)->getDatablock() ) );
            HlmsPbsDatablock *datablock = static_cast<HlmsPbsDatablock*>(item->getSubItem(0)->getDatablock() );
            HlmsSamplerblock samplerblock( *datablock->getSamplerblock( PBSM_DIFFUSE ) );  // hard copy
            samplerblock.mU = TAM_WRAP;
            samplerblock.mV = TAM_WRAP;
            samplerblock.mW = TAM_WRAP;
            datablock->setSamplerblock( PBSM_DIFFUSE, samplerblock );
            datablock->setSamplerblock( PBSM_NORMAL, samplerblock );
            datablock->setSamplerblock( PBSM_ROUGHNESS, samplerblock );
            datablock->setSamplerblock( PBSM_METALLIC, samplerblock );/**/
        }
    #endif

        LogO("---- new light");

        //  Light  ------------------------------------------------
        mSunLight = sceneManager->createLight();
        SceneNode *lightNode = rootNode->createChildSceneNode();
        lightNode->attachObject( mSunLight );
        
        mSunLight->setPowerScale( Math::PI * 3 );  //** par! 1.5 2 3* 4  should be * 1..
        mSunLight->setType( Light::LT_DIRECTIONAL );
        mSunLight->setDirection( Vector3( 0, -1, 0 ).normalisedCopy() );  //-

        //  ambient  set in update ..
        sceneManager->setAmbientLight(
            // ColourValue( 0.63f, 0.61f, 0.28f ) * 0.04f,
            // ColourValue( 0.52f, 0.63f, 0.76f ) * 0.04f,
            ColourValue( 0.33f, 0.61f, 0.98f ) * 0.01f,
            ColourValue( 0.02f, 0.53f, 0.96f ) * 0.01f,
            Vector3::UNIT_Y );

#ifdef OGRE_BUILD_COMPONENT_ATMOSPHERE
        mGraphicsSystem->createAtmosphere( mSunLight );
        OGRE_ASSERT_HIGH( dynamic_cast<AtmosphereNpr *>( sceneManager->getAtmosphere() ) );
        AtmosphereNpr *atmosphere = static_cast<AtmosphereNpr *>( sceneManager->getAtmosphere() );
        AtmosphereNpr::Preset p = atmosphere->getPreset();
        p.fogDensity = 0.0002f;  //** par
        p.densityCoeff = 0.27f;
        p.densityDiffusion = 0.75f;
        // p.densityCoeff = 0.47f;
        // p.densityDiffusion = 2.0f;
        p.horizonLimit = 0.025f;
        // p.sunPower = 1.0f;
        // p.skyPower = 1.0f;
        p.skyColour = Vector3(0.334f, 0.57f, 1.0f);
        p.fogBreakMinBrightness = 0.25f;
        p.fogBreakFalloff = 0.1f;
        // p.linkedLightPower = Math::PI;
        // p.linkedSceneAmbientUpperPower = 0.1f * Math::PI;
        // p.linkedSceneAmbientLowerPower = 0.01f * Math::PI;
        p.envmapScale = 1.0f;
        atmosphere->setPreset( p );
#endif

        //  camera  ------------------------------------------------
        mCameraController = new CameraController( mGraphicsSystem, false );
        mGraphicsSystem->getCamera()->setFarClipDistance( 100000.f );  // far

        //camPos = Vector3(-10.f, 80.f, 10.f );
        //camPos = Vector3(-2005.f, 40.f, -929.f);
        camPos = Vector3(-52.f, mTerra ? 735.f : 60.f, mTerra ? 975.f : 517.f);
        //camPos.y += mTerra->getHeightAt( camPos );
        mGraphicsSystem->getCamera()->setPosition( camPos );
        mGraphicsSystem->getCamera()->lookAt( camPos + Vector3(0.f, -0.5f, -1.f) );
        Vector3 objPos;


    #if 0  //  Sky Dome
        CreateSkyDome("sky-clearday1", 0.f);
        //CreateSkyDome("sky_photo6", 0.f);  // clouds
    #endif

        LogO("---- tutorial createScene");

        TutorialGameState::createScene01();
    }


    //  Destroy
    //-----------------------------------------------------------------------------------
    void Tutorial_TerrainGameState::destroyScene()
    {
        LogO("---- destroyScene");
        Root *root = mGraphicsSystem->getRoot();
        Hlms *hlmsPbs = root->getHlmsManager()->getHlms( HLMS_PBS );

        //Unset the PBS listener and destroy it
        if( hlmsPbs->getListener() == mHlmsPbsTerraShadows )
        {
            hlmsPbs->setListener( 0 );
            delete mHlmsPbsTerraShadows;
            mHlmsPbsTerraShadows = 0;
        }

        delete mTerra;
        mTerra = 0;

        LogO("---- tutorial destroyScene");

        TutorialGameState::destroyScene();
    }


    //  Update  frame
    //-----------------------------------------------------------------------------------------------------------------------------
    void Tutorial_TerrainGameState::update( float timeSinceLast )
    {
        //  Keys
        float mul = shift ? 0.2f : ctrl ? 3.f : 1.f;
        int d = right ? 1 : left ? -1 : 0;
        if (d)
        {
            SceneManager *sceneManager = mGraphicsSystem->getSceneManager();
            AtmosphereNpr *atmosphere = static_cast<AtmosphereNpr*>( sceneManager->getAtmosphere() );
            AtmosphereNpr::Preset p = atmosphere->getPreset();
            float mul1 = 1.f + 0.003f * mul * d;
            switch (param)
            {
            case 0:  p.fogDensity *= mul1;  break;
            case 1:  p.densityCoeff *= mul1;  break;
            case 2:  p.densityDiffusion *= mul1;  break;
            case 3:  p.horizonLimit *= mul1;  break;
            case 4:  p.sunPower *= mul1;  break;
            case 5:  p.skyPower *= mul1;  break;
            case 6:  p.skyColour.x *= mul1;  break;
            case 7:  p.skyColour.y *= mul1;  break;
            case 8:  p.skyColour.z *= mul1;  break;
            case 9:   p.fogBreakMinBrightness *= mul1;  break;
            case 10:  p.fogBreakFalloff *= mul1;  break;
            case 11:  p.linkedLightPower *= mul1;  break;
            case 12:  p.linkedSceneAmbientUpperPower *= mul1;  break;
            case 13:  p.linkedSceneAmbientLowerPower *= mul1;  break;
            case 14:  p.envmapScale *= mul1;  break;
            }
            atmosphere->setPreset(p);
        }

        d = mKeys[0] - mKeys[1];
        if (d)
        {
            mPitch += d * mul * 0.6f * timeSinceLast;
            mPitch = std::max( 0.f, std::min( mPitch, (float)Math::PI ) );
        }

        d = mKeys[2] - mKeys[3];
        if (d)
        {
            mYaw += d * mul * 1.5f * timeSinceLast;
            mYaw = fmodf( mYaw, Math::TWO_PI );
            if( mYaw < 0.f )
                mYaw = Math::TWO_PI + mYaw;
        }
        //  Light  sun dir  ----
        Vector3 dir = Quaternion( Radian(mYaw), Vector3::UNIT_Y ) *
            Vector3( cosf( mPitch ), -sinf( mPitch ), 0.0 ).normalisedCopy();
        mSunLight->setDirection( dir );


        SceneManager *sceneManager = mGraphicsSystem->getSceneManager();
        sceneManager->setAmbientLight(
            ColourValue( 0.99f, 0.94f, 0.90f ) * 0.04f,  //** par
            ColourValue( 0.90f, 0.93f, 0.96f ) * 0.04f,
            // ColourValue( 0.93f, 0.91f, 0.38f ) * 0.04f,
            // ColourValue( 0.22f, 0.53f, 0.96f ) * 0.04f,
            // ColourValue( 0.33f, 0.61f, 0.98f ) * 0.01f,
            // ColourValue( 0.02f, 0.53f, 0.96f ) * 0.01f,
            -dir );

    #ifdef OGRE_BUILD_COMPONENT_ATMOSPHERE
        OGRE_ASSERT_HIGH( dynamic_cast<AtmosphereNpr *>( sceneManager->getAtmosphere() ) );
        AtmosphereNpr *atmosphere = static_cast<AtmosphereNpr *>( sceneManager->getAtmosphere() );
        atmosphere->setSunDir( mSunLight->getDirection(), mPitch / Math::PI );
    #endif

        ///  Terrain  ----
        if (mTerra && mGraphicsSystem->getRenderWindow()->isVisible() )
        {
            // Force update the shadow map every frame to avoid the feeling we're "cheating" the
            // user in this sample with higher framerates than what he may encounter in many of
            // his possible uses.
            const float lightEpsilon = 0.0001f;  //** 0.0f slow
            mTerra->update( mSunLight->getDerivedDirectionUpdated(), lightEpsilon );
        }

        TutorialGameState::update( timeSinceLast );

        //  Camera must be locked to ground after we've moved it.
        //  Otherwise fast motion may go below the terrain for 1 or 2 frames.
        if( mLockCameraToGround && mTerra )
        {
            mTerra->getHeightAt( camPos );
            Camera *camera = mGraphicsSystem->getCamera();
            Vector3 camPos = camera->getPosition();
            camera->setPosition( camPos + Vector3::UNIT_Y * 10.0f );
        }
    }


    //  text
    //-----------------------------------------------------------------------------------------------------------------------------
    void Tutorial_TerrainGameState::generateDebugText( float timeSinceLast, String &outText )
    {
        //TutorialGameState::generateDebugText( timeSinceLast, outText );
        #define toStr(v, p)  StringConverter::toString(v,p)
        //auto toStr = [](auto v, auto p=1) {  return StringConverter::toString(v,p);  };
        outText = "";

        if( mDisplayHelpMode == 0 )
        {
            //  F1 still help
            outText += "\nCtrl+F4 will reload Terra's shaders.";
            
            outText += "\nF2 Lock Ground: ";
            outText += mLockCameraToGround ? "Yes" : "No";

            Vector3 camPos = mGraphicsSystem->getCamera()->getPosition();
            outText += "\n\nPos: " + toStr( camPos.x, 1) +" "+ toStr( camPos.y, 1) +" "+ toStr( camPos.z, 1);
        }
        else if( mDisplayHelpMode == 1 )
        {
            //  fps stats
            RenderSystem *rs = mGraphicsSystem->getRoot()->getRenderSystem();
            const RenderingMetrics& rm = rs->getMetrics();  //**
            const FrameStats *st = mGraphicsSystem->getRoot()->getFrameStats();
            
            outText += toStr( (int)st->getAvgFps(), 0) +"  "+ //"\n" +
                "f " + toStr( rm.mFaceCount/1000, 0) + //"k v " + toStr( rm.mVertexCount/1000 ) + 
                "k d " + toStr( rm.mDrawCount, 0) + " i " + toStr( rm.mInstanceCount, 0) + 
                " b " + toStr( rm.mBatchCount, 0) + "\n";

            outText += "Veget: " + toStr(vegetNodes.size(), 5);
            outText += "\n+ - sun Pitch " + toStr( mPitch * 180.f / Math::PI, 3 );
            outText += "\n/ * sun Yaw   " + toStr( mYaw * 180.f / Math::PI, 3 );

            outText += "\nParam: " + toStr( param, 0 );
            
            SceneManager *sceneManager = mGraphicsSystem->getSceneManager();
            AtmosphereNpr *atmosphere = static_cast<AtmosphereNpr*>( sceneManager->getAtmosphere() );
            AtmosphereNpr::Preset p = atmosphere->getPreset();
            
            outText += "\n";
            switch (param)
            {
            case 0:   outText += "Fog density: " + toStr( p.fogDensity, 5 );  break;
            case 1:   outText += "density coeff: " + toStr( p.densityCoeff, 5 );  break;
            case 2:   outText += "density diffusion: " + toStr( p.densityDiffusion, 5 );  break;
            case 3:   outText += "horizon limit: " + toStr( p.horizonLimit, 5 );  break;
            case 4:   outText += "Sun Power: " + toStr( p.sunPower, 5 );  break;
            case 5:   outText += "sky Power: " + toStr( p.skyPower, 5 );  break;
            case 6:   outText += "sky Colour R: " + toStr( p.skyColour.x, 5 );  break;
            case 7:   outText += "sky Colour G: " + toStr( p.skyColour.y, 5 );  break;
            case 8:   outText += "sky Colour B: " + toStr( p.skyColour.z, 5 );  break;
            case 9:   outText += "fog break MinBright: " + toStr( p.fogBreakMinBrightness, 5 );  break;
            case 10:  outText += "fog break Falloff: " + toStr( p.fogBreakFalloff, 5 );  break;
            case 11:  outText += "linked LightPower: " + toStr( p.linkedLightPower, 5 );  break;
            case 12:  outText += "ambient UpperPower: " + toStr( p.linkedSceneAmbientUpperPower, 5 );  break;
            case 13:  outText += "ambient LowerPower: " + toStr( p.linkedSceneAmbientLowerPower, 5 );  break;
            case 14:  outText += "envmap Scale: " + toStr( p.envmapScale, 5 );  break;
            }
            
            //**  veget cnt
            #if 0  // list all cnts
            for (const auto& lay : vegetLayers)
                outText += StringConverter::toString( lay.count ) + " " + lay.mesh + "\n";
            #endif
        }
    }


    //  Key events
    //-----------------------------------------------------------------------------------------------------------------------------
    void Tutorial_TerrainGameState::keyPressed( const SDL_KeyboardEvent &arg )
    {
        switch (arg.keysym.scancode)
        {
        case SDL_SCANCODE_LSHIFT:  shift = true;  break;
        case SDL_SCANCODE_LCTRL:   ctrl = true;   break;

        case SDL_SCANCODE_LEFT:   left = true;   break;
        case SDL_SCANCODE_RIGHT:  right = true;  break;
        case SDL_SCANCODE_UP:     --param;  break;
        case SDL_SCANCODE_DOWN:   ++param;  break;

        case SDL_SCANCODE_KP_PLUS:      mKeys[0] = 1;  break;
        case SDL_SCANCODE_KP_MINUS:     mKeys[1] = 1;  break;
        case SDL_SCANCODE_KP_MULTIPLY:  mKeys[2] = 1;  break;
        case SDL_SCANCODE_KP_DIVIDE:    mKeys[3] = 1;  break;
        
        //** terrain wireframe toggle
        case SDL_SCANCODE_R:
        {
            wireTerrain = !wireTerrain;
            Root *root = mGraphicsSystem->getRoot();
            HlmsManager *hlmsManager = root->getHlmsManager();
            HlmsDatablock *datablock = 0;
            datablock = hlmsManager->getDatablock( "TerraExampleMaterial" );
            if (wireTerrain)
            {
                datablock = hlmsManager->getHlms( HLMS_USER3 )->getDefaultDatablock();
                datablock->setMacroblock( macroblockWire );
            }
            mTerra->setDatablock( datablock );
        }   break;

        //  Trees add, destroy all
        case SDL_SCANCODE_V:  CreateTrees();  break;
        case SDL_SCANCODE_C:  DestroyTrees();  break;

        //  other
        case SDL_SCANCODE_F:  CreateParticles();  break;
        case SDL_SCANCODE_G:  CreateCar();  break;
        
        case SDL_SCANCODE_M:
        {
            Vector3 camPos(-52.f, mTerra ? 735.f : 60.f, mTerra ? 975.f : 517.f);
            CreateManualObj(camPos);
        }   break;
        }
        
        TutorialGameState::keyPressed( arg );
    }
    
    void Tutorial_TerrainGameState::keyReleased( const SDL_KeyboardEvent &arg )
    {
        switch (arg.keysym.scancode)
        {
        case SDL_SCANCODE_LSHIFT:  shift = false;  break;
        case SDL_SCANCODE_LCTRL:   ctrl = false;   break;

        case SDL_SCANCODE_LEFT:   left = false;   break;
        case SDL_SCANCODE_RIGHT:  right = false;  break;

        case SDL_SCANCODE_KP_PLUS:      mKeys[0] = 0;  break;
        case SDL_SCANCODE_KP_MINUS:     mKeys[1] = 0;  break;
        case SDL_SCANCODE_KP_MULTIPLY:  mKeys[2] = 0;  break;
        case SDL_SCANCODE_KP_DIVIDE:    mKeys[3] = 0;  break;


        case SDL_SCANCODE_F4:
        if (arg.keysym.mod & (KMOD_LCTRL|KMOD_RCTRL))
        {
            //Hot reload of Terra shaders.
            Root *root = mGraphicsSystem->getRoot();
            HlmsManager *hlmsManager = root->getHlmsManager();

            Hlms *hlms = hlmsManager->getHlms( HLMS_USER3 );
            GpuProgramManager::getSingleton().clearMicrocodeCache();
            hlms->reloadFrom( hlms->getDataFolder() );
        }   break;

        case SDL_SCANCODE_F2:
            mLockCameraToGround = !mLockCameraToGround;
            break;
        }

        TutorialGameState::keyReleased( arg );
    }
}
