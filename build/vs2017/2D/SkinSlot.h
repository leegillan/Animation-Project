#pragma once
#include <system/application.h>
#include "maths/math_utils.h"
#include "maths/matrix33.h"
#include <vector>
#include <map>
#include "graphics/sprite.h"

class SkinSlot
{
public:
	SkinSlot();
	~SkinSlot();

	void BuildTransform();

	gef::Matrix33 transform;

	float x;
	float y;
	float rot;

	std::string name;
	std::string partName;

};

