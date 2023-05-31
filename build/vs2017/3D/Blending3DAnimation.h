#pragma once

#include <system/application.h>
#include <graphics/sprite.h>
#include <maths/vector2.h>
#include <maths/vector4.h>
#include <maths/matrix44.h>
#include <vector>
#include <graphics/skinned_mesh_instance.h>
#include "motion_clip_player.h"
#include <input/keyboard.h>
#include <maths\math_utils.h>
#include <array>

//blend tree
#include <3D/BlendTree.h>

namespace gef
{
	class Platform;
	class Font;
	class Renderer3D;
	class Scene;
	class Animation;
	class Keyboard;
}

struct AnimScenes //temp
{
	gef::Scene* scene;
	class gef::Mesh* mesh;
};

struct AnimClips //temp
{
	gef::Animation* animation; //animation type
	MotionClipPlayer motionClip; //motion data
};

struct Animation3D //permanent
{
	AnimScenes animScene; //scene data
	std::vector<AnimClips> animClips; //animation clips data
};

struct BlendingTree //permanent
{
	std::vector<ClipNode> clipNodes; //clip nodes
	std::vector<Linear2BlendNode> linear2BlendNodes;
};

class Blending3DAnimation
{
public:
	Blending3DAnimation();
	~Blending3DAnimation();
	
	gef::Skeleton* GetFirstSkeleton(gef::Scene* scene);
	gef::Mesh* GetFirstMesh(gef::Scene* scene, gef::Platform& platform);
	gef::Animation* LoadAnimation(const char* anim_scene_filename, const char* anim_name, gef::Platform& platform);

	void Init(gef::Platform& platform);
	void CleanUp();
	void Update(float frameTime);
	void Render(gef::Renderer3D* renderer3D);
	void Input(const gef::Keyboard* keyInput);

	//Getters
	int GetCurrentAnim() { return anim; }
	float GetAnimBlend() { return animBlendAmount; }
	float GetAnimSpeed() { return speed; }
	float GetWalkingSpeed() { return walkingSpeed; }
	float GetRunningSpeed() { return runningSpeed; }
	float GetJumpingSpeed() { return jumpingSpeed; }
	bool GetBlending() { return blending; }
	bool GetUsingBlendTree() { return usingBlendTree; }
	bool GetToggleTransition() { return toggleTransition; }
	bool GetTransition() { return transition; }

	std::vector<Animation3D> GetAnimations() { return animations; }

private:
	void GatherScene(std::array<const char*, 2> sceneName, gef::Platform& platform);
	void ChangeSkeleton();
	void AnimationSetup(const char* pathName, gef::Platform& platform);
	void AddAnimation(std::vector<AnimScenes> animationScenes, std::vector<AnimClips> animationClips);

	void CalculateAnimBlendSpeed(float minValue, float maxValue);
	void SetAnimSpeed(bool add, float multiplier);
	void SetAnimSpeed(float multiplier);

	void InitBlendTree();
	void SetupClipNodes(gef::Animation* anim);
	void SetupLinearBlendNodes(std::string name, float startingSpeed);
	
	gef::SkinnedMeshInstance* player_;

	//blend tree
	BlendTree blendTree;
	BlendingTree tree;
	bool usingBlendTree;

	//Scenes
	gef::Scene* curScene;
	class gef::Mesh* curMesh;
	std::vector<AnimScenes> animScenes;
	std::array<const char*, 2> sceneNames{ "xbot/xbot.scn", "ybot/ybot.scn" };
	
	//Animations
	std::vector<AnimClips> animClips;
	std::vector<Animation3D> animations;
	int anim = 0;
	int character = 0;

	//blending
	gef::SkeletonPose blendedPose;
	bool blending;
	bool toggleTransition;
	bool transition;
	float animBlendAmount;
	float walkSpeedMax, walkSpeedMin, runSpeedMax, runSpeedMin;
	
	//playback
	const float runSpeed = 4.0f;
	const float walkSpeed = 1.0f;

	float speed = 1.0f;

	float walkingSpeed = 0.0f;
	float runningSpeed = 0.0f;
	float jumpingSpeed = 0.0f;
};

