#include "Ragdoll.h"

#include <btBulletWorldImporter.h>
#include <system/debug_log.h>

extern std::string model_name;

Ragdoll::Ragdoll() :
	scale_factor_(1.0f)
{
}

void Ragdoll::Init(const gef::SkeletonPose & bind_pose, btDiscreteDynamicsWorld* dynamics_world, const char * physics_filename)
{
	bind_pose_ = bind_pose;
	pose_ = bind_pose;

	gef::Matrix44 identity;
	identity.SetIdentity();

	bone_rb_offset_matrices_.resize(bind_pose_.skeleton()->joint_count(), identity);
	bone_rbs_.resize(bind_pose_.skeleton()->joint_count(), NULL);
	bone_world_matrices_.resize(bind_pose_.skeleton()->joint_count());

	btBulletWorldImporter* fileLoader = new btBulletWorldImporter(dynamics_world);

	std::string ragdoll_filename = model_name + "/ragdoll.bullet";
	fileLoader->loadFile(ragdoll_filename.c_str());

	int numRigidBodies = fileLoader->getNumRigidBodies();
	for (int i = 0; i < fileLoader->getNumRigidBodies(); i++)
	{
		btCollisionObject* obj = fileLoader->getRigidBodyByIndex(i);
		btRigidBody* body = btRigidBody::upcast(obj);

		// properties

		// The Blender object name
		std::string rb_name(fileLoader->getNameForPointer(body));

		// trim the blender rigid body name to match bone names
		rb_name = std::string(&rb_name.c_str()[sizeof("OBArmature_") - 1]);
		char* new_name = (char*)rb_name.c_str();
		new_name[rb_name.length() - sizeof("_hitbox") + 1] = 0;
		rb_name = std::string(new_name);

		gef::DebugOut("  object name = %s\n", rb_name.c_str());	
		gef::DebugOut("  get position = (%4.3f,%4.3f,%4.3f)\n",
			body->getCenterOfMassPosition().getX(),
			body->getCenterOfMassPosition().getY(),
			body->getCenterOfMassPosition().getZ());			// Blender CoM
		if (body->getInvMass() == 0)
			gef::DebugOut("  static object\n");
		else
		{
			gef::DebugOut("  mass = %4.3f\n", 1 / body->getInvMass());		// Blender Mass
		}
		gef::DebugOut("\n");

		if (bind_pose_.skeleton())
		{
			gef::StringId joint_name_id = gef::GetStringId(rb_name);
			if (joint_name_id != 0)
			{
				// find bone in skeleton that matches the name of the rigid body
				int joint_num = bind_pose_.skeleton()->FindJointIndex(joint_name_id);
				if (joint_num != -1)
				{
					bone_rbs_[joint_num] = body;

					// CALCULATE THE BONE TO RIGID BODY OFFSET FOR EVERY BONE
						/// rigidBodyOffset = rigidBodyWorldTrans * inverse(animBoneWorldTrans)

					//gets bone world transform
					gef::Matrix44 animBoneWorldTransMat = bind_pose_.global_pose()[joint_num];
					gef::Vector4 animBoneWorldPos = animBoneWorldTransMat.GetTranslation();

					//scale bone positions to match scale of ragdoll - gef -> centimetres, bullet -> metres
					animBoneWorldTransMat.SetTranslation(animBoneWorldPos * scale_factor_);

					//rigid body world transform
					btTransform rigidBodyWorldTrans = body->getCenterOfMassTransform();
					gef::Matrix44 rigidBodyWorldTransMat = btTransform2Matrix(rigidBodyWorldTrans); //convert to gef matrix

					//inverse of anim bone world transform
					gef::Matrix44 invAnimBoneWorldTransMat;
					invAnimBoneWorldTransMat.AffineInverse(animBoneWorldTransMat);

					//calc offset from bone transform to rigid body transform
					bone_rb_offset_matrices_[joint_num] = rigidBodyWorldTransMat * invAnimBoneWorldTransMat;
				}
			}
		}
	}

	usingVelocity = false;

	delete fileLoader;
	fileLoader = NULL;
}

Ragdoll::~Ragdoll()
{
}

void Ragdoll::UpdatePoseFromRagdoll()
{
	for (int bone_num = 0; bone_num < bind_pose_.skeleton()->joint_count(); ++bone_num)
	{
		const gef::Joint& joint = bind_pose_.skeleton()->joint(bone_num);
		gef::Matrix44 anim_bone_local_transform;

		btRigidBody* bone_rb = bone_rbs_[bone_num];
		if (bone_rb)
		{
			// CALCULATE THE BONE LOCAL TRANSFORM BASED ON THE RIGID BODY WORLD TRANSFORM
				/// animBoneLocalTrans = inverse(rigidBodyOffset) * inverse(parentWorldTrans)

			//rigid body world transform
			btTransform rigidBodyWorldTrans = bone_rb->getCenterOfMassTransform();
			gef::Matrix44 rigidBodyWorldTransMat = btTransform2Matrix(rigidBodyWorldTrans); //convert to gef matrix

			//inverse rigid body offset
			gef::Matrix44 invOffsetMat;
			invOffsetMat.AffineInverse(bone_rb_offset_matrices_[bone_num]);

			//inverse rigid body offset * rigid body world transform
			gef::Matrix44 animBoneWorldTransMat = invOffsetMat * rigidBodyWorldTransMat;

			//scale anim bone back to size of animation
			gef::Vector4 animBoneWorldPos = animBoneWorldTransMat.GetTranslation();

			float invScaleFactor = 1.0f / scale_factor_;

			animBoneWorldTransMat.SetTranslation(animBoneWorldPos * invScaleFactor);

			//calc local bone transform
			//check if bone has parent
			if (joint.parent == -1)
			{
				anim_bone_local_transform = animBoneWorldTransMat;
			}
			else
			{
				//inverse parent world transform
				gef::Matrix44 invParentWorldTransform;
				invParentWorldTransform.AffineInverse(bone_world_matrices_[joint.parent]);

				anim_bone_local_transform = animBoneWorldTransMat * invParentWorldTransform;
			}
		}
		else
		{
			anim_bone_local_transform = bind_pose_.local_pose()[bone_num].GetMatrix();
		}

		// calculate bone world transforms for anim skeleton
		if (joint.parent == -1)
		{
			bone_world_matrices_[bone_num] = anim_bone_local_transform;
		}
		else
		{
			bone_world_matrices_[bone_num] = anim_bone_local_transform * bone_world_matrices_[joint.parent];
		}
	}

	pose_.CalculateLocalPose(bone_world_matrices_);
	pose_.CalculateGlobalPose();
}

void Ragdoll::UpdateRagdollFromPose()
{
	for (int bone_num = 0; bone_num < bind_pose_.skeleton()->joint_count(); ++bone_num)
	{
		const gef::Joint& joint = bind_pose_.skeleton()->joint(bone_num);

		btRigidBody* bone_rb = bone_rbs_[bone_num];
		if (bone_rb)
		{
			// CALCULATE THE RIGID BODY WORLD TRANSFORM BASED ON THE CURRENT SKELETON POSE
				/// rigidBodyWorldTrans = rigidBodyOffset * boneWorldTransform

			//bone world transform
			gef::Matrix44 animBoneWorldTransMat = pose_.global_pose()[bone_num];
			gef::Vector4 animBoneWorldPos = animBoneWorldTransMat.GetTranslation();

			//scale bone position 
			animBoneWorldTransMat.SetTranslation(animBoneWorldPos * scale_factor_);

			//rigid body offset
			gef::Matrix44 offsetMat;
			offsetMat = bone_rb_offset_matrices_[bone_num];

			gef::Matrix44 rigidBodyWorldTransMat = offsetMat * animBoneWorldTransMat;
			btTransform rbWorldTrans = Matrix2btTransform(rigidBodyWorldTransMat); //convert to bullet transform

			//set bone data
			bone_rb->setCenterOfMassTransform(rbWorldTrans);

			if (usingVelocity)
			{
				bone_rb->setLinearVelocity(btVector3(bone_rbs_[bone_num]->getLinearVelocity().x(), bone_rbs_[bone_num]->getLinearVelocity().y(), bone_rbs_[bone_num]->getLinearVelocity().z()));
				bone_rb->setAngularVelocity(btVector3(bone_rbs_[bone_num]->getAngularVelocity().x(), bone_rbs_[bone_num]->getAngularVelocity().y(), bone_rbs_[bone_num]->getAngularVelocity().z()));
			}
			else
			{
				bone_rb->setLinearVelocity(btVector3(0.0f, 0.0f, 0.0f));
				bone_rb->setAngularVelocity(btVector3(0.0f, 0.0f, 0.0f));  // more stable but less realistic
			}
		}
	}
}

gef::Matrix44 btTransform2Matrix(const btTransform& transform)
{
	gef::Matrix44 result;

	btQuaternion rotation = transform.getRotation();
	btVector3 position = transform.getOrigin();

	result.Rotation(gef::Quaternion(rotation.x(), rotation.y(), rotation.z(), rotation.w()));
	result.SetTranslation(gef::Vector4(position.x(), position.y(), position.z()));

	return result;
}

btTransform Matrix2btTransform(const gef::Matrix44& mtx)
{
	gef::Vector4 mtx_position = mtx.GetTranslation();

	gef::Quaternion mtx_rot;
	mtx_rot.SetFromMatrix(mtx);

	btTransform result;
	result.setOrigin(btVector3(mtx_position.x(), mtx_position.y(), mtx_position.z()));
	result.setRotation(btQuaternion(mtx_rot.x, mtx_rot.y, mtx_rot.z, mtx_rot.w));

	return result;
}