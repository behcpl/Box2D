/*
* Copyright (c) 2006-2011 Erin Catto http://www.box2d.org
* Copyright (c) 2017 Bartosz Czuba BEHC
*
* This software is provided 'as-is', without any express or implied
* warranty.  In no event will the authors be held liable for any damages
* arising from the use of this software.
* Permission is granted to anyone to use this software for any purpose,
* including commercial applications, and to alter it and redistribute it
* freely, subject to the following restrictions:
* 1. The origin of this software must not be misrepresented; you must not
* claim that you wrote the original software. If you use this software
* in a product, an acknowledgment in the product documentation would be
* appreciated but is not required.
* 2. Altered source versions must be plainly marked as such, and must not be
* misrepresented as being the original software.
* 3. This notice may not be removed or altered from any source distribution.
*/

#ifndef B2_TELEPORT_JOINT_H
#define B2_TELEPORT_JOINT_H

#include "Box2D/Dynamics/Joints/b2Joint.h"

/// Teleport joint definition. You need to only specify relative offset
/// to maintain between bodies.
struct b2TeleportJointDef : public b2JointDef
{
	b2TeleportJointDef()
	{
		type = e_teleportJoint;
		offset.Set(0.0f, 0.0f);
	}

	/// Initialize the bodies and offset.
	void Initialize(b2Body* bodyA, b2Body* bodyB, const b2Vec2& offset);

	/// Exact displacement to maintain between two bodies
	b2Vec2 offset;
};

/// A teleport joint essentially glues two bodies together like weld joint,
/// but with a specified separation between them. It's different than weld
/// joint with distance, because bodies don't acts like they are glued
/// together, but like they are in separate coordinate system (that was the point)
class b2TeleportJoint : public b2Joint
{
public:
	b2Vec2 GetAnchorA() const;
	b2Vec2 GetAnchorB() const;

	b2Vec2 GetReactionForce(float32 inv_dt) const;
	float32 GetReactionTorque(float32 inv_dt) const;

	/// Dump to b2Log
	void Dump();

protected:

	friend class b2Joint;

	b2TeleportJoint(const b2TeleportJointDef* def);

	void InitVelocityConstraints(const b2SolverData& data);
	void SolveVelocityConstraints(const b2SolverData& data);
	bool SolvePositionConstraints(const b2SolverData& data);

	b2Vec2 m_offset;

	// Solver shared
	b2Vec3 m_impulse;

	// Solver temp
	int32 m_indexA;
	int32 m_indexB;
	b2Vec2 m_rA;
	b2Vec2 m_rB;
	b2Vec2 m_localCenterA;
	b2Vec2 m_localCenterB;
	float32 m_invMassA;
	float32 m_invMassB;
	float32 m_invIA;
	float32 m_invIB;
	b2Mat33 m_mass;
};

#endif
