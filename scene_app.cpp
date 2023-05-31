#include "scene_app.h"
#include <system/platform.h>
#include <graphics/sprite_renderer.h>
#include <graphics/font.h>
#include <system/debug_log.h>
#include <graphics/renderer_3d.h>
#include <graphics/mesh.h>
#include <maths/math_utils.h>
#include <input/sony_controller_input_manager.h>
#include <graphics/sprite.h>
#include "load_texture.h"
#include "load_json.h"

SceneApp::SceneApp(gef::Platform& platform) :
	Application(platform),
	sprite_renderer_(NULL),
	input_manager_(NULL),
	font_(NULL),
	renderer_3d_(NULL)
//	sprite_texture_(NULL)
{
}

void SceneApp::Init()
{
	//Inititalise application state
	appState.Init();

	// initialise sprite/3d renderers & input manager
	sprite_renderer_ = gef::SpriteRenderer::Create(platform_);
	renderer_3d_ = gef::Renderer3D::Create(platform_);
	input_manager_ = gef::InputManager::Create(platform_);

	InitFont();
	SetupCamera();
	SetupLights();

	//Inititialise 2D animation////////////

		///SPRITE Key Frame
	//2D Sprite Key Frame Animation - Ninja
	spriteKeyFrame2D.Init(platform_);

		///SKELETON
	//2D Sprite Skeleton Animation - Fighter/Dragon
	spriteSkeleton2D.Init(platform_);

	//Initialise 3D animation//////////////

		///MODEL LOADING/SKELETON/BLENDING
	//3D Model Blending Animation - yBot - running/walking blending
	modelBlending3D.Init(platform_);
	
		///RAGDOLLS
	modelRagdoll3D.Init(renderer_3d_, platform_, modelBlending3D.GetAnimations());

}

void SceneApp::CleanUp()
{
	//TODO:: Call cleanups of each class
	spriteKeyFrame2D.CleanUp();
	spriteSkeleton2D.CleanUp();
	modelBlending3D.CleanUp();
	modelRagdoll3D.CleanUp();

	delete input_manager_;
	input_manager_ = NULL;

	CleanUpFont();

	delete sprite_renderer_;
	sprite_renderer_ = NULL;

	delete renderer_3d_;
	renderer_3d_ = NULL;
}

bool SceneApp::Update(float frame_time)
{
	fps_ = 1.0f / frame_time;

	//TODO:: Call updates of each class
	Input();

	switch (appState.GetAppState())
	{
		case KEYFRAME2D:
			spriteKeyFrame2D.Update(frame_time);
			break;
		
		case SKELETON2D:
			spriteSkeleton2D.Update(frame_time);
			break;

		case BLENDING3D:
			modelBlending3D.Update(frame_time);
			break;

		case RAGDOLLS3D:
			modelRagdoll3D.Update(frame_time);
			break;
	}

	input_manager_->Update();

	return true;
}

void SceneApp::Input()
{
	const gef::Keyboard* keyInput = input_manager_->keyboard();

	if (keyInput->IsKeyPressed(gef::Keyboard::KC_N))
	{
		if (appState.GetAppState() == RAGDOLLS3D)
		{
			appState.SetAppState(KEYFRAME2D);
			appState.SetDimension(Anim2D);
			SetupCamera();
		}
		else if(appState.GetAppState() == KEYFRAME2D)
		{
			appState.SetAppState(SKELETON2D);
		}
		else if (appState.GetAppState() == SKELETON2D)
		{
			appState.SetAppState(BLENDING3D);
			appState.SetDimension(Anim3D);
		}
		else if (appState.GetAppState() == BLENDING3D)
		{
			appState.SetAppState(RAGDOLLS3D);
			SetupCamera();
		}
	}

	switch (appState.GetAppState())
	{
		case KEYFRAME2D:
			spriteKeyFrame2D.Input(keyInput);
			break;

		case SKELETON2D:
			spriteSkeleton2D.Input(keyInput, platform_);
			break;

		case BLENDING3D:
			modelBlending3D.Input(keyInput);
			break;

		case RAGDOLLS3D:
			modelRagdoll3D.Input(keyInput);
			break;
	}
}

void SceneApp::Render()
{
	// setup view and projection matrices
	gef::Matrix44 projection_matrix;
	gef::Matrix44 view_matrix;
	projection_matrix = platform_.PerspectiveProjectionFov(camera_fov_, (float)platform_.width() / (float)platform_.height(), near_plane_, far_plane_);
	view_matrix.LookAt(camera_eye_, camera_lookat_, camera_up_);
	renderer_3d_->set_projection_matrix(projection_matrix);
	renderer_3d_->set_view_matrix(view_matrix);

	switch (appState.GetDimension())
	{
		case Anim3D:
			renderer_3d_->Begin();

			switch (appState.GetAppState())
			{
				case BLENDING3D:
					modelBlending3D.Render(renderer_3d_);
					break;

				case RAGDOLLS3D:
					modelRagdoll3D.Render(renderer_3d_);
					break;
			}

			renderer_3d_->End();
			break;

		case Anim2D:
			sprite_renderer_->Begin();

			//TODO:: Call renders of each class
			switch (appState.GetAppState())
			{
				case KEYFRAME2D:
					spriteKeyFrame2D.Render(sprite_renderer_);
					break;

				case SKELETON2D:
					spriteSkeleton2D.Render(sprite_renderer_);
					break;
			}

			sprite_renderer_->End();
	}

	sprite_renderer_->Begin(false);
		
		DrawHUD();
	
	sprite_renderer_->End();
}

void SceneApp::InitFont()
{
	font_ = new gef::Font(platform_);
	font_->Load("comic_sans");
}

void SceneApp::CleanUpFont()
{
	delete font_;
	font_ = NULL;
}

void SceneApp::DrawHUD()
{
	if(font_)
	{
		// display frame rate
		font_->RenderText(sprite_renderer_, gef::Vector4(850.0f, 510.0f, -0.9f), 1.0f, 0xffffffff, gef::TJ_LEFT, "FPS: %.1f", fps_);

		//TODO:: ADD APPLICATION STATE CODE HERE TO SHOW TEXT PER STATE

		font_->RenderText(sprite_renderer_, gef::Vector4(550.0f, 30.0f, -0.9f), 1.0f, 0xff000000, gef::TJ_LEFT, "Press N to switch through animations");

		//checks which state the applcation is in to show the correct text
		switch (appState.GetAppState())
		{
			case KEYFRAME2D:
				font_->RenderText(sprite_renderer_, gef::Vector4(50.0f, 30.0f, -0.9f), 1.0f, 0xffffffff, gef::TJ_LEFT, "Animation State: 2D Key Frame");
				font_->RenderText(sprite_renderer_, gef::Vector4(50.0f, 60.0f, -0.9f), 1.0f, 0xffffffff, gef::TJ_LEFT, "Animation %i/5", spriteKeyFrame2D.GetCurrentAnim() + 1);

				// display 2D Animation Time
				font_->RenderText(sprite_renderer_, gef::Vector4(700.0f, 350.0f, -0.9f), 1.0f, 0xffffffff, gef::TJ_LEFT, "2d Anim Time: %.3f", spriteKeyFrame2D.GetAnimTime());

				//shows controls for this state
				font_->RenderText(sprite_renderer_, gef::Vector4(50.0f, 350.0f, -0.9f), 1.0f, 0xfff00000, gef::TJ_LEFT, "Controls:");
				font_->RenderText(sprite_renderer_, gef::Vector4(50.0f, 380.0f, -0.9f), 0.7f, 0xfff00000, gef::TJ_LEFT, "1, 2, 3, 4, 5 - Change Animation");
				font_->RenderText(sprite_renderer_, gef::Vector4(50.0f, 400.0f, -0.9f), 0.7f, 0xfff00000, gef::TJ_LEFT, "I/O - Increase/Decrease animation time");
				break;

			case SKELETON2D:
				//Checks which animation is being shown to show correct text
				if (!spriteSkeleton2D.GetCharacter())
				{
					font_->RenderText(sprite_renderer_, gef::Vector4(50.0f, 30.0f, -0.9f), 1.0f, 0xffffffff, gef::TJ_LEFT, "Animation State: 2D Skeleton");
					font_->RenderText(sprite_renderer_, gef::Vector4(50.0f, 60.0f, -0.9f), 1.0f, 0xffffffff, gef::TJ_LEFT, "Animation %i/4", spriteSkeleton2D.GetCurrentAnim() + 1);

					font_->RenderText(sprite_renderer_, gef::Vector4(50.0f, 420.0f, -0.9f), 0.7f, 0xfff00000, gef::TJ_LEFT, "1, 2, 3, 4 - Change Animation");
				}
				else 
				{
					font_->RenderText(sprite_renderer_, gef::Vector4(50.0f, 30.0f, -0.9f), 1.0f, 0xffffffff, gef::TJ_LEFT, "Animation State: 2D Frame");
				}
				
				// display 2D Animation Time
				font_->RenderText(sprite_renderer_, gef::Vector4(700.0f, 350.0f, -0.9f), 1.0f, 0xffffffff, gef::TJ_LEFT, "2d Anim Time: %.3f%", spriteSkeleton2D.GetAnimTime());

				//shows controls for this state
				font_->RenderText(sprite_renderer_, gef::Vector4(50.0f, 350.0f, -0.9f), 1.0f, 0xfff00000, gef::TJ_LEFT, "Controls:");
				font_->RenderText(sprite_renderer_, gef::Vector4(50.0f, 380.0f, -0.9f), 0.7f, 0xfff00000, gef::TJ_LEFT, "0 - Change Character");
				font_->RenderText(sprite_renderer_, gef::Vector4(50.0f, 400.0f, -0.9f), 0.7f, 0xfff00000, gef::TJ_LEFT, "I, O - Increase/Decrease animation time");
				break;

			case BLENDING3D:
				font_->RenderText(sprite_renderer_, gef::Vector4(50.0f, 30.0f, -0.9f), 1.0f, 0xffffffff, gef::TJ_LEFT, "Animation State: 3D Blending");
				font_->RenderText(sprite_renderer_, gef::Vector4(50.0f, 60.0f, -0.9f), 1.0f, 0xffffffff, gef::TJ_LEFT, "Animation %i/4", modelBlending3D.GetCurrentAnim() + 1);


				if (modelBlending3D.GetBlending())
				{
					font_->RenderText(sprite_renderer_, gef::Vector4(700.0f, 350.0f, -0.9f), 0.9f, 0xffffffff, gef::TJ_LEFT, "Blending w/out blend tree");

					font_->RenderText(sprite_renderer_, gef::Vector4(700.0f, 380.0f, -0.9f), 0.9f, 0xffffffff, gef::TJ_LEFT, "Blend Amount: %.2f", modelBlending3D.GetAnimBlend());
					font_->RenderText(sprite_renderer_, gef::Vector4(700.0f, 410.0f, -0.9f), 0.9f, 0xffffffff, gef::TJ_LEFT, "Anim Speed: %.2f", modelBlending3D.GetAnimSpeed());

					font_->RenderText(sprite_renderer_, gef::Vector4(50.0f, 440.0f, -0.9f), 0.7f, 0xfff00000, gef::TJ_LEFT, "I/O - Change Blending Amount");
					font_->RenderText(sprite_renderer_, gef::Vector4(50.0f, 460.0f, -0.9f), 0.7f, 0xfff00000, gef::TJ_LEFT, "T - Toggle Animation Transition");

					if (modelBlending3D.GetToggleTransition())
					{
						font_->RenderText(sprite_renderer_, gef::Vector4(700.0f, 200.0f, -0.9f), 1.0f, 0xffffffff, gef::TJ_LEFT, "Transition enabled");

						font_->RenderText(sprite_renderer_, gef::Vector4(350.0f, 480.0f, -0.9f), 0.9f, 0xfff00000, gef::TJ_LEFT, "SPACE - Change Transition Animation");

						if (modelBlending3D.GetTransition())
						{
							font_->RenderText(sprite_renderer_, gef::Vector4(700.0f, 230.0f, -0.9f), 1.0f, 0xffffffff, gef::TJ_LEFT, "To Run");
						}
						else
						{
							font_->RenderText(sprite_renderer_, gef::Vector4(700.0f, 230.0f, -0.9f), 1.0f, 0xffffffff, gef::TJ_LEFT, "To Walk");
						}
					}
				}
				else if (modelBlending3D.GetUsingBlendTree())
				{
					font_->RenderText(sprite_renderer_, gef::Vector4(50.0f, 440.0f, -0.9f), 0.7f, 0xfff00000, gef::TJ_LEFT, "I/O - Change Walking Speed Amount");
					font_->RenderText(sprite_renderer_, gef::Vector4(50.0f, 460.0f, -0.9f), 0.7f, 0xfff00000, gef::TJ_LEFT, "[/] - Change Running Speed Amount");
					font_->RenderText(sprite_renderer_, gef::Vector4(50.0f, 480.0f, -0.9f), 0.7f, 0xfff00000, gef::TJ_LEFT, "J/K - Change Jumping Speed Amount");

					font_->RenderText(sprite_renderer_, gef::Vector4(700.0f, 350.0f, -0.9f), 1.0f, 0xffffffff, gef::TJ_LEFT, "Running Speed: %.2f", modelBlending3D.GetRunningSpeed());
					font_->RenderText(sprite_renderer_, gef::Vector4(700.0f, 380.0f, -0.9f), 1.0f, 0xffffffff, gef::TJ_LEFT, "Walking Speed: %.2f", modelBlending3D.GetWalkingSpeed());
					font_->RenderText(sprite_renderer_, gef::Vector4(700.0f, 410.0f, -0.9f), 1.0f, 0xffffffff, gef::TJ_LEFT, "Jumping Speed: %.2f", modelBlending3D.GetJumpingSpeed());
				}
				else
				{
					font_->RenderText(sprite_renderer_, gef::Vector4(50.0f, 440.0f, -0.9f), 0.7f, 0xfff00000, gef::TJ_LEFT, "1, 2, 3, 4 - Choose Animation");
					font_->RenderText(sprite_renderer_, gef::Vector4(50.0f, 60.0f, -0.9f), 1.0f, 0xffffffff, gef::TJ_LEFT, "Animation %i/4", modelBlending3D.GetCurrentAnim() + 1);
				}

				//shows controls for this state
				font_->RenderText(sprite_renderer_, gef::Vector4(50.0f, 350.0f, -0.9f), 1.0f, 0xfff00000, gef::TJ_LEFT, "Controls:");
				font_->RenderText(sprite_renderer_, gef::Vector4(50.0f, 380.0f, -0.9f), 0.7f, 0xfff00000, gef::TJ_LEFT, "B - Toggle Blending w/out blend tree");
				font_->RenderText(sprite_renderer_, gef::Vector4(50.0f, 400.0f, -0.9f), 0.7f, 0xfff00000, gef::TJ_LEFT, "L - Toggle Blend Tree");
				font_->RenderText(sprite_renderer_, gef::Vector4(50.0f, 420.0f, -0.9f), 0.7f, 0xfff00000, gef::TJ_LEFT, "0 - Change Character");
				break;

			case RAGDOLLS3D:
				font_->RenderText(sprite_renderer_, gef::Vector4(50.0f, 30.0f, -0.9f), 1.0f, 0xffffffff, gef::TJ_LEFT, "Animation State: 3D Ragdoll");

				font_->RenderText(sprite_renderer_, gef::Vector4(650.0f, 310.0f, -0.9f), 1.0f, 0xffffffff, gef::TJ_LEFT, "Physics Time Step: %.2f", modelRagdoll3D.GetPhysicsTimeStep());

				if (modelRagdoll3D.GetUsingVelocity())
				{
					font_->RenderText(sprite_renderer_, gef::Vector4(750.0f, 370.0f, -0.9f), 1.0f, 0xffffffff, gef::TJ_LEFT, "Velocity enabled");
				}
				else
				{
					font_->RenderText(sprite_renderer_, gef::Vector4(750.0f, 370.0f, -0.9f), 1.0f, 0xffffffff, gef::TJ_LEFT, "Velocity disabled");
				}

				//shows controls for this state
				font_->RenderText(sprite_renderer_, gef::Vector4(50.0f, 350.0f, -0.9f), 1.0f, 0xfff00000, gef::TJ_LEFT, "Controls:");
				font_->RenderText(sprite_renderer_, gef::Vector4(50.0f, 380.0f, -0.9f), 0.7f, 0xfff00000, gef::TJ_LEFT, "1, 2, 3 - Choose Animation");
				font_->RenderText(sprite_renderer_, gef::Vector4(50.0f, 400.0f, -0.9f), 0.7f, 0xfff00000, gef::TJ_LEFT, "V - Toggle using velocity when going into ragdoll");
				font_->RenderText(sprite_renderer_, gef::Vector4(50.0f, 420.0f, -0.9f), 0.7f, 0xfff00000, gef::TJ_LEFT, "D - Toggle debug lines");
				font_->RenderText(sprite_renderer_, gef::Vector4(50.0f, 440.0f, -0.9f), 0.7f, 0xfff00000, gef::TJ_LEFT, "SPACE - Toggle and reset Ragdoll Sim");
				break;
		}
	}
}

void SceneApp::SetupLights()
{
	gef::PointLight default_point_light;
	default_point_light.set_colour(gef::Colour(0.7f, 0.7f, 1.0f, 1.0f));
	default_point_light.set_position(gef::Vector4(-300.0f, -500.0f, 100.0f));

	gef::Default3DShaderData& default_shader_data = renderer_3d_->default_shader_data();
	default_shader_data.set_ambient_light_colour(gef::Colour(0.5f, 0.5f, 0.5f, 1.0f));
	default_shader_data.AddPointLight(default_point_light);
}

void SceneApp::SetupCamera()
{
	if (appState.GetAppState() == RAGDOLLS3D)
	{
		camera_eye_ = gef::Vector4(1.0f, 2.5f, 5.0f);
		camera_lookat_ = gef::Vector4(0.0f, 1.0f, 0.0f);
		far_plane_ = 100.0f;
	}
	else
	{
		// initialise the camera settings
		camera_eye_ = gef::Vector4(-250.0f, 250.0f, 250.0f);
		camera_lookat_ = gef::Vector4(0.0f, 80.0f, 0.0f);
		far_plane_ = 1000.f;
	}

	camera_up_ = gef::Vector4(0.0f, 1.0f, 0.0f);
	camera_fov_ = gef::DegToRad(45.0f);
	near_plane_ = 0.01f;
}