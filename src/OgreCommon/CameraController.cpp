#include "CameraController.h"
#include "GraphicsSystem.h"

#include "OgreCamera.h"
#include "OgreWindow.h"

using namespace Demo;
using namespace Ogre;

namespace Demo
{

    CameraController::CameraController( GraphicsSystem *graphicsSystem, bool useSceneNode ) :
        // mUseNode( useSceneNode ),
        // mSpeed( 0.5f ), mInertia( 0.96f ),  // slow
        mSpeed( 0.5f ), mInertia( 0.2f ),
        mGraphicsSystem( graphicsSystem )
    {
    }

    //-----------------------------------------------------------------------------------
    void CameraController::update( float fDT )
    {
        Ogre::Camera *camera = mGraphicsSystem->getCamera();

    #if 0  //  old no smooth  ----------
        if( mCameraYaw != 0.0f || mCameraPitch != 0.0f )
        {
            if( mUseSceneNode )
            {
                Ogre::Node *cameraNode = camera->getParentNode();

                // Update now as yaw needs the derived orientation.
                cameraNode->_getFullTransformUpdated();
                cameraNode->yaw( Ogre::Radian( mCameraYaw ), Ogre::Node::TS_WORLD );
                cameraNode->pitch( Ogre::Radian( mCameraPitch ) );
            }else
            {
                camera->yaw( Ogre::Radian( mCameraYaw ) );
                camera->pitch( Ogre::Radian( mCameraPitch ) );
            }
            mCameraYaw   = 0.0f;
            mCameraPitch = 0.0f;
        }

        int camMoveZ = mWASDQE[2] - mWASDQE[0];
        int camMoveX = mWASDQE[3] - mWASDQE[1];
        int camMoveY = mWASDQE[5] - mWASDQE[4];

        if( camMoveZ || camMoveX || camMoveY )
        {
            Ogre::Vector3 camMoveDir( camMoveX, camMoveY, camMoveZ );
            camMoveDir.normalise();
            camMoveDir *= fDT * mCameraBaseSpeed
                * (mSpeed1 ? mCameraSpeed1 : mSpeed2 ? mCameraSpeed2 : 1.f);

            if( mUseSceneNode )
            {
                Ogre::Node *cameraNode = camera->getParentNode();
                cameraNode->translate( camMoveDir, Ogre::Node::TS_LOCAL );
            }
            else
                camera->moveRelative( camMoveDir );
        }
    #else
        //  speeds  ----------
        const Real moveMul = 1000.f, rotMul = 6300.f;
        const Real mulMove = mShift ? 0.2f : mCtrl ? 4.f : 1.f;
        const Real mulRot  = mShift ? 0.3f : mCtrl ? 2.f : 1.f;
        
        //  inputs
        const Real rotX = mYaw;    mYaw   = 0.f;
        const Real rotY = mPitch;  mPitch = 0.f;
        const Vector3 vTrans(
            mWASDQE[3] - mWASDQE[1], mWASDQE[5] - mWASDQE[4], mWASDQE[2] - mWASDQE[0]);

        //  const intervals, Fps independent smooth
        const double ivDT = 0.004;
        time += fDT;
        while (time > ivDT)
        {	time -= ivDT;
        
            Real fSmooth = (powf(1.0f - mInertia, 2.2f) * 40.f + 0.1f) * ivDT;

            const Real fMove = mSpeed * mulMove;
            const Degree fRot = Degree(mSpeed) * mulRot;

            Radian inYaw = rotMul * ivDT * (fRot* rotX);
            Radian inPth = rotMul * ivDT * (fRot* rotY);
            Vector3 inMove = moveMul * ivDT * (fMove * vTrans);

            sYaw   += (inYaw - sYaw) * fSmooth;
            sPitch += (inPth - sPitch) * fSmooth;
            sMove  += (inMove - sMove) * fSmooth;

            camera->yaw( sYaw );
            camera->pitch( sPitch );
            camera->moveRelative( sMove );
        }
    #endif
    }

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wswitch"
    //  Input vars
    //-----------------------------------------------------------------------------------
    bool CameraController::keyPressed( const SDL_KeyboardEvent &arg )
    {
        switch (arg.keysym.scancode)
        {
        case SDL_SCANCODE_LSHIFT:  mShift = true;  return true;
        case SDL_SCANCODE_LCTRL:   mCtrl = true;  return true;

        case SDL_SCANCODE_W:  mWASDQE[0] = true;  return true;
        case SDL_SCANCODE_A:  mWASDQE[1] = true;  return true;
        case SDL_SCANCODE_S:  mWASDQE[2] = true;  return true;
        case SDL_SCANCODE_D:  mWASDQE[3] = true;  return true;
        case SDL_SCANCODE_Q:  mWASDQE[4] = true;  return true;
        case SDL_SCANCODE_E:  mWASDQE[5] = true;  return true;
        }    
        return false;
    }

    //-----------------------------------------------------------------------------------
    bool CameraController::keyReleased( const SDL_KeyboardEvent &arg )
    {
        switch (arg.keysym.scancode)
        {
        case SDL_SCANCODE_LSHIFT:  mShift = false;  return true;
        case SDL_SCANCODE_LCTRL:   mCtrl = false;  return true;

        case SDL_SCANCODE_W:  mWASDQE[0] = false;  return true;
        case SDL_SCANCODE_A:  mWASDQE[1] = false;  return true;
        case SDL_SCANCODE_S:  mWASDQE[2] = false;  return true;
        case SDL_SCANCODE_D:  mWASDQE[3] = false;  return true;
        case SDL_SCANCODE_Q:  mWASDQE[4] = false;  return true;
        case SDL_SCANCODE_E:  mWASDQE[5] = false;  return true;
        }    
        return false;
    }
#pragma GCC diagnostic pop

    //-----------------------------------------------------------------------------------
    void CameraController::mouseMoved( const SDL_Event &arg )
    {
        float width  = static_cast<float>( mGraphicsSystem->getRenderWindow()->getWidth() );
        float height = static_cast<float>( mGraphicsSystem->getRenderWindow()->getHeight() );

        mYaw   += -arg.motion.xrel / width  *6;
        mPitch += -arg.motion.yrel / height *6;
    }
}
