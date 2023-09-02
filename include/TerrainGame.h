#ifndef _Demo_TerrainGame_H_
#define _Demo_TerrainGame_H_

#include "OgreHlmsDatablock.h"
#include "OgrePrerequisites.h"
#include "TutorialGameState.h"

#define LogO(s)  LogManager::getSingleton().logMessage(s)


namespace Ogre
{
    class Terra;
    class HlmsPbsTerraShadows;
	class Ocean;
}

namespace Demo
{

    struct VegetLayer
    {
        std::string mesh;
        float scaleMin, scaleMax, density;
        float angleMax, heightMax;  // terrain
        float visFar, down;  // visibility max, down offset
        bool rotAll;  // allow all axes rotation
        int count;  // auto
        // range ter angle, height..
        
        VegetLayer(std::string mesh1,
                float scMin, float scMax,
                float dens, float angMax, float hMax,
                float dn, float vis, bool rot)
            : mesh(mesh1), scaleMin(scMin), scaleMax(scMax), density(dens)
            , angleMax(angMax), heightMax(hMax)
            , visFar(vis), down(dn), rotAll(rot), count(0)
        {   }
    };

    enum IblQuality
    {
        MipmapsLowest,
        IblLow,
        IblHigh
    };


    class TerrainGame : public TutorialGameState
    {
        //  vars
        float mPitch;  // sun dir
        float mYaw;
        Ogre::Vector3 camPos;

        //  input
        int mKeys[4] = {0,0,0,0};  // sun keys
        int param = 0;  // to adjust
        bool left = false, right = false;  // arrows
        bool shift = false, ctrl = false;

        //  terrain
        Ogre::Terra *mTerra = 0;
        Ogre::Light *mSunLight = 0;
        Ogre::SceneNode *mSunNode = 0;

		Ogre::Ocean *mOcean = 0;
        bool mTriplanarMappingEnabled = true;
        void ToggleTriplanar();

        // Listener to make PBS objects also be affected by terrain's shadows
        Ogre::HlmsPbsTerraShadows *mHlmsPbsTerraShadows = 0;

        //  wireframe
        Ogre::HlmsMacroblock macroblockWire;
        bool wireTerrain = false;
        void ToggleWireframe();

        //  Fps info etc
        void generateDebugText( float timeSinceLast, Ogre::String &outText ) override;

    public:
        TerrainGame( const Ogre::String &helpDescription );

        //  main
        void createScene01() override;
        void destroyScene() override;

        void update( float timeSinceLast ) override;

        //  events
        void keyPressed( const SDL_KeyboardEvent &arg ) override;
        void keyReleased( const SDL_KeyboardEvent &arg ) override;
    
    protected:

        //  Reflection cube  ----
        Ogre::Camera *mCubeCamera = 0;
        Ogre::TextureGpu *mDynamicCubemap = 0;
        Ogre::CompositorWorkspace *mDynamicCubemapWorkspace = 0;
        int updReflSkip = 0;

        IblQuality mIblQuality = IblLow;  // par in ctor
        Ogre::CompositorWorkspace *setupCompositor();


        //  Terrain  ----
        Ogre::Real sizeXZ = 1000.f;
        void CreateTerrain(), DestroyTerrain();
        Ogre::SceneNode *nodeTerrain = 0;

        void CreatePlane(), DestroyPlane();
        Ogre::v1::MeshPtr planeMeshV1 = 0;
        Ogre::MeshPtr planeMesh = 0;
        Ogre::Item *planeItem = 0;
        Ogre::SceneNode *planeNode = 0;

        //  util
        Ogre::Real getHeight( Ogre::Real x, Ogre::Real z ) const;
        Ogre::Real getAngle( Ogre::Real x, Ogre::Real z, Ogre::Real s ) const;

        //  Sky  ----
        void CreateSkyDome(Ogre::String sMater, float yaw);
        int iSky = 0;
        Ogre::ManualObject* moSky = 0;
        Ogre::SceneNode* ndSky = 0;
        void DestroySkyDome();
    
        //  vegetation  ----
        void SetupVeget(), CreateVeget(), DestroyVeget();

        std::vector<VegetLayer> vegetLayers;
		std::vector<Ogre::Item*> vegetItems;
    	std::vector<Ogre::SceneNode*> vegetNodes;

        //  other
        void CreateManualObj(Ogre::Vector3 camPos);
        void CreateParticles();

        //  cars  ----
        void CreateCar();
        void DestroyCars();

		std::vector<Ogre::Item*> carItems;
    	std::vector<Ogre::SceneNode*> carNodes;
        int iCars = 0;

        int iCarType = 1;
        const static int nCarTypes = 3;
        const static Ogre::uint32 RV_Car = 2;
    };
}

#endif
