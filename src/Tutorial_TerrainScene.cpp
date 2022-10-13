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

    //  Trees
	//-----------------------------------------------------------------------------------
    void Tutorial_TerrainGameState::CreateTrees()
	{
        //  Trees  ------------------------------------------------
        SceneManager *sceneManager = mGraphicsSystem->getSceneManager();
        SceneNode *rootNode = sceneManager->getRootSceneNode( SCENE_STATIC );
   #if 0
        HlmsPbsDatablock *pbsdatablock = (HlmsPbsDatablock*)hlmsManager->getDatablock( "pine2norm" );
        pbsdatablock->setTwoSidedLighting( true );  //?
        //pbsdatablock->setMacroblock( macroblockWire );
    #endif

        // const int all = 3, use = 1, ofs = 2;  // test one
        // const int all = 3, use = 3, ofs = 0;  // all
        const int all = 3, use = 2, ofs = 0;  // two

        const String strMesh[all] =
        {   //  meshTool -v2 -l 10 -d 100 -p 11 jungle_tree.mesh
            "jungle_tree-lod8.mesh",
            //  meshTool -v2 -l 8 -d 200 -p 10 palm2.mesh
            "palm2-lod8.mesh",
            //  meshTool -v2 -l 9 -d 100 -p 9 pine2_tall_norm.mesh
            "pine2_tall_norm-lod9.mesh",  // 10,9, 11,5
        };
        const Real scales[all] = { 1.f, 2.5f, 0.8};

		//const int dim = 76;  // 
		const int dim = 46;  // 8650
		//const int dim = 26;  // 2800
		//const int dim = 12;  // 625
        const float step = 45.f;
		
        for (int i=-dim; i<=dim; ++i)
        {
            for (int j=-dim; j<=dim; ++j)
            {
                int n = rand() % use + ofs;
                //int n = abs(i+j) % all;
				//  Item  ----
				Item *item = sceneManager->createItem(
                    strMesh[n],
					ResourceGroupManager::AUTODETECT_RESOURCE_GROUP_NAME,
					SCENE_STATIC );
                //item->setDatablock( "pine2norm" );
                item->setRenderQueueGroup( 200 );  // after terrain
                //item->setRenderingDistance(2500);  //** par  how far visible
				vegetItems.push_back(item);

				//  Node  ----
				SceneNode *sceneNode = rootNode->createChildSceneNode( SCENE_STATIC );
				sceneNode->attachObject( item );
                
                //  scale
                Real s = (rand()%1000*0.001f * 2.f + 3.f) * scales[n];
				sceneNode->scale( s, s, s );
                
                //  pos
				Vector3 objPos{ i*step, 0.f, j*step };
            #if 1
				objPos += Vector3(
					(rand()%1000-1000)*0.001f*step,
                    !mTerra ? 0.f :  // plane
                    (rand()%1000-1000)*0.001f*step, // h
					(rand()%1000-1000)*0.001f*step);
            #endif
                if (mTerra)
                    mTerra->getHeightAt( objPos );
                objPos.y += std::min( item->getLocalAabb().getMinimum().y, Real(0.0f) ) * -0.1f - 0.1f;  // par
                sceneNode->setPosition( objPos );
                
                //  rot
                Degree k( (rand()%3600) * 0.1f );
                Quaternion q;  q.FromAngleAxis( k, Vector3::UNIT_Y );
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
        ManualObject * m;
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

}
