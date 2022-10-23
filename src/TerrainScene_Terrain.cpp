#include "TerrainGame.h"
#include "CameraController.h"
#include "GraphicsSystem.h"

#include "OgreSceneManager.h"
#include "OgreRoot.h"
#include "OgreLogManager.h"

#include "OgreCamera.h"
#include "OgreWindow.h"

#include "Terra/Hlms/OgreHlmsTerra.h"
#include "Terra/Hlms/PbsListener/OgreHlmsPbsTerraShadows.h"
#include "Terra/Terra.h"
#include "Terra/TerraShadowMapper.h"
#include "OgreGpuProgramManager.h"

#include "OgreItem.h"
#include "OgreMesh.h"
#include "OgreMeshManager.h"
#include "OgreMesh2.h"
#include "OgreMeshManager2.h"
#include "OgreManualObject2.h"

#include "OgreHlmsPbs.h"
#include "OgreHlmsPbsDatablock.h"

using namespace Demo;
using namespace Ogre;


namespace Demo
{

    //  Terrain
    //-----------------------------------------------------------------------------------------------------------------------------
    void TerrainGame::CreateTerrain()
    {
        if (mTerra) return;
        Root *root = mGraphicsSystem->getRoot();
        SceneManager *mgr = mGraphicsSystem->getSceneManager();
        SceneNode *rootNode = mgr->getRootSceneNode( SCENE_STATIC );
        
        HlmsManager *hlmsManager = root->getHlmsManager();
        HlmsDatablock *datablock = 0;

        LogO("---- new Terra");

        mTerra = new Terra( Id::generateNewId<MovableObject>(),
                            &mgr->_getEntityMemoryManager( SCENE_STATIC ),
                            mgr, 11u, root->getCompositorManager2(),
                            mGraphicsSystem->getCamera(), false );
        mTerra->setCastShadows( false );

        LogO("---- Terra load");

        //  Heightmap  ------------------------------------------------
        switch (1)
        {
        case 0:  //  64  flat
            sizeXZ = 12096.f;
            mTerra->load( "Heightmap64.png", Vector3( 64.0f, 4096.0f * 0.15f, 64.0f), Vector3(sizeXZ, 6096.0f, sizeXZ), false, false);  break;
        case 1:  //  1k  600 fps  4 tex
            sizeXZ = 4096.f;
            mTerra->load( "Heightmap.png", Vector3( 64.0f, 4096.0f * 0.5f, 64.0f), Vector3(sizeXZ, 4096.0f, sizeXZ), false, false);  break;
        case 2:  //  1k
            sizeXZ = 1024.f;
            mTerra->load( "Heightmap.png", Vector3( 64.f, 512.f, 64.f), Vector3(sizeXZ, 1.f, sizeXZ), false, false);  break;
        case 3:  //  2k
            sizeXZ = 12096.f;
            mTerra->load( "Heightmap2c.png", Vector3( 64.0f, 4096.0f * 0.15f, 64.0f), Vector3(sizeXZ, 6096.0f, sizeXZ), false, false);  break;
        case 4:  //  4k
            sizeXZ = 2.f* 4096.f;
            mTerra->load( "Heightmap4.png", Vector3( 64.0f, 4096.0f * 0.5f, 64.0f), Vector3(sizeXZ, 2.f* 4096.0f, sizeXZ), false, false);  break;
        }

        SceneNode *node = rootNode->createChildSceneNode( SCENE_STATIC );
        node->attachObject( mTerra );

        LogO("---- Terra attach");

        datablock = hlmsManager->getDatablock( "TerraExampleMaterial" );
        mTerra->setDatablock( datablock );

        mHlmsPbsTerraShadows = new HlmsPbsTerraShadows();
        mHlmsPbsTerraShadows->setTerra( mTerra );
        //Set the PBS listener so regular objects also receive terrain shadows
        Hlms *hlmsPbs = root->getHlmsManager()->getHlms( HLMS_PBS );
        hlmsPbs->setListener( mHlmsPbsTerraShadows );
    }

    void TerrainGame::DestroyTerrain()
    {
        LogO("---- destroy Terrain");
        Root *root = mGraphicsSystem->getRoot();
        Hlms *hlmsPbs = root->getHlmsManager()->getHlms( HLMS_PBS );

        if( hlmsPbs->getListener() == mHlmsPbsTerraShadows )
        {
            hlmsPbs->setListener( 0 );
            delete mHlmsPbsTerraShadows;  mHlmsPbsTerraShadows = 0;
        }
        delete mTerra;  mTerra = 0;
    }


    //  Plane
    //-----------------------------------------------------------------------------------------------------------------------------
    void TerrainGame::CreatePlane()
    {
        sizeXZ = 2000.0f;
        v1::MeshPtr planeMeshV1 = v1::MeshManager::getSingleton().createPlane(
            "Plane v1", ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
            Plane( Vector3::UNIT_Y, 1.0f ), sizeXZ, sizeXZ,
            10, 10, true, 1, 40.0f, 40.0f, Vector3::UNIT_Z,
            v1::HardwareBuffer::HBU_STATIC, v1::HardwareBuffer::HBU_STATIC );

        planeMesh = MeshManager::getSingleton().createByImportingV1(
            "Plane", ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
            planeMeshV1.get(), true, true, true );

        SceneManager *mgr = mGraphicsSystem->getSceneManager();
        SceneNode *rootNode = mgr->getRootSceneNode( SCENE_STATIC );

        planeItem = mgr->createItem( planeMesh, SCENE_STATIC );
        planeItem->setDatablock( "Ground" );
        
        planeNode = rootNode->createChildSceneNode( SCENE_STATIC );
        planeNode->setPosition( 0, 0, 0 );
        planeNode->attachObject( planeItem );
    }

    void TerrainGame::DestroyPlane()
    {
        LogO("---- destroy Plane");
        SceneManager *mgr = mGraphicsSystem->getSceneManager();
        // if (planeMesh)
        // {   MeshManager::getSingleton().destroy();  // todo: ?
        //     planeMesh = 0;  }
        if (planeItem)
        {   mgr->destroyItem(planeItem);  planeItem = 0;  }
        if (planeNode)
        {   mgr->destroySceneNode(planeNode);  planeNode = 0;  }
    }


    //  Sky dome
    //-----------------------------------------------------------------------------------------------------------------------------
    void TerrainGame::CreateSkyDome(String sMater, float yaw)
    {
        if (moSky)  return;
    	Vector3 scale = 25000 /*view_distance*/ * Vector3::UNIT_SCALE;
        
        SceneManager *mgr = mGraphicsSystem->getSceneManager();
        ManualObject* m = mgr->createManualObject();
        m->begin(sMater, OT_TRIANGLE_LIST);

        //  divisions- quality
        int ia = 32*2, ib = 24,iB = 24 +1/*below_*/, i=0;
        
        //  angles, max
        const Real B = Math::HALF_PI, A = Math::TWO_PI;
        Real bb = B/ib, aa = A/ia;  // add
        Real a,b;
        ia += 1;

        //  up/dn y  )
        for (b = 0.f; b <= B+bb/*1*/*iB; b += bb)
        {
            Real cb = sinf(b), sb = cosf(b);
            Real y = sb;

            //  circle xz  o
            for (a = 0.f; a <= A; a += aa, ++i)
            {
                Real x = cosf(a)*cb, z = sinf(a)*cb;
                m->position(x,y,z);

                m->textureCoord(a/A, b/B);

                if (a > 0.f && b > 0.f)  // rect 2tri
                {
                    m->index(i-1);  m->index(i);     m->index(i-ia);
                    m->index(i-1);  m->index(i-ia);  m->index(i-ia-1);
                }
            }
        }
        m->end();
        moSky = m;

        Aabb aab(Vector3(0,0,0), Vector3(1,1,1)*1000000);
        m->setLocalAabb(aab);  // always visible
        //m->setRenderQueueGroup(230);  //?
        m->setCastShadows(false);

        ndSky = mgr->getRootSceneNode()->createChildSceneNode();
        ndSky->attachObject(m);  // SCENE_DYNAMIC
        ndSky->setScale(scale);
        Quaternion q;  q.FromAngleAxis(Degree(-yaw), Vector3::UNIT_Y);
        ndSky->setOrientation(q);

        //  Change the addressing mode to wrap  ?
        /*Root *root = mGraphicsSystem->getRoot();
        Hlms *hlms = root->getHlmsManager()->getHlms( HLMS_UNLIT );
        HlmsUnlitDatablock *datablock = static_cast<HlmsUnlitDatablock*>(hlms->getDatablock(sMater));
        // HlmsPbsDatablock *datablock = static_cast<HlmsPbsDatablock*>(m->getDatablock() );
        HlmsSamplerblock samplerblock( *datablock->getSamplerblock( PBSM_DIFFUSE ) );  // hard copy
        samplerblock.mU = TAM_WRAP;
        samplerblock.mV = TAM_WRAP;
        samplerblock.mW = TAM_WRAP;
        datablock->setSamplerblock( PBSM_DIFFUSE, samplerblock );
        datablock->setSamplerblock( PBSM_NORMAL, samplerblock );
        datablock->setSamplerblock( PBSM_ROUGHNESS, samplerblock );
        datablock->setSamplerblock( PBSM_METALLIC, samplerblock );/**/
    }

    void TerrainGame::DestroySkyDome()
    {
        LogO("---- destroy SkyDome");
        SceneManager *mgr = mGraphicsSystem->getSceneManager();
        if (moSky)
        {   mgr->destroyManualObject(moSky);  moSky = 0;  }
        if (ndSky)
        {   mgr->destroySceneNode(ndSky);  ndSky = 0;  }
    }

}