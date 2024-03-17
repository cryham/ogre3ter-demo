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
    class PlanarReflections;
}

namespace Demo
{
    class PlanarReflectWsListener;

    struct VegetLayer
    {
        std::string mesh;
        float scaleMin, scaleMax, density;
        float angleMax, heightMax, waterMax;  // terrain
        float visFar, down;  // visibility max, down offset
        bool rotAll;  // allow all axes rotation
        int count;  // auto
        // range ter angle, height..
        
        VegetLayer(std::string mesh1,
                float scMin, float scMax,
                float dens, float angMax, float hMax, float wtrMax,
                float dn, float vis, bool rot)
            : mesh(mesh1), scaleMin(scMin), scaleMax(scMax), density(dens)
            , angleMax(angMax), heightMax(hMax), waterMax(wtrMax)
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
        Ogre::Vector3 camPos, sunDir;

        //  input
        int mKeys[4] = {0,0,0,0};  // sun
        bool left = false, right = false;  // arrows
        bool shift = false, ctrl = false;
        
        int param = -1;  // to adjust, fog etc
        
        int preset = 0;  // scene to create

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
        friend PlanarReflectWsListener;

        //  Reflection cube  ----
        Ogre::Camera *mCubeCamera = 0;
        Ogre::TextureGpu *mDynamicCubemap = 0;
        Ogre::CompositorWorkspace *mDynamicCubemapWorkspace = 0;
        int updReflSkip = 0;

        IblQuality mIblQuality = IblLow;  // par in ctor
        Ogre::CompositorWorkspace *setupCompositor();


        //  water  ----
		Ogre::Real yWaterHeight = 100.f;
        Ogre::String waterMaterial{"WaterBlue"};
       	void CreateWater(), DestroyWater();
        void CreateWaterRefract();

        Ogre::PlanarReflections *mPlanarReflect = 0;
        PlanarReflectWsListener *mWorkspaceListener = 0;

        Ogre::Item *waterItem = 0;
        Ogre::SceneNode *waterNode = 0;
		Ogre::MeshPtr waterMesh = 0;
        Ogre::v1::MeshPtr waterMeshV1 = 0;


        //  plane ground  ----
        void CreatePlane(), DestroyPlane();

        Ogre::Item *planeItem = 0;
        Ogre::SceneNode *planeNode = 0;
        Ogre::v1::MeshPtr planeMeshV1 = 0;
        Ogre::MeshPtr planeMesh = 0;


        //  Terrain  ----
        Ogre::Real sizeXZ = 1000.f, sizeY = 100.f, sizeXZ2 = 5000.f, sizeY2 = 100.f;
        void CreateTerrain(), DestroyTerrain();
        Ogre::SceneNode *nodeTerrain = 0;
        Ogre::Terra *mTerra = 0, *mTerra2 = 0;

        bool mTriplanarMappingEnabled = true;
        void ToggleTriplanar();

        // Listener to make PBS objects also be affected by terrain's shadows
        Ogre::HlmsPbsTerraShadows *mHlmsPbsTerraShadows = 0;

        //  wireframe
        Ogre::HlmsMacroblock macroblockWire;
        bool wireTerrain = false;
        void ToggleWireframe();

        //  terrain util
        Ogre::Real getHeight( Ogre::Real x, Ogre::Real z ) const;
        Ogre::Real getAngle( Ogre::Real x, Ogre::Real z, Ogre::Real s ) const;


        void CreateScene(int pre);

        //  sun
        Ogre::Light *mSunLight = 0;
        Ogre::SceneNode *mSunNode = 0;

        //  Sky  ----
        void CreateSkyDome(Ogre::String sMater, float yaw);
        int iSky = 0;
        Ogre::ManualObject* moSky = 0;
        Ogre::SceneNode* ndSky = 0;
        void DestroySkyDome();


        //  vegetation  ----
        void SetupVeget(bool pines), CreateVeget(), DestroyVeget();

        std::vector<VegetLayer> vegetLayers;
		std::vector<Ogre::Item*> vegetItems;
    	std::vector<Ogre::SceneNode*> vegetNodes;

        //  other
        void CreateManualObj(Ogre::Vector3 camPos);

        void CreateParticles(), DestroyParticles();
        std::vector<Ogre::ParticleSystem*> particles;
        std::vector<Ogre::SceneNode*> particleNodes;
        
        
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
