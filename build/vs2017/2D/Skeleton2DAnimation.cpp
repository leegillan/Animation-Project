#include "Skeleton2DAnimation.h"
#include <system/platform.h>
#include <graphics/sprite_renderer.h>
#include <system/debug_log.h>
#include <graphics/sprite.h>

#include "load_texture.h"
#include "load_json.h"

Skeleton2DAnimation::Skeleton2DAnimation() :
	sprite_texture_()
{
}

Skeleton2DAnimation::~Skeleton2DAnimation()
{
}

void Skeleton2DAnimation::Init(gef::Platform& platform_)
{
	//DRAGON SETUP
	///////////////////////////////////////////
	ParseJSON("Dragon_tex.json", "Dragon_ske.json", platform_, true);
	///////////////////////////////////////////

	//FIGHTER SETUP
	//////////////////////////////////////////
	ParseJSON("boy-attack/boy-attack_tex.json", "boy-attack/boy-attack_ske.json", platform_, false);
	///////////////////////////////////////////

	animTimer = 0.0f;
	animTime = 1.0f;

	character = true;
	usingBones = false;
}

void Skeleton2DAnimation::CleanUp()
{
	for (int i = 0; i < 2; i++)
	{
		delete sprite_texture_[i];
		sprite_texture_[i] = NULL;
	}
}

bool Skeleton2DAnimation::Update(float frameTime)
{
	animTimer += frameTime;

	timeBetweenFrame = animTime / curTexAtlas.armature->frameRate;

	if (animTimer >= timeBetweenFrame)
	{
		curFrame++;
		animTimer = 0;
	}

	if (usingBones)
	{
		if (curTexAtlas.armature->animations[anim] != NULL)
		{
			if (curFrame >= curTexAtlas.armature->animations[anim]->duration)
			{
				curFrame = 0;
			}
		}

		CalculateWorldBoneTransform(&curTexAtlas.armature->bones, curTexAtlas.armature->animations[anim], curFrame);
	}
	else if (curFrame >= curTexAtlas.armature->duration)
	{
		curFrame = 0;
	}

	return true;
}

void Skeleton2DAnimation::Render(gef::SpriteRenderer* spriteRenderer)
{
	if (usingBones)
	{
		rigTransform.SetIdentity();
		rigTransform.Scale(gef::Vector2(scale, scale));
		rigTransform.SetTranslation(gef::Vector2(sprite_pos_.x, sprite_pos_.y));

		for each(std::string part in parts)
		{
			gef::Matrix33 spriteOffsetTrans = curTexAtlas.armature->skinSlots.at(part).transform;
			gef::Matrix33 worldBoneTrans = curTexAtlas.armature->bones.at(part).worldTransform;

			std::string partName = curTexAtlas.armature->skinSlots.at(part).partName;

			gef::Matrix33 subTextureTrans = curTexAtlas.subTextures.at(partName).transform;
			gef::Matrix33 localBoneTrans = curTexAtlas.armature->bones.at(part).localTransform;

			SetUpSpriteProperties(&sprite, sprite_pos_.x, sprite_pos_.y, &curTexAtlas, curFrame, curTexAtlas.armature, partName);

			// Render button icon
			spriteRenderer->DrawSprite(sprite, subTextureTrans * spriteOffsetTrans * worldBoneTrans * rigTransform);
		}
	}
	else
	{
		SetUpSpriteProperties(&sprite, sprite_pos_.x, sprite_pos_.y, &curTexAtlas, curFrame);

		//Render sprite
		spriteRenderer->DrawSprite(sprite);
	}
}

void Skeleton2DAnimation::Input(const gef::Keyboard* keyInput, gef::Platform &platform_)
{
	//checks if the dragon is the current character to cahnge its animations
	if (usingBones)
	{
		//switches between animations in skeleton
		if (keyInput->IsKeyPressed(gef::Keyboard::KC_1))
		{
			anim = 0;
			//animTime = 0.75f;
		}

		if (keyInput->IsKeyPressed(gef::Keyboard::KC_2))
		{
			anim = 1;
			//animTime = 0.75f;
		}

		if (keyInput->IsKeyPressed(gef::Keyboard::KC_3))
		{
			anim = 2;
			//animTime = 1.6f;
		}

		if (keyInput->IsKeyPressed(gef::Keyboard::KC_4))
		{
			anim = 3;
			//animTime = 0.75f;
		}
	}

	if (keyInput->IsKeyDown(gef::Keyboard::KC_I))
	{
		animTime = (animTime >= 3.0f) ? 3.0f : animTime + 0.01f;
	}

	if (keyInput->IsKeyDown(gef::Keyboard::KC_O))
	{
		animTime = (animTime <= 0.1f) ? 0.1f : animTime - 0.01f;
	}

	//toggles character to animate & reads animation data into texture atlas to be used in scene
	if (keyInput->IsKeyPressed(gef::Keyboard::KC_0))
	{
		character = !character;
		usingBones = !usingBones;

		ChangeCharacter();
	}
}

void Skeleton2DAnimation::ChangeCharacter()
{
	if (!character)
	{
		curTexAtlas = textureAtlases[0];
		sprite.set_texture(sprite_texture_[0]);
	}
	else
	{
		curTexAtlas = textureAtlases[1];
		sprite.set_texture(sprite_texture_[1]);
	}
}

TextureAtlas* Skeleton2DAnimation::ReadJSONTextureAtlas(rapidjson::Document& texDoc, rapidjson::Document& skeDoc)
{
	TextureAtlas* texAtlas = new TextureAtlas();

	texAtlas->name = texDoc["name"].GetString();
	texAtlas->width = texDoc["width"].GetFloat();
	texAtlas->height = texDoc["height"].GetFloat();

	texAtlas->armature = ReadJSONArmature(skeDoc);

	const rapidjson::Value& subTexArray = texDoc["SubTexture"];

	for (int subTexNum = 0; subTexNum < (int)subTexArray.Size(); subTexNum++)
	{
		SubTexture* subTexture = ReadJSONSubTexture(subTexArray[subTexNum]);

		std::string name = texDoc["SubTexture"][subTexNum]["name"].GetString();

		texAtlas->subTextures[name] = *subTexture;

		delete subTexture;
	}

	return texAtlas;
}

Armature* Skeleton2DAnimation::ReadJSONArmature(rapidjson::Document& skeDoc)
{
	Armature* arma = new Armature();

	arma->frameRate = skeDoc["armature"][0]["frameRate"].GetFloat();
	arma->duration = skeDoc["armature"][0]["animation"][0]["duration"].GetFloat();
	arma->name = skeDoc["armature"][0]["name"].GetString();

	if (usingBones)
	{
		const rapidjson::Value& animArray = skeDoc["armature"][0]["animation"];

		for (int i = 0; i < animArray.Size(); i++)
		{
			arma->animations.push_back(ReadJSONAnimBones(skeDoc, i));
		}
		
		const rapidjson::Value& slotArray = skeDoc["armature"][0]["slot"];

		for (int i = 0; i < slotArray.Size(); i++)
		{
			std::string order = skeDoc["armature"][0]["slot"][i]["name"].GetString();

			arma->order.push_back(order);
			parts.push_back(order);
		}
	}
	else
	{
		for (int i = 0; i < arma->duration; i++)
		{
			std::string order = skeDoc["armature"][0]["skin"][0]["slot"][0]["display"][i]["name"].GetString();;
			
			arma->order.push_back(order);
		}
	}

	arma->skinSlots = ReadJSONSkinSlot(skeDoc);
	arma->bones = ReadJSONBones(skeDoc);

	return arma;
}

std::map<std::string, SkinSlot> Skeleton2DAnimation::ReadJSONSkinSlot(rapidjson::Document& skeDoc)
{
	const rapidjson::Value& slotArray = skeDoc["armature"][0]["skin"][0]["slot"];

	std::map<std::string, SkinSlot> sSlots;

	for (int i = 0; i < (int)slotArray.Size(); i++)
	{
		SkinSlot* skinSlot = new SkinSlot();

		skinSlot->name = slotArray[i]["name"].GetString();
		skinSlot->partName = slotArray[i]["display"][0]["name"].GetString();

		if (slotArray[i]["display"][0].HasMember("transform"))
		{
			skinSlot->x = slotArray[i]["display"][0]["transform"]["x"].GetFloat();
			skinSlot->y = slotArray[i]["display"][0]["transform"]["y"].GetFloat();

			if (skinSlot->rot = slotArray[i]["display"][0]["transform"].HasMember("skY"))
			{
				skinSlot->rot = slotArray[i]["display"][0]["transform"]["skY"].GetFloat();
			}
			else
			{
				skinSlot->rot = 0.0f;
			}
		}
		else
		{
			skinSlot->x = 0.0f;
			skinSlot->y = 0.0f;
			skinSlot->rot = 0.0f;
		}

		skinSlot->BuildTransform();

		sSlots.insert(strSSlot(skinSlot->name, *skinSlot));

		delete skinSlot;
	}

	return sSlots;
}

std::map<std::string, Bone> Skeleton2DAnimation::ReadJSONBones(rapidjson::Document& skeDoc)
{
	const rapidjson::Value& boneArray = skeDoc["armature"][0]["bone"];

	std::map<std::string, Bone> bones;

	for (int i = 0; i < (int)boneArray.Size(); i++)
	{
		Bone* bone = new Bone();

		bone->name = boneArray[i]["name"].GetString();

		if (boneArray[i].HasMember("parent"))
		{
			bone->parent = boneArray[i]["parent"].GetString();
		}
		else
		{
			bone->parent = "noParent";
		}

		if (boneArray[i].HasMember("transform"))
		{
			bone->x = boneArray[i]["transform"]["x"].GetFloat();
			bone->y = boneArray[i]["transform"]["y"].GetFloat();
			bone->rot = boneArray[i]["transform"]["skY"].GetFloat();
		}
		else
		{
			bone->x = 0.0f;
			bone->y = 0.0f;
			bone->rot = 0.0f;
		}

		bone->BuildLocalTransform();

		bones.insert(strBone(bone->name, *bone));

		delete bone;
	}

	BuildWorldTransform(&bones);

	return bones;
}

void Skeleton2DAnimation::BuildWorldTransform(std::map<std::string, Bone>* bones)
{
	std::map<std::string, Bone>::iterator bonePart;

	for (bonePart = bones->begin(); bonePart != bones->end(); bonePart++)
	{
		bool noParent = false;
		std::string parentBone;

		gef::Matrix33 worldTrans;
		worldTrans.SetIdentity();

		parentBone = bonePart->second.parent;

		worldTrans = bonePart->second.localTransform;

		do
		{
			if (parentBone == "noParent")
			{
				noParent = true;
			}
			else
			{
				worldTrans = worldTrans * bones->at(parentBone).localTransform;

				parentBone = bones->at(parentBone).parent;
			}

		} while (noParent == false);

		bonePart->second.worldTransform = worldTrans;
	}
}

SubTexture* Skeleton2DAnimation::ReadJSONSubTexture(const rapidjson::Value& subTextureArray)
{
	SubTexture* subTex = new SubTexture();

	subTex->name = subTextureArray["name"].GetString();
	subTex->width = subTextureArray["width"].GetFloat();
	subTex->height = subTextureArray["height"].GetFloat();

	subTex->x = subTextureArray["x"].GetFloat();
	subTex->y = subTextureArray["y"].GetFloat();

	if (subTextureArray.HasMember("frameWidth"))
	{
		subTex->frameWidth = subTextureArray["frameWidth"].GetFloat();
		subTex->frameHeight = subTextureArray["frameHeight"].GetFloat();
		subTex->frameX = subTextureArray["frameX"].GetFloat();
		subTex->frameY = subTextureArray["frameY"].GetFloat();
	}
	else
	{
		subTex->frameWidth = subTextureArray["width"].GetFloat();
		subTex->frameHeight = subTextureArray["height"].GetFloat();
		subTex->frameX = 0;
		subTex->frameY = 0;
	}

	if (usingBones)
	{
		subTex->BuildTransform();
	}

	return subTex;
}

Animation2D* Skeleton2DAnimation::ReadJSONAnimBones(rapidjson::Document& skeDoc, int i)
{
	Animation2D* newAnim = new Animation2D();

	newAnim->duration = skeDoc["armature"][0]["animation"][i]["duration"].GetFloat();
	newAnim->name = skeDoc["armature"][0]["animation"][i]["name"].GetString();

	const rapidjson::Value& boneArray = skeDoc["armature"][0]["animation"][i]["bone"];

	for (int boneNum = 0; boneNum < boneArray.Size(); boneNum++)
	{
		BoneKey* boneKey = new BoneKey();

		boneKey->name = boneArray[boneNum]["name"].GetString();

		//if any translation key, store data
		if (boneArray[boneNum].HasMember("translationFrame"))
		{
			for (int i = 0; i < boneArray[boneNum]["translationFrame"].Size(); i++)
			{
				boneKey->transKeys.push_back(ReadJSONTranslationKeyData(boneArray[boneNum]["translationFrame"][i]));
			}
		}

		//if any rotation key, store data
		if (boneArray[boneNum].HasMember("rotateFrame"))
		{
			for (int i = 0; i < boneArray[boneNum]["rotateFrame"].Size(); i++)
			{
				boneKey->rotKeys.push_back(ReadJSONRotationKeyData(boneArray[boneNum]["rotateFrame"][i]));
			}
		}

		newAnim->boneKeys.push_back(boneKey);
	}

	return newAnim;
}

TranslationKey* Skeleton2DAnimation::ReadJSONTranslationKeyData(const rapidjson::Value & arrayPos)
{
	TranslationKey* transKey = new TranslationKey();

	//get time to next key / frames until end
	float timeToNextKey = arrayPos["duration"].GetFloat();

	//checks if x and y are available
	float hasTranslationX = arrayPos.HasMember("x");
	float hasTranslationY = arrayPos.HasMember("y");

	//if available then set data else set to 0
	float translationX = hasTranslationX ? arrayPos["x"].GetFloat() : 0;
	float translationY = hasTranslationY ? arrayPos["y"].GetFloat() : 0;

	//set time to next key and translation data
	transKey->nextKeyTime = timeToNextKey;
	transKey->translation = gef::Vector2(translationX, translationY);

	return transKey;
}

RotationKey* Skeleton2DAnimation::ReadJSONRotationKeyData(const rapidjson::Value & arrayPos)
{
	RotationKey* rotKey = new RotationKey();

	//get time to next key / frames until end
	float timeToNextKey = arrayPos["duration"].GetFloat();

	//checks if x and y are available
	float hasRotation = arrayPos.HasMember("rotate");

	//if available then set data else set to 0
	float rotation = hasRotation ? arrayPos["rotate"].GetFloat() : 0;

	//set time to next key and rotation data
	rotKey->nextKeyTime = timeToNextKey;
	rotKey->rotation = rotation;

	return rotKey;
}

void Skeleton2DAnimation::SetUpSpriteProperties(gef::Sprite* sprite, float screenX, float screenY, TextureAtlas* texAtlas, int frame)
{
	std::vector<std::string> order = curTexAtlas.armature->order;

	float width = texAtlas->subTextures[order[frame]].width;
	float height = texAtlas->subTextures[order[frame]].height;
	float frameWidth = texAtlas->subTextures[order[frame]].frameWidth;
	float frameHeight = texAtlas->subTextures[order[frame]].frameHeight;
	float x = texAtlas->subTextures[order[frame]].x;
	float y = texAtlas->subTextures[order[frame]].y;
	float frameX = texAtlas->subTextures[order[frame]].frameX;
	float frameY = texAtlas->subTextures[order[frame]].frameY;

	sprite->set_width(width);
	sprite->set_height(height);
	sprite->set_uv_width(width / texAtlas->width);
	sprite->set_uv_height(height / texAtlas->height);

	float u = x / texAtlas->width;
	float v = y / texAtlas->height;

	sprite->set_uv_position(gef::Vector2(u, v));

	float spriteX = width * 0.5f - (frameWidth * 0.5f + frameX);
	float spriteY = height * 0.5f - (frameHeight * 0.5f + frameY);

	sprite->set_position(gef::Vector4(screenX + spriteX, screenY + spriteY, 0.0f));
}

void Skeleton2DAnimation::SetUpSpriteProperties(gef::Sprite* sprite, float screenX, float screenY, TextureAtlas* texAtlas, int frame, Armature* arma, std::string partName)
{
	std::vector<std::string> order = arma->order;

	float width = texAtlas->subTextures.at(partName).width;
	float height = texAtlas->subTextures.at(partName).height;
	float frameWidth = texAtlas->subTextures.at(partName).frameWidth;
	float frameHeight = texAtlas->subTextures.at(partName).frameHeight;
	float x = texAtlas->subTextures.at(partName).x;
	float y = texAtlas->subTextures.at(partName).y;
	float frameX = texAtlas->subTextures.at(partName).frameX;
	float frameY = texAtlas->subTextures.at(partName).frameY;

	sprite->set_width(width);
	sprite->set_height(height);

	sprite->set_uv_width(width / texAtlas->width);
	sprite->set_uv_height(height / texAtlas->height);

	float u = x / texAtlas->width;
	float v = y / texAtlas->height;

	sprite->set_uv_position(gef::Vector2(u, v));

	float spriteX = width * 0.5f - (frameWidth * 0.5f + frameX);
	float spriteY = height * 0.5f - (frameHeight * 0.5f + frameY);

	sprite->set_position(gef::Vector4(screenX + spriteX, screenY + spriteY, 0.0f));
}

void Skeleton2DAnimation::ParseJSON(const char* texPath, const char* skePath, gef::Platform& platform_, bool bones)
{
	TextureAtlas* texAtlas = new TextureAtlas();

	char* texJSON;
	char* skeJSON;

	usingBones = bones;

	//read json texture
	texJSON = LoadJSON(texPath);
	rapidjson::Document texDoc;
	texDoc.Parse(texJSON);

	//read json skeleton
	skeJSON = LoadJSON(skePath);
	rapidjson::Document skeDoc;
	skeDoc.Parse(skeJSON);

	texAtlas = ReadJSONTextureAtlas(texDoc, skeDoc);
	textureAtlases.push_back(*texAtlas);

	curTexAtlas = *texAtlas;

	delete texAtlas;

	free(texJSON);
	free(skeJSON);

	if (sprite_texture_[0] == NULL)
	{
		sprite_texture_[0] = CreateTextureFromPNG(texDoc["imagePath"].GetString(), platform_);
	}
	else
	{
		sprite_texture_[1] = CreateTextureFromPNG(texDoc["imagePath"].GetString(), platform_);
		sprite.set_texture(sprite_texture_[1]);
	}

	sprite_pos_ = gef::Vector2(platform_.width()*0.5f, platform_.height()*0.5f);
	
	if (!bones)
	{
		SetUpSpriteProperties(&sprite, sprite_pos_.x, sprite_pos_.y, &curTexAtlas, 0);
	}
}

void Skeleton2DAnimation::CalculateWorldBoneTransform(std::map<std::string, Bone>* bones, Animation2D* anim, int curFrame)
{
	//only get key start times if anim is valid
	if (anim != NULL)
	{
		std::vector<BoneKey*> boneKeys;

		for (auto& bKeys : anim->boneKeys)
		{
			//calculate the start time for translation keys
			float startTime = 0.0f;

			for (auto& tKeys : bKeys->transKeys)
			{
				tKeys->startTime = startTime;
				startTime += tKeys->nextKeyTime;
			}

			//calc start time of rotation keys
			startTime = 0.0f;

			for (auto& rKeys : bKeys->rotKeys)
			{
				rKeys->startTime = startTime;
				startTime += rKeys->nextKeyTime;
			}
		}
	}

	std::map<std::string, Bone>::iterator bonePart;

	for (bonePart = bones->begin(); bonePart != bones->end(); bonePart++)
	{
		gef::Matrix33 translation, rotation;
		translation.SetIdentity();
		rotation.SetIdentity();

		float localX = bonePart->second.x;
		float localY = bonePart->second.y;
		float localRot = bonePart->second.rot;

		if (anim != NULL)
		{
			for (auto& bKeys : anim->boneKeys)
			{
				if (bonePart->second.name == bKeys->name)
				{
					//calc rotation from rotation keys
					for (int curKey = 0; curKey < bKeys->rotKeys.size(); curKey++)
					{
						//loop through all rot keys
						int nextKey = curKey >= bKeys->rotKeys.size() - 1 ? 0 : curKey + 1;

						if (curFrame >= bKeys->rotKeys[curKey]->startTime && curFrame < bKeys->rotKeys[nextKey]->startTime)
						{
							//get lerp between rotation key data
							float time = (curFrame - bKeys->rotKeys[curKey]->startTime) / (bKeys->rotKeys[nextKey]->startTime - bKeys->rotKeys[curKey]->startTime);

							localRot += (Lerp(bKeys->rotKeys[curKey]->rotation, bKeys->rotKeys[nextKey]->rotation, time));
						}
						else if (curKey == nextKey)	//end of cycle, store data
						{
							//dont lerp
							localRot += bKeys->rotKeys[curKey]->rotation;
						}
					}

					//calc translation from translation keys
					for (int curKey = 0; curKey < bKeys->transKeys.size(); curKey++)
					{
						//loop through all trans keys
						int nextKey = curKey >= bKeys->transKeys.size() - 1 ? 0 : curKey + 1;

						if (curFrame >= bKeys->transKeys[curKey]->startTime && curFrame < bKeys->transKeys[nextKey]->startTime)
						{
							//get lerp between translation key data
							float time = (curFrame - bKeys->transKeys[curKey]->startTime) / (bKeys->transKeys[nextKey]->startTime - bKeys->transKeys[curKey]->startTime);

							gef::Vector2 lerpAnimPos = Lerp(bKeys->transKeys[curKey]->translation, bKeys->transKeys[nextKey]->translation, time);

							localX += lerpAnimPos.x;
							localY += lerpAnimPos.y;
						}
						else if (curKey == nextKey)	//end of cycle, store data
						{
							//dont lerp
							localX += bKeys->transKeys[curKey]->translation.x;
							localY += bKeys->transKeys[curKey]->translation.y;
						}
					}
				}
			}
		}

		rotation.Rotate(gef::DegToRad(localRot));
		translation.SetTranslation(gef::Vector2(localX, localY));
		bonePart->second.localTransform = rotation * translation;
	}

	//loop trough 'parent' bones
	for (bonePart = bones->begin(); bonePart != bones->end(); bonePart++)
	{
		std::map<std::string, Bone>::iterator boneChild;
		
		//loop through 'child' bones
		for (boneChild = bones->begin(); boneChild != bones->end(); boneChild++)
		{
			//check if the bone name matches the child bones parent name
			if (bonePart->second.name == boneChild->second.parent)
			{
				//set world transform of child
				boneChild->second.worldTransform = boneChild->second.localTransform * bonePart->second.worldTransform;
			}
		}
	}
}

gef::Vector2 Skeleton2DAnimation::Lerp(gef::Vector2 startPos, gef::Vector2 endPos, float time)
{
	float x = (1.0f - time) * startPos.x + time * endPos.x;
	float y = (1.0f - time) * startPos.y + time * endPos.y;

	return gef::Vector2(x, y);
}

float Skeleton2DAnimation::Lerp(float startAngle, float endAngle, float time)
{
	float angleDiff = endAngle - startAngle;

	float angle;

	if (angleDiff > 180)
	{
		angleDiff -= 360;
	}
	else if (angleDiff < -180)
	{
		angleDiff += 360;
	}

	angle = startAngle + time * angleDiff;

	return angle;
}
