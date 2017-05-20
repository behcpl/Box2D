/*
* Copyright (c) 2006-2010 Erin Catto http://www.box2d.org
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

#ifndef WRAPPED_WORLDS_H
#define WRAPPED_WORLDS_H

/// This is a simple test of spliced worlds.
/// Only two parts exist here, and there is no dynamic relocation of bodies.

/// The idea is to have very large worlds splited into chunks with acceptable
/// numeric stability. Each time body is close to border, a copy is made in
/// another space (moved back to the other edge using offset or chunk size),
/// and this two bodies are glued using teleport joint. When original body
/// is totally outside it's space, only copy remains (and becames main body)

/// This feature requires:
///  - teleport joint
///  - spaces

#define OFFSET 40.0f

class WrappedWorlds : public Test
{
public:
	WrappedWorlds()
	{
		m_world->SetSpacesCount(2);
		
		b2Body* body_sp1 = NULL;
		b2Body* body_sp2 = NULL;

		// Space 1 ground
		{
			b2BodyDef bd;
			bd.space = 0;
			b2Body* ground = m_world->CreateBody(&bd);

			b2Vec2 vs[11];
			vs[0].Set(-25.0f, 15.0f);
			vs[1].Set(-20.0f, 12.0f);
			vs[2].Set(-15.0f, 5.0f);
			vs[3].Set(-10.0f, 7.0f);
			vs[4].Set(-5.0f, 6.0f);
			vs[5].Set(0.0f, 3.0f);
			vs[6].Set(5.0f, 4.0f);
			vs[7].Set(10.0f, 0.0f);
			vs[8].Set(15.0f, 2.0f);
			vs[9].Set(20.0f, 0.0f);
			vs[10].Set(25.0f, 1.0f);
			b2ChainShape shape;
			shape.CreateChain(vs, 11);
			
			b2FixtureDef def;
			def.shape = &shape;
			ground->CreateFixture(&def);
		}
		
		// Space 2 ground
		{
			b2BodyDef bd;
			bd.space = 1;
			b2Body* ground = m_world->CreateBody(&bd);

			b2Vec2 vs[11];
			vs[0].Set(-25.0f, 2.0f);
			vs[1].Set(-20.0f, 0.0f);
			vs[2].Set(-15.0f, 1.0f);
			vs[3].Set(-10.0f, 1.0f);
			vs[4].Set(-5.0f, 3.0f);
			vs[5].Set(0.0f, 0.0f);
			vs[6].Set(5.0f, 2.0f);
			vs[7].Set(10.0f, 4.0f);
			vs[8].Set(15.0f, 4.0f);
			vs[9].Set(20.0f, 4.0f);
			vs[10].Set(25.0f, 10.0f);
			b2ChainShape shape;
			shape.CreateChain(vs, 11);

			b2FixtureDef def;
			def.shape = &shape;
			ground->CreateFixture(&def);
		}
		
		// Space 1 Test body
		{
			b2BodyDef bd;
			bd.space = 0;
			bd.position.Set(0.0f, 5.0f);
			bd.type = b2_dynamicBody;
			body_sp1 = m_world->CreateBody(&bd);

			b2PolygonShape shape;
			shape.SetAsBox(0.5f, 0.5f);

			b2FixtureDef def;
			def.shape = &shape;
			def.density = 20.0f;
			body_sp1->CreateFixture(&def);
		}	
		
		// Space 2 Test body
		{
			b2BodyDef bd;
			bd.space = 1;
			bd.position.Set(0.0f - OFFSET, 5.0f);
			bd.type = b2_dynamicBody;
			body_sp2 = m_world->CreateBody(&bd);

			b2PolygonShape shape;
			shape.SetAsBox(0.5f, 0.5f);

			b2FixtureDef def;
			def.shape = &shape;
			def.density = 20.0f;
			body_sp2->CreateFixture(&def);
		}
		// Teleport joint
		{
			b2TeleportJointDef teleport_desc;
			teleport_desc.Initialize(body_sp1, body_sp2, b2Vec2(-OFFSET, 0.0f));

			b2TeleportJoint* joint = (b2TeleportJoint*)m_world->CreateJoint(&teleport_desc);
		}
	}

	void Step(Settings* settings)
	{
		Test::Step(settings);
		g_debugDraw.DrawString(5, m_textLine, "This tests teleport joint and spaces.");
		m_textLine += DRAW_STRING_NEW_LINE;
		g_debugDraw.DrawString(5, m_textLine, "Limitation: Only space index 0 is interactable in this demo framework.");
		m_textLine += DRAW_STRING_NEW_LINE;
	}

	static Test* Create()
	{
		return new WrappedWorlds;
	}
};

#endif
