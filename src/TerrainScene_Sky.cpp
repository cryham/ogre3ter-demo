#include "TerrainGame.h"
#include "CameraController.h"
#include "GraphicsSystem.h"

#include "OgreSceneManager.h"
#include "OgreRoot.h"
#include "OgreLogManager.h"

#include "OgreCommon.h"
#include "OgreItem.h"
#include "OgreMesh.h"
#include "OgreMeshManager.h"
#include "OgreMesh2.h"
#include "OgreMeshManager2.h"
#include "OgreManualObject2.h"

#include "OgreHlmsPbs.h"
#include "OgreHlmsPbsDatablock.h"
#include "OgreHlmsCommon.h"
#include "OgreHlmsUnlit.h"
#include "OgreHlmsUnlitDatablock.h"

using namespace Demo;
using namespace Ogre;


namespace Demo
{

    //  Plane
    //-----------------------------------------------------------------------------------------------------------------------------
    void TerrainGame::CreatePlane()
    {
        sizeXZ = 2000.0f;
        planeMeshV1 = v1::MeshManager::getSingleton().createPlane(
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
        LogO("==== destroy Plane");
        SceneManager *mgr = mGraphicsSystem->getSceneManager();
        if (planeNode)
        {   mgr->destroySceneNode(planeNode);  planeNode = 0;  }
        if (planeItem)
        {   mgr->destroyItem(planeItem);  planeItem = 0;  }
        
		auto& ms = MeshManager::getSingleton();
		auto& m1 = v1::MeshManager::getSingleton();
        if (planeMesh)  ms.remove(planeMesh);  planeMesh.reset();
        if (planeMeshV1)  m1.remove(planeMeshV1);  planeMeshV1.reset();
    }


    //  Sky dome
    //-----------------------------------------------------------------------------------------------------------------------------
    void TerrainGame::CreateSkyDome(String sMtr, float yaw)
    {
        if (moSky)  return;
        //** par  view_distance
    	Vector3 scale = 35000 * Vector3::UNIT_SCALE;
        
        SceneManager *mgr = mGraphicsSystem->getSceneManager();
        ManualObject* m = mgr->createManualObject(SCENE_STATIC);
        m->begin(sMtr, OT_TRIANGLE_LIST);

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

        ndSky = mgr->getRootSceneNode(SCENE_STATIC)->createChildSceneNode(SCENE_STATIC);
        ndSky->attachObject(m);  // SCENE_DYNAMIC
        ndSky->setScale(scale);
        Quaternion q;  q.FromAngleAxis(Degree(-yaw), Vector3::UNIT_Y);
        ndSky->setOrientation(q);

        //  set wrap
        HlmsSamplerblock sb;
        sb.mMinFilter = FO_ANISOTROPIC;  sb.mMagFilter = FO_ANISOTROPIC;
        sb.mMipFilter = FO_LINEAR;       sb.mMaxAnisotropy = 4;
        sb.mU = TAM_MIRROR;  sb.mV = TAM_MIRROR;  sb.mW = TAM_MIRROR;

        HlmsUnlit* hlms = (HlmsUnlit*) Ogre::Root::getSingleton().getHlmsManager()->getHlms( HLMS_UNLIT );
        HlmsUnlitDatablock* db = (HlmsUnlitDatablock*) hlms->getDatablock(sMtr);
	
        db->setSamplerblock( 0, sb );
    

        //  set sun pitch, yaw for sky  ------
        const float d2r = Math::PI / 180.f;
        if (sMtr=="cloudy_04_blue"    ){  mPitch = 33 * d2r;  mYaw = d2r * 10;  } else
        if (sMtr=="day_clouds_02_blue"){  mPitch = 32 * d2r;  mYaw = d2r * -1;  } else
        if (sMtr=="day_clouds_04_blue"){  mPitch = 48 * d2r;  mYaw = d2r * -35;  } else
        if (sMtr=="day_clouds_07"     ){  mPitch = 34 * d2r;  mYaw = d2r * 0;  } else
        if (sMtr=="day_clouds_08"     ){  mPitch = 33 * d2r;  mYaw = d2r * 0;  }
    }

    void TerrainGame::DestroySkyDome()
    {
        LogO("==== destroy SkyDome");
        SceneManager *mgr = mGraphicsSystem->getSceneManager();
        if (moSky)
        {   mgr->destroyManualObject(moSky);  moSky = 0;  }
        if (ndSky)
        {   mgr->destroySceneNode(ndSky);  ndSky = 0;  }
    }

}
