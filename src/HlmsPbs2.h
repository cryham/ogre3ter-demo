#pragma once
#include <OgreHlms.h>
#include <OgreHlmsPbs.h>
#include <OgreArchive.h>
#include <OgreHlmsPbsDatablock.h>
#include <rapidjson/document.h>


//  HLMS override  used instead of default Pbs

//  our Pbs  ----------------
class HlmsPbs2 : public Ogre::HlmsPbs
{
public:
	HlmsPbs2( Ogre::Archive *dataFolder, Ogre::ArchiveVec *libraryFolders );
	~HlmsPbs2() override;
	
	//  setProperty for our shader types
	void calculateHashForPreCreate(
		Ogre::Renderable *renderable, Ogre::PiecesMap *inOutPieces ) override;

	void calculateHashForPreCaster(
		 Ogre::Renderable *renderable, Ogre::PiecesMap *inOutPieces ) override;

	//  ctor  new HlmsPbsDb2
	// Ogre::HlmsDatablock *createDatablockImpl(
	// 	Ogre::IdString name,
	// 	const Ogre::HlmsMacroblock *macro,
	// 	const Ogre::HlmsBlendblock *blend,
	// 	const Ogre::HlmsParamVec &params ) override;

	//  save  adds our types
	// void _saveJson(const Ogre::HlmsDatablock *datablock, Ogre::String &outString,
	// 	Ogre::HlmsJsonListener *listener, const Ogre::String &additionalTextureExtension) const override;

	// void _loadJson(const rapidjson::Value &jsonValue, const Ogre::HlmsJson::NamedBlocks &blocks,
	// 	Ogre::HlmsDatablock *datablock, const Ogre::String &resourceGroup,
	// 	Ogre::HlmsJsonListener *listener, const Ogre::String &additionalTextureExtension ) const override;
};
