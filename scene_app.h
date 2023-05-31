#ifndef _SCENE_APP_H
#define _SCENE_APP_H

#include <system/application.h>
#include <maths/vector2.h>
#include <graphics/sprite.h>
#include <input/input_manager.h>

#include <AppState.h>

//2D KeyFrame includes
#include <2D/Frame2DAnimation.h>

//2D Skeleton includes
#include <2D/Skeleton2DAnimation.h>

//3D Blending Includes
#include "3D/Blending3DAnimation.h"

//3D Ragdoll Includes
#include "3D/Ragdoll3DAnimation.h"

// FRAMEWORK FORWARD DECLARATIONS
namespace gef
{
	class Platform;
	class SpriteRenderer;
	class Font;
	class InputManager;
	class Renderer3D;
}

class SceneApp : public gef::Application
{
public:
	SceneApp(gef::Platform& platform);

	void Init();
	void CleanUp();
	bool Update(float frame_time);
	void Render();
	void Input();

private:
	void InitFont();
	void CleanUpFont();
	void DrawHUD();
	void SetupLights();
	void SetupCamera();

	//GEF variables
	gef::SpriteRenderer* sprite_renderer_;
	gef::Renderer3D* renderer_3d_;
	gef::Font* font_;
	gef::InputManager* input_manager_;

	AppState appState;

	Frame2DAnimation spriteKeyFrame2D;
	Skeleton2DAnimation spriteSkeleton2D;
	Blending3DAnimation modelBlending3D;
	Ragdoll3DAnimation modelRagdoll3D;

	float fps_;

	gef::Vector4 camera_eye_;
	gef::Vector4 camera_lookat_;
	gef::Vector4 camera_up_;

	float camera_fov_;
	float near_plane_;
	float far_plane_;

	//FRONTEND
	void FrontendInit();
	void FrontendRelease();
	void FrontendUpdate(float frame_time);
	void FrontendRender();
};

#endif // _SCENE_APP_H
