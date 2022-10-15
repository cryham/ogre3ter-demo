#include "Tutorial_TerrainGameState.h"
#include "CameraController.h"
#include "GraphicsSystem.h"
#include "Utils/MeshUtils.h"

#include "OgreSceneManager.h"

#include "OgreRoot.h"
#include "Vao/OgreVaoManager.h"
#include "Vao/OgreVertexArrayObject.h"

#include "OgreCamera.h"
#include "OgreWindow.h"

#include "Terra/Hlms/OgreHlmsTerra.h"
#include "Terra/Hlms/PbsListener/OgreHlmsPbsTerraShadows.h"
#include "Terra/Terra.h"
#include "Terra/TerraShadowMapper.h"
#include "OgreHlmsManager.h"
#include "OgreHlms.h"
#include "Compositor/OgreCompositorManager2.h"
#include "Compositor/OgreCompositorWorkspace.h"

#include "OgreTextureGpuManager.h"

#include "OgreLwString.h"
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


using namespace Demo;
using namespace Ogre;


namespace Demo
{

    void Tutorial_TerrainGameState::SetupTrees()
    {
        vegetLayers.clear();   //  OgreMeshTool args
        // sc min, max, dens, down
        vegetLayers.emplace_back(VegetLayer("jungle_tree-lod8.mesh",
            3.0f, 5.0f, 10.f, -0.1f, 5000, 0 ));  //  -v2 -l 10 -d 100 -p 11 jungle_tree.mesh
        vegetLayers.emplace_back(VegetLayer("palm2-lod8.mesh",
            7.5f,12.5f, 8.f,  -0.1f, 5000, 0 ));  //  -v2 -l 8 -d 200 -p 10 palm2.mesh

        vegetLayers.emplace_back(VegetLayer("plant_tropical-lod6.mesh",
            4.5f, 7.5f, 20.f, 0.5f, 1000, 0 ));  //  -v2 -l 6 -d 200 -p 15 plant_tropical.mesh
        vegetLayers.emplace_back(VegetLayer("fern-lod6.mesh",
            0.6f, 1.0f, 115.f, 1.0f, 600, 0 ));  //  -v2 -l 6 -d 200 -p 15 fern.mesh
        vegetLayers.emplace_back(VegetLayer("fern2-lod6.mesh",
            0.6f, 1.0f, 120.f, 1.0f, 600, 0 ));  //  -v2 -l 8 -d 200 -p 10 palm2.mesh

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
	//-----------------------------------------------------------------------------------
    void Tutorial_TerrainGameState::CreateTrees()
	{
        SceneManager *sceneManager = mGraphicsSystem->getSceneManager();
        SceneNode *rootNode = sceneManager->getRootSceneNode( SCENE_STATIC );
    #if 0
        HlmsPbsDatablock *pbsdatablock = (HlmsPbsDatablock*)hlmsManager->getDatablock( "pine2norm" );
        pbsdatablock->setTwoSidedLighting( true );  //?
        //pbsdatablock->setMacroblock( macroblockWire );
    #endif

		const Real mult = 1.f;
        const Real scaleXZ = 4095.f/2.f;  //par world
		
        for (auto& lay : vegetLayers)
        {
            int cnt = mult * lay.density * 200.f;  // count, fake..
            for (int i=0; i < cnt; ++i)
            {
                ++lay.count;
				//  Item  ----
				Item *item = sceneManager->createItem(
                    lay.mesh,
					ResourceGroupManager::AUTODETECT_RESOURCE_GROUP_NAME,
					SCENE_STATIC );
                //item->setDatablock( "pine2norm" );
                item->setRenderQueueGroup( 200 );  // after terrain
                item->setRenderingDistance( lay.visFar );  // how far visible
				vegetItems.push_back(item);

				//  Node  ----
				SceneNode *sceneNode = rootNode->createChildSceneNode( SCENE_STATIC );
				sceneNode->attachObject( item );
                
                //  scale
                Real s = Math::RangeRandom(lay.scaleMin, lay.scaleMax);
				sceneNode->scale( s, s, s );
                
                //  pos
				Vector3 objPos = Vector3(
                    Math::RangeRandom(-scaleXZ, scaleXZ), 0.f,
                    Math::RangeRandom(-scaleXZ, scaleXZ));
                if (mTerra)
                    mTerra->getHeightAt( objPos );
                objPos.y += std::min( item->getLocalAabb().getMinimum().y, Real(0.0f) ) * -0.1f + lay.down;  //par
                sceneNode->setPosition( objPos );
                
                //  rot
                Degree ang( Math::RangeRandom(0, 360.f) );
                Quaternion q;  q.FromAngleAxis( ang, Vector3::UNIT_Y );
                if (lay.rotAll)
                {
                    Degree ang( Math::RangeRandom(0, 180.f) );  //par ? range
                    Quaternion p;  p.FromAngleAxis( -ang, Vector3::UNIT_Z );
                    sceneNode->setOrientation( p * q );
                }else
                    sceneNode->setOrientation( q );

				vegetNodes.push_back(sceneNode);
			}
		}
	}

    //-----------------------------------------------------------------------------------
    void Tutorial_TerrainGameState::DestroyTrees()
	{
        SceneManager *sceneManager = mGraphicsSystem->getSceneManager();
		
		for (auto node : vegetNodes)
			sceneManager->destroySceneNode(node);
		vegetNodes.clear();
		
		for (auto item : vegetItems)
			sceneManager->destroyItem(item);
		vegetItems.clear();
	}


    //  Sky dome
	//-----------------------------------------------------------------------------------
    void Tutorial_TerrainGameState::CreateSkyDome(String sMater, float yaw)
    {
    	Vector3 scale = 25000 /*view_distance*/ * Vector3::UNIT_SCALE;
        
        SceneManager *sceneManager = mGraphicsSystem->getSceneManager();
        ManualObject* m = sceneManager->createManualObject();
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

        //AxisAlignedBox aab;  aab.setInfinite();
        //m->setBoundingBox(aab);
        Aabb aab(Vector3(0,0,0), Vector3(1,1,1)*1000000);
        m->setLocalAabb(aab);  // always visible
        //m->setRenderQueueGroup(230);  //?
        m->setCastShadows(false);

        SceneNode *ndSky = sceneManager->getRootSceneNode()->createChildSceneNode();
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


    //  Manual object
	//-----------------------------------------------------------------------------------
    void Tutorial_TerrainGameState::CreateManualObj(Ogre::Vector3 camPos)
	{
        SceneManager *sceneManager = mGraphicsSystem->getSceneManager();

        LogO("---- new Manual object");
        ManualObject *m;
        std::vector<Vector3> mVertices;

        m = sceneManager->createManualObject();
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

        SceneNode *sceneNodeGrid = sceneManager->getRootSceneNode( SCENE_DYNAMIC )->
                                     createChildSceneNode( SCENE_DYNAMIC );
        sceneNodeGrid->attachObject(m);
        sceneNodeGrid->scale(4.0f, 4.0f, 4.0f);
        Vector3 objPos = camPos + Vector3( 0.f, -5.f, -10.f);
        sceneNodeGrid->translate(objPos, SceneNode::TS_LOCAL);
	}


    //  Particles
	//-----------------------------------------------------------------------------------
    void Tutorial_TerrainGameState::CreateParticles()
    {
        SceneManager *sceneManager = mGraphicsSystem->getSceneManager();
        SceneNode *rootNode = sceneManager->getRootSceneNode( SCENE_STATIC );
        LogO("---- new Particles");

        Vector3 camPos = mGraphicsSystem->getCamera()->getPosition();
        Vector3 dir = mGraphicsSystem->getCamera()->getDirection();
        camPos += dir * 40.f;

        for (int i=0; i < 2; ++i)  // 20
        {
            ParticleSystem* parSys = sceneManager->createParticleSystem(
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
	//-----------------------------------------------------------------------------------
    void Tutorial_TerrainGameState::CreateCar()
    {
        SceneManager *sceneManager = mGraphicsSystem->getSceneManager();
        SceneNode *rootNode = sceneManager->getRootSceneNode( SCENE_STATIC );

        Vector3 camPos = mGraphicsSystem->getCamera()->getPosition();
        Vector3 dir = mGraphicsSystem->getCamera()->getDirection();
        camPos += dir * 40.f;

        const String cars[3] = { "ES", "SX", "HI" };
        const String car = cars[1];  // par

        const int carParts = 3;
        const String carPart[carParts] = {"_body.mesh", "_interior.mesh", "_glass.mesh"}, sWheel = "_wheel.mesh";

        //  scale
        Real s = 6.f;

        //  pos
        Vector3 objPos = camPos;
        Real ymin = !mTerra ? 0.85f * s : mTerra->getHeightAt( objPos ) + 12.85f * s;
        if (objPos.y < ymin)
            objPos.y = ymin;

        for (int i=0; i < carParts; ++i)
        {
            Item *item = sceneManager->createItem( car + carPart[i],
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
            Item *item = sceneManager->createItem( car + sWheel,
                ResourceGroupManager::AUTODETECT_RESOURCE_GROUP_NAME, SCENE_STATIC );

            SceneNode *node = rootNode->createChildSceneNode( SCENE_STATIC );
            node->attachObject( item );
            
            node->scale( s, s, s );
            
            //if (car == "SX")  // todo rest
            node->setPosition( objPos + s * Vector3(
                i/2 ? -1.29f : 1.28f,  -0.34f, 
                i%2 ? -0.8f : 0.8f ) );
            
            //  rot
            Quaternion q;  q.FromAngleAxis( Degree(-180), Vector3::UNIT_Z );
            Quaternion r;  r.FromAngleAxis( Degree(i%2 ? 90 : -90), Vector3::UNIT_Y );
            node->setOrientation( r * q );
        }
    }

}