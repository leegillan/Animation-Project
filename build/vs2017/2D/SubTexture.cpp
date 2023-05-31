#include "SubTexture.h"

SubTexture::SubTexture()
{
}


SubTexture::~SubTexture()
{
}

void SubTexture::BuildTransform()
{
	transform.SetIdentity();
	translationMat.SetIdentity();
	scaleMat.SetIdentity();

	float x = width * 0.5f - (frameWidth * 0.5 + frameX);
	float y = height * 0.5f - (frameHeight * 0.5 + frameY);

	translationMat.SetTranslation(gef::Vector2(x, y));
	scaleMat.Scale(gef::Vector2(width, height));

	transform = scaleMat * translationMat;
}
