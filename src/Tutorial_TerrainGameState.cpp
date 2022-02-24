/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2021 Torus Knot Software Ltd

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
-----------------------------------------------------------------------------
*/

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

#define LogO(s)  LogManager::getSingleton().logMessage(s)


namespace Demo
{
    Tutorial_TerrainGameState::Tutorial_TerrainGameState( const String &helpDescription ) :
        TutorialGameState( helpDescription ),
        mLockCameraToGround( false ),
        mPitch( Math::PI * 0.55f ),  // par
        mYaw( 0 ),
        mTerra( 0 ),
        mSunLight( 0 ),
        mHlmsPbsTerraShadows( 0 )
    {
    }

    
    //-----------------------------------------------------------------------------------
    void Tutorial_TerrainGameState::createScene01()
    {
        Root *root = mGraphicsSystem->getRoot();
        SceneManager *sceneManager = mGraphicsSystem->getSceneManager();
        
        HlmsManager *hlmsManager = root->getHlmsManager();
        HlmsDatablock *datablock = 0;
        HlmsMacroblock macroblockWire;
        macroblockWire.mPolygonMode = PM_WIREFRAME;
        
        SceneNode *rootNode = sceneManager->getRootSceneNode( SCENE_STATIC );

        LogManager::getSingleton().setLogDetail(LoggingLevel::LL_BOREME);

        LogO("---- createScene");

        RenderSystem *renderSystem = root->getRenderSystem();  //**
        renderSystem->setMetricsRecordingEnabled( true );


        LogO("---- new Terra");

        // Render terrain after most objects, to improve performance by taking advantage of early Z

    #if 0  //** 1 terrain
        mTerra = new Terra( Id::generateNewId<MovableObject>(),
                                  &sceneManager->_getEntityMemoryManager( SCENE_STATIC ),
                                  sceneManager, 11u, root->getCompositorManager2(),
                                  mGraphicsSystem->getCamera(), false );
        mTerra->setCastShadows( false );

        LogO("---- Terra load");

        //  Heightmap  ------------------------------------------------
        //  64 flat
        //mTerra->load( "Heightmap64.png", Vector3( 64.0f, 4096.0f * 0.15f, 64.0f ), Vector3( 12096.0f, 6096.0f, 12096.0f ), false, false );
        //  1k  600 fps  (2 tex)
        mTerra->load( "Heightmap.png", Vector3( 64.0f, 4096.0f * 0.5f, 64.0f ), Vector3( 4096.0f, 4096.0f, 4096.0f ), false, false );
        //  2k  260 fps
        //mTerra->load( "Heightmap2c.png", Vector3( 64.0f, 4096.0f * 0.15f, 64.0f ), Vector3( 12096.0f, 6096.0f, 12096.0f ), false, false );
        //  4k  93 fps
        //mTerra->load( "Heightmap4.png", Vector3( 64.0f, 4096.0f * 0.5f, 64.0f ), Vector3( 4096.0f, 4096.0f, 4096.0f ), false, false );

        SceneNode *sceneNode = rootNode->createChildSceneNode( SCENE_STATIC );
        sceneNode->attachObject( mTerra );

        LogO("---- Terra attach");

        datablock = hlmsManager->getDatablock( "TerraExampleMaterial" );
        #if 0  //** terrain wireframe
            datablock = hlmsManager->getHlms( HLMS_USER3 )->getDefaultDatablock();
            datablock->setMacroblock( macroblockWire );
        #endif
        mTerra->setDatablock( datablock );

        {
            mHlmsPbsTerraShadows = new HlmsPbsTerraShadows();
            mHlmsPbsTerraShadows->setTerra( mTerra );
            //Set the PBS listener so regular objects also receive terrain shadows
            Hlms *hlmsPbs = root->getHlmsManager()->getHlms( HLMS_PBS );
            hlmsPbs->setListener( mHlmsPbsTerraShadows );
        }
    #else
        //  Plane  ------------------------------------------------
        v1::MeshPtr planeMeshV1 = v1::MeshManager::getSingleton().createPlane( "Plane v1",
            ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
            Plane( Vector3::UNIT_Y, 1.0f ), 2000.0f, 2000.0f,
            10, 10, true, 1, 40.0f, 40.0f, Vector3::UNIT_Z,
            v1::HardwareBuffer::HBU_STATIC, v1::HardwareBuffer::HBU_STATIC );

        MeshPtr planeMesh = MeshManager::getSingleton().createByImportingV1(
            "Plane", ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
            planeMeshV1.get(), true, true, true );
        {
            Item *item = sceneManager->createItem( planeMesh, SCENE_STATIC );
            item->setDatablock( "Ground" );
            SceneNode *sceneNode = rootNode->createChildSceneNode( SCENE_STATIC );
            sceneNode->setPosition( 0, 0, 0 );
            sceneNode->attachObject( item );

            //  Change the addressing mode to wrap
            assert( dynamic_cast<HlmsPbsDatablock*>( item->getSubItem(0)->getDatablock() ) );
            HlmsPbsDatablock *datablock = static_cast<HlmsPbsDatablock*>(item->getSubItem(0)->getDatablock() );
            HlmsSamplerblock samplerblock( *datablock->getSamplerblock( PBSM_DIFFUSE ) );  // hard copy
            samplerblock.mU = TAM_WRAP;
            samplerblock.mV = TAM_WRAP;
            samplerblock.mW = TAM_WRAP;
            datablock->setSamplerblock( PBSM_DIFFUSE, samplerblock );
            datablock->setSamplerblock( PBSM_NORMAL, samplerblock );
            datablock->setSamplerblock( PBSM_ROUGHNESS, samplerblock );
            datablock->setSamplerblock( PBSM_METALLIC, samplerblock );/**/
        }
    #endif

        LogO("---- new light");

        //  Light  ------------------------------------------------
        mSunLight = sceneManager->createLight();
        SceneNode *lightNode = rootNode->createChildSceneNode();
        lightNode->attachObject( mSunLight );
        mSunLight->setPowerScale( Math::PI * 2 );  //** par! 1.5 2 3, should be 1..
        mSunLight->setType( Light::LT_DIRECTIONAL );
        mSunLight->setDirection( Vector3( 0, -1, 0 ).normalisedCopy() );  //-

        //  ambient  set in update ..
        sceneManager->setAmbientLight(
            ColourValue( 0.63f, 0.61f, 0.28f ) * 0.04f,
            ColourValue( 0.52f, 0.63f, 0.76f ) * 0.04f,
            // ColourValue( 0.33f, 0.61f, 0.98f ) * 0.01f,
            // ColourValue( 0.02f, 0.53f, 0.96f ) * 0.01f,
            Vector3::UNIT_Y );


        //  camera  ------------------------------------------------
        mCameraController = new CameraController( mGraphicsSystem, false );
        mGraphicsSystem->getCamera()->setFarClipDistance( 100000.f );  // far

        //Vector3 camPos(-10.f, 80.f, 10.f );
        //Vector3 camPos(-2005.f, 40.f, -929.f);
        Vector3 camPos(-52.f, mTerra ? 735.f : 60.f, mTerra ? 975.f : 517.f);
        //camPos.y += mTerra->getHeightAt( camPos );
        mGraphicsSystem->getCamera()->setPosition( camPos );
        mGraphicsSystem->getCamera()->lookAt( camPos + Vector3(0.f, -0.5f, -1.f) );
        Vector3 objPos;

#if 0
        //  Car  ------------------------------------------------
        const int carParts = 3;
        const String cars[carParts] = {
            "ES_body.mesh", "ES_interior.mesh", "ES_glass.mesh",
            //"XZ_body.mesh", "XZ_interior.mesh", "XZ_glass.mesh",
        };
        for (int i=0; i < carParts; ++i)
        {
            Item *item = sceneManager->createItem( cars[i],
                ResourceGroupManager::AUTODETECT_RESOURCE_GROUP_NAME, SCENE_STATIC );
            //item->setDatablock( "pine2norm" );

            SceneNode *sceneNode = rootNode->createChildSceneNode( SCENE_STATIC );
            sceneNode->attachObject( item );
            if (i==2)
                item->setRenderQueueGroup( 202 );  // after trees
            
            //  scale
            Real s = 6.f;
            sceneNode->scale( s, s, s );
            
            //  pos
            objPos = camPos + Vector3(0.f, -20.f, -140.f);
            if (mTerra)
                objPos.y += mTerra->getHeightAt( objPos ) + 5.f;
            //objPos.y += std::min( item->getLocalAabb().getMinimum().y, Real(0.0f) ) * -0.1f + 0.1f;  // par
            sceneNode->setPosition( objPos );
            
            //  rot
            Degree k( 180 );  // ES
            //Degree k( 0 );  // XZ
            Quaternion q;  q.FromAngleAxis( k, Vector3::UNIT_Z );
            sceneNode->setOrientation( q );
        }
#endif

#if 0
        //  Particles  ------------------------------------------------
        LogO("---- new Particles");

        for (int i=0; i < 2; ++i)  // 20
        {
            ParticleSystem* parSys = sceneManager->createParticleSystem(
                i%2 ? "Smoke" : "Fire");
            //parHit->setVisibilityFlags(RV_Particles);
            SceneNode* node = rootNode->createChildSceneNode();
            node->attachObject( parSys );
            parSys->setRenderQueueGroup( 105 );  //? after trees

            objPos = camPos + Vector3( i/2 * 2.f, -5.f + i%2 * 4.f, -10.f);
            node->setPosition( objPos );
            //parHit->getEmitter(0)->setEmissionRate(20);
        }
#endif

#if 1
        //  Trees  ------------------------------------------------
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

		//const int dim = 46;  // 8650
		//const int dim = 26;  // 2800
		const int dim = 12;  // 625
        const float step = 45.f;
		
        for (int i=-dim; i<=dim; ++i)
        {
            for (int j=-dim; j<=dim; ++j)
            {
                int n = rand() % use + ofs;
                //int n = abs(i+j) % all;
				Item *item = sceneManager->createItem(
                    strMesh[n],
					ResourceGroupManager::AUTODETECT_RESOURCE_GROUP_NAME,
					SCENE_STATIC );
                //item->setDatablock( "pine2norm" );
                item->setRenderQueueGroup( 200 );  // after terrain
                //item->setRenderingDistance(1500);  //** par  how far visible

				SceneNode *sceneNode = rootNode->createChildSceneNode( SCENE_STATIC );
				sceneNode->attachObject( item );
                
                //  scale
                Real s = (rand()%1000*0.001f * 2.f + 3.f) * scales[n];
				sceneNode->scale( s, s, s );
                
                //  pos
				objPos = Vector3( i*step, 0.f, j*step );
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
			}
		}
#endif

#if 0
        //  Manual object  ------------------------------------------------
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
                mVertices.push_back(Vector3(GridStep * i, GridStep * j, 0.0f));
                mVertices.push_back(Vector3(GridStep * (i + 1), GridStep * j, 0.0f));
                mVertices.push_back(Vector3(GridStep * i, GridStep * (j + 1), 0.0f));
                mVertices.push_back(Vector3(GridStep * (i + 1), GridStep * (j + 1), 0.0f));
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
        objPos = camPos + Vector3( 0.f, -5.f, -10.f);
        sceneNodeGrid->translate(objPos, SceneNode::TS_LOCAL);
#endif

#if 1
        //  Sky Dome  ------------------------------------------------
        CreateSkyDome("sky-clearday1", 0.f);
        //CreateSkyDome("sky_photo6", 0.f);  // clouds
#endif

        LogO("---- tutorial createScene");

        TutorialGameState::createScene01();
    }


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
    }


    //-----------------------------------------------------------------------------------
    void Tutorial_TerrainGameState::destroyScene()
    {
        LogO("---- destroyScene");
        Root *root = mGraphicsSystem->getRoot();
        Hlms *hlmsPbs = root->getHlmsManager()->getHlms( HLMS_PBS );

        //Unset the PBS listener and destroy it
        if( hlmsPbs->getListener() == mHlmsPbsTerraShadows )
        {
            hlmsPbs->setListener( 0 );
            delete mHlmsPbsTerraShadows;
            mHlmsPbsTerraShadows = 0;
        }

        delete mTerra;
        mTerra = 0;

        LogO("---- tutorial destroyScene");

        TutorialGameState::destroyScene();
    }


    //-----------------------------------------------------------------------------------
    void Tutorial_TerrainGameState::update( float timeSinceLast )
    {
        int d = mKeys[0] - mKeys[1];
        if (d)
        {
            mPitch += d * 0.6f * timeSinceLast;
            mPitch = std::max( 0.f, std::min( mPitch, (float)Math::PI ) );
        }

        d = mKeys[2] - mKeys[3];
        if (d)
        {
            mYaw += d * 1.5f * timeSinceLast;
            mYaw = fmodf( mYaw, Math::TWO_PI );
            if( mYaw < 0.f )
                mYaw = Math::TWO_PI + mYaw;
        }

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

        //Do not call update() while invisible, as it will cause an assert because the frames
        //are not advancing, but we're still mapping the same GPU region over and over.
        if (mTerra && mGraphicsSystem->getRenderWindow()->isVisible() )
        {
            //Force update the shadow map every frame to avoid the feeling we're "cheating" the
            //user in this sample with higher framerates than what he may encounter in many of
            //his possible uses.
            const float lightEpsilon = 0.0001f;  //** 0.f slow
            mTerra->update( mSunLight->getDerivedDirectionUpdated(), lightEpsilon );
        }

        TutorialGameState::update( timeSinceLast );

        //Camera must be locked to ground *after* we've moved it. Otherwise
        //fast motion may go below the terrain for 1 or 2 frames.
        Camera *camera = mGraphicsSystem->getCamera();
        Vector3 camPos = camera->getPosition();
        if( mLockCameraToGround && mTerra->getHeightAt( camPos ) )
            camera->setPosition( camPos + Vector3::UNIT_Y * 10.0f );
    }


    //-----------------------------------------------------------------------------------
    void Tutorial_TerrainGameState::generateDebugText( float timeSinceLast, String &outText )
    {
        TutorialGameState::generateDebugText( timeSinceLast, outText );

        if( mDisplayHelpMode == 0 )
        {
            outText += "\nCtrl+F4 will reload Terra's shaders.";
        }
        else if( mDisplayHelpMode == 1 )
        {
            char tmp[128];
            LwString str( LwString::FromEmptyPointer(tmp, sizeof(tmp)) );
            Vector3 camPos = mGraphicsSystem->getCamera()->getPosition();

            using namespace Ogre;

            RenderSystem *renderSystem = mGraphicsSystem->getRoot()->getRenderSystem();
            const RenderingMetrics& rm = renderSystem->getMetrics();  //**
            outText +=
                "f " + StringConverter::toString( rm.mFaceCount/1000 ) + 
                "k v " + StringConverter::toString( rm.mVertexCount/1000 ) + 
                "k d " + StringConverter::toString( rm.mDrawCount ) + 
                " i " + StringConverter::toString( rm.mInstanceCount ) + 
                " b " + StringConverter::toString( rm.mBatchCount ) + "\n";

            outText += "\nF2 Lock Ground: [";
            outText += mLockCameraToGround ? "Yes]" : "No]";

            outText += "\n+ - Pitch  ";
            outText += StringConverter::toString( mPitch * 180.0f / Math::PI );
            outText += "\n/ * Yaw ";
            outText += StringConverter::toString( mYaw * 180.0f / Math::PI );
            
            outText += "\n\nCamera: ";
            str.a( "", LwString::Float( camPos.x, 2, 2 ), " ",
                       LwString::Float( camPos.y, 2, 2 ), " ",
                       LwString::Float( camPos.z, 2, 2 ), "" );
            outText += str.c_str();
            outText += "\nLightDir: ";
            str.clear();
            str.a( "", LwString::Float( mSunLight->getDirection().x, 2, 2 ), " ",
                       LwString::Float( mSunLight->getDirection().y, 2, 2 ), " ",
                       LwString::Float( mSunLight->getDirection().z, 2, 2 ), "" );
            outText += str.c_str();
        }
    }


    //-----------------------------------------------------------------------------------
    void Tutorial_TerrainGameState::keyPressed( const SDL_KeyboardEvent &arg )
    {
        if( arg.keysym.scancode == SDL_SCANCODE_KP_PLUS )
            mKeys[0] = 1;
        if( arg.keysym.scancode == SDL_SCANCODE_KP_MINUS )
            mKeys[1] = 1;

        if( arg.keysym.scancode == SDL_SCANCODE_KP_MULTIPLY )
            mKeys[2] = 1;
        if( arg.keysym.scancode == SDL_SCANCODE_KP_DIVIDE )
            mKeys[3] = 1;

        TutorialGameState::keyPressed( arg );
    }
    
    void Tutorial_TerrainGameState::keyReleased( const SDL_KeyboardEvent &arg )
    {
        if( arg.keysym.sym == SDLK_F4 && (arg.keysym.mod & (KMOD_LCTRL|KMOD_RCTRL)) )
        {
            //Hot reload of Terra shaders.
            Root *root = mGraphicsSystem->getRoot();
            HlmsManager *hlmsManager = root->getHlmsManager();

            Hlms *hlms = hlmsManager->getHlms( HLMS_USER3 );
            GpuProgramManager::getSingleton().clearMicrocodeCache();
            hlms->reloadFrom( hlms->getDataFolder() );
        }
        else if( (arg.keysym.mod & ~(KMOD_NUM|KMOD_CAPS)) != 0 )
        {
            TutorialGameState::keyReleased( arg );
            return;
        }

        if( arg.keysym.scancode == SDL_SCANCODE_KP_PLUS )
            mKeys[0] = 0;
        if( arg.keysym.scancode == SDL_SCANCODE_KP_MINUS )
            mKeys[1] = 0;

        if( arg.keysym.scancode == SDL_SCANCODE_KP_MULTIPLY )
            mKeys[2] = 0;
        if( arg.keysym.scancode == SDL_SCANCODE_KP_DIVIDE )
            mKeys[3] = 0;


        if( arg.keysym.scancode == SDL_SCANCODE_F2 )
            mLockCameraToGround = !mLockCameraToGround;
        else
            TutorialGameState::keyReleased( arg );
    }
}
