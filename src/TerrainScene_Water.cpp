#include "OgreHlmsManager.h"
#include "TerrainGame.h"
#include "OgrePlanarReflections.h"
#include "GraphicsSystem.h"

#include "OgreLogManager.h"
#include "OgreCamera.h"
#include "OgreSceneNode.h"
#include "OgreItem.h"
#include "OgreMeshManager.h"
#include "OgreMeshManager2.h"

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
    
    class PlanarReflectionsWorkspaceListener final : public CompositorWorkspaceListener
    {
        PlanarReflections *mPlanarReflections;

    public:
        PlanarReflectionsWorkspaceListener( PlanarReflections *planarReflections ) :
            mPlanarReflections( planarReflections )
        {
        }
        ~PlanarReflectionsWorkspaceListener() {}

        void workspacePreUpdate( CompositorWorkspace *workspace ) override
        {
			if (mPlanarReflections)
            	mPlanarReflections->beginFrame();
        }

        void passEarlyPreExecute( CompositorPass *pass ) override
        {
            // Ignore non-scene passes
            if( pass->getType() != PASS_SCENE )
                return;
            assert( dynamic_cast<const CompositorPassSceneDef *>( pass->getDefinition() ) );
            const CompositorPassSceneDef *passDef =
                static_cast<const CompositorPassSceneDef *>( pass->getDefinition() );

            // Ignore scene passes that belong to a shadow node.
            if( passDef->mShadowNodeRecalculation == SHADOW_NODE_CASTER_PASS )
                return;

            // Ignore scene passes we haven't specifically tagged to receive reflections
            if( passDef->mIdentifier != 25001 )
                return;

            CompositorPassScene *passScene = static_cast<CompositorPassScene *>( pass );
            Camera *camera = passScene->getCamera();

            // Note: The Aspect Ratio must match that of the camera we're reflecting.
			if (mPlanarReflections)
            	mPlanarReflections->update( camera, camera->getAutoAspectRatio()
                                            ? pass->getViewportAspectRatio( 0u )
                                            : camera->getAspectRatio() );
        }
    };


	void TerrainGame::setupPlanarReflections()
	{
        LogO("---- tutorial createScene");

        Root *root = mGraphicsSystem->getRoot();
        SceneManager *sceneManager = mGraphicsSystem->getSceneManager();

        bool useComputeMipmaps = false;
#if !OGRE_NO_JSON
        useComputeMipmaps =
            root->getRenderSystem()->getCapabilities()->hasCapability( RSC_COMPUTE_PROGRAM );
#endif

        // Setup PlanarReflections
        mPlanarReflections =
            new PlanarReflections( sceneManager, root->getCompositorManager2(), 1.0, 0 );
        mWorkspaceListener = new PlanarReflectionsWorkspaceListener( mPlanarReflections );
        {
            CompositorWorkspace *workspace = mGraphicsSystem->getCompositorWorkspace();
            workspace->addListener( mWorkspaceListener );
        }

        // The perfect mirror doesn't need mipmaps.
        // mPlanarReflections->setMaxActiveActors( 1u, "PlanarReflectionsReflectiveWorkspace", true, 512,
        //                                         512, false, PFG_RGBA8_UNORM_SRGB,
        //                                         useComputeMipmaps );
        // The rest of the reflections do.  2u
        mPlanarReflections->setMaxActiveActors( 1u, "PlanarReflectionsReflectiveWorkspace", true, 512,
                                                512, true, PFG_RGBA8_UNORM_SRGB,
                                                useComputeMipmaps );
        const Vector2 mirrorSize( 500.0f, 500.0f );

        // Create the plane mesh
        // Note that we create the plane to look towards +Z; so that sceneNode->getOrientation
        // matches the orientation for the PlanarReflectionActor
        v1::MeshPtr planeMeshV1 = v1::MeshManager::getSingleton().createPlane(
            "Plane Mirror Unlit", ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
            Plane( Vector3::UNIT_Z, 0.0f ), mirrorSize.x, mirrorSize.y, 1, 1, true, 1, 1.0f,
            1.0f, Vector3::UNIT_Y, v1::HardwareBuffer::HBU_STATIC,
            v1::HardwareBuffer::HBU_STATIC );
        MeshPtr planeMesh = MeshManager::getSingleton().createByImportingV1(
            "Plane Mirror Unlit", ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
            planeMeshV1.get(), true, true, true );


		//---------------------------------------------------------------------
        // Setup mirror for PBS.
        //---------------------------------------------------------------------
        Hlms *hlms = root->getHlmsManager()->getHlms( HLMS_PBS );
        assert( dynamic_cast<HlmsPbs *>( hlms ) );
        HlmsPbs *pbs = static_cast<HlmsPbs *>( hlms );
        pbs->setPlanarReflections( mPlanarReflections );

        Item* item = sceneManager->createItem( planeMesh, SCENE_DYNAMIC );
        item->setDatablock( "GlassRoughness" );
		SceneNode*
        sceneNode = sceneManager->getRootSceneNode( SCENE_DYNAMIC )
                        ->createChildSceneNode( SCENE_DYNAMIC );
        sceneNode->setPosition( 0, 12.5f, 0 );
        sceneNode->setOrientation(
            Quaternion( Radian( Math::HALF_PI ), Vector3::UNIT_Y ) );
        // sceneNode->setScale( Vector3( 0.75f, 0.5f, 1.0f ) );
        sceneNode->attachObject( item );

        PlanarReflectionActor* actor = mPlanarReflections->addActor( PlanarReflectionActor(
            sceneNode->getPosition(), mirrorSize /** Vector2( 0.75f, 0.5f )*/,
            sceneNode->getOrientation() ) );

        PlanarReflections::TrackedRenderable trackedRenderable(
            item->getSubItem( 0 ), item, Vector3::UNIT_Z, Vector3( 0, 0, 0 ) );
        mPlanarReflections->addRenderable( trackedRenderable );

	}

}
