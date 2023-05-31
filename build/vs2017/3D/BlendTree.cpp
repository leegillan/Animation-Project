#include "BlendTree.h"

//Blend node/////////////

BlendNode::BlendNode(BlendTree* _tree) :
	tree(_tree)
{
	if (_tree)
	{
		outputPose = tree->bindPose;
	}
}

bool BlendNode::Update(float deltaTime)
{
	bool allInputsValid = true;

	//try and update all children
	if (inputs.size() > 0)
	{
		for (int inputNum = 0; inputNum < inputs.size(); ++inputNum)
		{
			BlendNodeInput& input = inputs[inputNum];

			bool inputValid;

			if (input.node)
			{
				inputValid = input.node->Update(deltaTime);
			}
			else
			{
				inputValid = false;
			}

			if (!inputValid && allInputsValid)
			{
				allInputsValid = false;
			}
		}
	}

	bool allVariablesValid = true;

	//check if all variables exist
	if (variables.size() > 0)
	{
		for (int variableNum = 0; variableNum < variables.size(); variableNum++)
		{
			const std::string& variable = variables[variableNum];

			bool variableValid = tree->variables.find(variable) != tree->variables.end();

			if (!variableValid && allVariablesValid)
			{
				allVariablesValid = false;
			}
		}
	}

	bool outputValid = false;

	//only update parent nose if all children have succesfully updated and all variables are initialised
	if (allInputsValid && allVariablesValid)
	{
		outputValid = UpdateInternal(deltaTime);
	}

	return outputValid;
}

void BlendNode::Start()
{
	for (int inputNum = 0; inputNum < inputs.size(); inputNum++)
	{
		BlendNodeInput& input = inputs[inputNum];

		if (input.node)
		{
			input.node->Start();
		}
	}

	StartInternal();
}

void BlendNode::SetInput(int inputNum, BlendNode* node)
{
	if (node && inputNum < inputs.size())
	{
		inputs[inputNum].node = node;
	}
}

void BlendNode::SetVariable(int variableNum, const std::string& variable)
{
	if (variable.size() > 0 && variableNum < variables.size())
	{
		variables[variableNum] = variable;
	}
}

//Output node/////////////

OutputNode::OutputNode(BlendTree* _tree):
	BlendNode(_tree)
{
	inputs.resize(1);
}

bool OutputNode::UpdateInternal(float deltaTime)
{
	outputPose = inputs[0].node->outputPose;

	return true;
}

//Blend tree/////////////

BlendTree::BlendTree() :
	output(this)
{
}

void BlendTree::Init(const gef::SkeletonPose& bindPose_)
{
	bindPose = bindPose_;
	output.outputPose = bindPose;
}

void BlendTree::CleanUp()
{
}

void BlendTree::Start()
{
	output.Start();
}

void BlendTree::Update(float frameTime)
{
	bool valid = output.Update(frameTime);
}

//Clip node/////////////

ClipNode::ClipNode(BlendTree* _tree) :
	BlendNode(_tree)
{
	if (tree)
	{
		motionClip.Init(tree->bindPose);
	}

	motionClip.set_looping(true);
}

void ClipNode::SetClip(const gef::Animation * anim)
{
	motionClip.set_clip(anim);
}

void ClipNode::StartInternal()
{
	motionClip.set_anim_time(0.0f);
}

bool ClipNode::UpdateInternal(float deltaTime)
{
	bool valid = false;

	if (motionClip.clip())
	{
		motionClip.Update(deltaTime, tree->bindPose);
		outputPose = motionClip.pose();

		valid = true;
	}

	return valid;
}

//2-pose blend node/////////////

Linear2BlendNode::Linear2BlendNode(BlendTree * _tree) :
	BlendNode(_tree)

{
	inputs.resize(2);
	variables.resize(1);
}

bool Linear2BlendNode::UpdateInternal(float deltaTime)
{
	float blendValue = tree->variables[variables[0]];

	//first parameter is blend value
	outputPose.Linear2PoseBlend(inputs[0].node->outputPose, inputs[1].node->outputPose, blendValue);

	return true;
}