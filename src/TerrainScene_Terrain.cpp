#include "TerrainGame.h"
#include "GraphicsSystem.h"

#include "OgreSceneManager.h"
#include "OgreRoot.h"
#include "OgreLogManager.h"

#include "Terra/Hlms/OgreHlmsTerra.h"
#include "Terra/Hlms/OgreHlmsTerraDatablock.h"
#include "Terra/Hlms/PbsListener/OgreHlmsPbsTerraShadows.h"
#include "Terra/Terra.h"
#include "Terra/TerraShadowMapper.h"

#include "OgreHlmsManager.h"
#include "OgreHlmsPbs.h"
#include "OgreHlmsPbsDatablock.h"
#include "OgreHlmsUnlit.h"
#include "OgreHlmsUnlitDatablock.h"

using namespace Demo;
using namespace Ogre;


namespace Demo
{

    //  Terrain Create
    //-----------------------------------------------------------------------------------------------------------------------------
    void TerrainGame::CreateTerrain()
    {
        if (mTerra) return;
        Root *root = mGraphicsSystem->getRoot();
        SceneManager *mgr = mGraphicsSystem->getSceneManager();
        SceneNode *rootNode = mgr->getRootSceneNode( SCENE_STATIC );
        
        HlmsManager *hlmsManager = root->getHlmsManager();
        HlmsDatablock *datablock = 0;

        //  Terrain
        //------------------------------------------------
        LogO("---- new Terra");

        mTerra = new Terra( Id::generateNewId<MovableObject>(),
                            &mgr->_getEntityMemoryManager( SCENE_STATIC ),
                            mgr, 11u, root->getCompositorManager2(),
                            mGraphicsSystem->getCamera(), false );
        mTerra->setCastShadows( false );

        LogO("---- Terra load");

        //  Heightmap
        switch (preset)
        {
        case 1:  //  1k  med  4km  4tex  600 fps
            yWaterHeight = 48.65f;
            sizeXZ = 4096.f;
            mTerra->load( "Heightmap.png", Vector3( 64.0f, 4096.0f * 0.5f, 64.0f), Vector3(sizeXZ, 4096.0f, sizeXZ), false, false);
            datablock = hlmsManager->getDatablock( "TerraExampleMaterial" );  // coeffs
            break;
        case 2:  //  1k  big  8km  4tex  380k veget 50fps
            yWaterHeight = 101.f;
            sizeXZ = 3.f * 4096.f;
            mTerra->load( "Heightmap.png", Vector3( 64.0f, 4096.0f * 0.5f, 64.0f), Vector3(sizeXZ, 4096.0f, sizeXZ), false, false);
            datablock = hlmsManager->getDatablock( "TerraExampleMaterial" );  // coeffs
            // datablock = hlmsManager->getDatablock( "TerraExampleMaterial_R_M" );  // maps
            break;
                    
        case 3:  //  tropic
        case 4:
            yWaterHeight = -250.f;
            sizeXZ = 1146.78 * 7.14348;
            sizeY = 272.328 * 7.14348;
            mTerra->load( "Jng13-Tropic.png", Vector3( 0.f, 0.f, 0.f), Vector3(sizeXZ, sizeY, sizeXZ), false, false);
            datablock = hlmsManager->getDatablock( "Jng13-Tropic" );  // coeffs
            break;
        }

        SceneNode *node = rootNode->createChildSceneNode( SCENE_STATIC );
        node->attachObject( mTerra );

        mTerra->setDatablock( datablock );


        //  Horizon  far  * * * * *
        //------------------------------------------------
        sizeXZ2 = sizeXZ;  sizeY2 = sizeY;
        if (preset >= 4)
        {
            LogO("---- new Terra2");

            mTerra2 = new Terra( Id::generateNewId<MovableObject>(),
                                &mgr->_getEntityMemoryManager( SCENE_STATIC ),
                                mgr, 11u, root->getCompositorManager2(),
                                mGraphicsSystem->getCamera(), false );
            mTerra2->setCastShadows( false );

            LogO("---- Terra2 load");

            //  Heightmap
            switch (preset)
            {
            case 4:  //  tropic  horiz  ofs-
                yWaterHeight = -221.f;
                sizeXZ2 = 10556.2 * 7.14348;
                sizeY2 = 645.912 * 7.14348;
                mTerra2->load( "Jng13-Tropic2.png", Vector3( 50.f, 1300.f, 0.f), Vector3(sizeXZ2, sizeY2, sizeXZ2), false, false);
                datablock = hlmsManager->getDatablock( "Jng13-Tropic2" );  // coeffs
                break;
            }

            node->attachObject( mTerra2 );

            LogO("---- Terra2 attach");

            mTerra2->setDatablock( datablock );
        }


        //  shadows only on 1st terrain
        LogO("---- new TerraShadows");
        mHlmsPbsTerraShadows = new HlmsPbsTerraShadows();
        mHlmsPbsTerraShadows->setTerra( mTerra );
        //  Set the PBS listener so regular objects also receive terrain shadows
        Hlms *hlmsPbs = root->getHlmsManager()->getHlms( HLMS_PBS );
        hlmsPbs->setListener( mHlmsPbsTerraShadows );
    }


    //  Terrain util
    //-----------------------------------------------------------------------------------------------------------------------------
    void TerrainGame::DestroyTerrain()
    {
        LogO("==== destroy Terrain");
        Root *root = mGraphicsSystem->getRoot();
        Hlms *hlmsPbs = root->getHlmsManager()->getHlms( HLMS_PBS );

        if( hlmsPbs->getListener() == mHlmsPbsTerraShadows )
        {
            hlmsPbs->setListener( 0 );
            delete mHlmsPbsTerraShadows;  mHlmsPbsTerraShadows = 0;
        }
        delete mTerra;  mTerra = 0;

        delete mTerra2;  mTerra2 = 0;
    }

    void TerrainGame::ToggleWireframe()
    {
        if (!mTerra)
            return;
        wireTerrain = !wireTerrain;
        Root *root = mGraphicsSystem->getRoot();
        HlmsManager *hlmsManager = root->getHlmsManager();
        HlmsDatablock *datablock = 0;
        datablock = hlmsManager->getDatablock( "TerraExampleMaterial" );
        if (datablock && wireTerrain)
        {
            datablock = hlmsManager->getHlms( HLMS_USER3 )->getDefaultDatablock();
            datablock->setMacroblock( macroblockWire );
        }
        mTerra->setDatablock( datablock );
    }

    void TerrainGame::ToggleTriplanar()
    {
        mTriplanarMappingEnabled = !mTriplanarMappingEnabled;
        if (!mTerra)
            return;

        Ogre::HlmsManager *hlmsManager = mGraphicsSystem->getRoot()->getHlmsManager();
        Ogre::HlmsTerraDatablock *datablock = static_cast<Ogre::HlmsTerraDatablock *>(
            hlmsManager->getDatablock( "TerraExampleMaterial" ) );
        if (!datablock)
            return;

        // datablock->setBrdf(TerraBrdf::BlinnPhongLegacyMath);  //** try, rough no-
        datablock->setDetailTriplanarDiffuseEnabled( mTriplanarMappingEnabled );
        // datablock->setDetailTriplanarNormalEnabled( mTriplanarMappingEnabled );  // todo fixme
        // datablock->setDetailTriplanarRoughnessEnabled( mTriplanarMappingEnabled );  // only in TerraExampleMaterial_R_M
        // datablock->setDetailTriplanarMetalnessEnabled( mTriplanarMappingEnabled );

        Ogre::Vector2 terrainDimensions = mTerra->getXZDimensions();

        Ogre::Vector4 detailMapOffsetScale[2];
        detailMapOffsetScale[0] = datablock->getDetailMapOffsetScale( 0 );
        detailMapOffsetScale[1] = datablock->getDetailMapOffsetScale( 1 );

        // Switch between "common" UV mapping and world coordinates-based UV mapping (and vice versa)
        if( mTriplanarMappingEnabled )
        {
            detailMapOffsetScale[0].z = 1.0f / ( terrainDimensions.x / detailMapOffsetScale[0].z );
            detailMapOffsetScale[0].w = 1.0f / ( terrainDimensions.y / detailMapOffsetScale[0].w );
            detailMapOffsetScale[1].z = 1.0f / ( terrainDimensions.x / detailMapOffsetScale[1].z );
            detailMapOffsetScale[1].w = 1.0f / ( terrainDimensions.y / detailMapOffsetScale[1].w );
        }
        else
        {
            detailMapOffsetScale[0].z *= terrainDimensions.x;
            detailMapOffsetScale[0].w *= terrainDimensions.y;
            detailMapOffsetScale[1].z *= terrainDimensions.x;
            detailMapOffsetScale[1].w *= terrainDimensions.y;
        }

        datablock->setDetailMapOffsetScale( 0, detailMapOffsetScale[0] );
        datablock->setDetailMapOffsetScale( 1, detailMapOffsetScale[1] );
    }

}
