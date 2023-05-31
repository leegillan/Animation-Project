#include "Blending3DAnimation.h"

#include <system/platform.h>
#include <graphics/texture.h>
#include <graphics/mesh.h>
#include <maths/vector2.h>
#include <graphics/renderer_3d.h>
#include <graphics/scene.h>
#include <animation/skeleton.h>
#include <animation/animation.h>

Blending3DAnimation::Blending3DAnimation() :
	curMesh(NULL),
	curScene(NULL),
	player_(NULL),
	animClips{ NULL },
	animScenes{ NULL },
	animations{ NULL }
{
}

Blending3DAnimation::~Blending3DAnimation()
{
}

void Blending3DAnimation::Init(gef::Platform& platform)
{
	{
		animClips.clear();
		animScenes.clear();
		animations.clear();

		{
			// create a new scene object and read in the data from the file
			GatherScene(sceneNames, platform); //set animation scene temporary

			//XBOT CHARACTER MOTION CLIPS
			AnimationSetup("xbot/xbot@idle.scn", platform); //set animation clip 1 temporary
			AnimationSetup("xbot/xbot@walking_inplace.scn", platform); //set animation clip 2 temporary
			AnimationSetup("xbot/xbot@running_inplace.scn", platform); //set animation clip 3 temporary
			AnimationSetup("xbot/xbot@jump.scn", platform); //set animation clip 4 temporary

			//YBOT CHARACTER MOTION CLIPS
			AnimationSetup("ybot/ybot@idle.scn", platform); //set animation clip 1 temporary
			AnimationSetup("ybot/ybot@walking_inplace.scn", platform); //set animation clip 2 temporary
			AnimationSetup("ybot/ybot@running_inplace.scn", platform); //set animation clip 3 temporary
			AnimationSetup("ybot/ybot@jump.scn", platform); //set animation clip 4 temporary

			AddAnimation(animScenes, animClips); //add animation scene and clips to store animation and then clear temp storage	

			// get the first skeleton in the scene
			character = 0;
			ChangeSkeleton();
		}

		InitBlendTree();
	}

	animBlendAmount = 0.0f;

	blending = false;
	usingBlendTree = false;
	toggleTransition = false;
}


void Blending3DAnimation::InitBlendTree()
{
	if (player_ && player_->bind_pose().skeleton())
	{
		//initialise blend tree
		blendTree.Init(player_->bind_pose());

		SetupClipNodes(animations[0].animClips[0].animation);
		SetupClipNodes(animations[0].animClips[1].animation);
		SetupClipNodes(animations[0].animClips[2].animation);
		SetupClipNodes(animations[0].animClips[3].animation);

		//create a linear blending node
		SetupLinearBlendNodes((std::string)"IdleToWalkBlend", walkingSpeed);
		SetupLinearBlendNodes((std::string)"WalkToRunBlend", runningSpeed);
		SetupLinearBlendNodes((std::string)"RunToJumpBlend", jumpingSpeed);

		//connect nodes - idle and walk are first two animations
		for (int i = 0; i < 2; i++)
		{
			tree.linear2BlendNodes[0].SetInput(i, &tree.clipNodes[i]);
		}

		//walk to run
		tree.linear2BlendNodes[1].SetInput(0, &tree.linear2BlendNodes[0]);
		tree.linear2BlendNodes[1].SetInput(1, &tree.clipNodes[2]);

		//run to jump
		tree.linear2BlendNodes[2].SetInput(0, &tree.linear2BlendNodes[1]);
		tree.linear2BlendNodes[2].SetInput(1, &tree.clipNodes[3]);

		//set output pose to be blended pose of the two connected nodes/animations
		blendTree.output.SetInput(0, &tree.linear2BlendNodes[2]);

		blendTree.Start();
	}
}

void Blending3DAnimation::SetupLinearBlendNodes(std::string variableName, float startingSpeed)
{
	Linear2BlendNode* newNode = new Linear2BlendNode(&blendTree);

	//set variables
	blendTree.variables[variableName] = startingSpeed;
	newNode->SetVariable(0, variableName);

	tree.linear2BlendNodes.push_back(*newNode);
}

void Blending3DAnimation::SetupClipNodes(gef::Animation* anim)
{
	ClipNode* newNode = new ClipNode(&blendTree);
	newNode->SetClip(anim);

	tree.clipNodes.push_back(*newNode);
}

void Blending3DAnimation::CleanUp()
{
	delete player_;
	player_ = NULL;

	for (int i = 0; i < animations[character].animClips.size(); i++)
	{
		delete animations[character].animClips[i].animation;
		animations[character].animClips[i].animation = NULL;
	}

	delete curMesh;
	curMesh = NULL;

	delete curScene;
	curScene = NULL;
}

void Blending3DAnimation::Update(float frameTime)
{
	if (player_)
	{
		for (int i = 0; i < animations[character].animClips.size(); i++)
		{
			// update the pose in the anim player from the animation
			animations[character].animClips[i].motionClip.Update(frameTime, player_->bind_pose());
		}

		// from the newly updated pose in the anim player
		if (blending)
		{
			if (toggleTransition)
			{
				SetAnimSpeed(0.5f);
			}

			if (speed < walkSpeed)
			{
				player_->UpdateBoneMatrices(animations[character].animClips[0].motionClip.pose());
			}
			else
			{
				CalculateAnimBlendSpeed(walkSpeed, runSpeed);

				blendedPose.Linear2PoseBlend(animations[character].animClips[1].motionClip.pose(), animations[character].animClips[2].motionClip.pose(), animBlendAmount); //1D lerp blending

				// update the bone matrices that are used for rendering the character
				player_->UpdateBoneMatrices(blendedPose);
			}
		}
		else if (usingBlendTree)
		{
			blendTree.variables["IdleToWalkBlend"] = walkingSpeed;
			blendTree.variables["WalkToRunBlend"] = runningSpeed;
			blendTree.variables["RunToJumpBlend"] = jumpingSpeed;

			blendTree.Update(frameTime);
			blendedPose = blendTree.output.outputPose; //blend tree holds output and output holds output pose

			player_->UpdateBoneMatrices(blendedPose);
		}
		else
		{
			// update the bone matrices that are used for rendering the character
			player_->UpdateBoneMatrices(animations[character].animClips[anim].motionClip.pose());
		}
	}

	// build a transformation matrix that will position the character
	// use this to move the player around, scale it, etc.
	if (player_)
	{
		gef::Matrix44 player_transform;
		player_transform.SetIdentity();
		player_->set_transform(player_transform);
	}
}

void Blending3DAnimation::Render(gef::Renderer3D* renderer3D)
{
	// draw the player, the pose is defined by the bone matrices
	if (player_)
	{
		renderer3D->DrawSkinnedMesh(*player_, player_->bone_matrices());
	}
}

void Blending3DAnimation::Input(const gef::Keyboard* keyInput)
{
	float multi = 1.0f;

	if (usingBlendTree)
	{
		//changes walking blending amount
		if (keyInput->IsKeyDown(gef::Keyboard::KC_I))
		{
			walkingSpeed = (walkingSpeed >= 1.0f) ? 1.0f : walkingSpeed + 0.02f;
		}

		if (keyInput->IsKeyDown(gef::Keyboard::KC_O))
		{
			walkingSpeed = (walkingSpeed <= 0.0f) ? 0.0f : walkingSpeed - 0.02f;
		}

		//changes runningSpeed blending amount
		if (keyInput->IsKeyDown(gef::Keyboard::KC_LBRACKET))
		{
			runningSpeed = (runningSpeed >= 1.0f) ? 1.0f : runningSpeed + 0.02f;
		}

		if (keyInput->IsKeyDown(gef::Keyboard::KC_RBRACKET))
		{
			runningSpeed = (runningSpeed <= 0.0f) ? 0.0f : runningSpeed - 0.02f;
		}

		//changes runningSpeed blending amount
		if (keyInput->IsKeyDown(gef::Keyboard::KC_J))
		{
			jumpingSpeed = (jumpingSpeed >= 1.0f) ? 1.0f : jumpingSpeed + 0.02f;
		}

		if (keyInput->IsKeyDown(gef::Keyboard::KC_K))
		{
			jumpingSpeed = (jumpingSpeed <= 0.0f) ? 0.0f : jumpingSpeed - 0.02f;
		}
	}
	else if (blending)
	{
		//changes animation blending amount
		if (keyInput->IsKeyDown(gef::Keyboard::KC_I))
		{
			SetAnimSpeed(true, multi);
		}

		if (keyInput->IsKeyDown(gef::Keyboard::KC_O))
		{
			SetAnimSpeed(false, multi);
		}

		//toggles transitioning
		if (keyInput->IsKeyPressed(gef::Keyboard::KC_T))
		{
			toggleTransition = !toggleTransition;
		}

		//toggles the transtion to animation
		if (toggleTransition)
		{
			if (keyInput->IsKeyPressed(gef::Keyboard::KC_SPACE))
			{
				transition = !transition;
			}
		}
	}
	else //checks if blending is enable to determine if changing the animations shoudl be done
	{
		//reads animation data into texture atlas to be used in scene
		if (keyInput->IsKeyPressed(gef::Keyboard::KC_1))
		{
			anim = 0;
		}

		if (keyInput->IsKeyPressed(gef::Keyboard::KC_2))
		{
			anim = 1;
		}
		if (keyInput->IsKeyPressed(gef::Keyboard::KC_3))
		{
			anim = 2;
		}
		if (keyInput->IsKeyPressed(gef::Keyboard::KC_4))
		{
			anim = 3;
		}
	}

	//toggles simple blending
	if (keyInput->IsKeyPressed(gef::Keyboard::KC_B))
	{
		usingBlendTree = false;

		blending = !blending;
	}

	//toggles using blend tree
	if (keyInput->IsKeyPressed(gef::Keyboard::KC_L))
	{
		blending = false;
		toggleTransition = false;

		usingBlendTree = !usingBlendTree;
	}

	//Changes animation character and animations with character	
	if (keyInput->IsKeyPressed(gef::Keyboard::KC_0))
	{
		if (character != 0)
		{
			character = 0;
			curScene = animations[0].animScene.scene;
			curMesh = animations[0].animScene.mesh;

			ChangeSkeleton();
		}
		else
		{
			character = 1;
			curScene = animations[1].animScene.scene;
			curMesh = animations[1].animScene.mesh;

			ChangeSkeleton();
		}
	}
}

gef::Skeleton* Blending3DAnimation::GetFirstSkeleton(gef::Scene* scene)
{
	gef::Skeleton* skeleton = NULL;
	if (scene)
	{
		// check to see if there is a skeleton in the the scene file
		// if so, pull out the bind pose and create an array of matrices
		// that wil be used to store the bone transformations
		if (scene->skeletons.size() > 0)
			skeleton = scene->skeletons.front();
	}

	return skeleton;
}

gef::Mesh* Blending3DAnimation::GetFirstMesh(gef::Scene* scene, gef::Platform& platform)
{
	gef::Mesh* mesh = NULL;

	if (scene)
	{
		// now check to see if there is any mesh data in the file, if so lets create a mesh from it
		if (scene->mesh_data.size() > 0)
		{
			mesh = scene->CreateMesh(platform, scene->mesh_data.front());
		}
	}

	return mesh;
}

gef::Animation* Blending3DAnimation::LoadAnimation(const char* anim_scene_filename, const char* anim_name, gef::Platform& platform)
{
	gef::Animation* anim = NULL;

	gef::Scene anim_scene;

	if (anim_scene.ReadSceneFromFile(platform, anim_scene_filename))
	{
		// if the animation name is specified then try and find the named anim
		// otherwise return the first animation if there is one
		std::map<gef::StringId, gef::Animation*>::const_iterator anim_node_iter;
	
		if (anim_name)
		{
			anim_node_iter = anim_scene.animations.find(gef::GetStringId(anim_name));
		}
		else
		{
			anim_node_iter = anim_scene.animations.begin();
		}

		if (anim_node_iter != anim_scene.animations.end())
		{
			anim = new gef::Animation(*anim_node_iter->second);
		}
	}

	return anim;
}

void Blending3DAnimation::SetAnimSpeed(bool add, float multiplier)
{
	if (add)
	{
		speed = (speed >= runSpeed) ? runSpeed : speed + 0.02f * multiplier;
	}
	else
	{
		speed = (speed <= walkSpeed) ? walkSpeed : speed - 0.02f * multiplier;
	}
}

void Blending3DAnimation::SetAnimSpeed(float multiplier)
{
	if(transition)
	{
		speed = (speed >= runSpeed) ? runSpeed : speed + 0.02f * multiplier;
	}
	else
	{
		speed = (speed <= walkSpeed) ? walkSpeed : speed - 0.02f * multiplier;
	}
}

void Blending3DAnimation::CalculateAnimBlendSpeed(float minValue, float maxValue)
{
	//get a fractional value to show how far into the transition from one animation into another - (current speed - minValue) / (maxValue - minValue) - gives value from 0-1
	animBlendAmount = (speed - minValue) / (maxValue - minValue);
	
	//calculate speed modification - difference based on current speed of character
	float walkSpeedMod = (walkSpeedMax - walkSpeedMin) * animBlendAmount;
	float runSpeedMod = (runSpeedMax - runSpeedMin) * animBlendAmount;

	//set walking and running playback speed
	animations[character].animClips[1].motionClip.set_playback_speed(walkSpeedMin + walkSpeedMod);
	animations[character].animClips[2].motionClip.set_playback_speed(runSpeedMin + runSpeedMod);
}

void Blending3DAnimation::GatherScene(std::array<const char*, 2> sceneName, gef::Platform& platform)
{
	for (int i = 0; i < sceneName.size(); i++)
	{
		AnimScenes* scene = new AnimScenes();
		animScenes.push_back(*scene);

		// create a new scene object and read in the data from the file
		animScenes[i].scene = new gef::Scene();
		animScenes[i].scene->ReadSceneFromFile(platform, sceneName[i]);

		// we do want to render the data stored in the scene file so lets create the materials from the material data present in the scene file
		animScenes[i].scene->CreateMaterials(platform);

		// if there is mesh data in the scene, create a mesh to draw from the first mesh
		animScenes[i].mesh = GetFirstMesh(animScenes[i].scene, platform);

		delete scene;
	}

	curScene = animScenes[0].scene;
	curMesh = animScenes[0].mesh;
}

void Blending3DAnimation::AnimationSetup(const char* pathName, gef::Platform& platform)
{
	AnimClips* clip = new AnimClips();

	clip->animation = LoadAnimation(pathName, "", platform);

	if (clip->animation)
	{
		clip->motionClip.set_clip(clip->animation);
		clip->motionClip.set_looping(true);
		clip->motionClip.set_anim_time(0.0f);
	}

	animClips.push_back(*clip);

	delete clip;
}

void Blending3DAnimation::AddAnimation(std::vector<AnimScenes> animationScenes, std::vector<AnimClips> animationClips)
{
	for(int i = 0; i < animationScenes.size(); i++)
	{
		Animation3D* anim = new Animation3D();
		animations.push_back(*anim);

		animations[i].animScene = animScenes[i];

		for (int j = 0; j < animationClips.size() / 2; j++)
		{
			AnimClips* animClip = new AnimClips();
			animations[i].animClips.push_back(*animClip);

			if (i == 0)
			{
				animations[i].animClips[j] = animationClips[j];
			}
			else
			{
				animations[i].animClips[j] = animationClips[j + (animationClips.size() / 2)];
			}

			delete animClip;
		}
		
		delete anim;
	}

	animScenes.clear();
	animClips.clear();
}

void Blending3DAnimation::ChangeSkeleton()
{
	gef::Skeleton* skeleton = GetFirstSkeleton(curScene);

	if (skeleton)
	{
		player_ = new gef::SkinnedMeshInstance(*skeleton);

		for (int i = 0; i < animations[character].animClips.size(); i++)
		{
			animations[character].animClips[i].motionClip.Init(player_->bind_pose());
		}

		blendedPose = player_->bind_pose();

		player_->set_mesh(curMesh);
	}

	walkSpeedMin = 1.0f;
	walkSpeedMax = animations[character].animClips[1].animation->duration() / animations[character].animClips[2].animation->duration();

	runSpeedMin = animations[character].animClips[2].animation->duration() / animations[character].animClips[1].animation->duration();
	runSpeedMax = 1.0f;
}