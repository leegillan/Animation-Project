#pragma once
#include <system/application.h>
#include <graphics/sprite.h>

#include <sstream>
#include <iostream>
#include <string>
#include <input/keyboard.h>

#include <vector>

// FRAMEWORK FORWARD DECLARATIONS
namespace gef
{
	class Platform;
	class SpriteRenderer;
	class Font;
	class InputManager;
	class Texture;
}

struct AnimTextures
{
	gef::Texture* tex[10];
};

class Frame2DAnimation
{
public:
	Frame2DAnimation();
	~Frame2DAnimation();

	void Init(gef::Platform& platform);
	void CleanUp();
	bool Update(float frame_time);
	void Render(gef::SpriteRenderer* spriteRenderer);
	void Input(const gef::Keyboard* keyInput);

	//getters
	float GetAnimTime() { return animTime; }
	int GetCurrentAnim() { return anim; }

private:
	void SetupAnimation(const char* pathName, gef::Platform& platform_);

	//2D Sprites textures
	std::vector<AnimTextures> textures;

	gef::Sprite sprite_;

	int curFrame;
	float timePassed;

	int anim;
	float animTime;
};

