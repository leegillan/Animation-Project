#include "Ragdoll3DAnimation.h"
#include <system/platform.h>
#include <graphics/texture.h>
#include <graphics/mesh.h>
#include <graphics/primitive.h>
#include <assets/png_loader.h>
#include <graphics/image_data.h>
#include <maths/vector2.h>
#include <input/input_manager.h>
#include <input/keyboard.h>
#include <maths/math_utils.h>
#include <graphics/renderer_3d.h>
#include <graphics/scene.h>
#include <animation/skeleton.h>
#include <animation/animation.h>
#include <system/debug_log.h>
#include "3D/ragdoll.h"

std::string model_name("xbot");

Ragdoll3DAnimation::Ragdoll3DAnimation():
	player_(NULL),
	primitive_builder_(NULL),
	primitive_renderer_(NULL),
	dynamics_world_(NULL),
	solver_(NULL),
	overlapping_pair_cache_(NULL),
	dispatcher_(NULL),
	debug_drawer_(NULL),
	floor_mesh_(NULL),
	ragdoll_(NULL)
{
}

Ragdoll3DAnimation::~Ragdoll3DAnimation()
{
}

void Ragdoll3DAnimation::Init(gef::Renderer3D* renderer3D, gef::Platform& platform, std::vector<Animation3D> anims)
{
	//set animation with already filled animations and scene data to be used in ragdoll scene
	animation = anims[0];

	//sets skeleton
	SetupSkeleton();

	primitive_builder_ = new PrimitiveBuilder(platform);
	primitive_renderer_ = new PrimitiveRenderer(platform);

	InitPhysicsWorld(renderer3D);
	CreateRigidBodies();

	InitRagdoll();

	usingVelocity = true;
	drawDebugLines = true;

	physicsTimeStep = 0.1f;

	anim = 2;
}

void Ragdoll3DAnimation::CleanUp()
{
	CleanUpRagdoll();

	CleanUpRigidBodies();

	CleanUpPhysicsWorld();

	delete primitive_renderer_;
	primitive_renderer_ = NULL;

	delete primitive_builder_;
	primitive_builder_ = NULL;

	delete player_;
	player_ = NULL;
}

void Ragdoll3DAnimation::CleanUpRagdoll()
{
	delete ragdoll_;
	ragdoll_ = NULL;
}

bool Ragdoll3DAnimation::Update(float frameTime)
{
	// update the current animation that is playing
	if (player_)
	{
		animation.animClips[anim].motionClip.Update(frameTime, player_->bind_pose());
		player_->UpdateBoneMatrices(animation.animClips[anim].motionClip.pose());
	}

	if (player_ && ragdoll_)
	{
		if (is_ragdoll_simulating_)
		{
			if (timer > physicsTimeStep)
			{
				ragdoll_->UpdatePoseFromRagdoll();
			}

			player_->UpdateBoneMatrices(ragdoll_->pose());
		}
		else
		{
			ragdoll_->set_pose(animation.animClips[anim].motionClip.pose());

			if (timer > physicsTimeStep)
			{
				ragdoll_->UpdateRagdollFromPose();
			}
		}
	}

	if (timer > physicsTimeStep)
	{	
		UpdatePhysicsWorld(frameTime);

		timer = 0.0f;
	}
	else
	{
		timer += frameTime;
	}

	return true;
}

void Ragdoll3DAnimation::Render(gef::Renderer3D* renderer3D)
{
	// draw the player, the pose is defined by the bone matrices
	if (player_)
	{
		renderer3D->DrawSkinnedMesh(*player_, player_->bone_matrices());
	}

	renderer3D->DrawMesh(floor_gfx_);

	if (dynamics_world_)
	{
		if (drawDebugLines)
		{
			dynamics_world_->debugDrawWorld();
		}
	}
}

void Ragdoll3DAnimation::Input(const gef::Keyboard * keyInput)
{
	if (keyInput->IsKeyPressed(gef::Keyboard::KC_SPACE))
	{
		is_ragdoll_simulating_ = !is_ragdoll_simulating_;
	}

	//reads animation data into texture atlas to be used in scene
	if (keyInput->IsKeyPressed(gef::Keyboard::KC_1))
	{
		anim = 0;
	}

	if (keyInput->IsKeyPressed(gef::Keyboard::KC_2))
	{
		anim = 1;
	}
	if (keyInput->IsKeyPressed(gef::Keyboard::KC_3))
	{
		anim = 2;
	}
	if (keyInput->IsKeyPressed(gef::Keyboard::KC_4))
	{
		anim = 3;
	}

	//toggles using velocity for ragdoll
	if (keyInput->IsKeyPressed(gef::Keyboard::KC_V))
	{
		ragdoll_->SetUsingVelocity(!ragdoll_->GetUsingVelocity());
	}

	//toggles using velocity for ragdoll
	if (keyInput->IsKeyPressed(gef::Keyboard::KC_D))
	{
		drawDebugLines = !drawDebugLines;
	}

	//toggles using velocity for ragdoll
	if (keyInput->IsKeyDown(gef::Keyboard::KC_I))
	{
		physicsTimeStep = (physicsTimeStep >= 1.0f) ? 1.0f : physicsTimeStep + 0.02f;
	}

	//toggles using velocity for ragdoll
	if (keyInput->IsKeyDown(gef::Keyboard::KC_O))
	{
		physicsTimeStep = (physicsTimeStep <= 0.02f) ? 0.02f : physicsTimeStep - 0.02f;
	}
}

void Ragdoll3DAnimation::InitRagdoll()
{
	if (player_->bind_pose().skeleton())
	{
		ragdoll_ = new Ragdoll();
		ragdoll_->set_scale_factor(0.01f);

		std::string ragdoll_filename;
		ragdoll_filename = model_name + "/ragdoll.bullet";

		ragdoll_->Init(player_->bind_pose(), dynamics_world_, ragdoll_filename.c_str());
	}

	is_ragdoll_simulating_ = false;
}

void Ragdoll3DAnimation::InitPhysicsWorld(gef::Renderer3D* renderer3D)
{
	/// collision configuration contains default setup for memory , collision setup . Advanced users can create their own configuration .
	btDefaultCollisionConfiguration * collision_configuration = new btDefaultCollisionConfiguration();

	/// use the default collision dispatcher . For parallel processing you can use a diffent dispatcher(see Extras / BulletMultiThreaded)
	dispatcher_ = new btCollisionDispatcher(collision_configuration);

	/// btDbvtBroadphase is a good general purpose broadphase . You can also try out btAxis3Sweep .
	overlapping_pair_cache_ = new btDbvtBroadphase();

	/// the default constraint solver . For parallel processing you can use a different solver (see Extras / BulletMultiThreaded)
	solver_ = new btSequentialImpulseConstraintSolver;

	dynamics_world_ = new btDiscreteDynamicsWorld(dispatcher_, overlapping_pair_cache_, solver_, collision_configuration);
	dynamics_world_->setGravity(btVector3(0, -9.8f, 0));

	debug_drawer_ = new GEFDebugDrawer(renderer3D);
	debug_drawer_->setDebugMode(btIDebugDraw::DBG_DrawAabb | btIDebugDraw::DBG_DrawFrames);
	dynamics_world_->setDebugDrawer(debug_drawer_);
}

void Ragdoll3DAnimation::CleanUpPhysicsWorld()
{
	delete debug_drawer_;
	debug_drawer_ = NULL;

	for (int i = dynamics_world_->getNumConstraints() - 1; i >= 0; i--)
	{
		btTypedConstraint* constraint = dynamics_world_->getConstraint(i);
		dynamics_world_->removeConstraint(constraint);
		delete constraint;
	}


	// remove the rigidbodies from the dynamics world and delete them
	for (int i = dynamics_world_->getNumCollisionObjects() - 1; i >= 0; i--)
	{
		btCollisionObject * obj = dynamics_world_->getCollisionObjectArray()[i];
		btRigidBody * body = btRigidBody::upcast(obj);
		if (body && body->getMotionState())
		{
			delete body->getMotionState();
		}
		dynamics_world_->removeCollisionObject(obj);
		delete obj;
	}

	// delete collision shapes
	for (int j = 0; j< collision_shapes_.size(); j++)
	{
		btCollisionShape * shape = collision_shapes_[j];
		collision_shapes_[j] = 0;
		delete shape;
	}

	// delete dynamics world
	delete dynamics_world_;

	// delete solver
	delete solver_;

	// delete broadphase
	delete overlapping_pair_cache_;

	// delete dispatcher
	delete dispatcher_;

	dynamics_world_ = NULL;
	solver_ = NULL;
	overlapping_pair_cache_ = NULL;
	dispatcher_ = NULL;

	// next line is optional : it will be cleared by the destructor when the array goes out of scope
	collision_shapes_.clear();
}

void Ragdoll3DAnimation::UpdatePhysicsWorld(float delta_time)
{
	const btScalar simulation_time_step = physicsTimeStep;

	const int max_sub_steps = 1;

	dynamics_world_->stepSimulation(simulation_time_step, max_sub_steps);
}

void Ragdoll3DAnimation::CreateRigidBodies()
{
	//the ground is a cube of side 100 at position y = 0.
	{
		btVector3 groundHalfExtents(btScalar(50.), btScalar(1.), btScalar(50.));
		btCollisionShape* groundShape = new btBoxShape(groundHalfExtents);

		collision_shapes_.push_back(groundShape);

		btTransform groundTransform;
		groundTransform.setIdentity();
		groundTransform.setOrigin(btVector3(0, -groundHalfExtents.y(), 0));

		btScalar mass(0.);

		//rigidbody is dynamic if and only if mass is non zero, otherwise static
		bool isDynamic = (mass != 0.f);

		btVector3 localInertia(0, 0, 0);
		if (isDynamic)
			groundShape->calculateLocalInertia(mass, localInertia);

		//using motionstate is optional, it provides interpolation capabilities, and only synchronizes 'active' objects
		btDefaultMotionState* myMotionState = new btDefaultMotionState(groundTransform);
		btRigidBody::btRigidBodyConstructionInfo rbInfo(mass, myMotionState, groundShape, localInertia);
		btRigidBody* body = new btRigidBody(rbInfo);

		//add the body to the dynamics world
		dynamics_world_->addRigidBody(body);

		floor_mesh_ = primitive_builder_->CreateBoxMesh(gef::Vector4(groundHalfExtents.x(), groundHalfExtents.y(), groundHalfExtents.z()));
		floor_gfx_.set_mesh(floor_mesh_);
		floor_gfx_.set_transform(btTransform2Matrix(groundTransform));
	}
}

void Ragdoll3DAnimation::CleanUpRigidBodies()
{
	delete floor_mesh_;
	floor_mesh_ = NULL;
}

void Ragdoll3DAnimation::SetupSkeleton()
{
	gef::Skeleton* skeleton = blending3D.GetFirstSkeleton(animation.animScene.scene);

	if (skeleton)
	{
		player_ = new gef::SkinnedMeshInstance(*skeleton);
		player_->set_mesh(animation.animScene.mesh);
		player_->UpdateBoneMatrices(player_->bind_pose());

		// output skeleton joint names
		for (int joint_num = 0; joint_num < skeleton->joints().size(); ++joint_num)
		{
			std::string bone_name;
			animation.animScene.scene->string_id_table.Find(skeleton->joint(joint_num).name_id, bone_name);
			gef::DebugOut("%d: %s\n", joint_num, bone_name.c_str());
		}

		// animated model is scaled down to match the size of the physics ragdoll
		gef::Matrix44 player_transform;
		const float scale = 0.01f;
		player_transform.Scale(gef::Vector4(scale, scale, scale));
		player_->set_transform(player_transform);
	}
}