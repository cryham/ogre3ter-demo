
#ifndef _Demo_TutorialGameState_H_
#define _Demo_TutorialGameState_H_

#include "OgrePrerequisites.h"
#include "GameState.h"

namespace Ogre
{
    namespace v1
    {
        class TextAreaOverlayElement;
    }
}

namespace Demo
{
    class GraphicsSystem;
    class CameraController;

    ///  Base game state for the tutorials
    class TutorialGameState : public GameState
    {
    protected:
        GraphicsSystem      *mGraphicsSystem = 0;

        CameraController    *mCameraController = 0;

        Ogre::uint16        mHelpMode = 0;
        Ogre::uint16        mNumHelpModes = 3;

        Ogre::v1::TextAreaOverlayElement *mDebugText = 0;
        Ogre::v1::TextAreaOverlayElement *mDebugTextShadow = 0;

        virtual void createDebugTextOverlay();
        virtual void generateDebugText( float timeSinceLast, Ogre::String &outText );

    public:
        TutorialGameState( const Ogre::String &helpDescription );
        virtual ~TutorialGameState();

        void _notifyGraphicsSystem( GraphicsSystem *graphicsSystem );

        virtual void createScene01();

        virtual void update( float timeSinceLast );

        virtual void keyPressed( const SDL_KeyboardEvent &arg );
        virtual void keyReleased( const SDL_KeyboardEvent &arg );

        virtual void mouseMoved( const SDL_Event &arg );
    };
}

#endif
