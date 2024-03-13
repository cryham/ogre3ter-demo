#pragma once
#include "HlmsPbs2.h"

#include <OgreHlmsJsonPbs.h>
#include <OgreShaderPrimitives.h>
using namespace Ogre;


//  our Pbs
//----------------------------------------------------------------
HlmsPbs2::HlmsPbs2( Archive *dataFolder, ArchiveVec *libraryFolders )
	: HlmsPbs( dataFolder, libraryFolders )
{
}
HlmsPbs2::~HlmsPbs2()
{
}


void HlmsPbs2::calculateHashForPreCreate(
	Renderable *rnd, PiecesMap *inOut )
{
	HlmsPbs::calculateHashForPreCreate( rnd, inOut );
	
	const auto& mtr = rnd->getDatablockOrMaterialName();
	// const auto* db = (HlmsPbsDb2*)rnd->getDatablock();
	// LogManager::getSingleton().logMessage("- calc Pbs: " + mtr);   // on every item
	
	if (mtr.substr(0,5) == "Water")
	{
		setProperty( "water", 1 );
	}
}

void HlmsPbs2::calculateHashForPreCaster(
	Renderable *rnd, PiecesMap *inOut )
{
    HlmsPbs::calculateHashForPreCaster( rnd, inOut );

	// const auto& mtr = rnd->getDatablockOrMaterialName();
	// const auto* db = (HlmsPbsDb2*)rnd->getDatablock();
	// LogO("- cast for: " + mtr);   // on every item

	// if (mtr.substr(0,5) == "grass")
		// setProperty( "grass", 1 );
}



#if 0
//----------------------------------------------------------------
///  **Bookmarks**

HlmsPbsTerraShadows::preparePassBuffer  +

Overload Hlms::preparePassHash or HlmsListener::preparePassHash to define a custom property that follows an entirely different shader path


custom_passBuffer 	can add extra info for pass buffer (only useful if the user is using HlmsListener or overloaded HlmsPbs).
custom_materialBuffer  ..
custom_VStoPS 	Piece where users can add more interpolants for passing data from the vertex to the pixel shader.
|
custom_vs_attributes 	Custom vertex shader attributes in the Vertex Shader (i.e. a special texcoord, etc).
custom_vs_uniformDeclaration 	Data declaration (textures, texture buffers, uniform buffers) in the Vertex Shader.
custom_vs_uniformStructDeclaration  ..
custom_vs_posMaterialLoad  ..
custom_vs_preTransform  ..
custom_vs_preExecution 	Executed before Ogre code from the Vertex Shader.
custom_vs_posExecution 	Executed after all code from the Vertex Shader has been performed.
|
custom_ps_uniformDeclaration 	Same as custom_vs_uniformDeclaration, but for the Pixel Shader
custom_ps_uniformStructDeclaration  ..
custom_ps_preExecution 	Executed before Ogre code from the Pixel Shader.
custom_ps_posMaterialLoad 	Executed right after loading material data; and before anything else. May not get executed if there is no relevant material data (i.e. doesnt have normals or QTangents for lighting calculation)
custom_ps_posSampleNormal  ..
custom_ps_preLights 	Executed right before any light (i.e. to perform your own ambient / global illumination pass). All relevant texture data should be loaded by now.
custom_ps_posExecution 	Executed after all code from the Pixel Shader has been performed.
custom_ps_uv_modifier_macros 	PBS specific. Allows you to override the macros defined in Samples/Media/Hlms/Pbs/Any/UvModifierMacros_piece_ps.any so you can apply custom transformations to each UV. e.g. #undef UV_DIFFUSE #define UV_DIFFUSE( x ) ((x) * 2.0)
custom_ps_functions 	Used to declare functions outside the main body of the shader
custom_ps_pixelData 	Declare additional data in struct PixelData from Pixel Shader 


PassStructDecl

#endif
