#include "Frame2DAnimation.h"
#include <system/platform.h>
#include <graphics/sprite_renderer.h>
#include <system/debug_log.h>
#include <graphics/sprite.h>
#include "load_texture.h"

Frame2DAnimation::Frame2DAnimation():
	textures{ NULL }
{
}

Frame2DAnimation::~Frame2DAnimation()
{
}

void Frame2DAnimation::Init(gef::Platform& platform_)
{
	//setup and add textures to vectors
	SetupAnimation("ninja/Attack__00", platform_);
	SetupAnimation("ninja/Dead__00", platform_);
	SetupAnimation("ninja/Slide__00", platform_);
	SetupAnimation("ninja/Glide_00", platform_);
	SetupAnimation("ninja/Jump_Attack__00", platform_);

	//pass through as getter from parser
	curFrame = 0;
	timePassed = 0;

	anim = 0;
	animTime = 0.03f;

	sprite_.set_texture(textures[0].tex[curFrame]);
	sprite_.set_position(gef::Vector4(platform_.width()*0.5f, platform_.height()*0.5f, -0.99f));
	sprite_.set_height(128.0f);
	sprite_.set_width(128.0f);
}

void Frame2DAnimation::SetupAnimation(const char* pathName, gef::Platform& platform_)
{
	AnimTextures* texture = new AnimTextures();

	std::stringstream animPNG;

	for (int i = 0; i < 10; i++)
	{
		animPNG << pathName << i << ".png";
		texture->tex[i] = CreateTextureFromPNG(animPNG.str().c_str(), platform_);
		animPNG.str(std::string());
	}

	textures.push_back(*texture);
}

void Frame2DAnimation::CleanUp()
{
	for (int i = 0; i < textures.size(); i++)
	{
		for (int j = 0; j < 10; j++)
		{
			delete textures[i].tex[j];
			textures[i].tex[j] = NULL;
		}
	}
}

bool Frame2DAnimation::Update(float frame_time)
{
	if (timePassed <= animTime)
	{
		timePassed += frame_time;
	}
	else
	{
		timePassed = 0;

		if (curFrame > 8)
		{
			curFrame = 0;
		}
		else
		{
			curFrame++;
		}
	
		sprite_.set_texture(textures[anim].tex[curFrame]);
	}

	return true;
}

void Frame2DAnimation::Render(gef::SpriteRenderer* spriteRenderer)
{
	// Render button icon
	spriteRenderer->DrawSprite(sprite_);
}

void Frame2DAnimation::Input(const gef::Keyboard* keyInput)
{
	if (keyInput->IsKeyDown(gef::Keyboard::KC_I))
	{
		animTime = (animTime >= 0.1f) ? 0.1f : animTime + 0.0001f;
	}

	if (keyInput->IsKeyDown(gef::Keyboard::KC_O))
	{
		animTime = (animTime <= 0.00f) ? 0.00f : animTime - 0.0001f;
	}

	if (keyInput->IsKeyPressed(gef::Keyboard::KC_1))
	{
		animTime = 0.03f;
		anim = 0;
	}
	if (keyInput->IsKeyPressed(gef::Keyboard::KC_2))
	{
		animTime = 0.05f;
		anim = 1;
	}
	if (keyInput->IsKeyPressed(gef::Keyboard::KC_3))
	{
		animTime = 0.03f;
		anim = 2;
	}
	if (keyInput->IsKeyPressed(gef::Keyboard::KC_4))
	{
		animTime = 0.02f;
		anim = 3;
	}
	if (keyInput->IsKeyPressed(gef::Keyboard::KC_5))
	{
		animTime = 0.04f;
		anim = 4;
	}
}
