#pragma once

#include <system/application.h>
#include "maths/math_utils.h"
#include "maths/matrix33.h"
#include <vector>
#include <map>
#include "graphics/sprite.h"

class Bone
{
public:
	Bone();
	~Bone();

	void BuildLocalTransform();

	gef::Matrix33 worldTransform;
	gef::Matrix33 localTransform;
	
	std::string name;
	std::string parent;

	float x;
	float y;
	float rot;
};

