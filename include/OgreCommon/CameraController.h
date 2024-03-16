
#ifndef _Demo_CameraController_H_
#define _Demo_CameraController_H_

#include "OgrePrerequisites.h"
#include "TutorialGameState.h"
#include <OgreVector3.h>

namespace Demo
{
    class CameraController
    {
        // bool    mUseNode = 0;
        bool    mShift = 0;
        bool    mCtrl = 0;
        bool    mWASDQE[6] = {0,0,0,0,0,0};
        float   mYaw = 0.f;
        float   mPitch = 0.f;

        //  state vars, are smoothed
        Ogre::Radian  sYaw{0}, sPitch{0};
        Ogre::Vector3  sMove{0,0,0};
        double time = 0.0;
    public:
        float   mSpeed, mInertia;

    private:
        GraphicsSystem*  mGraphicsSystem;

    public:
        CameraController( GraphicsSystem *graphicsSystem, bool useSceneNode=false );

        void update( float timeSinceLast );

        bool keyPressed( const SDL_KeyboardEvent &arg );
        bool keyReleased( const SDL_KeyboardEvent &arg );

        void mouseMoved( const SDL_Event &arg );
    };
}

#endif
