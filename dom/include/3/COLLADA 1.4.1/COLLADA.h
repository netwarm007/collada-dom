
/*THIS CODE IS PROCEDURALLY-GENERATED. PLEASE EDIT THE GENERATOR; NOT THIS CODE.
 *
 * Copyright 2006 Sony Computer Entertainment Inc.
 *
 * Licensed under the SCEA Shared Source License, Version 1.0 (the "License"); you may not use this
 * file except in compliance with the License. You may obtain a copy of the License at:
 * http://research.scea.com/scea_shared_source_license.html
 *
 * Unless required by applicable law or agreed to in writing, software distributed under the License
 * is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or
 * implied. See the License for the specific language governing permissions and limitations under the
 * License.
 */

#ifndef __COLLADA_h__http_www_collada_org_2005_11_COLLADASchema__ColladaDOM_g1__
#define __COLLADA_h__http_www_collada_org_2005_11_COLLADASchema__ColladaDOM_g1__

#include "http_www_collada_org_2005_11_COLLADASchema.h"
COLLADA_H(__FILE__)
COLLADA_(namespace)
{
    COLLADA_(http_www_collada_org_2005_11_COLLADASchema,namespace)
    {//-.
//<-----'
COLLADA_DOM_NOTE(0,COLLADA::xmlns,enum{ has_default=1 };)
COLLADA_DOM_NOTE(1,COLLADA::version,enum{ has_default=1 };)
COLLADA_DOM_NOTE(2,COLLADA::xml_base)
COLLADA_DOM_NOTE(3,COLLADA::library_animations)
COLLADA_DOM_NOTE(4,COLLADA::library_animation_clips)
COLLADA_DOM_NOTE(5,COLLADA::library_cameras)
COLLADA_DOM_NOTE(6,COLLADA::library_controllers)
COLLADA_DOM_NOTE(7,COLLADA::library_geometries)
COLLADA_DOM_NOTE(8,COLLADA::library_effects)
COLLADA_DOM_NOTE(9,COLLADA::library_force_fields)
COLLADA_DOM_NOTE(10,COLLADA::library_images)
COLLADA_DOM_NOTE(11,COLLADA::library_lights)
COLLADA_DOM_NOTE(12,COLLADA::library_materials)
COLLADA_DOM_NOTE(13,COLLADA::library_nodes)
COLLADA_DOM_NOTE(14,COLLADA::library_physics_materials)
COLLADA_DOM_NOTE(15,COLLADA::library_physics_models)
COLLADA_DOM_NOTE(16,COLLADA::library_physics_scenes)
COLLADA_DOM_NOTE(17,COLLADA::library_visual_scenes)
COLLADA_DOM_NOTE(18,COLLADA::extra)
COLLADA_DOM_NOTE(22,COLLADA::asset)
COLLADA_DOM_NOTE(23,COLLADA::scene)
/**
 * The COLLADA element declares the root of the document that comprises some
 * of the content in the COLLADA schema.
 */
class COLLADA
: 
public DAEP::Elemental<COLLADA>, public DAEP::Schema<0x0001000300000002ULL>
{
public: //NESTED ELEMENTS
	
	typedef class __COLLADA__scene__
	/**
	 * The scene embodies the entire set of information that can be visualized
	 * from the contents of a COLLADA resource. The scene element declares the
	 * base of the scene hierarchy or scene graph. The scene contains elements
	 * that comprise much of the visual and transformational information content
	 * as created by the authoring tools.
	 * @see XSD @c COLLADA::local__scene
	 */
	local__scene; 

public: //Parameters

	typedef struct:Elemental,Schema
	{   DAEP::Value<0,xs::anyURI>
	_0; DAEP::Value<1,VersionType>
	_1; DAEP::Value<2,xml::base>
	_2; COLLADA_WORD_ALIGN
		DAEP::Child<18,library_animations>
	_3; DAEP::Child<17,library_animation_clips>
	_4; DAEP::Child<16,library_cameras>
	_5; DAEP::Child<15,library_controllers>
	_6; DAEP::Child<14,library_geometries>
	_7; DAEP::Child<13,library_effects>
	_8; DAEP::Child<12,library_force_fields>
	_9; DAEP::Child<11,library_images>
	_10; DAEP::Child<10,library_lights>
	_11; DAEP::Child<9,library_materials>
	_12; DAEP::Child<8,library_nodes>
	_13; DAEP::Child<7,library_physics_materials>
	_14; DAEP::Child<6,library_physics_models>
	_15; DAEP::Child<5,library_physics_scenes>
	_16; DAEP::Child<4,library_visual_scenes>
	_17; DAEP::Child<3,extra>
	_18; COLLADA_DOM_Z(1,1)
	DAEP::Value<20,dae_Array<>> _Z; enum{ _No=20 };
	DAEP::Value<24,daeContents> content; typedef __NS__<0> notestart;
	}_;

public: //Attributes
	/**
	 * This element may specify its own xmlns.
	 * @see XSD @c xs::anyURI
	 */
	DAEP::Value<0,xs::anyURI,_,(_::_)&_::_0> xmlns;
	/**
	 * The version attribute is the COLLADA schema revision with which the instance
	 * document conforms. Required Attribute.
	 * @see XSD @c VersionType
	 */
	DAEP::Value<1,VersionType,_,(_::_)&_::_1> version;
	/**
	 * The xml:base attribute allows you to define the base URI for this COLLADA
	 * document. See http://www.w3.org/TR/xmlbase/ for more information.
	 * @see XSD @c xml::base
	 */
	DAEP::Value<2,xml::base,_,(_::_)&_::_2> xml_base;

public: //Elements
		
	COLLADA_WORD_ALIGN
	/**
	 * The COLLADA element may contain any number of library_animations elements.
	 * @see XSD @c library_animations
	 */
	DAEP::Child<18,library_animations,_,(_::_)&_::_3> library_animations;
	/**
	 * The COLLADA element may contain any number of library_animation_clips elements.
	 * @see XSD @c library_animation_clips
	 */
	DAEP::Child<17,library_animation_clips,_,(_::_)&_::_4> library_animation_clips;
	/**
	 * The COLLADA element may contain any number of library_cameras elements.
	 * @see XSD @c library_cameras
	 */
	DAEP::Child<16,library_cameras,_,(_::_)&_::_5> library_cameras;
	/**
	 * The COLLADA element may contain any number of library_controllerss elements.
	 * @see XSD @c library_controllers
	 */
	DAEP::Child<15,library_controllers,_,(_::_)&_::_6> library_controllers;
	/**
	 * The COLLADA element may contain any number of library_geometriess elements.
	 * @see XSD @c library_geometries
	 */
	DAEP::Child<14,library_geometries,_,(_::_)&_::_7> library_geometries;
	/**
	 * The COLLADA element may contain any number of library_effects elements.
	 * @see XSD @c library_effects
	 */
	DAEP::Child<13,library_effects,_,(_::_)&_::_8> library_effects;
	/**
	 * The COLLADA element may contain any number of library_force_fields elements.
	 * @see XSD @c library_force_fields
	 */
	DAEP::Child<12,library_force_fields,_,(_::_)&_::_9> library_force_fields;
	/**
	 * The COLLADA element may contain any number of library_images elements.
	 * @see XSD @c library_images
	 */
	DAEP::Child<11,library_images,_,(_::_)&_::_10> library_images;
	/**
	 * The COLLADA element may contain any number of library_lights elements.
	 * @see XSD @c library_lights
	 */
	DAEP::Child<10,library_lights,_,(_::_)&_::_11> library_lights;
	/**
	 * The COLLADA element may contain any number of library_materials elements.
	 * @see XSD @c library_materials
	 */
	DAEP::Child<9,library_materials,_,(_::_)&_::_12> library_materials;
	/**
	 * The COLLADA element may contain any number of library_nodes elements.
	 * @see XSD @c library_nodes
	 */
	DAEP::Child<8,library_nodes,_,(_::_)&_::_13> library_nodes;
	/**
	 * The COLLADA element may contain any number of library_materials elements.
	 * @see XSD @c library_physics_materials
	 */
	DAEP::Child<7,library_physics_materials,_,(_::_)&_::_14> library_physics_materials;
	/**
	 * The COLLADA element may contain any number of library_physics_models elements.
	 * @see XSD @c library_physics_models
	 */
	DAEP::Child<6,library_physics_models,_,(_::_)&_::_15> library_physics_models;
	/**
	 * The COLLADA element may contain any number of library_physics_scenes elements.
	 * @see XSD @c library_physics_scenes
	 */
	DAEP::Child<5,library_physics_scenes,_,(_::_)&_::_16> library_physics_scenes;
	/**
	 * The COLLADA element may contain any number of library_visual_scenes elements.
	 * @see XSD @c library_visual_scenes
	 */
	DAEP::Child<4,library_visual_scenes,_,(_::_)&_::_17> library_visual_scenes;
	/**
	 * The extra element may appear any number of times.
	 * @see XSD @c extra
	 */
	DAEP::Child<3,extra,_,(_::_)&_::_18> extra;

	COLLADA_DOM_Z(1,1) union //NO-NAMES & ONLY-CHILDS
	{
	/**NO-NAMES
	 * These elements are invalid according to the schema. They may be user-defined 
	 * additions and substitutes.
	 */
	DAEP::Child<1,xs::any,_,(_::_)&_::_Z> extra_schema__unnamed;
	/**
	 * The COLLADA element must contain an asset element.
	 * @see XSD @c asset
	 */
	DAEP::Child<-1,asset,_,(_::_)&_::_Z> asset;
	/**
	 * The scene embodies the entire set of information that can be visualized
	 * from the contents of a COLLADA resource. The scene element declares the
	 * base of the scene hierarchy or scene graph. The scene contains elements
	 * that comprise much of the visual and transformational information content
	 * as created by the authoring tools.
	 * @see XSD @c local__scene
	 */
	DAEP::Child<-2,local__scene,_,(_::_)&_::_Z> scene;
	};

public: //Content
	/**
	 * Children, mixed-text, comments & processing-instructions.
	 */
	DAEP::Value<24,daeContents,_,(_::_)&_::content> content;
};
COLLADA_DOM_NOTE(25,scene::instance_physics_scene)
COLLADA_DOM_NOTE(26,scene::extra)
COLLADA_DOM_NOTE(30,scene::instance_visual_scene)
/**
 * The scene embodies the entire set of information that can be visualized
 * from the contents of a COLLADA resource. The scene element declares the
 * base of the scene hierarchy or scene graph. The scene contains elements
 * that comprise much of the visual and transformational information content
 * as created by the authoring tools.
 * @see XSD @c COLLADA::local__scene
 */
class __COLLADA__scene__
: 
public DAEP::Elemental<__COLLADA__scene__>, public DAEP::Schema<0x0002000000000006ULL>
{
public: //Parameters

	typedef struct:Elemental,Schema
	{   COLLADA_WORD_ALIGN
		DAEP::Child<4,InstanceWithExtra>
	_0; DAEP::Child<3,extra>
	_1; COLLADA_DOM_Z(1,1)
	DAEP::Value<3,dae_Array<>> _Z; enum{ _No=3 };
	DAEP::Value<6,daeContents> content; typedef __NS__<25> notestart;
	}_;

public: //Elements
		
	COLLADA_WORD_ALIGN
	/**
	 * The instance_physics_scene element declares the instantiation of a COLLADA
	 * physics_scene resource. The instance_physics_scene element may appear any
	 * number of times.
	 * @see XSD @c InstanceWithExtra
	 */
	DAEP::Child<4,InstanceWithExtra,_,(_::_)&_::_0> instance_physics_scene;
	/**
	 * The extra element may appear any number of times.
	 * @see XSD @c extra
	 */
	DAEP::Child<3,extra,_,(_::_)&_::_1> extra;

	COLLADA_DOM_Z(1,1) union //NO-NAMES & ONLY-CHILDS
	{
	/**NO-NAMES
	 * These elements are invalid according to the schema. They may be user-defined 
	 * additions and substitutes.
	 */
	DAEP::Child<1,xs::any,_,(_::_)&_::_Z> extra_schema__unnamed;
	/**
	 * The instance_visual_scene element declares the instantiation of a COLLADA
	 * visual_scene resource. The instance_visual_scene element may only appear
	 * once.
	 * @see XSD @c InstanceWithExtra
	 */
	DAEP::Child<-1,InstanceWithExtra,_,(_::_)&_::_Z> instance_visual_scene;
	};

public: //Content
	/**
	 * Children, mixed-text, comments & processing-instructions.
	 */
	DAEP::Value<6,daeContents,_,(_::_)&_::content> content;
};
//-------.
    }//<-'
}

#include "asset.h"
#include "library_animations.h"
#include "library_animation_clips.h"
#include "library_cameras.h"
#include "library_controllers.h"
#include "library_geometries.h"
#include "library_effects.h"
#include "library_force_fields.h"
#include "library_images.h"
#include "library_lights.h"
#include "library_materials.h"
#include "library_nodes.h"
#include "library_physics_materials.h"
#include "library_physics_models.h"
#include "library_physics_scenes.h"
#include "library_visual_scenes.h"
#include "InstanceWithExtra.h"
#include "extra.h"
#define COLLADA_target_namespace \
COLLADA::http_www_collada_org_2005_11_COLLADASchema
COLLADA_(namespace)
{
	namespace DAEP //GCC
	{//-.
//<-----'
#ifndef COLLADA_DOM_LITE
template<>
COLLADA_(inline) DAEP::Model& DAEP::Elemental
<::COLLADA_target_namespace:: COLLADA
>::__DAEP__Object__v1__model()const
{
	static DAEP::Model *om = nullptr; if(om!=nullptr) return *om;

	Elemental::TOC toc; daeMetaElement &el = 
	::COLLADA_target_namespace::__XS__().addElement(toc,"COLLADA");

	XS::Attribute *a;
	a = el.addAttribute(toc->xmlns,"xs:anyURI","xmlns");
	a->setDefaultString("http://www.collada.org/2005/11/COLLADASchema");
	a->setIsRequired();
	a = el.addAttribute(toc->version,"VersionType","version");
	a->setDefaultString("1.4.1");
	a->setIsRequired();
	a = el.addAttribute(toc->xml_base,"xml:base","xml:base");
		
	om = &el.getModel();
	daeCM *cm = nullptr;
	el.addCM<XS::Sequence>(cm,0,1,1);
	el.addCM<XS::Element>(cm,0,1,1).setChild(toc->asset,"asset");
	el.addCM<XS::Choice>(cm,1,0,-1);
		el.addCM<XS::Element>(cm,0,1,1).setChild(toc->library_animations,"library_animations");
		el.addCM<XS::Element>(cm,1,1,1).setChild(toc->library_animation_clips,"library_animation_clips");
		el.addCM<XS::Element>(cm,2,1,1).setChild(toc->library_cameras,"library_cameras");
		el.addCM<XS::Element>(cm,3,1,1).setChild(toc->library_controllers,"library_controllers");
		el.addCM<XS::Element>(cm,4,1,1).setChild(toc->library_geometries,"library_geometries");
		el.addCM<XS::Element>(cm,5,1,1).setChild(toc->library_effects,"library_effects");
		el.addCM<XS::Element>(cm,6,1,1).setChild(toc->library_force_fields,"library_force_fields");
		el.addCM<XS::Element>(cm,7,1,1).setChild(toc->library_images,"library_images");
		el.addCM<XS::Element>(cm,8,1,1).setChild(toc->library_lights,"library_lights");
		el.addCM<XS::Element>(cm,9,1,1).setChild(toc->library_materials,"library_materials");
		el.addCM<XS::Element>(cm,10,1,1).setChild(toc->library_nodes,"library_nodes");
		el.addCM<XS::Element>(cm,11,1,1).setChild(toc->library_physics_materials,"library_physics_materials");
		el.addCM<XS::Element>(cm,12,1,1).setChild(toc->library_physics_models,"library_physics_models");
		el.addCM<XS::Element>(cm,13,1,1).setChild(toc->library_physics_scenes,"library_physics_scenes");
		el.addCM<XS::Element>(cm,14,1,1).setChild(toc->library_visual_scenes,"library_visual_scenes");
		el.popCM<15,3000>(cm);
	el.addCM<XS::Element>(cm,45001,0,1).setChild(toc->scene,"scene");
	el.addCM<XS::Element>(cm,45002,0,-1).setChild(toc->extra,"extra");
	el.addContentModel<45003,1>(cm,toc); return *om;
}
template<>
COLLADA_(inline) DAEP::Model& DAEP::Elemental
<::COLLADA_target_namespace:: COLLADA::local__scene
>::__DAEP__Object__v1__model()const
{
	static DAEP::Model *om = nullptr; if(om!=nullptr) return *om;

	Elemental::TOC toc; daeMetaElement &el = 
	::COLLADA_target_namespace::__XS__().addElement(toc,"scene");
		
	om = &el.getModel();
	daeCM *cm = nullptr;
	el.addCM<XS::Sequence>(cm,0,1,1);
	el.addCM<XS::Element>(cm,0,0,-1).setChild(toc->instance_physics_scene,"instance_physics_scene");
	el.addCM<XS::Element>(cm,1,0,1).setChild(toc->instance_visual_scene,"instance_visual_scene");
	el.addCM<XS::Element>(cm,2,0,-1).setChild(toc->extra,"extra");
	el.addContentModel<3,1>(cm,toc); return *om;
}
#endif //!COLLADA_DOM_LITE
//----------.
		//<-'			
        COLLADA_(http_www_collada_org_2005_11_COLLADASchema,namespace)
        {//-.
//<---------'
/**WYSIWYG
 * The COLLADA element declares the root of the document that comprises some
 * of the content in the COLLADA schema.
 * @see XSD @c COLLADA
 */
struct COLLADA
:
daeSmartRef<::COLLADA_target_namespace::COLLADA>
{
	COLLADA_DOM_3(COLLADA,struct,daeSmartRef)
	/**
	 * This element may specify its own xmlns.
	 * @see XSD @c xs::anyURI
	 */
	typedef xs::anyURI xmlns_anyURI;
	/**
	 * The version attribute is the COLLADA schema revision with which the instance
	 * document conforms. Required Attribute.
	 * @see XSD @c VersionType
	 */
	typedef ::COLLADA_target_namespace::VersionType version_VersionType;
	/**
	 * The xml:base attribute allows you to define the base URI for this COLLADA
	 * document. See http://www.w3.org/TR/xmlbase/ for more information.
	 * @see XSD @c xml::base
	 */
	typedef xml::base xml_base;
	/**
	 * The COLLADA element must contain an asset element.
	 * @see XSD @c asset
	 */
	typedef struct asset asset;
	/**
	 * The COLLADA element may contain any number of library_animations elements.
	 * @see XSD @c library_animations
	 */
	typedef struct library_animations library_animations;
	/**
	 * The COLLADA element may contain any number of library_animation_clips elements.
	 * @see XSD @c library_animation_clips
	 */
	typedef struct library_animation_clips library_animation_clips;
	/**
	 * The COLLADA element may contain any number of library_cameras elements.
	 * @see XSD @c library_cameras
	 */
	typedef struct library_cameras library_cameras;
	/**
	 * The COLLADA element may contain any number of library_controllerss elements.
	 * @see XSD @c library_controllers
	 */
	typedef struct library_controllers library_controllers;
	/**
	 * The COLLADA element may contain any number of library_geometriess elements.
	 * @see XSD @c library_geometries
	 */
	typedef struct library_geometries library_geometries;
	/**
	 * The COLLADA element may contain any number of library_effects elements.
	 * @see XSD @c library_effects
	 */
	typedef struct library_effects library_effects;
	/**
	 * The COLLADA element may contain any number of library_force_fields elements.
	 * @see XSD @c library_force_fields
	 */
	typedef struct library_force_fields library_force_fields;
	/**
	 * The COLLADA element may contain any number of library_images elements.
	 * @see XSD @c library_images
	 */
	typedef struct library_images library_images;
	/**
	 * The COLLADA element may contain any number of library_lights elements.
	 * @see XSD @c library_lights
	 */
	typedef struct library_lights library_lights;
	/**
	 * The COLLADA element may contain any number of library_materials elements.
	 * @see XSD @c library_materials
	 */
	typedef struct library_materials library_materials;
	/**
	 * The COLLADA element may contain any number of library_nodes elements.
	 * @see XSD @c library_nodes
	 */
	typedef struct library_nodes library_nodes;
	/**
	 * The COLLADA element may contain any number of library_materials elements.
	 * @see XSD @c library_physics_materials
	 */
	typedef struct library_physics_materials library_physics_materials;
	/**
	 * The COLLADA element may contain any number of library_physics_models elements.
	 * @see XSD @c library_physics_models
	 */
	typedef struct library_physics_models library_physics_models;
	/**
	 * The COLLADA element may contain any number of library_physics_scenes elements.
	 * @see XSD @c library_physics_scenes
	 */
	typedef struct library_physics_scenes library_physics_scenes;
	/**
	 * The COLLADA element may contain any number of library_visual_scenes elements.
	 * @see XSD @c library_visual_scenes
	 */
	typedef struct library_visual_scenes library_visual_scenes;
	/**
	 * The scene embodies the entire set of information that can be visualized
	 * from the contents of a COLLADA resource. The scene element declares the
	 * base of the scene hierarchy or scene graph. The scene contains elements
	 * that comprise much of the visual and transformational information content
	 * as created by the authoring tools.
	 * @see XSD @c local__scene
	 */
	typedef struct scene scene;
	/**
	 * The extra element may appear any number of times.
	 * @see XSD @c extra
	 */
	typedef struct extra extra;
};
/**WYSIWYG, CONST-FORM
 * The COLLADA element declares the root of the document that comprises some
 * of the content in the COLLADA schema.
 * @see XSD @c COLLADA
 */
struct const_COLLADA
:
daeSmartRef<const ::COLLADA_target_namespace::COLLADA>
{
	COLLADA_DOM_3(const_COLLADA,struct,daeSmartRef)
	/**CONST-FORM
	 * This element may specify its own xmlns.
	 * @see XSD @c xs::anyURI
	 */
	typedef const xs::anyURI xmlns_anyURI;
	/**CONST-FORM
	 * The version attribute is the COLLADA schema revision with which the instance
	 * document conforms. Required Attribute.
	 * @see XSD @c VersionType
	 */
	typedef const ::COLLADA_target_namespace::VersionType version_VersionType;
	/**CONST-FORM
	 * The xml:base attribute allows you to define the base URI for this COLLADA
	 * document. See http://www.w3.org/TR/xmlbase/ for more information.
	 * @see XSD @c xml::base
	 */
	typedef const xml::base xml_base;
	/**CONST-FORM
	 * The COLLADA element must contain an asset element.
	 * @see XSD @c asset
	 */
	typedef struct const_asset asset;
	/**CONST-FORM
	 * The COLLADA element may contain any number of library_animations elements.
	 * @see XSD @c library_animations
	 */
	typedef struct const_library_animations library_animations;
	/**CONST-FORM
	 * The COLLADA element may contain any number of library_animation_clips elements.
	 * @see XSD @c library_animation_clips
	 */
	typedef struct const_library_animation_clips library_animation_clips;
	/**CONST-FORM
	 * The COLLADA element may contain any number of library_cameras elements.
	 * @see XSD @c library_cameras
	 */
	typedef struct const_library_cameras library_cameras;
	/**CONST-FORM
	 * The COLLADA element may contain any number of library_controllerss elements.
	 * @see XSD @c library_controllers
	 */
	typedef struct const_library_controllers library_controllers;
	/**CONST-FORM
	 * The COLLADA element may contain any number of library_geometriess elements.
	 * @see XSD @c library_geometries
	 */
	typedef struct const_library_geometries library_geometries;
	/**CONST-FORM
	 * The COLLADA element may contain any number of library_effects elements.
	 * @see XSD @c library_effects
	 */
	typedef struct const_library_effects library_effects;
	/**CONST-FORM
	 * The COLLADA element may contain any number of library_force_fields elements.
	 * @see XSD @c library_force_fields
	 */
	typedef struct const_library_force_fields library_force_fields;
	/**CONST-FORM
	 * The COLLADA element may contain any number of library_images elements.
	 * @see XSD @c library_images
	 */
	typedef struct const_library_images library_images;
	/**CONST-FORM
	 * The COLLADA element may contain any number of library_lights elements.
	 * @see XSD @c library_lights
	 */
	typedef struct const_library_lights library_lights;
	/**CONST-FORM
	 * The COLLADA element may contain any number of library_materials elements.
	 * @see XSD @c library_materials
	 */
	typedef struct const_library_materials library_materials;
	/**CONST-FORM
	 * The COLLADA element may contain any number of library_nodes elements.
	 * @see XSD @c library_nodes
	 */
	typedef struct const_library_nodes library_nodes;
	/**CONST-FORM
	 * The COLLADA element may contain any number of library_materials elements.
	 * @see XSD @c library_physics_materials
	 */
	typedef struct const_library_physics_materials library_physics_materials;
	/**CONST-FORM
	 * The COLLADA element may contain any number of library_physics_models elements.
	 * @see XSD @c library_physics_models
	 */
	typedef struct const_library_physics_models library_physics_models;
	/**CONST-FORM
	 * The COLLADA element may contain any number of library_physics_scenes elements.
	 * @see XSD @c library_physics_scenes
	 */
	typedef struct const_library_physics_scenes library_physics_scenes;
	/**CONST-FORM
	 * The COLLADA element may contain any number of library_visual_scenes elements.
	 * @see XSD @c library_visual_scenes
	 */
	typedef struct const_library_visual_scenes library_visual_scenes;
	/**CONST-FORM
	 * The scene embodies the entire set of information that can be visualized
	 * from the contents of a COLLADA resource. The scene element declares the
	 * base of the scene hierarchy or scene graph. The scene contains elements
	 * that comprise much of the visual and transformational information content
	 * as created by the authoring tools.
	 * @see XSD @c local__scene
	 */
	typedef struct const_scene scene;
	/**CONST-FORM
	 * The extra element may appear any number of times.
	 * @see XSD @c extra
	 */
	typedef struct const_extra extra;
};
/**WYSIWYG
 * The scene embodies the entire set of information that can be visualized
 * from the contents of a COLLADA resource. The scene element declares the
 * base of the scene hierarchy or scene graph. The scene contains elements
 * that comprise much of the visual and transformational information content
 * as created by the authoring tools.
 * @see XSD @c COLLADA::local__scene
 */
struct scene
:
daeSmartRef<::COLLADA_target_namespace::COLLADA::local__scene>
{
	COLLADA_DOM_3(scene,struct,daeSmartRef)
	/**
	 * The instance_physics_scene element declares the instantiation of a COLLADA
	 * physics_scene resource. The instance_physics_scene element may appear any
	 * number of times.
	 * @see XSD @c InstanceWithExtra
	 */
	typedef struct __XSD__InstanceWithExtra__ instance_physics_scene;
	/**
	 * The instance_visual_scene element declares the instantiation of a COLLADA
	 * visual_scene resource. The instance_visual_scene element may only appear
	 * once.
	 * @see XSD @c InstanceWithExtra
	 */
	typedef struct __XSD__InstanceWithExtra__ instance_visual_scene;
	/**
	 * The extra element may appear any number of times.
	 * @see XSD @c extra
	 */
	typedef struct extra extra;
};
/**WYSIWYG, CONST-FORM
 * The scene embodies the entire set of information that can be visualized
 * from the contents of a COLLADA resource. The scene element declares the
 * base of the scene hierarchy or scene graph. The scene contains elements
 * that comprise much of the visual and transformational information content
 * as created by the authoring tools.
 * @see XSD @c COLLADA::local__scene
 */
struct const_scene
:
daeSmartRef<const ::COLLADA_target_namespace::COLLADA::local__scene>
{
	COLLADA_DOM_3(const_scene,struct,daeSmartRef)
	/**CONST-FORM
	 * The instance_physics_scene element declares the instantiation of a COLLADA
	 * physics_scene resource. The instance_physics_scene element may appear any
	 * number of times.
	 * @see XSD @c InstanceWithExtra
	 */
	typedef struct __XSD__const_InstanceWithExtra__ instance_physics_scene;
	/**CONST-FORM
	 * The instance_visual_scene element declares the instantiation of a COLLADA
	 * visual_scene resource. The instance_visual_scene element may only appear
	 * once.
	 * @see XSD @c InstanceWithExtra
	 */
	typedef struct __XSD__const_InstanceWithExtra__ instance_visual_scene;
	/**CONST-FORM
	 * The extra element may appear any number of times.
	 * @see XSD @c extra
	 */
	typedef struct const_extra extra;
};
//-----------.
        }//<-'
    }
}
#undef COLLADA_target_namespace
#endif //__COLLADA_h__http_www_collada_org_2005_11_COLLADASchema__ColladaDOM_g1__
/*C1071*/