#include "Tutorial_TerrainGameState.h"
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
#include "OgreParticleSystem.h"

#include "OgreMesh.h"
#include "OgreMeshManager.h"
#include "OgreMesh2.h"
#include "OgreMeshManager2.h"
#include "OgreManualObject2.h"

using namespace Demo;
using namespace Ogre;


namespace Demo
{

    //  Trees setup
    void Tutorial_TerrainGameState::SetupTrees()
    {
        vegetLayers.clear();   //  OgreMeshTool args
        // sc min, max, dens, down
        vegetLayers.emplace_back(VegetLayer("jungle_tree-lod8.mesh",
            3.0f, 5.0f, 10.f, -0.1f, 5000, 0 ));  //  -v2 -l 10 -d 100 -p 11 jungle_tree.mesh
        vegetLayers.emplace_back(VegetLayer("palm2-lod8.mesh",
            7.5f,12.5f, 8.f,  -0.1f, 5000, 0 ));  //  -v2 -l 8 -d 200 -p 10 palm2.mesh

        vegetLayers.emplace_back(VegetLayer("plant_tropical-lod6.mesh",
            4.5f, 7.5f, 30.f, 0.5f, 1000, 0 ));  //  -v2 -l 6 -d 200 -p 15 plant_tropical.mesh
        vegetLayers.emplace_back(VegetLayer("fern-lod6.mesh",
            0.6f, 1.0f, 55.f, 1.0f, 600, 0 ));  //  -v2 -l 6 -d 200 -p 15 fern.mesh
        vegetLayers.emplace_back(VegetLayer("fern2-lod6.mesh",
            0.6f, 1.0f, 50.f, 1.0f, 600, 0 ));  //  -v2 -l 8 -d 200 -p 10 palm2.mesh

        vegetLayers.emplace_back(VegetLayer("rock02brown2flat.mesh",
            1.1f, 5.0f, 5.0f, 1.0f, 3000, 1 ));  //  -v2 -l 6 -d 200 -p 15 rock*.mesh
        vegetLayers.emplace_back(VegetLayer("rock25dark2Harsh2.mesh",
            0.6f, 3.0f, 5.0f, 1.0f, 3000, 1 ));
        vegetLayers.emplace_back(VegetLayer("rock30grayGreen.mesh",
            2.1f, 6.0f, 5.0f, 1.0f, 3000, 1 ));
        vegetLayers.emplace_back(VegetLayer("rock37brGr1tall.mesh",
            1.1f, 3.0f, 5.0f, 1.0f, 3000, 1 ));
        // vegetLayers.emplace_back(VegetLayer("rock18black3.mesh",
        //     1.6f,7.f, 5.f ));
        // vegetLayers.emplace_back(VegetLayer("rock_B02.mesh",
        //     0.5f,2.f, 5.f ));

        //vegetLayers.emplace_back(VegetLayer("pine2_tall_norm-lod9.mesh",
        //    2.5f,4.f, 10.f ));  //  -v2 -l 9 -d 100 -p 9 pine2_tall_norm.mesh
    }

    //  Trees
    //-----------------------------------------------------------------------------------------------------------------------------
    void Tutorial_TerrainGameState::CreateTrees()
	{
        SceneManager *mgr = mGraphicsSystem->getSceneManager();
        SceneNode *rootNode = mgr->getRootSceneNode( SCENE_STATIC );
    #if 0
        HlmsPbsDatablock *pbsdatablock = (HlmsPbsDatablock*)hlmsManager->getDatablock( "pine2norm" );
        pbsdatablock->setTwoSidedLighting( true );  //?
        //pbsdatablock->setMacroblock( macroblockWire );
    #endif

		const Real mult  = !mTerra ? 0.1f : 1.f;
        const Real scale = sizeXZ * 0.49f;  // world
		
        for (auto& lay : vegetLayers)
        {
            int cnt = mult * lay.density * 200.f;  // count, fake..
            for (int i=0; i < cnt; ++i)
            {
                ++lay.count;
				//  Item  ----
                Item *item;
                try 
				{   item = mgr->createItem( lay.mesh,
					    ResourceGroupManager::AUTODETECT_RESOURCE_GROUP_NAME,
					    SCENE_STATIC );
                }catch(...)
                {
                    LogO("error: mesh not found: " + lay.mesh);
                    return;
                }
                //item->setDatablock( "pine2norm" );
                item->setRenderQueueGroup( 200 );  // after terrain
                item->setRenderingDistance( lay.visFar );  // how far visible
				vegetItems.push_back(item);

				//  Node  ----
				SceneNode *node = rootNode->createChildSceneNode( SCENE_STATIC );
				node->attachObject( item );
                
                //  scale
                Real s = Math::RangeRandom(lay.scaleMin, lay.scaleMax);
				node->scale( s, s, s );
                
                //  pos
				Vector3 objPos = Vector3(
                    Math::RangeRandom(-scale, scale), 0.f,
                    Math::RangeRandom(-scale, scale));
                if (mTerra)
                    mTerra->getHeightAt( objPos );
                objPos.y += std::min( item->getLocalAabb().getMinimum().y, Real(0.0f) ) * -0.1f + lay.down;  //par
                node->setPosition( objPos );
                
                //  rot
                Degree ang( Math::RangeRandom(0, 360.f) );
                Quaternion q;  q.FromAngleAxis( ang, Vector3::UNIT_Y );
                if (lay.rotAll)
                {
                    Degree ang( Math::RangeRandom(0, 180.f) );  //par ? range
                    Quaternion p;  p.FromAngleAxis( -ang, Vector3::UNIT_Z );
                    node->setOrientation( p * q );
                }else
                    node->setOrientation( q );

				vegetNodes.push_back(node);
			}
		}
	}

    //-----------------------------------------------------------------------------------
    void Tutorial_TerrainGameState::DestroyTrees()
	{
        SceneManager *mgr = mGraphicsSystem->getSceneManager();
		
		for (auto node : vegetNodes)
			mgr->destroySceneNode(node);
		vegetNodes.clear();
		
		for (auto item : vegetItems)
			mgr->destroyItem(item);
		vegetItems.clear();
	}


    //  Terrain
    //-----------------------------------------------------------------------------------------------------------------------------
    void Tutorial_TerrainGameState::CreateTerrain()
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

    void Tutorial_TerrainGameState::DestroyTerrain()
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
    void Tutorial_TerrainGameState::CreatePlane()
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

    void Tutorial_TerrainGameState::DestroyPlane()
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
    void Tutorial_TerrainGameState::CreateSkyDome(String sMater, float yaw)
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

    void Tutorial_TerrainGameState::DestroySkyDome()
    {
        LogO("---- destroy SkyDome");
        SceneManager *mgr = mGraphicsSystem->getSceneManager();
        if (moSky)
        {   mgr->destroyManualObject(moSky);  moSky = 0;  }
        if (ndSky)
        {   mgr->destroySceneNode(ndSky);  ndSky = 0;  }
    }


    //  Particles
    //-----------------------------------------------------------------------------------------------------------------------------
    void Tutorial_TerrainGameState::CreateParticles()
    {
        SceneManager *mgr = mGraphicsSystem->getSceneManager();
        SceneNode *rootNode = mgr->getRootSceneNode( SCENE_STATIC );
        LogO("---- new Particles");

        Vector3 camPos = mGraphicsSystem->getCamera()->getPosition();
        Vector3 dir = mGraphicsSystem->getCamera()->getDirection();
        camPos += dir * 40.f;

        for (int i=0; i < 2; ++i)  // 20
        {
            ParticleSystem* parSys = mgr->createParticleSystem(
                i%2 ? "Smoke" : "Fire");
            //parHit->setVisibilityFlags(RV_Particles);
            SceneNode* node = rootNode->createChildSceneNode();
            node->attachObject( parSys );
            parSys->setRenderQueueGroup( 225 );  //? after trees

            Vector3 objPos = camPos + Vector3( i/2 * 2.f, -5.f + i%2 * 4.f, 0.f);
            // if (mTerra)
            //     objPos.y += mTerra->getHeightAt( objPos ) + 5.f;
            node->setPosition( objPos );
            //parHit->getEmitter(0)->setEmissionRate(20);
        }
    }


    //  Car
    //-----------------------------------------------------------------------------------------------------------------------------
    void Tutorial_TerrainGameState::CreateCar()
    {
        SceneManager *mgr = mGraphicsSystem->getSceneManager();
        SceneNode *rootNode = mgr->getRootSceneNode( SCENE_STATIC );

        Vector3 camPos = mGraphicsSystem->getCamera()->getPosition();
        Vector3 dir = mGraphicsSystem->getCamera()->getDirection();
        camPos += dir * 40.f;

        const String cars[nCars] = { "ES", "SX", "HI" };
        const String car = cars[iCar];
        ++iCar;  if (iCar == nCars)  iCar = 0;  // next

        const int carParts = 3;
        const String carPart[carParts] = {"_body.mesh", "_interior.mesh", "_glass.mesh"}, sWheel = "_wheel.mesh";

        //  scale
        Real s = 6.f;

        //  pos
        Vector3 objPos = camPos, pos = objPos;
        if (mTerra)
            mTerra->getHeightAt( pos );
        Real ymin = !mTerra ? 0.85f * s : pos.y + 0.68f * s;
        // if (objPos.y < ymin)
            objPos.y = ymin;

        for (int i=0; i < carParts; ++i)
        {
            Item *item = mgr->createItem( car + carPart[i],
                ResourceGroupManager::AUTODETECT_RESOURCE_GROUP_NAME, SCENE_STATIC );

            SceneNode *node = rootNode->createChildSceneNode( SCENE_STATIC );
            node->attachObject( item );
            if (i==2)
                item->setRenderQueueGroup( 202 );  // glass after trees
            
            node->scale( s, s, s );
            
            node->setPosition( objPos );
            
            //  rot
            Quaternion q;  q.FromAngleAxis( Degree(180), Vector3::UNIT_Z );
            node->setOrientation( q );
        }

        //  wheels
        for (int i=0; i < 4; ++i)
        {
            Item *item = mgr->createItem( car + sWheel,
                ResourceGroupManager::AUTODETECT_RESOURCE_GROUP_NAME, SCENE_STATIC );

            SceneNode *node = rootNode->createChildSceneNode( SCENE_STATIC );
            node->attachObject( item );
            
            node->scale( s, s, s );
            
            if (car == "SX")
                node->setPosition( objPos + s * Vector3(
                    i/2 ? -1.29f : 1.28f,  -0.34f,
                    i%2 ? -0.8f : 0.8f ) );
            else if (car == "HI")
                node->setPosition( objPos + s * Vector3(
                    i/2 ? -1.29f : 1.30f,  -0.34f,   // front+
                    i%2 ? -0.88f : 0.88f ) );
            if (car == "ES")
                node->setPosition( objPos + s * Vector3(
                    i/2 ? -1.21f : 1.44f,  -0.36f,
                    i%2 ? -0.71f : 0.71f ) );
            
            //  rot
            Quaternion q;  q.FromAngleAxis( Degree(-180), Vector3::UNIT_Z );
            Quaternion r;  r.FromAngleAxis( Degree(i%2 ? 90 : -90), Vector3::UNIT_Y );
            node->setOrientation( r * q );
        }
    }


    //  Manual object
    //-----------------------------------------------------------------------------------------------------------------------------
    void Tutorial_TerrainGameState::CreateManualObj(Ogre::Vector3 camPos)
	{
        SceneManager *mgr = mGraphicsSystem->getSceneManager();

        LogO("---- new Manual object");
        ManualObject *m;
        std::vector<Vector3> mVertices;

        m = mgr->createManualObject();
        m->begin("jungle_tree", OT_TRIANGLE_LIST);
        //m->begin("ParSmoke", OT_TRIANGLE_LIST);

        //m->beginUpdate(0);
        const size_t GridSize = 15;
        const float GridStep = 1.0f / GridSize;

        for (size_t i = 0; i < GridSize; i++)
        {
            for (size_t j = 0; j < GridSize; j++)
            {
                mVertices.push_back(Vector3(GridStep * i,       GridStep * j,       0.00f));
                mVertices.push_back(Vector3(GridStep * (i + 1), GridStep * j,       0.00f));
                mVertices.push_back(Vector3(GridStep * i,       GridStep * (j + 1), 0.00f * j));
                mVertices.push_back(Vector3(GridStep * (i + 1), GridStep * (j + 1), 0.00f * i));
            }
        }

        float uvOffset = 0.f;
        {
            for (size_t i = 0; i < mVertices.size(); )
            {
                m->position(mVertices[i]);
                m->normal(0.0f, 1.0f, 0.0f);
                m->tangent(1.0f, 0.0f, 0.0f);
                m->textureCoord(0.0f + uvOffset, 0.0f + uvOffset);

                m->position(mVertices[i + 1]);
                m->normal(0.0f, 1.0f, 0.0f);
                m->tangent(1.0f, 0.0f, 0.0f);
                m->textureCoord(1.0f + uvOffset, 0.0f + uvOffset);

                m->position(mVertices[i + 2]);
                m->normal(0.0f, 1.0f, 0.0f);
                m->tangent(1.0f, 0.0f, 0.0f);
                m->textureCoord(0.0f + uvOffset, 1.0f + uvOffset);

                m->position(mVertices[i + 3]);
                m->normal(0.0f, 1.0f, 0.0f);
                m->tangent(1.0f, 0.0f, 0.0f);
                m->textureCoord(1.0f + uvOffset, 1.0f + uvOffset);

                m->quad(i, i + 1, i + 3, i + 2);
                i += 4;
            }
        }
        m->end();

        SceneNode *sceneNodeGrid = mgr->getRootSceneNode( SCENE_DYNAMIC )->createChildSceneNode( SCENE_DYNAMIC );
        sceneNodeGrid->attachObject(m);
        sceneNodeGrid->scale(4.0f, 4.0f, 4.0f);
        Vector3 objPos = camPos + Vector3( 0.f, -5.f, -10.f);
        sceneNodeGrid->translate(objPos, SceneNode::TS_LOCAL);
	}

}
