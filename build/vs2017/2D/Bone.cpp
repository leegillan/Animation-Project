#include "Bone.h"

Bone::Bone()
{
}


Bone::~Bone()
{
}

void Bone::BuildLocalTransform()
{
	gef::Matrix33 rotationMat;
	gef::Matrix33 translationMat;

	rotationMat.SetIdentity();
	translationMat.SetIdentity();
	localTransform.SetIdentity();

	rotationMat.Rotate(gef::DegToRad(rot));
	translationMat.SetTranslation(gef::Vector2(x, y));

	localTransform = rotationMat * translationMat;
}
