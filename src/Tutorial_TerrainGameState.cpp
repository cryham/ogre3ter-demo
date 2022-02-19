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

#include "OgreItem.h"
#include "OgreLogManager.h"
#include "OgreMesh.h"
#include "OgreMeshManager.h"
#include "OgreMesh2.h"
#include "OgreMeshManager2.h"


using namespace Demo;
using namespace Ogre;

#define LogO(s)  LogManager::getSingleton().logMessage(s)


namespace Demo
{
    Tutorial_TerrainGameState::Tutorial_TerrainGameState( const String &helpDescription ) :
        TutorialGameState( helpDescription ),
        mLockCameraToGround( false ),
        mTimeOfDay( Math::PI * 0.55f ),  // par
        mAzimuth( 0 ),
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

        // Render terrain after most objects, to improve performance by taking advantage of early Z
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

        SceneNode *rootNode = sceneManager->getRootSceneNode( SCENE_STATIC );
        SceneNode *sceneNode = rootNode->createChildSceneNode( SCENE_STATIC );
        sceneNode->attachObject( mTerra );

        LogO("---- Terra attach");

        HlmsManager *hlmsManager = root->getHlmsManager();
        HlmsDatablock *datablock = hlmsManager->getDatablock( "TerraExampleMaterial" );
        #if 0  //** terrain wireframe
            datablock = hlmsManager->getHlms( HLMS_USER3 )->getDefaultDatablock();
            HlmsMacroblock macroblock;
            macroblock.mPolygonMode = PM_WIREFRAME;
            datablock->setMacroblock( macroblock );
        #endif
        mTerra->setDatablock( datablock );

        {
            mHlmsPbsTerraShadows = new HlmsPbsTerraShadows();
            mHlmsPbsTerraShadows->setTerra( mTerra );
            //Set the PBS listener so regular objects also receive terrain shadows
            Hlms *hlmsPbs = root->getHlmsManager()->getHlms( HLMS_PBS );
            hlmsPbs->setListener( mHlmsPbsTerraShadows );
        }

        LogO("---- new light");

        //  Light  ------------------------------------------------
        mSunLight = sceneManager->createLight();
        SceneNode *lightNode = rootNode->createChildSceneNode();
        lightNode->attachObject( mSunLight );
        mSunLight->setPowerScale( Math::PI );
        mSunLight->setType( Light::LT_DIRECTIONAL );
        mSunLight->setDirection( Vector3( -1, -1, -1 ).normalisedCopy() );

        sceneManager->setAmbientLight( ColourValue( 0.33f, 0.61f, 0.98f ) * 0.01f,
                                       ColourValue( 0.02f, 0.53f, 0.96f ) * 0.01f,
                                       Vector3::UNIT_Y );

        mCameraController = new CameraController( mGraphicsSystem, false );
        mGraphicsSystem->getCamera()->setFarClipDistance( 100000.0f );
        mGraphicsSystem->getCamera()->setPosition( -10.0f, 80.0f, 10.0f );


#if 1
        //  Mesh  ------------------------------------------------
        LogO("---- new mesh");
        auto name = 
            "tudorhouse.mesh";
            //"Blob.mesh";
            //"knot.mesh";
            //"sphere.mesh";
        MeshUtils::importV1Mesh( name,
                                 ResourceGroupManager::AUTODETECT_RESOURCE_GROUP_NAME );

        //Create some meshes to show off terrain shadows.
        Item *item = sceneManager->createItem( name
            ResourceGroupManager::AUTODETECT_RESOURCE_GROUP_NAME, SCENE_STATIC );
        Vector3 objPos( 3.5f, 4.5f, -2.0f );
        mTerra->getHeightAt( objPos );
        objPos.y += -std::min( item->getLocalAabb().getMinimum().y, Real(0.0f) ) * 0.01f - 0.5f;
        sceneNode = rootNode->createChildSceneNode( SCENE_STATIC, objPos );
        sceneNode->scale( 0.01f, 0.01f, 0.01f );
        sceneNode->attachObject( item );

        item = sceneManager->createItem( name
            ResourceGroupManager::AUTODETECT_RESOURCE_GROUP_NAME, SCENE_STATIC );
        objPos = Vector3( -3.5f, 4.5f, -2.0f );
        mTerra->getHeightAt( objPos );
        objPos.y += -std::min( item->getLocalAabb().getMinimum().y, Real(0.0f) ) * 0.01f - 0.5f;
        sceneNode = rootNode->createChildSceneNode( SCENE_STATIC, objPos );
        sceneNode->scale( 0.01f, 0.01f, 0.01f );
        sceneNode->attachObject( item );
#endif

        TutorialGameState::createScene01();
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
        mSunLight->setDirection( Quaternion( Radian(mAzimuth), Vector3::UNIT_Y ) *
                                 Vector3( cosf( mTimeOfDay ), -sinf( mTimeOfDay ), 0.0 ).normalisedCopy() );

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

            RenderSystem *renderSystem = mGraphicsSystem->getRoot()->getRenderSystem();  //**
            const RenderingMetrics& rm = renderSystem->getMetrics();
            outText +=
                " b " + StringConverter::toString( rm.mBatchCount ) + 
                " f " + StringConverter::toString( rm.mFaceCount/1000 ) + 
                "k v " + StringConverter::toString( rm.mVertexCount/1000 ) + 
                "k d " + StringConverter::toString( rm.mDrawCount ) + 
                " i " + StringConverter::toString( rm.mInstanceCount ) + "\n";

            outText += "\nF2 Lock Ground: [";
            outText += mLockCameraToGround ? "Yes]" : "No]";

            outText += "\n+ - Pitch  ";
            outText += StringConverter::toString( mTimeOfDay * 180.0f / Math::PI );
            outText += "\n/ * Yaw ";
            outText += StringConverter::toString( mAzimuth * 180.0f / Math::PI );
            
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
        {
            mTimeOfDay += 0.1f;
            mTimeOfDay = std::min( mTimeOfDay, (float)Math::PI );
        }
        else if( arg.keysym.scancode == SDL_SCANCODE_MINUS ||
                 arg.keysym.scancode == SDL_SCANCODE_KP_MINUS )
        {
            mTimeOfDay -= 0.1f;
            mTimeOfDay = std::max( mTimeOfDay, 0.0f );
        }

        if( arg.keysym.scancode == SDL_SCANCODE_KP_9 )
        {
            mAzimuth += 0.1f;
            mAzimuth = fmodf( mAzimuth, Math::TWO_PI );
        }
        else if( arg.keysym.scancode == SDL_SCANCODE_KP_6 )
        {
            mAzimuth -= 0.1f;
            mAzimuth = fmodf( mAzimuth, Math::TWO_PI );
            if( mAzimuth < 0 )
                mAzimuth = Math::TWO_PI + mAzimuth;
        }

        if( arg.keysym.scancode == SDL_SCANCODE_F2 )
            mLockCameraToGround = !mLockCameraToGround;
        else
            TutorialGameState::keyReleased( arg );
    }
}
