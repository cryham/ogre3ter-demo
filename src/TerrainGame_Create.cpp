#include "TerrainGame.h"
#include "CameraController.h"
#include "GraphicsSystem.h"
#include "SDL_scancode.h"
#include "OgreLogManager.h"

#include "OgreSceneManager.h"
#include "OgreRoot.h"
#include "OgreCamera.h"
#include "OgreWindow.h"
#include "OgreFrameStats.h"

#include "Terra/Terra.h"
#include "OgreItem.h"
#include "OgreHlms.h"
#include "OgreHlmsPbs.h"
#include "OgreHlmsManager.h"
#include "OgreGpuProgramManager.h"
#include "OgreTextureGpuManager.h"
#include "OgrePixelFormatGpuUtils.h"

#ifdef OGRE_BUILD_COMPONENT_ATMOSPHERE
#    include "OgreAtmosphereNpr.h"
#endif

using namespace Demo;
using namespace Ogre;


namespace Demo
{
    TerrainGame::TerrainGame( const String &helpDescription )
        : TutorialGameState( helpDescription )
        , mPitch( 50.f * Math::PI / 180.f )  // par
        , mYaw( 102 * Math::PI / 180.f )
        //, mIblQuality( IblHigh )  // par
        , mIblQuality( MipmapsLowest )
    {
        macroblockWire.mPolygonMode = PM_WIREFRAME;
    }

    
    //  Create
    //-----------------------------------------------------------------------------------------------------------------------------
    void TerrainGame::createScene01()
    {
        mGraphicsSystem->mWorkspace = setupCompositor();

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
        mSunNode = rootNode->createChildSceneNode();
        mSunNode->attachObject( mSunLight );
        
        mSunLight->setPowerScale( Math::PI * 2 );  //** par! 1.5 2 3* 4  should be * 1..
        mSunLight->setType( Light::LT_DIRECTIONAL );
        mSunLight->setDirection( Vector3( 0, -1, 0 ).normalisedCopy() );  //-

        //  ambient  set in update ..
        sceneManager->setAmbientLight(
            // ColourValue( 0.63f, 0.61f, 0.28f ) * 0.04f,
            // ColourValue( 0.52f, 0.63f, 0.76f ) * 0.04f,
            ColourValue( 0.33f, 0.61f, 0.98f ) * 0.01f,
            ColourValue( 0.02f, 0.53f, 0.96f ) * 0.01f,
            Vector3::UNIT_Y );

        //  Atmosphere  ------------------------------------------------
    #ifdef OGRE_BUILD_COMPONENT_ATMOSPHERE
        LogO("---- new Atmosphere");
        mGraphicsSystem->createAtmosphere( mSunLight );
    #endif

        //**  Camera  ------------------------------------------------
        mCameraController = new CameraController( mGraphicsSystem, false );
        auto* camera = mGraphicsSystem->getCamera();
        camera->setFarClipDistance( 100000.f );  // par far

        #if 1  // close to water
            camera->setPosition( Ogre::Vector3( -489, 73, -715 ) );
            camera->lookAt( Ogre::Vector3( -230, 42, -755 ) );
            yWaterHeight = 48.65f;
        #elif 1  // terrain view  from screen
            camera->setPosition( Ogre::Vector3( -979, 407, -912 ) );
            camera->setPosition( Ogre::Vector3( -1089, 448, -815 ) );
            camera->lookAt( Ogre::Vector3( 0, 20, 0 ) );
            yWaterHeight = 43.65f;
        #elif 0
            camPos = Vector3(-52.f, mTerra ? 735.f : 60.f, mTerra ? 975.f : 517.f);
            //camPos.y += mTerra->getHeightAt( camPos );
            camera->setPosition( camPos );
            camera->lookAt( camPos + Vector3(0.f, -0.5f, -1.f) );
        #endif

	    //**  shadows par
        // const float shadow_dist = 1200.f;  // low
	    // const float shadow_dist = 2200.f;  // far
	    const float shadow_dist = 3200.f;  // very far
        sceneManager->setShadowDirectionalLightExtrusionDistance( shadow_dist );
	    sceneManager->setShadowFarDistance( shadow_dist );


        //  Init  ------------------------------------------------
        CreateScene(1);  //** par

        // CreatePlane();  // fastest

        LogO("---- tutorial createScene");

        TutorialGameState::createScene01();
    }


    //  Create scene, full setup
    //-----------------------------------------------------------------------------------
    void TerrainGame::CreateScene(int pre)
    {
        LogO("==== Destroy Scene");

        DestroyCars();
        DestroyPlane();
        DestroyParticles();

        DestroySkyDome();
        DestroyTerrain();
        DestroyWater();
        DestroyVeget();

        preset = pre;
        SetupVeget(preset == 2);

        auto* camera = mGraphicsSystem->getCamera();
        LogO("++++ Create Scene");//+toStr(pre));

        int nn = 0;
        float fogDens = 0.00012f;
        switch (preset)
        {
        case 0:  CreateSkyDome("day_clouds_08", 0.f);  // medium jng fog
            CreatePlane();  // fastest
            CreateVeget();
            camera->setPosition( Ogre::Vector3( 0, 16, 40 ) );
            camera->lookAt( Ogre::Vector3( 0, 6, 0 ) );
            CreateCar();
            fogDens = 0.00032f;
            break;

        case 1:  nn = 2;  CreateSkyDome("day_clouds_02_cyan", 0.f);  // medium jng fog
            fogDens = 0.00024f;  waterMaterial = "WaterClear";
            camera->setPosition( Ogre::Vector3( -489, 73, -715 ) );
            camera->lookAt( Ogre::Vector3( -230, 42, -755 ) );
            break;
        case 2:  nn = 2;  CreateSkyDome("day_clouds_04_blue", 0.f);  // vast forest
            fogDens = 0.00008f;  waterMaterial = "WaterDarkSoft";
            camera->setPosition( Ogre::Vector3( -950, 231, 615 ) );
            camera->lookAt( Ogre::Vector3( -950, 210, 450 ) );
            break;
        
        case 3:  nn = 5;  CreateSkyDome("cloudy_04_blue", 0.f);  // tropic
            fogDens = 0.00005f;  waterMaterial = "WaterBlue";
            camera->setPosition( Ogre::Vector3( 3048, 118, -1271 ) );
            camera->lookAt( Ogre::Vector3( 2400, -20, -1200 ) );
            break;
        case 4:  nn = 7;  CreateSkyDome("day_clouds_07", 0.f);  // tropic horiz
            fogDens = 0.00004f;  waterMaterial = "WaterBlue";
            camera->setPosition( Ogre::Vector3( 3048, 118, -1271 ) );
            camera->lookAt( Ogre::Vector3( 2400, -20, -1200 ) );
            break;
        }

        if (preset > 0)
        {
            CreateTerrain();
            CreateWater();
        
            for (int n=0; n < nn; ++n)
                CreateVeget();
        }

        //  set fog
    #ifdef OGRE_BUILD_COMPONENT_ATMOSPHERE
        SceneManager *sceneManager = mGraphicsSystem->getSceneManager();
        OGRE_ASSERT_HIGH( dynamic_cast<AtmosphereNpr *>( sceneManager->getAtmosphere() ) );
        AtmosphereNpr *atmosphere = static_cast<AtmosphereNpr *>( sceneManager->getAtmosphere() );
        AtmosphereNpr::Preset p = atmosphere->getPreset();
        p.fogDensity = fogDens;  //** par
        p.densityCoeff = 0.33f;  //0.27f  0.47f
        p.densityDiffusion = 0.01f;  //0.75f  2.0f
        p.horizonLimit = 0.025f;
        p.sunPower = 0.7f;  //1.0f;
        // p.skyPower  1.0f;
        p.skyColour = Vector3(0.234f, 0.57f, 1.0f);
        p.fogBreakMinBrightness = 0.25f;
        p.fogBreakFalloff = 0.1f;
        // p.linkedLightPower = Math::PI;
        // p.linkedSceneAmbientUpperPower = 0.1f * Math::PI;
        // p.linkedSceneAmbientLowerPower = 0.01f * Math::PI;
        p.envmapScale = 1.0f;
        atmosphere->setPreset( p );
    #endif
        LogO("---- Created scene");
    }


    //  Destroy
    //-----------------------------------------------------------------------------------
    void TerrainGame::destroyScene()
    {
        LogO("---- destroyScene");

        DestroyWater();
        
        DestroyVeget();
        DestroyCars();

        DestroyTerrain();
        DestroyPlane();
        
        DestroySkyDome();

        LogO("---- tutorial destroyScene");

        TutorialGameState::destroyScene();
    }

}
