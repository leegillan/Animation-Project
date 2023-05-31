#pragma once
#include <system/application.h>
#include <graphics/sprite.h>
#include <maths/vector2.h>
#include <maths/vector4.h>
#include <maths/matrix44.h>
#include <vector>
#include <graphics/skinned_mesh_instance.h>
#include "motion_clip_player.h"
#include "primitive_builder.h"
#include "primitive_renderer.h"
#include "gef_debug_drawer.h"

#include "btBulletDynamicsCommon.h"
#include "3D/Blending3DAnimation.h"
#include "3D/Ragdoll.h"

// FRAMEWORK FORWARD DECLARATIONS
namespace gef
{
	class Platform;
	class SpriteRenderer;
	class Font;
	class Renderer3D;
	class Mesh;
	class Scene;
	class Skeleton;
	class InputManager;
	class Animation;
}

class Ragdoll3DAnimation
{
public:
	Ragdoll3DAnimation();
	~Ragdoll3DAnimation();

	void Init(gef::Renderer3D* renderer3d, gef::Platform& platform, std::vector<Animation3D> anims);
	void CleanUp();
	void CleanUpRagdoll();
	bool Update(float frame_time);
	void Render(gef::Renderer3D* renderer3D);
	void Input(const gef::Keyboard* keyInput);

	//Getters
	int GetCurrentAnim() { return anim; }
	bool GetUsingVelocity() { return ragdoll_->GetUsingVelocity(); }
	float GetPhysicsTimeStep() { return physicsTimeStep; }

private:
	void InitRagdoll();

	void InitPhysicsWorld(gef::Renderer3D* renderer3D);
	void CleanUpPhysicsWorld();
	void UpdatePhysicsWorld(float delta_time);

	void CreateRigidBodies();
	void CleanUpRigidBodies();

	void SetupSkeleton();

	Blending3DAnimation blending3D;

	//animations variables
	Animation3D animation;
	gef::SkinnedMeshInstance* player_;
	int anim = 0;

	//primitve variables
	PrimitiveBuilder* primitive_builder_;
	PrimitiveRenderer* primitive_renderer_;

	//physics world
	btDiscreteDynamicsWorld* dynamics_world_;
	btSequentialImpulseConstraintSolver* solver_;
	btBroadphaseInterface* overlapping_pair_cache_;
	btCollisionDispatcher* dispatcher_;
	btAlignedObjectArray<btCollisionShape*> collision_shapes_;
	GEFDebugDrawer* debug_drawer_;

		//floor
	gef::Mesh* floor_mesh_;
	gef::MeshInstance floor_gfx_;

	//ragdoll variables
	Ragdoll* ragdoll_;
	bool is_ragdoll_simulating_;
	bool usingVelocity;

	float timer = 0.0f;
	float physicsTimeStep;
	bool drawDebugLines;
};

