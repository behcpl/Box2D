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

#include "Box2D/Dynamics/Joints/b2TeleportJoint.h"
#include "Box2D/Dynamics/b2Body.h"
#include "Box2D/Dynamics/b2TimeStep.h"

void b2TeleportJointDef::Initialize(b2Body* bA, b2Body* bB, const b2Vec2& o)
{
	bodyA = bA;
	bodyB = bB;
	offset = o;
}

b2TeleportJoint::b2TeleportJoint(const b2TeleportJointDef* def)
: b2Joint(def)
{
	m_offset = def->offset;

	m_impulse.SetZero();
}

void b2TeleportJoint::InitVelocityConstraints(const b2SolverData& data)
{
	m_indexA = m_bodyA->m_islandIndex;
	m_indexB = m_bodyB->m_islandIndex;
	m_localCenterA = m_bodyA->m_sweep.localCenter;
	m_localCenterB = m_bodyB->m_sweep.localCenter;
	m_invMassA = m_bodyA->m_invMass;
	m_invMassB = m_bodyB->m_invMass;
	m_invIA = m_bodyA->m_invI;
	m_invIB = m_bodyB->m_invI;

	float32 aA = data.positions[m_indexA].a;
	b2Vec2 vA = data.velocities[m_indexA].v;
	float32 wA = data.velocities[m_indexA].w;

	float32 aB = data.positions[m_indexB].a;
	b2Vec2 vB = data.velocities[m_indexB].v;
	float32 wB = data.velocities[m_indexB].w;

	b2Rot qA(aA), qB(aB);

	m_rA = b2Mul(qA, -m_localCenterA);
	m_rB = b2Mul(qB, -m_localCenterB);

	// J = [-I -r1_skew I r2_skew]
	//     [ 0       -1 0       1]
	// r_skew = [-ry; rx]

	// Matlab
	// K = [ mA+r1y^2*iA+mB+r2y^2*iB,  -r1y*iA*r1x-r2y*iB*r2x,          -r1y*iA-r2y*iB]
	//     [  -r1y*iA*r1x-r2y*iB*r2x, mA+r1x^2*iA+mB+r2x^2*iB,           r1x*iA+r2x*iB]
	//     [          -r1y*iA-r2y*iB,           r1x*iA+r2x*iB,                   iA+iB]

	float32 mA = m_invMassA, mB = m_invMassB;
	float32 iA = m_invIA, iB = m_invIB;

	b2Mat33 K;
	K.ex.x = mA + mB + m_rA.y * m_rA.y * iA + m_rB.y * m_rB.y * iB;
	K.ey.x = -m_rA.y * m_rA.x * iA - m_rB.y * m_rB.x * iB;
	K.ez.x = -m_rA.y * iA - m_rB.y * iB;
	K.ex.y = K.ey.x;
	K.ey.y = mA + mB + m_rA.x * m_rA.x * iA + m_rB.x * m_rB.x * iB;
	K.ez.y = m_rA.x * iA + m_rB.x * iB;
	K.ex.z = K.ez.x;
	K.ey.z = K.ez.y;
	K.ez.z = iA + iB;

	if (K.ez.z == 0.0f)
	{
		K.GetInverse22(&m_mass);
	}
	else
	{
		K.GetSymInverse33(&m_mass);
	}

	if (data.step.warmStarting)
	{
		// Scale impulses to support a variable time step.
		m_impulse *= data.step.dtRatio;

		b2Vec2 P(m_impulse.x, m_impulse.y);

		vA -= mA * P;
		wA -= iA * (b2Cross(m_rA, P) + m_impulse.z);

		vB += mB * P;
		wB += iB * (b2Cross(m_rB, P) + m_impulse.z);
	}
	else
	{
		m_impulse.SetZero();
	}

	data.velocities[m_indexA].v = vA;
	data.velocities[m_indexA].w = wA;
	data.velocities[m_indexB].v = vB;
	data.velocities[m_indexB].w = wB;
}

void b2TeleportJoint::SolveVelocityConstraints(const b2SolverData& data)
{
	b2Vec2 vA = data.velocities[m_indexA].v;
	float32 wA = data.velocities[m_indexA].w;
	b2Vec2 vB = data.velocities[m_indexB].v;
	float32 wB = data.velocities[m_indexB].w;

	float32 mA = m_invMassA, mB = m_invMassB;
	float32 iA = m_invIA, iB = m_invIB;

	b2Vec2 Cdot1 = vB + b2Cross(wB, m_rB) - vA - b2Cross(wA, m_rA);
	float32 Cdot2 = wB - wA;
	b2Vec3 Cdot(Cdot1.x, Cdot1.y, Cdot2);

	b2Vec3 impulse = -b2Mul(m_mass, Cdot);
	m_impulse += impulse;

	b2Vec2 P(impulse.x, impulse.y);

	vA -= mA * P;
	wA -= iA * (b2Cross(m_rA, P) + impulse.z);

	vB += mB * P;
	wB += iB * (b2Cross(m_rB, P) + impulse.z);

	data.velocities[m_indexA].v = vA;
	data.velocities[m_indexA].w = wA;
	data.velocities[m_indexB].v = vB;
	data.velocities[m_indexB].w = wB;
}

bool b2TeleportJoint::SolvePositionConstraints(const b2SolverData& data)
{
	b2Vec2 cA = data.positions[m_indexA].c;
	float32 aA = data.positions[m_indexA].a;
	b2Vec2 cB = data.positions[m_indexB].c - m_offset;
	float32 aB = data.positions[m_indexB].a;

	b2Rot qA(aA), qB(aB);

	float32 mA = m_invMassA, mB = m_invMassB;
	float32 iA = m_invIA, iB = m_invIB;

	b2Vec2 rA = b2Mul(qA, -m_localCenterA);
	b2Vec2 rB = b2Mul(qB, -m_localCenterB);

	float32 positionError, angularError;

	b2Mat33 K;
	K.ex.x = mA + mB + rA.y * rA.y * iA + rB.y * rB.y * iB;
	K.ey.x = -rA.y * rA.x * iA - rB.y * rB.x * iB;
	K.ez.x = -rA.y * iA - rB.y * iB;
	K.ex.y = K.ey.x;
	K.ey.y = mA + mB + rA.x * rA.x * iA + rB.x * rB.x * iB;
	K.ez.y = rA.x * iA + rB.x * iB;
	K.ex.z = K.ez.x;
	K.ey.z = K.ez.y;
	K.ez.z = iA + iB;

	b2Vec2 C1 =  cB + rB - cA - rA;
	float32 C2 = aB - aA;

	positionError = C1.Length();
	angularError = b2Abs(C2);

	b2Vec3 C(C1.x, C1.y, C2);
	
	b2Vec3 impulse;
	if (K.ez.z > 0.0f)
	{
		impulse = -K.Solve33(C);
	}
	else
	{
		b2Vec2 impulse2 = -K.Solve22(C1);
		impulse.Set(impulse2.x, impulse2.y, 0.0f);
	}

	b2Vec2 P(impulse.x, impulse.y);

	cA -= mA * P;
	aA -= iA * (b2Cross(rA, P) + impulse.z);

	cB += mB * P;
	aB += iB * (b2Cross(rB, P) + impulse.z);

	data.positions[m_indexA].c = cA;
	data.positions[m_indexA].a = aA;
	data.positions[m_indexB].c = cB + m_offset;
	data.positions[m_indexB].a = aB;

	return positionError <= b2_linearSlop && angularError <= b2_angularSlop;
}

b2Vec2 b2TeleportJoint::GetAnchorA() const
{
	return m_bodyA->GetPosition();
}

b2Vec2 b2TeleportJoint::GetAnchorB() const
{
	return m_bodyB->GetPosition();
}

b2Vec2 b2TeleportJoint::GetReactionForce(float32 inv_dt) const
{
	b2Vec2 P(m_impulse.x, m_impulse.y);
	return inv_dt * P;
}

float32 b2TeleportJoint::GetReactionTorque(float32 inv_dt) const
{
	return inv_dt * m_impulse.z;
}

void b2TeleportJoint::Dump()
{
	int32 indexA = m_bodyA->m_islandIndex;
	int32 indexB = m_bodyB->m_islandIndex;

	b2Log("  b2TeleportJointDef jd;\n");
	b2Log("  jd.bodyA = bodies[%d];\n", indexA);
	b2Log("  jd.bodyB = bodies[%d];\n", indexB);
	b2Log("  jd.collideConnected = bool(%d);\n", m_collideConnected);
	b2Log("  jd.offset.Set(%.15lef, %.15lef);\n", m_offset.x, m_offset.y);
	b2Log("  joints[%d] = m_world->CreateJoint(&jd);\n", m_index);
}
