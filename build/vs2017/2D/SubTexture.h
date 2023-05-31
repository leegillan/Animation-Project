#pragma once

#include <maths/math_utils.h>
#include <maths/matrix33.h>
#include <string>

class SubTexture
{
public:
	SubTexture();
	~SubTexture();

	void BuildTransform();
	
	std::string name;
	float width;
	float height;
	float frameWidth;
	float frameHeight;
	float x;
	float y;
	float frameX;
	float frameY;

	gef::Matrix33 transform;
	gef::Matrix33 translationMat;
	gef::Matrix33 scaleMat;
};

