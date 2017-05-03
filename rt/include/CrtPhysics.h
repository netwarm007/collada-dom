/*
 * Copyright 2006 Sony Computer Entertainment Inc.
 *
 * Licensed under the MIT Open Source License, for details please see license.txt or the website
 * http://www.opensource.org/licenses/mit-license.php
 *
 */
/*
Bullet Continuous Collision Detection and Physics Library
Copyright (c) 2003-2006 Erwin Coumans  http://continuousphysics.com/Bullet/

This software is provided 'as-is', without any express or implied warranty.
In no event will the authors be held liable for any damages arising from the use of this software.
Permission is granted to anyone to use this software for any purpose, 
including commercial applications, and to alter it and redistribute it freely, 
subject to the following restrictions:

1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
3. This notice may not be removed or altered from any source distribution.
*/

#ifndef __COLLADA_RT__PHYSICS_H__
#define __COLLADA_RT__PHYSICS_H__ 

#ifdef SPU_BULLET //PlayStation 3
#include "Physics/TaskUtil/SpursTask.h"
#include "BulletMultiThreaded/SequentialThreadSupport.h"
#include "Physics/TaskUtil/spurs_util_spu_printf_service.h"
#include "SpuDispatch/btPhysicsEffectsWorld.h"
#include "BulletDynamics/Dynamics/btDiscreteDynamicsWorld.h"
#endif //SPU_BULLET

//http://bullet.sf.net Erwin Coumans
class btBroadphaseInterface;
class btCollisionDispatcher;
class btCollisionShape;
class btConstraintSolver;
class btDefaultCollisionConfiguration;
class btDiscreteDynamicsWorld;
class btRigidBody;
class btThreadSupportInterface;

#include "CrtScene.h"

COLLADA_(namespace)
{
	namespace RT
	{//-.
//<-----'
		
struct RigidBody
{		
	xs::string Sid;

	btCollisionShape *Shape;

	RT::Float Mass; bool Dynamic; 

	Collada05::const_physics_material Material;

	typedef Collada05::const_rigid_body type;

	RigidBody():Sid(),Shape(),Mass(),Dynamic(){}

	//This belongs in CrtSceneRead by this point.
	template<class T>
	/**HACK
	 * Assuming <rigid_body><technique_common> and
	 * <instance_rigid_body><technique_common> are 
	 * compatible with <shape> but ONLY WITH SHAPE.
	 */
	inline bool OverrideWith(T &in, xs::anyURI &URI)
	{
		//The schema requires this element, but it's very often empty.
		if(!in.technique_common.empty())
		{
			const T::local__technique_common *tc = in.technique_common;
			Sid = in.sid;
			if(!tc->mass.empty())
			Mass = tc->mass->value;
			if(!tc->dynamic.empty()) 
			Dynamic = tc->dynamic->value;
			if(!tc->physics_material.empty())
			Material = tc->physics_material;			
			else if(!tc->instance_physics_material.empty())
			{
				URI = tc->instance_physics_material->url;
				Material = URI.get<Collada05::physics_material>();
			}
			_LoadShape(*(Collada05::rigid_body::technique_common*)&tc);
		}
		return Shape!=nullptr;
	}
	void _LoadShape(Collada05::const_rigid_body::technique_common);
};

//SCHEDULED FOR REMOVAL?
/**
 * This class is mainly so the look up routine can work with 
 * it or @c RT::RigidBody. That routine is crazy complicated.
 */
struct RigidConstraint
{		
	xs::string Sid;

	Collada05::const_rigid_constraint rigid_constraint;
    
	typedef Collada05::const_rigid_constraint type;	 

	RigidConstraint(Collada05::const_rigid_constraint cp)
	{
		if(cp->ref_attachment!=nullptr&&cp->attachment!=nullptr)
		{
			rigid_constraint = cp; Sid = cp->sid;
		}
	}
};

class PhysicsModel : public RT::Base 
{
COLLADA_(public)

	std::vector<RT::RigidBody> RigidBodies;	
	std::vector<RT::RigidConstraint> RigidConstraints;
};

/**				
 * RT::Physics manages the physical systems section of COLLADA.
 *
 * EXTENSIBILITY
 * This class historically used http://bullet.sf.net to do its
 * physics simulations. If clients want to use a different way
 * going forward, maybe a @c template based approach is better
 * than using @c virtual based interfaces. That would mean the
 * @c RT::Frame class must either choose one or update physics
 * by calling a client procedure when it would otherwise do so.
 */
class Physics
{
COLLADA_(public)

	#ifdef NO_BULLET
	enum{ OK=0 };
	#else
	enum{ OK=1 };
	#endif

	//Physics();
	void Init(); bool Initialized;
	
	/**WARNING, DEBUGGING API, LEGACY
	 * @warning This is a routine that had existed for overwriting <node> transforms
	 * and saving the modified DOM to a file name with a snapshot number appended to
	 * it. It's rewritten so to not be destructive, but this requires the file names
	 * to be translated to new URIs. This translation is severely limited. It treats
	 * @c RT::Main.URL like an index.html file and won't write outside the directory.
	 * Instances are not duplicated. They could be if it's that important to someone.
	 * @param remote_directory is the directory to write/rebase the files to. To get
	 * the old behavior of appending a number for each snapshot, it must be manually
	 * included in @a remote_directory.
	 */
	daeOK Snapshot(const xs::anyURI &remote_directory);
	
	/**
	 * Up is UP_Y; units is meters.
	 */
	void SetGravity(RT::Float x, RT::Float y, RT::Float z);

	void Update(RT::Float delta_time),Reset();

	void VisualizeWorld();

COLLADA_(private)
		
	friend class RT::Frame;
	//This could just empty out the simulation,
	//but the "bt" memory model is complicated.
	inline void Clear()
	{
		this->~Physics(); new(this) RT::Physics; Initialized = false; //Init();
	}

	friend struct RT::RigidBody;
	friend struct RT::DBase::LoadScene_Physics;
	void Bind_instance_rigid_body(int,RT::RigidBody&,RT::Stack_Data&);
	void Init_velocity(RT::Stack_Data&,Collada05::const_instance_rigid_body&);
	void Bind_instance_rigid_constraint(int,int,Collada05::const_rigid_constraint);	

	//http://bullet.sf.net Erwin Coumans
	struct bt : RT::Memory 
	{
		btCollisionDispatcher *dispatcher;
		btBroadphaseInterface *pairCache;
		btConstraintSolver *constraintSolver;
		btDefaultCollisionConfiguration *collisionConfiguration;
		btDiscreteDynamicsWorld *dynamicsWorld;

		#ifdef SPU_BULLET //PlayStation 3
		btThreadSupportInterface* collisionThreadSupport;
		CellSpurs SpursInstance;	
		SampleUtilSpursPrintfService SpursPrintfService;
		#endif
	}bt;

	struct InitialVelocity
	{
		btRigidBody *Physics;
		RT::Float Linear[3],Angular[3];
		void SetVelocities();
	};
	std::vector<InitialVelocity> InitialVelocities;
};

//-------.
	}//<-'
}

#endif //__COLLADA_RT__PHYSICS_H__
/*C1071*/