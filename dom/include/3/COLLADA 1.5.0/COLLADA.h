
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

#ifndef __COLLADA_h__http_www_collada_org_2008_03_COLLADASchema__ColladaDOM_g1__
#define __COLLADA_h__http_www_collada_org_2008_03_COLLADASchema__ColladaDOM_g1__

#include "http_www_collada_org_2008_03_COLLADASchema.h"
COLLADA_H(__FILE__)
COLLADA_(namespace)
{
    COLLADA_(http_www_collada_org_2008_03_COLLADASchema,namespace)
    {//-.
//<-----'
COLLADA_DOM_NOTE(5752,COLLADA::xmlns,enum{ has_default=1 };)
COLLADA_DOM_NOTE(5753,COLLADA::version,enum{ has_default=1 };)
COLLADA_DOM_NOTE(5754,COLLADA::xml_base)
COLLADA_DOM_NOTE(5755,COLLADA::library_animations)
COLLADA_DOM_NOTE(5756,COLLADA::library_animation_clips)
COLLADA_DOM_NOTE(5757,COLLADA::library_cameras)
COLLADA_DOM_NOTE(5758,COLLADA::library_controllers)
COLLADA_DOM_NOTE(5759,COLLADA::library_geometries)
COLLADA_DOM_NOTE(5760,COLLADA::library_effects)
COLLADA_DOM_NOTE(5761,COLLADA::library_force_fields)
COLLADA_DOM_NOTE(5762,COLLADA::library_images)
COLLADA_DOM_NOTE(5763,COLLADA::library_lights)
COLLADA_DOM_NOTE(5764,COLLADA::library_materials)
COLLADA_DOM_NOTE(5765,COLLADA::library_nodes)
COLLADA_DOM_NOTE(5766,COLLADA::library_physics_materials)
COLLADA_DOM_NOTE(5767,COLLADA::library_physics_models)
COLLADA_DOM_NOTE(5768,COLLADA::library_physics_scenes)
COLLADA_DOM_NOTE(5769,COLLADA::library_visual_scenes)
COLLADA_DOM_NOTE(5770,COLLADA::library_joints)
COLLADA_DOM_NOTE(5771,COLLADA::library_kinematics_models)
COLLADA_DOM_NOTE(5772,COLLADA::library_articulated_systems)
COLLADA_DOM_NOTE(5773,COLLADA::library_kinematics_scenes)
COLLADA_DOM_NOTE(5774,COLLADA::library_formulas)
COLLADA_DOM_NOTE(5775,COLLADA::extra)
COLLADA_DOM_NOTE(5780,COLLADA::asset)
COLLADA_DOM_NOTE(5781,COLLADA::scene)
/**
 * The COLLADA element declares the root of the document that comprises some
 * of the content in the COLLADA schema.
 */
class COLLADA
: 
public DAEP::Elemental<COLLADA>, public DAEP::Schema<0x030c000300000002ULL>
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
	_0; DAEP::Value<1,version_enum>
	_1; DAEP::Value<2,xml::base>
	_2; COLLADA_WORD_ALIGN
		DAEP::Child<24,library_animations_type>
	_3; DAEP::Child<23,library_animation_clips_type>
	_4; DAEP::Child<22,library_cameras_type>
	_5; DAEP::Child<21,library_controllers_type>
	_6; DAEP::Child<20,library_geometries_type>
	_7; DAEP::Child<19,library_effects_type>
	_8; DAEP::Child<18,library_force_fields_type>
	_9; DAEP::Child<17,library_images_type>
	_10; DAEP::Child<16,library_lights_type>
	_11; DAEP::Child<15,library_materials_type>
	_12; DAEP::Child<14,library_nodes_type>
	_13; DAEP::Child<13,library_physics_materials_type>
	_14; DAEP::Child<12,library_physics_models_type>
	_15; DAEP::Child<11,library_physics_scenes_type>
	_16; DAEP::Child<10,library_visual_scenes_type>
	_17; DAEP::Child<9,library_joints_type>
	_18; DAEP::Child<8,library_kinematics_models_type>
	_19; DAEP::Child<7,library_articulated_systems_type>
	_20; DAEP::Child<6,library_kinematics_scenes_type>
	_21; DAEP::Child<5,library_formulas_type>
	_22; DAEP::Child<4,extra_type>
	_23; COLLADA_DOM_N(1,2)
	DAEP::Value<26,dae_Array<>> _N; enum{ _No=26 };
	DAEP::Value<30,daeContents> content; typedef __NS__<5752> notestart;
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
	 * @see XSD @c version_enum
	 */
	DAEP::Value<1,version_enum,_,(_::_)&_::_1> version;
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
	 * @see XSD @c library_animations_type
	 */
	DAEP::Child<24,library_animations_type,_,(_::_)&_::_3> library_animations;
	/**
	 * The COLLADA element may contain any number of library_animation_clips elements.
	 * @see XSD @c library_animation_clips_type
	 */
	DAEP::Child<23,library_animation_clips_type,_,(_::_)&_::_4> library_animation_clips;
	/**
	 * The COLLADA element may contain any number of library_cameras elements.
	 * @see XSD @c library_cameras_type
	 */
	DAEP::Child<22,library_cameras_type,_,(_::_)&_::_5> library_cameras;
	/**
	 * The COLLADA element may contain any number of library_controllerss elements.
	 * @see XSD @c library_controllers_type
	 */
	DAEP::Child<21,library_controllers_type,_,(_::_)&_::_6> library_controllers;
	/**
	 * The COLLADA element may contain any number of library_geometriess elements.
	 * @see XSD @c library_geometries_type
	 */
	DAEP::Child<20,library_geometries_type,_,(_::_)&_::_7> library_geometries;
	/**
	 * The COLLADA element may contain any number of library_effects elements.
	 * @see XSD @c library_effects_type
	 */
	DAEP::Child<19,library_effects_type,_,(_::_)&_::_8> library_effects;
	/**
	 * The COLLADA element may contain any number of library_force_fields elements.
	 * @see XSD @c library_force_fields_type
	 */
	DAEP::Child<18,library_force_fields_type,_,(_::_)&_::_9> library_force_fields;
	/**
	 * The COLLADA element may contain any number of library_images elements.
	 * @see XSD @c library_images_type
	 */
	DAEP::Child<17,library_images_type,_,(_::_)&_::_10> library_images;
	/**
	 * The COLLADA element may contain any number of library_lights elements.
	 * @see XSD @c library_lights_type
	 */
	DAEP::Child<16,library_lights_type,_,(_::_)&_::_11> library_lights;
	/**
	 * The COLLADA element may contain any number of library_materials elements.
	 * @see XSD @c library_materials_type
	 */
	DAEP::Child<15,library_materials_type,_,(_::_)&_::_12> library_materials;
	/**
	 * The COLLADA element may contain any number of library_nodes elements.
	 * @see XSD @c library_nodes_type
	 */
	DAEP::Child<14,library_nodes_type,_,(_::_)&_::_13> library_nodes;
	/**
	 * The COLLADA element may contain any number of library_materials elements.
	 * @see XSD @c library_physics_materials_type
	 */
	DAEP::Child<13,library_physics_materials_type,_,(_::_)&_::_14> library_physics_materials;
	/**
	 * The COLLADA element may contain any number of library_physics_models elements.
	 * @see XSD @c library_physics_models_type
	 */
	DAEP::Child<12,library_physics_models_type,_,(_::_)&_::_15> library_physics_models;
	/**
	 * The COLLADA element may contain any number of library_physics_scenes elements.
	 * @see XSD @c library_physics_scenes_type
	 */
	DAEP::Child<11,library_physics_scenes_type,_,(_::_)&_::_16> library_physics_scenes;
	/**
	 * The COLLADA element may contain any number of library_visual_scenes elements.
	 * @see XSD @c library_visual_scenes_type
	 */
	DAEP::Child<10,library_visual_scenes_type,_,(_::_)&_::_17> library_visual_scenes;
	/**UNDOCUMENTED
	 * The XSD schema does not provide documentation in this case.
	 * @see XSD @c library_joints_type
	 */
	DAEP::Child<9,library_joints_type,_,(_::_)&_::_18> library_joints;
	/**UNDOCUMENTED
	 * The XSD schema does not provide documentation in this case.
	 * @see XSD @c library_kinematics_models_type
	 */
	DAEP::Child<8,library_kinematics_models_type,_,(_::_)&_::_19> library_kinematics_models;
	/**UNDOCUMENTED
	 * The XSD schema does not provide documentation in this case.
	 * @see XSD @c library_articulated_systems_type
	 */
	DAEP::Child<7,library_articulated_systems_type,_,(_::_)&_::_20> library_articulated_systems;
	/**UNDOCUMENTED
	 * The XSD schema does not provide documentation in this case.
	 * @see XSD @c library_kinematics_scenes_type
	 */
	DAEP::Child<6,library_kinematics_scenes_type,_,(_::_)&_::_21> library_kinematics_scenes;
	/**UNDOCUMENTED
	 * The XSD schema does not provide documentation in this case.
	 * @see XSD @c library_formulas_type
	 */
	DAEP::Child<5,library_formulas_type,_,(_::_)&_::_22> library_formulas;
	/**
	 * The extra element may appear any number of times.
	 * @see XSD @c extra_type
	 */
	DAEP::Child<4,extra_type,_,(_::_)&_::_23> extra;

	COLLADA_DOM_N(1,2) union //NO-NAMES & ONLY-CHILDS
	{
	/**NO-NAMES
	 * These elements are invalid according to the schema. They may be user-defined 
	 * additions and substitutes.
	 */
	DAEP::Child<1,xs::any,_,(_::_)&_::_N> extra_schema__unnamed;
	/**
	 * The COLLADA element must contain an asset element.
	 * @see XSD @c asset_type
	 */
	DAEP::Child<-1,asset_type,_,(_::_)&_::_N> asset;
	/**
	 * The scene embodies the entire set of information that can be visualized
	 * from the contents of a COLLADA resource. The scene element declares the
	 * base of the scene hierarchy or scene graph. The scene contains elements
	 * that comprise much of the visual and transformational information content
	 * as created by the authoring tools.
	 * @see XSD @c local__scene
	 */
	DAEP::Child<-2,local__scene,_,(_::_)&_::_N> scene;
	};

public: //Content
	/**
	 * Children, mixed-text, comments & processing-instructions.
	 */
	DAEP::Value<30,daeContents,_,(_::_)&_::content> content;
};
COLLADA_DOM_NOTE(5783,scene::instance_physics_scene)
COLLADA_DOM_NOTE(5784,scene::instance_kinematics_scene)
COLLADA_DOM_NOTE(5785,scene::extra)
COLLADA_DOM_NOTE(5790,scene::instance_visual_scene)
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
public DAEP::Elemental<__COLLADA__scene__>, public DAEP::Schema<0x030d000000000006ULL>
{
public: //Parameters

	typedef struct:Elemental,Schema
	{   COLLADA_WORD_ALIGN
		DAEP::Child<6,instance_with_extra_type>
	_0; DAEP::Child<5,instance_kinematics_scene_type>
	_1; DAEP::Child<4,extra_type>
	_2; COLLADA_DOM_N(1,2)
	DAEP::Value<5,dae_Array<>> _N; enum{ _No=5 };
	DAEP::Value<8,daeContents> content; typedef __NS__<5783> notestart;
	}_;

public: //Elements
		
	COLLADA_WORD_ALIGN
	/**
	 * The instance_physics_scene element declares the instantiation of a COLLADA
	 * physics_scene resource. The instance_physics_scene element may appear any
	 * number of times.
	 * @see XSD @c instance_with_extra_type
	 */
	DAEP::Child<6,instance_with_extra_type,_,(_::_)&_::_0> instance_physics_scene;
	/**UNDOCUMENTED
	 * The XSD schema does not provide documentation in this case.
	 * @see XSD @c instance_kinematics_scene_type
	 */
	DAEP::Child<5,instance_kinematics_scene_type,_,(_::_)&_::_1> instance_kinematics_scene;
	/**
	 * The extra element may appear any number of times.
	 * @see XSD @c extra_type
	 */
	DAEP::Child<4,extra_type,_,(_::_)&_::_2> extra;

	COLLADA_DOM_N(1,2) union //NO-NAMES & ONLY-CHILDS
	{
	/**NO-NAMES
	 * These elements are invalid according to the schema. They may be user-defined 
	 * additions and substitutes.
	 */
	DAEP::Child<1,xs::any,_,(_::_)&_::_N> extra_schema__unnamed;
	/**
	 * The instance_visual_scene element declares the instantiation of a COLLADA
	 * visual_scene resource. The instance_visual_scene element may only appear
	 * once.
	 * @see XSD @c instance_with_extra_type
	 */
	DAEP::Child<-1,instance_with_extra_type,_,(_::_)&_::_N> instance_visual_scene;
	};

public: //Content
	/**
	 * Children, mixed-text, comments & processing-instructions.
	 */
	DAEP::Value<8,daeContents,_,(_::_)&_::content> content;
};
//-------.
    }//<-'
}

#include "asset_type.h"
#include "library_animations_type.h"
#include "library_animation_clips_type.h"
#include "library_cameras_type.h"
#include "library_controllers_type.h"
#include "library_geometries_type.h"
#include "library_effects_type.h"
#include "library_force_fields_type.h"
#include "library_images_type.h"
#include "library_lights_type.h"
#include "library_materials_type.h"
#include "library_nodes_type.h"
#include "library_physics_materials_type.h"
#include "library_physics_models_type.h"
#include "library_physics_scenes_type.h"
#include "library_visual_scenes_type.h"
#include "library_joints_type.h"
#include "library_kinematics_models_type.h"
#include "library_articulated_systems_type.h"
#include "library_kinematics_scenes_type.h"
#include "library_formulas_type.h"
#include "instance_with_extra_type.h"
#include "instance_kinematics_scene_type.h"
#include "extra_type.h"
#define COLLADA_target_namespace \
COLLADA::http_www_collada_org_2008_03_COLLADASchema
COLLADA_(namespace)
{//-.
//<-'
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
	a->setDefaultString("http://www.collada.org/2008/03/COLLADASchema");
	a->setIsRequired();
	a = el.addAttribute(toc->version,"version_enum","version");
	a->setDefaultString("1.5.0");
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
		el.addCM<XS::Element>(cm,15,1,1).setChild(toc->library_joints,"library_joints");
		el.addCM<XS::Element>(cm,16,1,1).setChild(toc->library_kinematics_models,"library_kinematics_models");
		el.addCM<XS::Element>(cm,17,1,1).setChild(toc->library_articulated_systems,"library_articulated_systems");
		el.addCM<XS::Element>(cm,18,1,1).setChild(toc->library_kinematics_scenes,"library_kinematics_scenes");
		el.addCM<XS::Element>(cm,19,1,1).setChild(toc->library_formulas,"library_formulas");
		el.popCM<20,3000>(cm);
	el.addCM<XS::Element>(cm,60001,0,1).setChild(toc->scene,"scene");
	el.addCM<XS::Element>(cm,60002,0,-1).setChild(toc->extra,"extra");
	el.addContentModel<60003,1>(cm,toc); return *om;
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
	el.addCM<XS::Element>(cm,2,0,-1).setChild(toc->instance_kinematics_scene,"instance_kinematics_scene");
	el.addCM<XS::Element>(cm,3,0,-1).setChild(toc->extra,"extra");
	el.addContentModel<4,1>(cm,toc); return *om;
}
#endif //!COLLADA_DOM_LITE
//------.
    //<-'
	namespace DAEP //WYSIWYG
	{
        COLLADA_(http_www_collada_org_2008_03_COLLADASchema,namespace)
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
	 * @see XSD @c version_enum
	 */
	typedef ::COLLADA_target_namespace::version_enum version_enum;
	/**
	 * The xml:base attribute allows you to define the base URI for this COLLADA
	 * document. See http://www.w3.org/TR/xmlbase/ for more information.
	 * @see XSD @c xml::base
	 */
	typedef xml::base xml_base;
	/**
	 * The COLLADA element must contain an asset element.
	 * @see XSD @c asset_type
	 */
	typedef struct asset asset;
	/**
	 * The COLLADA element may contain any number of library_animations elements.
	 * @see XSD @c library_animations_type
	 */
	typedef struct library_animations library_animations;
	/**
	 * The COLLADA element may contain any number of library_animation_clips elements.
	 * @see XSD @c library_animation_clips_type
	 */
	typedef struct library_animation_clips library_animation_clips;
	/**
	 * The COLLADA element may contain any number of library_cameras elements.
	 * @see XSD @c library_cameras_type
	 */
	typedef struct library_cameras library_cameras;
	/**
	 * The COLLADA element may contain any number of library_controllerss elements.
	 * @see XSD @c library_controllers_type
	 */
	typedef struct library_controllers library_controllers;
	/**
	 * The COLLADA element may contain any number of library_geometriess elements.
	 * @see XSD @c library_geometries_type
	 */
	typedef struct library_geometries library_geometries;
	/**
	 * The COLLADA element may contain any number of library_effects elements.
	 * @see XSD @c library_effects_type
	 */
	typedef struct library_effects library_effects;
	/**
	 * The COLLADA element may contain any number of library_force_fields elements.
	 * @see XSD @c library_force_fields_type
	 */
	typedef struct library_force_fields library_force_fields;
	/**
	 * The COLLADA element may contain any number of library_images elements.
	 * @see XSD @c library_images_type
	 */
	typedef struct library_images library_images;
	/**
	 * The COLLADA element may contain any number of library_lights elements.
	 * @see XSD @c library_lights_type
	 */
	typedef struct library_lights library_lights;
	/**
	 * The COLLADA element may contain any number of library_materials elements.
	 * @see XSD @c library_materials_type
	 */
	typedef struct library_materials library_materials;
	/**
	 * The COLLADA element may contain any number of library_nodes elements.
	 * @see XSD @c library_nodes_type
	 */
	typedef struct library_nodes library_nodes;
	/**
	 * The COLLADA element may contain any number of library_materials elements.
	 * @see XSD @c library_physics_materials_type
	 */
	typedef struct library_physics_materials library_physics_materials;
	/**
	 * The COLLADA element may contain any number of library_physics_models elements.
	 * @see XSD @c library_physics_models_type
	 */
	typedef struct library_physics_models library_physics_models;
	/**
	 * The COLLADA element may contain any number of library_physics_scenes elements.
	 * @see XSD @c library_physics_scenes_type
	 */
	typedef struct library_physics_scenes library_physics_scenes;
	/**
	 * The COLLADA element may contain any number of library_visual_scenes elements.
	 * @see XSD @c library_visual_scenes_type
	 */
	typedef struct library_visual_scenes library_visual_scenes;
	/**UNDOCUMENTED
	 * The XSD schema does not provide documentation in this case.
	 * @see XSD @c library_joints_type
	 */
	typedef struct library_joints library_joints;
	/**UNDOCUMENTED
	 * The XSD schema does not provide documentation in this case.
	 * @see XSD @c library_kinematics_models_type
	 */
	typedef struct library_kinematics_models library_kinematics_models;
	/**UNDOCUMENTED
	 * The XSD schema does not provide documentation in this case.
	 * @see XSD @c library_articulated_systems_type
	 */
	typedef struct library_articulated_systems library_articulated_systems;
	/**UNDOCUMENTED
	 * The XSD schema does not provide documentation in this case.
	 * @see XSD @c library_kinematics_scenes_type
	 */
	typedef struct library_kinematics_scenes library_kinematics_scenes;
	/**UNDOCUMENTED
	 * The XSD schema does not provide documentation in this case.
	 * @see XSD @c library_formulas_type
	 */
	typedef struct library_formulas library_formulas;
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
	 * @see XSD @c extra_type
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
	 * @see XSD @c version_enum
	 */
	typedef const ::COLLADA_target_namespace::version_enum version_enum;
	/**CONST-FORM
	 * The xml:base attribute allows you to define the base URI for this COLLADA
	 * document. See http://www.w3.org/TR/xmlbase/ for more information.
	 * @see XSD @c xml::base
	 */
	typedef const xml::base xml_base;
	/**CONST-FORM
	 * The COLLADA element must contain an asset element.
	 * @see XSD @c asset_type
	 */
	typedef struct const_asset asset;
	/**CONST-FORM
	 * The COLLADA element may contain any number of library_animations elements.
	 * @see XSD @c library_animations_type
	 */
	typedef struct const_library_animations library_animations;
	/**CONST-FORM
	 * The COLLADA element may contain any number of library_animation_clips elements.
	 * @see XSD @c library_animation_clips_type
	 */
	typedef struct const_library_animation_clips library_animation_clips;
	/**CONST-FORM
	 * The COLLADA element may contain any number of library_cameras elements.
	 * @see XSD @c library_cameras_type
	 */
	typedef struct const_library_cameras library_cameras;
	/**CONST-FORM
	 * The COLLADA element may contain any number of library_controllerss elements.
	 * @see XSD @c library_controllers_type
	 */
	typedef struct const_library_controllers library_controllers;
	/**CONST-FORM
	 * The COLLADA element may contain any number of library_geometriess elements.
	 * @see XSD @c library_geometries_type
	 */
	typedef struct const_library_geometries library_geometries;
	/**CONST-FORM
	 * The COLLADA element may contain any number of library_effects elements.
	 * @see XSD @c library_effects_type
	 */
	typedef struct const_library_effects library_effects;
	/**CONST-FORM
	 * The COLLADA element may contain any number of library_force_fields elements.
	 * @see XSD @c library_force_fields_type
	 */
	typedef struct const_library_force_fields library_force_fields;
	/**CONST-FORM
	 * The COLLADA element may contain any number of library_images elements.
	 * @see XSD @c library_images_type
	 */
	typedef struct const_library_images library_images;
	/**CONST-FORM
	 * The COLLADA element may contain any number of library_lights elements.
	 * @see XSD @c library_lights_type
	 */
	typedef struct const_library_lights library_lights;
	/**CONST-FORM
	 * The COLLADA element may contain any number of library_materials elements.
	 * @see XSD @c library_materials_type
	 */
	typedef struct const_library_materials library_materials;
	/**CONST-FORM
	 * The COLLADA element may contain any number of library_nodes elements.
	 * @see XSD @c library_nodes_type
	 */
	typedef struct const_library_nodes library_nodes;
	/**CONST-FORM
	 * The COLLADA element may contain any number of library_materials elements.
	 * @see XSD @c library_physics_materials_type
	 */
	typedef struct const_library_physics_materials library_physics_materials;
	/**CONST-FORM
	 * The COLLADA element may contain any number of library_physics_models elements.
	 * @see XSD @c library_physics_models_type
	 */
	typedef struct const_library_physics_models library_physics_models;
	/**CONST-FORM
	 * The COLLADA element may contain any number of library_physics_scenes elements.
	 * @see XSD @c library_physics_scenes_type
	 */
	typedef struct const_library_physics_scenes library_physics_scenes;
	/**CONST-FORM
	 * The COLLADA element may contain any number of library_visual_scenes elements.
	 * @see XSD @c library_visual_scenes_type
	 */
	typedef struct const_library_visual_scenes library_visual_scenes;
	/**CONST-FORM, UNDOCUMENTED
	 * The XSD schema does not provide documentation in this case.
	 * @see XSD @c library_joints_type
	 */
	typedef struct const_library_joints library_joints;
	/**CONST-FORM, UNDOCUMENTED
	 * The XSD schema does not provide documentation in this case.
	 * @see XSD @c library_kinematics_models_type
	 */
	typedef struct const_library_kinematics_models library_kinematics_models;
	/**CONST-FORM, UNDOCUMENTED
	 * The XSD schema does not provide documentation in this case.
	 * @see XSD @c library_articulated_systems_type
	 */
	typedef struct const_library_articulated_systems library_articulated_systems;
	/**CONST-FORM, UNDOCUMENTED
	 * The XSD schema does not provide documentation in this case.
	 * @see XSD @c library_kinematics_scenes_type
	 */
	typedef struct const_library_kinematics_scenes library_kinematics_scenes;
	/**CONST-FORM, UNDOCUMENTED
	 * The XSD schema does not provide documentation in this case.
	 * @see XSD @c library_formulas_type
	 */
	typedef struct const_library_formulas library_formulas;
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
	 * @see XSD @c extra_type
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
	 * @see XSD @c instance_with_extra_type
	 */
	typedef struct __XSD__instance_with_extra_type__ instance_physics_scene;
	/**
	 * The instance_visual_scene element declares the instantiation of a COLLADA
	 * visual_scene resource. The instance_visual_scene element may only appear
	 * once.
	 * @see XSD @c instance_with_extra_type
	 */
	typedef struct __XSD__instance_with_extra_type__ instance_visual_scene;
	/**UNDOCUMENTED
	 * The XSD schema does not provide documentation in this case.
	 * @see XSD @c instance_kinematics_scene_type
	 */
	typedef struct instance_kinematics_scene instance_kinematics_scene;
	/**
	 * The extra element may appear any number of times.
	 * @see XSD @c extra_type
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
	 * @see XSD @c instance_with_extra_type
	 */
	typedef struct __XSD__const_instance_with_extra_type__ instance_physics_scene;
	/**CONST-FORM
	 * The instance_visual_scene element declares the instantiation of a COLLADA
	 * visual_scene resource. The instance_visual_scene element may only appear
	 * once.
	 * @see XSD @c instance_with_extra_type
	 */
	typedef struct __XSD__const_instance_with_extra_type__ instance_visual_scene;
	/**CONST-FORM, UNDOCUMENTED
	 * The XSD schema does not provide documentation in this case.
	 * @see XSD @c instance_kinematics_scene_type
	 */
	typedef struct const_instance_kinematics_scene instance_kinematics_scene;
	/**CONST-FORM
	 * The extra element may appear any number of times.
	 * @see XSD @c extra_type
	 */
	typedef struct const_extra extra;
};
//-----------.
        }//<-'
    }
}
#undef COLLADA_target_namespace
#endif //__COLLADA_h__http_www_collada_org_2008_03_COLLADASchema__ColladaDOM_g1__
/*C1071*/