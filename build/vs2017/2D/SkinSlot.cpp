#include "SkinSlot.h"


SkinSlot::SkinSlot()
{
}


SkinSlot::~SkinSlot()
{
}

void SkinSlot::BuildTransform()
{
	gef::Matrix33 rotationMat;
	gef::Matrix33 translationMat;

	rotationMat.SetIdentity();
	translationMat.SetIdentity();
	transform.SetIdentity();

	rotationMat.Rotate(gef::DegToRad(rot));
	translationMat.SetTranslation(gef::Vector2(x, y));

	transform = rotationMat * translationMat;
}
