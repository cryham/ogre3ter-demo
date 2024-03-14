#include "TerrainGame.h"
#include "OgrePlanarReflections.h"
#include "GraphicsSystem.h"
#include "Terra/Terra.h"

#include "OgreLogManager.h"
#include "OgreCamera.h"
#include "OgreSceneNode.h"
#include "OgreItem.h"
#include "OgreMesh.h"
#include "OgreMeshManager.h"
#include "OgreMesh2.h"
#include "OgreMeshManager2.h"
#include "OgreHlmsManager.h"
#include "OgreHlmsPbsDatablock.h"

#include "OgreHlmsPbs.h"
#include "Compositor/OgreCompositorManager2.h"
#include "Compositor/OgreCompositorNodeDef.h"
#include "Compositor/OgreCompositorWorkspaceDef.h"
#include "Compositor/Pass/PassIblSpecular/OgreCompositorPassIblSpecularDef.h"

#include "Compositor/OgreCompositorWorkspace.h"
#include "Compositor/OgreCompositorWorkspaceListener.h"
#include "Compositor/Pass/PassScene/OgreCompositorPassScene.h"
#include "Compositor/Pass/PassScene/OgreCompositorPassSceneDef.h"

using namespace Demo;
using namespace Ogre;


namespace Demo
{
    
    class PlanarReflectWsListener final : public CompositorWorkspaceListener
    {
        PlanarReflections *mPlanarReflections;
		TerrainGame* mGame;

    public:
        PlanarReflectWsListener(
                PlanarReflections *planarReflections, TerrainGame* game )
			: mPlanarReflections( planarReflections )
			, mGame( game )
        {
        }
        ~PlanarReflectWsListener()
        {   }

        void workspacePreUpdate( CompositorWorkspace *workspace ) override
        {
			if (mPlanarReflections)
            	mPlanarReflections->beginFrame();
        }

        //-----------------------------------------------------------------------------------
        void passEarlyPreExecute( CompositorPass *pass ) override
        {
            //  Ignore non-scene passes
            if( pass->getType() != PASS_SCENE )
                return;
            assert( dynamic_cast<const CompositorPassSceneDef *>( pass->getDefinition() ) );
            const CompositorPassSceneDef *passDef =
                static_cast<const CompositorPassSceneDef *>( pass->getDefinition() );

            //  Ignore scene passes that belong to a shadow node.
            if( passDef->mShadowNodeRecalculation == SHADOW_NODE_CASTER_PASS )
                return;

            CompositorPassScene *passScene = static_cast<CompositorPassScene *>( pass );
            Camera *camera = passScene->getCamera();

			//**  update Terra  here?
            #if 0
	        if (mGame && mGame->mTerra)
			{
	            const float lightEpsilon = 0.0f;  //**?
	            // const float lightEpsilon = 0.0001f;  //** 0.0f slow
				mGame->mTerra->setCamera( camera );
    	        mGame->mTerra->update( mGame->mSunLight->getDerivedDirectionUpdated(), lightEpsilon );
			}
            #endif

            //  Ignore scene passes we haven't specifically tagged to receive reflections
            if( passDef->mIdentifier != 25001 )
                return;

            //  Note: The Aspect Ratio must match that of the camera we're reflecting.
			if (mPlanarReflections)
            	mPlanarReflections->update(
                    mGame->mTerra, mGame->sunDir,
                    mGame->mGraphicsSystem->getCamera(),
                    camera, camera->getAutoAspectRatio()
                                            ? pass->getViewportAspectRatio( 0u )
                                            : camera->getAspectRatio() );
        }
    };


    //  Create Water  Refract only
    //-----------------------------------------------------------------------------------
    void TerrainGame::CreateWaterRefract()
    {
        Ogre::SceneManager *sceneManager = mGraphicsSystem->getSceneManager();
        Ogre::HlmsManager *hlmsManager = mGraphicsSystem->getRoot()->getHlmsManager();
        assert( dynamic_cast<Ogre::HlmsPbs *>( hlmsManager->getHlms( Ogre::HLMS_PBS ) ) );
        Ogre::HlmsPbs *hlmsPbs = static_cast<Ogre::HlmsPbs *>( hlmsManager->getHlms( Ogre::HLMS_PBS ) );

        //  thin cube
        waterItem = sceneManager->createItem(
            "Cube_d.mesh", Ogre::ResourceGroupManager::AUTODETECT_RESOURCE_GROUP_NAME,
            Ogre::SCENE_DYNAMIC );

        waterItem->setVisibilityFlags( 2 );
        waterItem->setCastShadows( false );
        //  important: Only Refractive materials must be rendered during the refractive pass
        waterItem->setRenderQueueGroup( 220 );

        waterNode = sceneManager->getRootSceneNode( Ogre::SCENE_DYNAMIC )
                                ->createChildSceneNode( Ogre::SCENE_DYNAMIC );
        waterNode->setPosition( -240, yWaterHeight, -720 );
        waterNode->setScale( 1222.f, 2.f, 1222.f );
        waterNode->attachObject( waterItem );

        //  Set material to refractive  ----
        auto* datablock = (HlmsPbsDatablock*)hlmsPbs->getDatablock(
            // "Water");  //** test flat
            // "WaterBump");
            "WaterBumpDetail");
            // "WaterBumpMax");
        datablock->setTransparency( 0.15f, Ogre::HlmsPbsDatablock::Refractive );
        datablock->setFresnel( Ogre::Vector3( 0.5f ), false );
        datablock->setRefractionStrength( 0.8f );  // par+

        waterItem->setDatablock( datablock );

        // createRefractivePlaceholder( item, sceneNode, datablock );
    }


    //  Create Water  planar Reflect + Refract
    //-----------------------------------------------------------------------------------
	void TerrainGame::CreateWater()
	{
		if (mPlanarReflect || waterItem)
			return;
        LogO("---- create water");

        Root *root = mGraphicsSystem->getRoot();
        SceneManager *sceneManager = mGraphicsSystem->getSceneManager();

        bool useCompute = false;
	#if !OGRE_NO_JSON
        useCompute = root->getRenderSystem()->getCapabilities()->hasCapability( RSC_COMPUTE_PROGRAM );
	#endif

        //  Setup PlanarReflections
        //-------------------------------------------------------
        mPlanarReflect = new PlanarReflections( sceneManager, root->getCompositorManager2(),
			100.f, 0 );  // par?
		
        mWorkspaceListener = new PlanarReflectWsListener( mPlanarReflect, this );
        CompositorWorkspace *workspace = mGraphicsSystem->getCompositorWorkspace();
        workspace->addListener( mWorkspaceListener );

        //** water params  ----
		const Vector2 waterSize( 8000.f, 8000.f );
		const int size = 2 * 512;
		const int segments = 64;  //** 1  more ok
		const Real tile = 5.0f;  // 1
		// const Real tile = 21.0f;  // 1

        mPlanarReflect->setMaxActiveActors( 1u, "PlanarReflectionsReflectiveWorkspace",
			true, size, size, true, // accurate, with mipmaps
			PFG_RGBA8_UNORM_SRGB, useCompute );

        //  Create the plane mesh
        //  Note that we create the plane to look towards +Z; so that sceneNode->getOrientation
        //  matches the orientation for the PlanarReflectionActor
        waterMeshV1 = v1::MeshManager::getSingleton().createPlane(
            "Plane Water V1", ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
            Plane( Vector3::UNIT_Z, 0.0f ), waterSize.x, waterSize.y,
			segments, segments, true, 1,
			tile, tile,
			Vector3::UNIT_Y, v1::HardwareBuffer::HBU_STATIC,
            v1::HardwareBuffer::HBU_STATIC );
		waterMeshV1->buildTangentVectors();  // for normal maps

		waterMesh = MeshManager::getSingleton().createByImportingV1(
            "Plane Water", ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
            waterMeshV1.get(), false, false, false );
            // waterMeshV1.get(), true, true, true );


        //  Setup mirror for PBS
        //-------------------------------------------------------
        Hlms *hlms = root->getHlmsManager()->getHlms( HLMS_PBS );
        assert( dynamic_cast<HlmsPbs *>( hlms ) );
        HlmsPbs *pbs = static_cast<HlmsPbs *>( hlms );
        pbs->setPlanarReflections( mPlanarReflect );


        const auto type = SCENE_DYNAMIC;
		waterItem = sceneManager->createItem( waterMesh, type );
		waterItem->setCastShadows( false );

        //  Tutorial_TerrainWorkspace_NoRefract  works
        waterItem->setDatablock( "Water" );  // test flat --
        // waterItem->setDatablock( "WaterDetail" );  // bumpy +


    #if 1  //  Refract  ----
        //  Tutorial_TerrainWorkspace  needed
        auto* datablock = (HlmsPbsDatablock*)pbs->getDatablock(
            // "Water");  //** test flat --
            // "WaterBump");  //-
            // "WaterBumpDetail");  // strong
            // "WaterBumpSoft");  // nice
            "WaterBumpSoftAnim");  // nice anim
            // "WaterBumpMax");  // par_
        datablock->setTransparency( 0.02f, Ogre::HlmsPbsDatablock::Refractive );
        datablock->setFresnel( Ogre::Vector3( 0.31f ), false );
        // datablock->setFresnel( Ogre::Vector3( 0.1f, 0.4f, 0.9f ), true );  // crash, shader F0?
        datablock->setRefractionStrength( 0.9f );  // par-
        waterItem->setDatablock( datablock );
    #endif
        // important: Only Refractive materials must be rendered during the refractive pass
        // bad: inverses reflect cam
        waterItem->setRenderQueueGroup( 220 );
        waterItem->setVisibilityFlags( 2 );


		waterNode = sceneManager->getRootSceneNode( type )->createChildSceneNode( type );
        waterNode->setPosition( 0, yWaterHeight, 0 );
		if (yWaterHeight > yWaterVertical)  //** test |
			waterNode->setOrientation( Quaternion( Radian( -Math::HALF_PI ), Vector3::UNIT_X ) );  // -- flat
		else
        	waterNode->setOrientation( Quaternion( Radian( Math::HALF_PI ), Vector3::UNIT_Y ) );  // | vertical
        waterNode->attachObject( waterItem );


        // PlanarReflectionActor* actor =
		mPlanarReflect->addActor( PlanarReflectionActor(
            waterNode->getPosition(), waterSize,
            waterNode->getOrientation()
            // * Quaternion( Radian( Math::PI ), Vector3::UNIT_Y )
            ) );

        PlanarReflections::TrackedRenderable trackedRenderable(
            waterItem->getSubItem( 0 ), waterItem, Vector3::UNIT_Z, Vector3( 0, 0, 0 ) );
        mPlanarReflect->addRenderable( trackedRenderable );
	}


    //  Destroy
    //-----------------------------------------------------------------------------------
	void TerrainGame::DestroyWater()
	{
		if (mPlanarReflect)
			mPlanarReflect->destroyAllActors();

        SceneManager *mgr = mGraphicsSystem->getSceneManager();
        if (waterNode)
        {   mgr->destroySceneNode(waterNode);  waterNode = 0;  }
        if (waterItem)
        {   mgr->destroyItem(waterItem);  waterItem = 0;  }
        
		auto& ms = MeshManager::getSingleton();
		auto& m1 = v1::MeshManager::getSingleton();
        if (waterMesh)  ms.remove(waterMesh);  waterMesh.reset();
        if (waterMeshV1)  m1.remove(waterMeshV1);  waterMeshV1.reset();


		Root *root = mGraphicsSystem->getRoot();
		Hlms *hlms = root->getHlmsManager()->getHlms( HLMS_PBS );
		assert( dynamic_cast<HlmsPbs *>( hlms ) );
		HlmsPbs *pbs = static_cast<HlmsPbs *>( hlms );
		pbs->setPlanarReflections( 0 );  // off

		CompositorWorkspace *ws = mGraphicsSystem->getCompositorWorkspace();
		if (mWorkspaceListener)
			ws->removeListener( mWorkspaceListener );
		delete mWorkspaceListener;  mWorkspaceListener = 0;

		delete mPlanarReflect;  mPlanarReflect = 0;
	}

}
