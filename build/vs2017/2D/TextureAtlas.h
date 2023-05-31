#pragma once

#include "SubTexture.h"
#include "Armature.h"
#include <vector>
#include <map>
#include <string.h>

class TextureAtlas
{
public:
	TextureAtlas();
	~TextureAtlas();

	//container of subtextures
	std::map<std::string, SubTexture> subTextures; //std::StringId
	
	Armature* armature;

	std::string name;
	float width;
	float height;
};

