#pragma once
#include <system/application.h>
#include <maths/vector2.h>
#include <graphics/sprite.h>
#include <input/keyboard.h>
#include <vector>
#include <map>

//rapidjson include
#include "rapidjson\document.h"

//Skeleton includes
#include <2D/TextureAtlas.h>
#include <2D/SubTexture.h>
#include <2D/Armature.h>
#include <2D/SkinSlot.h>

// FRAMEWORK FORWARD DECLARATIONS
namespace gef
{
	class Platform;
	class SpriteRenderer;
}

class Skeleton2DAnimation
{
public:
	Skeleton2DAnimation();
	~Skeleton2DAnimation();

	void Init(gef::Platform& platform);
	void CleanUp();
	bool Update(float frame_time);
	void Render(gef::SpriteRenderer* spriteRenderer);
	void Input(const gef::Keyboard* keyInput, gef::Platform& platform);

	TextureAtlas* ReadJSONTextureAtlas(rapidjson::Document& tex, rapidjson::Document& skeDoc);
	SubTexture* ReadJSONSubTexture(const rapidjson::Value& subTextureArray);
	Armature* ReadJSONArmature(rapidjson::Document& skeDoc);

	Animation2D* ReadJSONAnimBones(rapidjson::Document& skeDoc, int i);
	TranslationKey* ReadJSONTranslationKeyData(const rapidjson::Value& arrayPos);
	RotationKey* ReadJSONRotationKeyData(const rapidjson::Value& arrayPos);

	std::map<std::string, SkinSlot> ReadJSONSkinSlot(rapidjson::Document& skeDoc);
	std::map<std::string, Bone> ReadJSONBones(rapidjson::Document& skeDoc);

	void BuildWorldTransform(std::map<std::string, Bone>* bones);
	void CalculateWorldBoneTransform(std::map<std::string, Bone>* bones, Animation2D* anim, int curFrame);

	void SetUpSpriteProperties(gef::Sprite* sprite, float screenX, float screenY, TextureAtlas* texAtlas, int frame);
	void SetUpSpriteProperties(gef::Sprite* sprite, float screenX, float screenY, TextureAtlas* texAtlas, int frame, Armature* arma, std::string partName);

	void ParseJSON(const char* texPath, const char* skePath, gef::Platform& platform, bool bones);

	//getters
	int GetCurrentFrame() { return curFrame; }
	int GetCurrentAnim() { return anim; }
	float GetAnimTime() { return timeBetweenFrame; }
	bool GetCharacter() { return character; }

private:
	void ChangeCharacter();

	gef::Vector2 Lerp(gef::Vector2 startPos, gef::Vector2 endPos, float time);
	float Lerp(float startAngle, float endAngle, float time);

	//vector to store multiple animations
	std::vector<TextureAtlas> textureAtlases;
	TextureAtlas curTexAtlas;

	gef::SpriteRenderer* sprite_renderer_;
	gef::Texture* sprite_texture_[2];
	gef::Sprite sprite;
	gef::Vector2 sprite_pos_;

	//Bones specific
	typedef std::pair<std::string, SkinSlot> strSSlot;
	typedef std::pair<std::string, Bone> strBone;
	std::vector<std::string> parts;

	gef::Matrix33 rigTransform;

	bool usingBones;

	int curFrame = 0;
	int anim = 0;

	float animTimer;
	float animTime;
	float timeBetweenFrame;
	bool character;

	const float scale = 0.5f;
};

