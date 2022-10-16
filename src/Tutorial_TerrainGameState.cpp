#include "Tutorial_TerrainGameState.h"
#include "CameraController.h"
#include "GraphicsSystem.h"
#include "SDL_scancode.h"
#include "Utils/MeshUtils.h"

#include "OgreSceneManager.h"
#include "OgreRoot.h"
// #include "Vao/OgreVaoManager.h"
// #include "Vao/OgreVertexArrayObject.h"

#include "OgreCamera.h"
#include "OgreWindow.h"
#include "OgreFrameStats.h"

// #include "Terra/Hlms/OgreHlmsTerra.h"
// #include "Terra/Hlms/PbsListener/OgreHlmsPbsTerraShadows.h"
#include "Terra/Terra.h"
// #include "Terra/TerraShadowMapper.h"
#include "OgreHlmsManager.h"
#include "OgreHlms.h"
// #include "Compositor/OgreCompositorManager2.h"
// #include "Compositor/OgreCompositorWorkspace.h"

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
    Tutorial_TerrainGameState::Tutorial_TerrainGameState( const String &helpDescription )
        : TutorialGameState( helpDescription )
        , mPitch( Math::PI * 0.55f )  // par
        , mYaw( 0 )
    {
        macroblockWire.mPolygonMode = PM_WIREFRAME;
        SetupTrees();
    }

    
    //  Create
    //-----------------------------------------------------------------------------------------------------------------------------
    void Tutorial_TerrainGameState::createScene01()
    {
        SceneManager *sceneManager = mGraphicsSystem->getSceneManager();
        SceneNode *rootNode = sceneManager->getRootSceneNode( SCENE_STATIC );

        LogManager::getSingleton().setLogDetail(LoggingLevel::LL_BOREME);

        LogO("---- createScene");
        Root *root = mGraphicsSystem->getRoot();
        RenderSystem *renderSystem = root->getRenderSystem();
        renderSystem->setMetricsRecordingEnabled( true );


        //  Light  ------------------------------------------------
        LogO("---- new light");
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

        //  atmosphere  ------------------------------------------------
    #ifdef OGRE_BUILD_COMPONENT_ATMOSPHERE
        LogO("---- new Atmosphere");
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


        //  Terrain  ------------------------------------------------
        CreatePlane();  // fast
        // CreateTerrain();  // 5sec
        // CreateTrees();

        LogO("---- tutorial createScene");

        TutorialGameState::createScene01();
    }


    //  Destroy
    //-----------------------------------------------------------------------------------
    void Tutorial_TerrainGameState::destroyScene()
    {
        LogO("---- destroyScene");

        DestroyTerrain();
        DestroyPlane();

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
    }


    //  text
    //-----------------------------------------------------------------------------------------------------------------------------
    void Tutorial_TerrainGameState::generateDebugText( float timeSinceLast, String &outText )
    {
        //auto toStr = [](auto v, auto p=1) {  return StringConverter::toString(v,p);  };
        #define toStr(v, p)  StringConverter::toString(v,p)
        outText = "";

        if( mDisplayHelpMode == 0 )
        {
            TutorialGameState::generateDebugText( timeSinceLast, outText );
            
            Vector3 camPos = mGraphicsSystem->getCamera()->getPosition();
            outText += "Pos: " + toStr( camPos.x, 4) +" "+ toStr( camPos.y, 4) +" "+ toStr( camPos.z, 4) + "\n\n";

            #if 1  // list all veget cnts
            for (const auto& lay : vegetLayers)
                outText += toStr( lay.count, 4 ) + " " + lay.mesh + "\n";
            #endif
        }
        else if( mDisplayHelpMode == 1 )
        {
            //  fps stats  ------------------------------------------------
            RenderSystem *rs = mGraphicsSystem->getRoot()->getRenderSystem();
            const RenderingMetrics& rm = rs->getMetrics();  //** fps
            const FrameStats *st = mGraphicsSystem->getRoot()->getFrameStats();
            
            outText += toStr( (int)st->getAvgFps(), 4) +"  "+ //"\n" +
                "f " + toStr( rm.mFaceCount/1000, 0) + //"k v " + toStr( rm.mVertexCount/1000 ) + 
                "k d " + toStr( rm.mDrawCount, 0) + " i " + toStr( rm.mInstanceCount, 0)
                +"\n";
                // +" b " + toStr( rm.mBatchCount, 0) + "\n";

            outText += "Veget all: " + toStr(vegetNodes.size(), 5);
            outText += "\n- + Sun Pitch " + toStr( mPitch * 180.f / Math::PI, 3 );
            outText += "\n/ * Sun Yaw   " + toStr( mYaw * 180.f / Math::PI, 3 );

            outText += "\n^ v Param: " + toStr( param, 0 );
            
            SceneManager *sceneManager = mGraphicsSystem->getSceneManager();
            AtmosphereNpr *atmosphere = static_cast<AtmosphereNpr*>( sceneManager->getAtmosphere() );
            AtmosphereNpr::Preset p = atmosphere->getPreset();
            
            outText += "\n< > ";  const int d = 3;
            switch (param)
            {
            case 0:   outText += "Fog density: " + toStr( p.fogDensity, d );  break;
            case 1:   outText += "density coeff: " + toStr( p.densityCoeff, d );  break;
            case 2:   outText += "density diffusion: " + toStr( p.densityDiffusion, d );  break;
            case 3:   outText += "horizon limit: " + toStr( p.horizonLimit, d );  break;
            case 4:   outText += "Sun Power: " + toStr( p.sunPower, d );  break;
            case 5:   outText += "sky Power: " + toStr( p.skyPower, d );  break;
            case 6:   outText += "sky Colour   Red: " + toStr( p.skyColour.x, d );  break;
            case 7:   outText += "sky Colour Green: " + toStr( p.skyColour.y, d );  break;
            case 8:   outText += "sky Colour   Blu: " + toStr( p.skyColour.z, d );  break;
            case 9:   outText += "fog break MinBright: " + toStr( p.fogBreakMinBrightness, d );  break;
            case 10:  outText += "fog break Falloff: " + toStr( p.fogBreakFalloff, d );  break;
            case 11:  outText += "linked LightPower: " + toStr( p.linkedLightPower, d );  break;
            case 12:  outText += "ambient UpperPower: " + toStr( p.linkedSceneAmbientUpperPower, d );  break;
            case 13:  outText += "ambient LowerPower: " + toStr( p.linkedSceneAmbientLowerPower, d );  break;
            case 14:  outText += "envmap Scale: " + toStr( p.envmapScale, d );  break;
            }
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

        case SDL_SCANCODE_T:
            if (mTerra)
            {   DestroyTerrain();  CreatePlane();  }
            else
            {   CreateTerrain();  DestroyPlane();  }
            break;

        //  Trees add, destroy all
        case SDL_SCANCODE_V:  CreateTrees();  break;
        case SDL_SCANCODE_C:  DestroyTrees();  break;

        //  other
        case SDL_SCANCODE_F:  CreateParticles();  break;
        case SDL_SCANCODE_G:  CreateCar();  break;

        case SDL_SCANCODE_K:  
            if (ndSky)
                DestroySkyDome();
            else
            {
                switch (iSky)
                {
                case 0:  CreateSkyDome("sky-clearday1", 0.f);  ++iSky;  break;
                case 1:  CreateSkyDome("sky_photo6", 0.f);  iSky = 0;  break;  // clouds
                }
            }
            break;
        
        case SDL_SCANCODE_M:
        {
            // Vector3 camPos(-52.f, mTerra ? 735.f : 60.f, mTerra ? 975.f : 517.f);
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
        }

        TutorialGameState::keyReleased( arg );
    }
}
