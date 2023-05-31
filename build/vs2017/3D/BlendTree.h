#pragma once

#include "motion_clip_player.h"
#include <vector>
#include <map>

class BlendTree;
class BlendNode;

struct BlendNodeInput
{
	BlendNode* node;

	BlendNodeInput() :
		node(nullptr)
	{
	}
};

class BlendNode
{
public:
	BlendNode(BlendTree* _tree);

	std::vector<BlendNodeInput> inputs;
	std::vector<std::string> variables;
	gef::SkeletonPose outputPose;

	BlendTree* tree;

	bool Update(float deltaTime);
	void Start();

	virtual bool UpdateInternal(float deltaTime) = 0;
	virtual void StartInternal() {};
	void SetInput(int inputNum, BlendNode* node);
	void SetVariable(int variableNum, const std::string& variable);
};

class OutputNode : public BlendNode
{
public:
	OutputNode(BlendTree* _tree);

	bool UpdateInternal(float deltaTime);
};

class ClipNode : public BlendNode
{
public:
	ClipNode(BlendTree* _tree);

	void SetClip(const gef::Animation* anim);
	void StartInternal() override;
	bool UpdateInternal(float deltaTime) override;

	MotionClipPlayer motionClip;
};

class Linear2BlendNode : public BlendNode
{
public:
	Linear2BlendNode(BlendTree* _tree);

	bool UpdateInternal(float deltaTime) override;
};

class BlendTree
{
public:
	BlendTree();

	void Init(const gef::SkeletonPose& bindPose_);
	void CleanUp();
	void Start();
	void Update(float frameTime);

	OutputNode output;
	gef::SkeletonPose bindPose;

	std::map<std::string, float> variables;
};

