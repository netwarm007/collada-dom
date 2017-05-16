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

#include <RT.pch.h> //PCH

#include "CrtScene.h"
#include "CrtPhysics.h"
#include "CrtRender.h"
#include "CrtGeometry.h"

#if 2==COLLADA_DOM_PRECISION
#define BT_USE_DOUBLE_PRECISION
#endif

#ifdef NO_BULLET
class btTransform;
class btTypedConstraint;
class btVector3;
#else //http://bullet.sf.net Erwin Coumans
#include "btBulletDynamicsCommon.h"
#include "LinearMath/btQuickprof.h"
#include "LinearMath/btIDebugDraw.h"
#include "LinearMath/btTransform.h"
#include "LinearMath/btVector3.h"
#include "LinearMath/btDefaultMotionState.h"
#include "BulletCollision/CollisionShapes/btShapeHull.h"
#include "BulletCollision/CollisionShapes/btBoxShape.h"
#include "BulletCollision/CollisionShapes/btSphereShape.h"
#include "BulletCollision/CollisionShapes/btCylinderShape.h"
#include "BulletCollision/CollisionShapes/btConeShape.h"
#include "BulletCollision/CollisionShapes/btStaticPlaneShape.h"
#include "BulletCollision/CollisionShapes/btConvexHullShape.h"
#include "BulletCollision/CollisionShapes/btTriangleMesh.h"
#include "BulletCollision/CollisionShapes/btConvexTriangleMeshShape.h"
#include "BulletCollision/CollisionShapes/btTriangleMeshShape.h"
#include "BulletCollision/CollisionShapes/btCompoundShape.h"
#ifdef SPU_BULLET //PlayStation 3
#include "SpuDispatch/btPhysicsEffectsWorld.h"//currently in Bullet/SpuDispatch
#include "BulletCollision/BroadphaseCollision/btDbvtBroadphase.h"
#include "SpuDispatch/BulletDebugDrawer.h"
#include "BulletMultiThreaded/SequentialThreadSupport.h"
#include "BulletMultiThreaded/SequentialThreadSupport.h"
#include "BulletMultiThreaded/SpuGatheringCollisionDispatcher.h"
#include "SpuDispatch/BulletCollisionSpursSupport.h"
#include "SpuDispatch/BulletConstraintSolverSpursSupport.h"
#include "SpuDispatch/btPhysicsEffectsWorld.h"
#include "Physics/TaskUtil/SpursTask.h"
#include "Physics/TaskUtil/spurs_util_spu_printf_service.h"
#define SPU_THREAD_GROUP_PRIORITY 250
#define SPURS_THREAD_PRIORITY 1000
#define SPURS_NAME "PhysicsEffects"
#define NUM_MAX_SPU 3
#endif //SPU_BULLET //PlayStation 3
#endif //NO_BULLET 

COLLADA_(namespace)
{
	namespace RT
	{//-.
//<-----'

void RT::Physics::Reset()
{
	if(!Initialized) return;

	#ifndef NO_BULLET		
	//This seems to do the opposite.
	//bt.dynamicsWorld->synchronizeMotionStates();
	size_t i=0,iN = bt.dynamicsWorld->getNumCollisionObjects();
	for(btVector3 zero(0,0,0);i<iN;i++)
	{
		btRigidBody *rb = btRigidBody::upcast
		(bt.dynamicsWorld->getCollisionObjectArray()[i]);
		if(rb==nullptr) continue;

		rb->clearForces();
		rb->setLinearVelocity(zero);
		rb->setAngularVelocity(zero);
		//Doesn't help???
		//rb->applyGravity();
		rb->getMotionState()->getWorldTransform(rb->getWorldTransform());
	}
	//Doesn't help???
	//bt.dynamicsWorld->applyGravity();
	for(size_t i=0;i<InitialVelocities.size();i++)
	InitialVelocities[i].SetVelocities();
	#endif
}
void RT::Physics::Update(RT::Float delta_time)
{
	if(!Initialized) return;
	//bt.dynamicsWorld->updateAabbs();

	#ifndef NO_BULLET		
	//Is this correct????
	//Adding this because things just hang in the air after Reset.
	bt.dynamicsWorld->applyGravity();
	#ifdef NDEBUG
	#error What about <time_step>?
	#endif
	const float fixedTimeStep = 0.25f/60.f;	
	bt.dynamicsWorld->stepSimulation(delta_time,8,fixedTimeStep);
	#endif
}	 
void RT::Physics::VisualizeWorld()
{
	if(!Initialized) return;
	#ifndef NO_BULLET
	static struct : btIDebugDraw
	{	
		virtual void drawLine(const btVector3 &a, const btVector3 &b, const btVector3 &color)
		{
			glVertex3d(a[0],a[1],a[2]); glVertex3d(b[0],b[1],b[2]); (void)color;
		}		
		virtual void drawContactPoint(const btVector3&,const btVector3&,btScalar,int,const btVector3&){}
		virtual void reportErrorWarning(const char*){}
		virtual void draw3dText(const btVector3&,const char*){}	
		virtual void setDebugMode(int debugMode){}	
		virtual int getDebugMode() const{ return DBG_DrawWireframe; }
	}DrawHierarchy;
	bt.dynamicsWorld->setDebugDrawer(&DrawHierarchy);
	bt.dynamicsWorld->debugDrawWorld();
	#endif
}

void RT::Physics::SetGravity(RT::Float x, RT::Float y, RT::Float z)
{
	if(!Initialized) return;
	#ifndef NO_BULLET		
	bt.dynamicsWorld->setGravity(btVector3(x,y,z));
	#endif
	daeEH::Verbose<<"Set gravity to "<<x<<","<<y<<","<<z;
}

static void MatrixToBulletPhysicsOrViceVersa(RT::Float *dst)
{
	if(RT::Matrix_is_COLLADA_order) 
	{
		//btTransform is a rotation matrix with vector.
		std::swap(dst[M30],dst[M03]);
		std::swap(dst[M31],dst[M13]);
		std::swap(dst[M32],dst[M23]);
	}
	else RT::Matrix3x3Transpose(dst); //debugDrawWorld
}

#ifdef NO_BULLET //EXPERIMENTAL
#define using_namespace_NO_BULLET using namespace CrtPhysics;
#else
#define using_namespace_NO_BULLET
#endif
namespace CrtPhysics //EXPERIMENTAL
{
	typedef struct NO_BULLET_shape
	{
		NO_BULLET_shape(...){}
		void addChildShape(){}
	}btCompoundShape,btCollisionShape
	,btStaticPlaneShape,btBoxShape,btSphereShape,btCylinderShapeZ
	,btShapeHull,btConvexHullShape;

	//16B ALIGNED MEMORY
	struct transform : btTransform
	{
		RT::Matrix &thisMatrix()
		{
			return *(RT::Matrix*)this;
		}
		transform(const transform &cp):btTransform(cp)
		{}
		transform(const daeContents &content)
		{
			RT::Matrix &m = thisMatrix();
			//Not sure if btTransform can handle non-rotation 
			//matrices. Might want to try and see?
			if(1)
			{			
				//Are these equivalent?
				RT::MatrixLoadIdentity(thisMatrix());
				if(RT::Asset.Up==RT::Up::X_UP)
				RT::MatrixRotateAngleAxis(m,0,0,1,90);				
				else if(RT::Asset.Up==RT::Up::Z_UP)
				RT::MatrixRotateAngleAxis(m,1,0,0,90);
			}
			else RT::MatrixLoadAsset(m,RT::Asset.Up);

			content.for_each_child(*this);			

			RT::MatrixToBulletPhysicsOrViceVersa((RT::Float*)this);
		}
		void operator()(const const_daeChildRef &e)
		{
			RT::Matrix &m = thisMatrix(); RT::Float x,y,z,a;
			
			switch(e->getElementType())
			{
			case DAEP::Schematic<Collada05::rotate>::genus:
			case DAEP::Schematic<Collada08::rotate>::genus:		
			{
				if(1!=e.name())
				if(4==COLLADA_RT_cast(rotate,e)->value->get4at(0,x,y,z,a))
				MatrixRotateAngleAxis(m,x,y,z,a);
				break;		
			}
			case DAEP::Schematic<Collada05::translate>::genus:
			case DAEP::Schematic<Collada08::translate>::genus:
			{
				if(1!=e.name())
				if(3==COLLADA_RT_cast(translate,e)->value->get3at(0,x,y,z))
				RT::MatrixTranslate(m,x,y,z);
				break;	
			}}
		}
	};
}

void RT::Physics::Bind_instance_rigid_constraint
(int constrain1, int constrain2, Collada05::const_rigid_constraint rigid_constraint)
{	
	btRigidBody *body1 = 0, *body2 = 0;
	#ifdef NDEBUG
	#error This is an exponential lookup.
	#endif
	int many1 = 0, many2 = 0;
	for(size_t i=0;i<RT::Main.Stack.Data.size();i++)
	{	
		btRigidBody *body = RT::Main.Stack.Data[i].Physics;
		if(body==nullptr) continue;

		if((void*)constrain1==body->getUserPointer())
		{
			many1++; body1 = body;
		}
		if((void*)constrain2==body->getUserPointer())
		{
			many2++; body2 = body;
		}
	}
	if(1!=many1||1!=many2)
	{
		daeEH::Error<<"Too few or too many of one or both rigid bodies "
		<<rigid_constraint->ref_attachment->rigid_body<<"("<<many1<<"),"
		<<rigid_constraint->attachment->rigid_body<<"("<<many2<<") for constraint "<<rigid_constraint->sid<<"\n"
		<<"(Expected both to be 1. Are many-to-many semantics called for?)";
		return;
	}

	Collada05::const_limits limits = rigid_constraint->technique_common->limits;
	btVector3 linearMin(0,0,0),linearMax(0,0,0),angularMin(0,0,0),angularMax(0,0,0);
	if(limits!=nullptr)
	{
		Collada05::const_min min,max;
		min = limits->linear->min; max = limits->linear->max;
		if(min!=nullptr) min->value->get3at(0,linearMin[0],linearMin[1],linearMin[2]);
		if(max!=nullptr) max->value->get3at(0,linearMax[0],linearMax[1],linearMax[2]);

		min = limits->swing_cone_and_twist->min; max = limits->swing_cone_and_twist->max;		
		if(min!=nullptr) min->value->get3at(0,angularMin[0],angularMin[1],angularMin[2]);
		if(max!=nullptr) max->value->get3at(0,angularMax[0],angularMax[1],angularMax[2]);
	}
	
	//Assuming the same <asset> space applies to both attachments.
	//It can be interpreted as using each attachment's body/node??
	CrtPhysics::transform 
	ref_transform(rigid_constraint->ref_attachment->content),
	transform(rigid_constraint->attachment->content);

	const bool useReferenceFrameA = true;
	btGeneric6DofConstraint *c = bt.New<btGeneric6DofConstraint>
	(*body1,*body2,ref_transform,transform,useReferenceFrameA);
			
	//convert INF / -INF into lower > upper			
	const RT::Float linearCheckThreshold = 999999;
	const RT::Float angularCheckThreshold = 180; //check this											   
	//free means upper < lower, 
	//locked means upper == lower
	//limited means upper > lower
	//limitIndex: first 3 are linear, next 3 are angular		
	for(size_t i=0;i<3;i++)
	{
		if(linearMin[i]<-linearCheckThreshold
		 ||linearMax[i]>+linearCheckThreshold)
		{
			//disable limits
			linearMin[i] = 1; linearMax[i] = 0;
		}

		if(angularMin[i]<-angularCheckThreshold
		 ||angularMax[i]>+angularCheckThreshold)
		{
			//disable limits
			angularMin[i] = 1; angularMax[i] = 0;
		}
	}
	c->setLinearLowerLimit(linearMin);
	c->setLinearUpperLimit(linearMax);
	c->setAngularLowerLimit(angularMin);
	c->setAngularUpperLimit(angularMax); bt.dynamicsWorld->addConstraint(c); 	
}

void RT::Physics::Bind_instance_rigid_body(int constraint, RT::RigidBody &out, RT::Stack_Data &in)
{	
	RT::Float mass = out.Mass;
	btVector3 localInertia(0,0,0);
	if(!out.Dynamic&&mass!=0)
	{
		//This is an implementation detail.
		//daeEH::Warning<<"Non-dynamic objects need to have zero mass!";
		mass = 0;
	}
	if(out.Dynamic&&mass==0)
	{
		//This is an implementation detail.
		//daeEH::Warning<<"Dynamic rigidbodies need nonzero mass!";
		mass = 1;
	}	
	if(mass!=0)
	out.Shape->calculateLocalInertia(mass,localInertia);
	
	struct _m : btMotionState
	{
		RT::Matrix &m; _m(RT::Matrix &m):m(m){}

		static void copy(RT::Float *dst, const RT::Float *src)
		{
			MatrixCopy(*(RT::Matrix*)src,*(RT::Matrix*)dst); 	

			RT::MatrixToBulletPhysicsOrViceVersa(dst);
		}
		virtual void getWorldTransform(btTransform &wt)const
		{
			daeCTC<sizeof(wt)==sizeof(RT::Matrix)>();
			copy(&wt.getBasis()[0][0],m);
		}
		virtual void setWorldTransform(const btTransform &wt)
		{
			copy(m,&wt.getBasis()[0][0]); 
			m[M33] = 1; assert(m[M03]==0&&m[M13]==0&&m[M23]==0);
		}
	}*ms = bt.New<_m>(in.Matrix);
	//using motionstate is recommended, it provides interpolation capabilities,
	//and only synchronizes 'active' objects
	btRigidBody *body = bt.New<btRigidBody>(mass,ms,out.Shape,localInertia);
	
	if(out.Material!=nullptr)
	{
		Collada05::const_physics_material::technique_common tc;
		tc = out.Material->technique_common;
		//It seems like dynamic_friction would be less if not the only value.
		//http://www.bulletphysics.org/Bullet/phpBB3/viewtopic.php?f=9&t=3821
		body->setFriction(std::max<RT::Float>
		(tc->static_friction->value->*RT::Float(0)
		,tc->dynamic_friction->value->*RT::Float(0)));		
		body->setRestitution(tc->restitution->value->*RT::Float(0));
	}

	//Trying to keep from sliding slowly forever when rested.
	body->setDeactivationTime(0.05f);
	body->setSleepingThresholds(9,9); //Linear/angular what??
	//body->setDamping(0.3f,0.3f);

	//This is an opaque identifier for attaching constraints.
	//It's hierarchical.
	body->setUserPointer((void*)constraint);

	bt.dynamicsWorld->addRigidBody(body); in.Physics = body;
}
void RT::Physics::Init_velocity(RT::Stack_Data &in, Collada05::const_instance_rigid_body &iv)
{	
	Collada05::const_velocity vv = iv->technique_common->velocity;
	Collada05::const_angular_velocity av = iv->technique_common->angular_velocity;	
	InitialVelocities.push_back(InitialVelocity());
	InitialVelocity &i = InitialVelocities.back(); 
	i.Physics = in.Physics;
	if(vv!=nullptr)
	{
		vv->value->get3at(0,i.Linear[0],i.Linear[1],i.Linear[2]);
		RT::Asset.Mult(i.Linear[0],i.Linear[1],i.Linear[2]);
	}
	if(av!=nullptr)
	{	
		av->value->get3at(0,i.Angular[0],i.Angular[1],i.Angular[2]);
		RT::Float m = RT::DEGREES_TO_RADIANS;
		std::swap(m,RT::Asset.Meter); 
		RT::Asset.Mult(i.Angular[0],i.Angular[1],i.Angular[2]);
		std::swap(m,RT::Asset.Meter);
	}
	i.SetVelocities();
}
void RT::Physics::InitialVelocity::SetVelocities()
{
	#ifndef NO_BULLET
	Physics->setLinearVelocity(btVector3(Linear[0],Linear[1],Linear[2]));
	Physics->setAngularVelocity(btVector3(Angular[0],Angular[1],Angular[2]));
	#endif
}

//Things work without this but the debugDrawWorld shows boxes that are
//in no way accurrate to the shape, with very large spaces around them.
#if 1
#define CrtPhysics_setMargin(x) x->setMargin(0.0001f*10);
#else
#define CrtPhysics_setMargin(x)
#endif
void RT::RigidBody::_LoadShape(Collada05::const_rigid_body::technique_common &in)
{
	using_namespace_NO_BULLET //EXPERIMENTAL

	struct RT::Physics::bt &bt = RT::Main.Physics.bt;

	//POINT-OF-NO-RETURN// //POINT-OF-NO-RETURN//

	//A setLocalScaling API implements <unit meter> last.
	//1 makes CrtPhysics::transform not scale <translate>.
	const RT::Float m = RT::Asset.Meter;
	if(m!=1) RT::Asset.Meter = 1; //POINT-OF-NO-RETURN
	bool requires_compound = RT::Asset.Up!=RT::Up::Y_UP;	

	btCompoundShape *compound = nullptr;
	btCollisionShape *current = nullptr;
	Collada05::const_rigid_body::technique_common::shape shape;
	for(size_t i=0;i<in->shape.size();i++)
	{
		shape = in->shape[i]; current = nullptr;

		if(!shape->plane.empty())
		{
			Collada05::const_equation plane = shape->plane->equation;
			if(plane!=nullptr)
			{
				RT::Float a,b,c,d;
				if(4==plane->value->get4at(0,a,b,c,d))
				current = bt.New<btStaticPlaneShape>(btVector3(a,b,c),d);
			}
		}
		else if(!shape->box.empty())
		{
			Collada05::const_half_extents box = shape->box->half_extents;			
			RT::Float x,y,z;
			if(box!=nullptr&&3==box->value->get3at(0,x,y,z))
			current = bt.New<btBoxShape>(btVector3(x,y,z));
		}
		else if(!shape->sphere.empty())
		{
			RT::Float radius = shape->sphere->radius->value->*RT::Float(-1);
			if(radius>0)			
			current = bt.New<btSphereShape>(radius);
		}
		else if(!shape->cylinder.empty())
		{
			Collada05::const_cylinder cylinder = shape->cylinder;
			RT::Float radius1 = 0, radius2, height = 0;
			if(!cylinder->radius.empty())
			{
				switch(cylinder->radius->value->get2at(0,radius1,radius2))
				{
				case 1: radius2 = radius1; case 2: break; default: continue;
				}
				height = cylinder->height->value->*RT::Float(-1);
			}
			if(height>=0&&radius1>0&&radius2>0)			
			current = bt.New<btCylinderShapeZ>(btVector3(radius1,height,radius2));
		}
		else if(!shape->instance_geometry.empty())
		{
			xs::anyURI URI = shape->instance_geometry->url;
			linked_geometry:
			//HACK: This fix is temporary until something better is in reach.
			//Collada05::const_geometry geom = URI->get<Collada05::geometry>();
			xs::const_any yy = URI.getTargetedFragment();
			Collada05::const_geometry g05 = yy->a<Collada05::geometry>();
			Collada08::const_geometry g08 = yy->a<Collada08::geometry>();
			if(g05!=nullptr) URI = g05->convex_mesh->convex_hull_of->*"";
			if(g08!=nullptr) URI = g08->convex_mesh->convex_hull_of->*"";
			if(!URI.empty())
			{
				URI.setParentObject(yy);
				daeEH::Verbose<<"Linked geometry: "<<URI.getURI();
				//In theory convex_hull_of can go on indefinitely.
				goto linked_geometry; 
			}
			RT::Geometry *g = nullptr;
			RT::DBase *dbase = const_cast<RT::DBase*>(RT::Main.Data);
			if(g05!=nullptr) g = dbase->LoadGeometry(g05);
			if(g08!=nullptr) g = dbase->LoadGeometry(g08); //OVERLOAD
			if(nullptr==g) continue;

			#ifdef NDEBUG
			#error Use a table to not duplicate mesh-based shapes.
			#endif

			if(Dynamic&&g05!=nullptr&&g05->convex_mesh.empty()
			 ||Dynamic&&g08!=nullptr&&g08->convex_mesh.empty())
			{
				#ifdef NDEBUG
				#error Take a look at ConvexDecompositionDemo.cpp?
				#endif
				daeEH::Verbose<<"Assuming Physics <mesh> is sufficiently convex.\n"
				"(Decomposition support may be added later on if it is practical.)";
			}
			
			//THIS IS PRETTY SLOW.
			const size_t s = g->Positions.Stride;
			const size_t o = g->Positions.Offset;
			const RT::Float *v0 = &g->Positions->value[o];
			//UNUSED: There might be reasons to turn this back on later.
			if(0/*in.triangles.size*/) 
			{
				//2017: This path was coded for <mesh> with <triangles>.
				//It's been rewritten for preservation. But documentation
				//seems to advise against this approach.
				//Note: This formulation includes all of the mesh's faces.

				btTriangleMesh *trimesh = bt.New<btTriangleMesh>();
				
				for(size_t i=0;i<g->Elements.size();i++)
				if(GL_TRIANGLES==g->Elements[i].Mode)
				{
					GLuint *e = &g->ElementBuffer[g->Elements[i].Region];
					GLuint *et = e+g->Elements[i].Width;
					for(;e<et;e+=3)
					{
						const RT::Float *v;
						v = v0+s*e[0];
						btVector3 x(v[0],v[1],v[2]);
						v = v0+s*e[1];
						btVector3 y(v[0],v[1],v[2]);
						v = v0+s*e[2];
						btVector3 z(v[0],v[1],v[2]);
						trimesh->addTriangle(x,y,z);
					}
				}				

				if(Dynamic)
				{
					daeEH::Warning<<"Moving concave mesh not supported, transformed into convex.";
					current = bt.New<btConvexTriangleMeshShape>(trimesh);
				}
				else
				{
					daeEH::Verbose<<"Static concave triangle mesh added.";
					bool useQuantizedAabbCompression = false;
					current = bt.New<btBvhTriangleMeshShape>(trimesh,useQuantizedAabbCompression);
				}
			}
			else //BUILD A POINT-CLOUD HULL (HOW ACCURATE CAN IT BE?)
			{
				btConvexHullShape *convexHullShape = bt.New<btConvexHullShape>();

				for(size_t i=0,iN=i+s*g->Vertices;i<iN;i+=s)	
				convexHullShape->addPoint(btVector3(v0[i],v0[i+1],v0[i+2]));	
				daeEH::Verbose<<"Created convexHullShape with "<<g->Vertices<<" points.";
				current = convexHullShape;
			}
			
			#if 0
			#error This works but the shape is trash.
			//2017: The BT wiki says advises to not use large meshes.
			//http://www.bulletphysics.org/mediawiki-1.5.8/index.php/Collision_Shapes
			if(Dynamic&&g->Vertices>100)
			{
				assert(current->isConvex());
				btConvexShape *currently_convex = (btConvexShape*)current;
				
				//Trying to improve collision detection accuracy.
				CrtPhysics_setMargin(current);

				//http://www.bulletphysics.org/mediawiki-1.5.8/index.php/BtShapeHull_vertex_reduction_utility
				btShapeHull hull(currently_convex);
				btScalar margin = current->getMargin(); hull.buildHull(margin);

				//MEMORY-LEAK? The code in the link about doesn't delete the input shape.
				//http://bulletphysics.org/Bullet/BulletFull/btShapeHull_8cpp_source.html
				bt.Delete_back(current); //delete current;
				const btScalar *bt_New_compat = (btScalar*)hull.getVertexPointer();
				int bt_New_compat2 = hull.numVertices();
				current = bt.New<btConvexHullShape>(bt_New_compat,bt_New_compat2);

				assert(hull.numVertices()<=100&&hull.numVertices()<(int)g->Vertices);
				daeEH::Warning<<"Reduced large convexHullShape to "<<hull.numVertices()<<" points.";
			}
			#endif
		}

		//compound if more then 1 shape, or a non-identity local shapetransform
		if(current!=nullptr)
		{
			//Trying to improve collision detection accuracy.
			CrtPhysics_setMargin(current);

			if(requires_compound||!shape->rotate.empty()||!shape->translate.empty())
			{
				if(compound==nullptr)
				{
					requires_compound = true; compound = bt.New<btCompoundShape>();
				}
				compound->addChildShape(CrtPhysics::transform(shape->content),current);			
			}	 
			else requires_compound = true;

			//Doing this to the compound shape container isn't working below.
			if(m!=1) current->setLocalScaling(btVector3(m,m,m));
		}
	}

	//RESTORE TO ORIGINAL STATUS!
	if(m!=1) RT::Asset.Meter = m;
	//Don't override if shape is undefined.
	if(compound!=nullptr)
	{
		//Trying to improve collision detection accuracy.
		CrtPhysics_setMargin(compound);
		current = compound;
	}if(current!=nullptr) 
	{	
		Shape = current;
		
		//Doing this to the compound shape container isn't working.
		//So the shapes are scaled individually in the loop.
		//if(m!=1) Shape->setLocalScaling(btVector3(m,m,m));
	}
}
	
void RT::Physics::Init()
{
	if(Initialized) return; Initialized = true;

	const float world = 1000; assert(RT::Physics::OK);
	#ifndef NO_BULLET	
	///collision configuration contains default setup for memory, collision setup
	bt.collisionConfiguration = bt.New<btDefaultCollisionConfiguration>();
	#ifdef SPU_BULLET //PlayStation 3
	//----------------------------------------------------------
	//SPURS instance
	CellSpursAttribute attributeSpurs;
	int ret = cellSpursAttributeInitialize(&attributeSpurs,NUM_MAX_SPU,SPU_THREAD_GROUP_PRIORITY,SPURS_THREAD_PRIORITY,false);
	if(ret!=0) daeEH::Verbose<<"cellSpursAttributeInitialize failed: "<<ret;
	if(ret!=0) return;
	ret = cellSpursAttributeSetNamePrefix(&attributeSpurs,SPURS_NAME,strlen(SPURS_NAME));
	if(ret!=0) daeEH::Verbose<<"cellSpursAttributeSetNamePrefix failed: "<<ret;
	if(ret!=0) return;
	ret = cellSpursInitializeWithAttribute(&bt.SpursInstance,&attributeSpurs);
	if(ret!=0) daeEH::Verbose<<"cellSpursInitializeWithAttribute failed "<<ret;
	if(ret!=0) return;
	//----------------------------------------------------------
	//SPURS printfserver
	ret = sampleSpursUtilSpuPrintfServiceInitialize(&bt.SpursPrintfService,&bt.SpursInstance,SPURS_THREAD_PRIORITY);
	if(ret!=0) daeEH::Verbose<<"spurs_printf_service_initialize failed: "<<ret;
	if(ret!=0) return;
	int numSpuTasks = NUM_MAX_SPU;
	bt.collisionThreadSupport = bt.New<BulletCollisionSpursSupport>(&bt.SpursInstance,numSpuTasks,numSpuTasks);
	bt.dispatcher = bt.New<SpuGatheringCollisionDispatcher>(bt.collisionThreadSupport,numSpuTasks,bt.collisionConfiguration);
	#else
	bt.dispatcher = bt.New<btCollisionDispatcher>(bt.collisionConfiguration);
	#endif	
	bt.pairCache = bt.New<btAxisSweep3>
	(btVector3(-world,-world,-world),btVector3(world,world,world));
	bt.constraintSolver = bt.New<btSequentialImpulseConstraintSolver>();
	bt.dynamicsWorld = bt.New<btDiscreteDynamicsWorld>
	(bt.dispatcher,bt.pairCache,bt.constraintSolver,bt.collisionConfiguration); 
	//return;
	daeCTC<Physics::OK
	&&sizeof(btScalar)==sizeof(RT::Float)
	&&sizeof(btTransform)==sizeof(RT::Matrix)>();
	#endif //NO_BULLET

	//Not sure what -10 is. Or units.
	SetGravity(0,-9.8f/*-10*/,0); 
}

daeOK RT::Physics::Snapshot(const xs::anyURI &remote_directory)
{	
	if(!Initialized) return DAE_ERR_INVALID_CALL;

	xs::anyURI base = RT::Main.URL;
	base.erase(base.getURI_filenameCP());		
	base.setIsResolved(); remote_directory.resolve();
	if(base==remote_directory)
	{
		//Writing to the same directory is not done.
		assert(0); return false; 
	}

	//This is really just protecting the ref-counts.
	daeArray<xs::any> undo;

	//Ready a snapshot, saving the old "transforms."	
	for(size_t i=0;i<RT::Main.Stack.Data.size();i++)
	{	
		RT::Stack_Data &data = RT::Main.Stack.Data[i];
		btRigidBody *body = data.Physics;
		if(body==nullptr) continue;

		//SCHEDULED FOR REMOVAL
		Collada05_XSD::node *node = 
		(Collada05_XSD::node*)data.Node->Fragment;
		daeMeta &meta = dae(node)->getMeta();
		for(size_t i=0;i<data.Node->Transforms.size();i++)
		{
			//SCHEDULED FOR REMOVAL
			undo.push_back((DAEP::Element*)data.Node->Transforms[i].Fragment);
			meta.removeWRT(dae(node),undo.back());
		}

	#ifdef NDEBUG
	#error //// Do this with ColladaLAB.inl /////////////
	#endif

		RT::Quaternion q;
		RT::Matrix &m = data.Matrix;

		//First, translation.
		q[0] = m[M30]; q[1] = m[M31]; q[2] = m[M32];
		if(data.Parent!=nullptr)
		{
			//Convert to local translation.
			RT::Matrix &mm = data.Parent->Matrix;
			q[0]-=mm[M30]; q[1]-=mm[M31]; q[2]-=mm[M32];
		}
		//if(dot(q)>3*SIMD_EPSILON*SIMD_EPSILON)
		node->translate->value->assign(q,3);

		//Solve for local rotation.
		RT::MatrixToQuat(m,q);
		if(data.Parent!=nullptr)
		{
			RT::Quaternion qq;
			RT::MatrixToQuat(data.Parent->Matrix,qq);
			for(int i=0;i<3;i++) 
			qq[i] = -qq[1]; //invert (unit conjugate)
			#ifdef NDEBUG
			#error A*B or B*A?
			#endif
			QuaternionMult(qq,q); 
		}
		
		//Convert to angle representation.
		q[3] = RT::Float(2*acos(q[3])*SIMD_DEGS_PER_RAD);
		if(q[3]>SIMD_EPSILON)
		{
			RT::Float dot = q[0]*q[0]+q[1]*q[1]+q[2]*q[2];
			if(dot>3*SIMD_EPSILON*SIMD_EPSILON)
			{
				//Convert to axis representation.
				for(int i=0;i<3;i++) 
				q[i]*=1/sqrt(dot);			
				node->rotate->value->assign(q,4);
			}
		}
	}

	//Note, this a debug feature for developers. It
	//isn't this package's place to write documents.
	daeOK OK = DAE_ERROR; xs::anyURI URI;
	daeDOM &DOM = const_cast<daeDOM&>(RT::Main.DOM);
	for(size_t i=0,iN=DOM.getDocs().size();i<iN;i++)	
	{
		const xs::anyURI &rel = DOM.getDoc(i)->getDocURI();
		if(!rel.transitsURI(base)
		 ||!URI.setURI(remote_directory,rel.data()+base.size()))
		continue;
		URI.setIsResolved();

		Collada05::COLLADA COLLADA = DOM.getDoc<Collada05::COLLADA>(i);
		if(COLLADA==nullptr) 
		continue;
		
		Collada05::contributor contributor = ++COLLADA->asset->contributor;

		contributor->author->value = 
		"http://bullet.sf.net Erwin Coumans";
		contributor->authoring_tool->value =
		#ifdef WIN32
		"Bullet ColladaPhysicsViewer-Win32-0.8";
		#elif defined(__APPLE__)
		"Bullet ColladaPhysicsViewer-MacOSX-0.8";
		#else
		"Bullet ColladaPhysicsViewer-UnknownPlatform-0.8";
		#endif
		contributor->comments->value = 
		"Comments to Physics Forum at http://www.continuousphysics.com/Bullet/phpBB2/index.php";

		OK = DOM.getDoc(i)->writeTo(URI); if(!OK) break;
	}

	//Undo the "transform" changes made by the snapshot.
	for(size_t t=0,i=0;i<RT::Main.Stack.Data.size();i++)
	{	
		RT::Stack_Data &data = RT::Main.Stack.Data[i];
		if(data.Physics==nullptr) continue;

		//SCHEDULED FOR REMOVAL
		Collada05_XSD::node *node = 
		(Collada05_XSD::node*)data.Node->Fragment;
		node->translate = ""; 
		node->rotate = "";
		daeMeta &meta = dae(node)->getMeta();
		for(size_t i=0;i<data.Node->Transforms.size();i++)
		meta.placeWRT(dae(node),undo[t++]);
	}	

	return OK;
}

//-------.
	}//<-'
}

/*C1071*/