#pragma once

#include <2D/SkinSlot.h>
#include <2D/Bone.h>
#include <vector>
#include <map>

struct TranslationKey
{
	float nextKeyTime;
	float startTime;
	gef::Vector2 translation;
};

struct RotationKey
{
	float nextKeyTime;
	float startTime;
	float rotation;
};

struct BoneKey
{
	std::string name;
	std::vector<TranslationKey*> transKeys;
	std::vector<RotationKey*> rotKeys;
};

struct Animation2D
{
	float duration;
	std::string name;
	std::vector<BoneKey*> boneKeys;
};

class Armature
{
public:
	Armature();
	~Armature();

	//Skinslot and skeleton data
	std::map<std::string, SkinSlot> skinSlots;
	std::map<std::string, Bone> bones;
	std::vector<Animation2D*> animations;
	std::vector<std::string> order;
	std::string name;

	float frameRate;
	float duration;
};

