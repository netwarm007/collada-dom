/*
 * Copyright 2006 Sony Computer Entertainment Inc.
 *
 * Licensed under the MIT Open Source License, for details please see license.txt or the website
 * http://www.opensource.org/licenses/mit-license.php
 *
 */

#ifndef __COLLADA_DOM__DAEP_H__
#define __COLLADA_DOM__DAEP_H__

#include "dae/daePlatonic.h"

COLLADA_(namespace)
{	
	/**namespace COLLADA::DAEP
	 *
	 * DAEP is an initialism of:
	 *
	 * Digital-Artifact-Exchange-Protocol.
	 *
	 * Artifact can be shortened to "Art."
	 *
	 * DAEP is an abstract portable API definition.
	 * (This just means that conforming code can be
	 * compiled against a conforming implementation.)
	 *
	 * It's used by COLLADA-DOM to hide implementation
	 * details. Primarily so that generated classes may
	 * use the same names as the XSD files they are from.
	 *
	 * DAEP classes should have data members and virtual
	 * method layouts, and no more (names don't matter.)
	 */
	namespace DAEP
	{
		class Make;
		class Model;
		class Object;
		class Element;//-.
//<----------------------'

template<class Type=void>
/**
 * @class COLLADA::DAEP::Proto
 *
 * DAEP Proto is used by @c InnerValue::InnerValue() to initialize
 * its base-class's constructor. This is a signature for prototype
 * construction. (@c DAEP::Value is a big messy template, and it's
 * necessary to unambiguously qualify the underlying data's type.)
 *
 * DAEP Proto is also used by DAEP Elemental's default constructor.
 */
class Proto : public Type
{
COLLADA_(public) COLLADA_SUPPRESS_C(4624) 

	enum
	{   
	/** 
	 * These can't go in DAEP InnerChild and InnerValue respectively
	 * because of C++ point-of-instantiation rules (ODR?) or because
	 * of how some compilers (potentially older?) instantiation them.
	 */
	is_fixed=Type::note::is_fixed,has_default=Type::note::has_default,
	};
};
/**NOP @c note is undefined. */
template<> class Proto<void>{};

/**ABSTRACT BASE-CLASS 
 *
 * @class COLLADA::DAEP::Object
 *
 * @c Object is the top class in the object model hierarchy.
 * @see @c daeObject Doxygentation for explanations of the variables.
 *
 * @remarks The size of this class is FIXED.
 * (Changes to the size are breaking-changes.)
 */
class Object
{
	friend class COLLADA::daeObject;
	friend class COLLADA::daeFeature;

COLLADA_(public)
	/**HACK
	 * @c daeStringRef is using this to detect
	 * if a constructor is object-based.
	 */
	typedef signed __DAEP__Object__signed;

COLLADA_(public) //VIRTUAL METHOD TABLE
	/**
	 * Virtual Destructor
	 * Best to come first in the vtable layout. 
	 * Same as @c daeObject::_release().
	 * @see dae.cpp TU's definition.
	 *
	 * @remarks The "NOP" constructor sets up "parent==this".
	 * Theorectically a compiler can elide construction, and
	 * call through the virtual-method-table near-statically.
	 */
	virtual ~Object()
	{
		//Notice. This is similar to daeObject::_release().
		//Except. Notice it is ==0. This is to keep sub-objects from reentering
		//the parent's destructor. (That triggers the sub-objects' destructors.)
		if(this!=__DAEP__Object__parent)
		if(0==--__DAEP__Object__parent->__DAEP__Object__refs)	
		__DAEP__Object__parent->__DAEP__Object__0(); 
	}

	/**PURE, ABSTRACT-INTERFACE
	 * Gets the object's Platonic-object. The pre-2.5 metadata
	 * is moved into @c DAEP::Model::__DAEP__Model__meta, which
	 * is a @c daeMetaObject, or potentially a @c daeMetaElement.
	 */
	virtual DAEP::Model &__DAEP__Object__v1__model()const
	{
		//Visual Studio wants this to instantiate the objects in
		//the DBase classes. It didn't want it when they were base
		//classes, not sure why, but C++ doesn't describe the layout
		//of base classes, so they have to be members/can't be casted.
		assert(0); return *(DAEP::Model*)nullptr;
	}
								   		
	/** 
	 * "1" is the virtual table version. 
	 * This needs to increase if builds add
	 * to the virtual table here, or elsewhere.
	 * 
	 * @see @c daeElementTags::interfaceTag, which is
	 * used to add methods to @c daeElement, and is the
	 * same data member used by @c daeObject::_getExtTag().
	 */
	enum{ __DAEP__Object__vN__=1 };

	//The virtual table reserves these.	
	//If added, @c daeObject::_getVersion() must be checked.
	//(The working assumption is that these methods can be renamed without breaking the system.)
	virtual daeError __DAEP__Object__reserved_vtable_entry_0(){ return DAE_ERR_NOT_IMPLEMENTED; }
	virtual daeError __DAEP__Object__reserved_vtable_entry_1(){ return DAE_ERR_NOT_IMPLEMENTED; }
	virtual daeError __DAEP__Object__reserved_vtable_entry_2(){ return DAE_ERR_NOT_IMPLEMENTED; }
	virtual daeError __DAEP__Object__reserved_vtable_entry_3(){ return DAE_ERR_NOT_IMPLEMENTED; }

COLLADA_(private) //DATA-MEMBERS

	friend class COLLADA::daeDocument;
	/**32bit--should align before _tags. */
	mutable int __DAEP__Object__refs;	
	/**32bit--should align behind _refs. */
	int __DAEP__Object__tags;
	/**
	 * This is circular for @c daeDOM and if @isUnparentedObject().
	 * (In other words: Such objects are their own "parent.")
	 *
	 * @c nullptr
	 * ==========
	 * The value of @c nullptr is reserved. In some scenarios @c nullptr
	 * is a valid value for a DAEP Object pointer. It may be decided one
	 * day that @c nullptr should behave meaningfully always. This would
	 * have a toll, however DAEP is not a time-sensitive problem domain.
	 */
	const DAEP::Object *__DAEP__Object__parent; 

COLLADA_(private) //MISCELLANEOUS

	/** @see dae.cpp TU's definition. 
	 * Manages removal from the database.
	 * Called by @c daeObject::_release() on 0.
	 */
	COLLADA_DOM_LINKAGE void __DAEP__Object__0()const;
	
COLLADA_(protected) //CONSTRUCTORS	
	/**
	 * Prototype Non-Constructor
	 * 
	 * It does install the "vptr" even though it's not necessary.
	 */
	explicit Object(const DAEP::Proto<>&){ /*NOP*/ }

COLLADA_(public) //CONSTRUCTORS (MUST BE PUBLIC)
	/**
	 * Non-Constructor
	 * This is used to extract a "vptr" or prior to placement-new.
	 *
	 * @warning All classes based on DAEP Object must have a direct
	 * line to this constructor. It isn't called by the base-class.
	 */
	Object() 
	COLLADA_SUPPRESS_C(4355)
	:__DAEP__Object__parent(this) //neutralizing ~Object()
	{ /*NOP*/ }
	/**
	 * Constructor
	 *
	 * @param c can be @c this object. 
	 * Most code assumes the container is not @c nullptr.
	 *
	 * @warning All classes based on DAEP Object must have a direct
	 * line to this constructor. It isn't called by the base-class.
	 *
	 * TOWARD MULTI-THREAD
	 * ===================
	 * InterlockedIncrement(c->__DAEP__Object__refs)
	 */
	explicit Object(const Object *c)
	:__DAEP__Object__parent(c)
	,__DAEP__Object__refs(__DAEP__Object__refs_embedding_threshold)
	,__DAEP__Object__tags(__DAEP__Object__vN__<<2*sizeof(daeByte)*CHAR_BIT)
	{
		assert(0!=COLLADA::DOM_process_share);
		((daeTag*)&__DAEP__Object__tags)[3] = COLLADA::DOM_process_share;

		if(c!=this) c->__DAEP__Object__refs++;
		//If this does not hold, _tags must be larger on such systems.
		daeCTC<sizeof(__DAEP__Object__tags)>=sizeof(daeTag)*4>();
		//Visual Studio is showing 0 in tooltips.
		assert(__DAEP__Object__tags>65535); 
	}
	/**
	 * This is used to identify embedded objects. 
	 * Embedded objects do not auto-delete themselves.
	 * The constructor marks all objects as initially embedded.
	 */
	enum{ __DAEP__Object__refs_embedding_threshold=2147483647/2 };
	/**
	 * This is makes an object self-destruct/delete itself.
	 * @c assert(__DAEP__Object__refs>=1) is triggered if the 
	 * object doesn't have a reference holder prior to calling.
	 */
	inline void __DAEP__Object__unembed(daeTag data_bit=0)
	{
		__DAEP__Object__refs-=__DAEP__Object__refs_embedding_threshold;
		assert(__DAEP__Object__refs>=0);
		//While technically unrelated, there's no reason for two APIs.
		assert(data_bit<=1&&((daeTag*)&__DAEP__Object__tags)[3]%2==0);
		if(1==data_bit) ((daeTag*)&__DAEP__Object__tags)[3]|=data_bit;
	}
	/**
	 * This is marks an object as an internal structure. By
	 * making its reference count permanently less than 0, it
	 * notifies its parent object each time its count decreases.
	 */
	inline void __DAEP__Object__embed_subobject(const DAEP::Object *c)
	{
		__DAEP__Object__refs-=2*__DAEP__Object__refs_embedding_threshold;		
		assert(__DAEP__Object__refs<0);
		assert(__DAEP__Object__parent==this||__DAEP__Object__parent==c); 
		if(c!=__DAEP__Object__parent) __DAEP__Object__parent = c; 
		else c->__DAEP__Object__refs--; 
	}

COLLADA_(private) //INHERITED NON-CONSTRUCTORS
	/**
	 * Disabled Copy Constructor & Assignment Operator
	 *
	 * As @c private, derived classes will error if copy-constructed.
	 *
	 * IF THIS IS NOT LINKING, ADD A PRIVATE COPY-CTOR TO daeObject AND
	 * TRACE IT TO WHERE A PARENT SHOULD PROBABLY BE PASSED VIA POINTER.
	 */
	 Object(const Object&); void operator=(const Object&);
};
		   
/**ABSTRACT BASE-CLASS 
 *
 * @class COLLADA::DAEP::Element
 *
 * @c Element is the XML-like element class.
 * @see @c daeElement.
 * @see @c DAEP::Elemental.
 * @remarks The size of this class is FIXED.
 * (Changes to the size are breaking-changes.)
 *
 * VIRTUAL INHERITANCE
 * ===================
 * @note Virtual inheritance is used only to
 * better ensure that the DAEP classes overlay
 * properly when they're @c reinterpret_cast to
 * their "daeX" counterparts. 
 * (This is because @c daeElement cannot inherit
 * from both @c DAEP::Element and @c daeObject.)
 */
class Element : public DAEP::Object
{ 	
COLLADA_(public) //INTERNAL
	/**wARNING
	 * Initiates "teardown" sequence. 
	 *
	 * @warning This is more of a formality than
	 * anything else. It's hard to think of good
	 * reasons to use it. If you need to be sure
	 * a freestanding element is deleted, it can
	 * do that; but calling "__COLLADA__atomize"
	 * isn't something to do inside of user-code.
	 */
	COLLADA_DOM_LINKAGE void __COLLADA__atomize();

COLLADA_(public) //OPERATORS

	//Unfortunately DAEP Element is no longer inherited.
	//ON-THE-ODD-SIDE: Operator-> is exposing daeElement.
	COLLADA_DOM_OBJECT_OPERATORS(daeElement)
	/**
	 * This is to avoid writing @c dae() in many places.
	 */
	inline operator daeElement&(){ return *(daeElement*)this; }
	/**CONST-FORM
	 * This is to avoid writing @c dae() in many places.
	 */
	inline operator const daeElement&()const{ return *(daeElement*)this; }

COLLADA_(protected) //DATA-MEMBERS

	friend class Change;
	friend class COLLADA::daeElement;
	friend union COLLADA::daeContent;
	friend class COLLADA::daeDocument;
	struct __DAEP__Element__Data
	{
		daePseudonym NCName;
		/**
		 * These values are cached for the lifetime of
		 * the object.
		 * @c meta is technically required to implement
		 * multiple schema versions.
		 * @c DOM is not exposed, but its slot should be
		 * permanent, whether it's decided to keep or not.
		 */
		daeMeta *const meta; daeDOM *const DOM;		
		/**
		 * These are @c const just to prove we mean business.
		 */
		__DAEP__Element__Data():meta(meta),DOM(DOM){ /*NOP*/ };

		/** Change this if a @c daeChildID is added to this structure. */
		enum{ _have_child_ID_onboard=0 };

		//THESE ARE FLAGS. THEY MAY NEED TO BE OCCUPY 64-BITS?
		/**BITFIELD
		 * This is for migration handling. Traversing a graph can
		 * be painful. Typically elements are inserted without any
		 * content in tow. Testing this flag can avoid digging deep
		 * into the metadata, e.g. calling daeElement::getContents().		 
		 *
		 * @note THIS IS ONLY A HINT, AND DOESN'T CONTEMPLATE EMBEDDED
		 * CONTENTS-ARRAYS. (As of now, embedding is not implemented.)
		 */
		mutable unsigned daeContents_capacity_is_nonzero:1;		
		/**BITFIELD-CONTINUED
		 * This means the element metadata contains an "id" like attribute.
		 * These are registered with @c XS::Schema::getIDs().
		 */
		mutable unsigned getMeta_getFirstID_is_nonzero:1;		
		/**BITFIELD-CONTINUED
		 * This means the element metadata contains an "id" like attribute.
		 * These are registered with @c XS::Schema::getIDs().
		 */
		mutable unsigned is_document_daePseudoElement:1;

	}__DAEP__Element__data; 	
	char __DAEP__Element__data_reserved
	[sizeof(void*)*12-sizeof(DAEP::Object)-sizeof(__DAEP__Element__Data)];

COLLADA_(protected) //CONSTRUCTORS

	//////REMINDER////////////////////////////////////////////////
	////These must be REPLICATED by daeElement in daeElement.h////
	//////////////////////////////////////////////////////////////

	/**
	 * Non-Constructor
	 * This is used to extract a "vptr" or prior to placement-new.
	 */
	Element(){ /*NOP*/ }
	/**
	 * Prototype Constructor
	 *
	 * This "NOP" differs slightly from @c Element::Element() in that
	 * it doesn't even do @c __DAEP__Object_parent=this, because that
	 * would make the outer constructor unable to locate its database.
	 */
	explicit Element(const DAEP::Proto<> &pt):Object(pt){ /*NOP*/ }
	/**WARNING
	 * Prototypes' Constructor
	 *
	 * This is following regular DAEP Object construction rules, even
	 * though DAEP Elemental is its only caller.
	 *
	 * @remarks This must be @c inline to @c capture DOM_process_share.
	 * Initialization is handled internally, so clients aren't calling
	 * exported APIs for no real reason.
	 */
	explicit Element(const Object *_this):Object(_this){ __DAEP__Object__unembed(1); }
};

/**ENUM, INTERNALS
 *
 * @c enum COLLADA::DAEP::VPTR & PTYPE
 *
 * These are used by DAEP Elemental's constructors.
 * "VPTR" stands for virtual method table pointers.
 * "PTYPE" stands for "prototype," a la DAEP Proto.
 * These don't belong to ELEMENT/CONTENT/ATTRIBUTE.
 */
enum VPTR{ VPTR=0 }; enum PTYPE{ PTYPE=0 };

//NEW: Legacy is one of daeElement or DAEP Element.
template<class T, class Legacy=daeLegacyOf<T>::type>
/**
 * @class COLLADA::DAEP::Elemental
 *
 * @tparam T must be based on @c DAEP::Elemental<T>.
 *
 * DAEP Elemental is used to efficiently extract the Model
 * from a generated class based on @c DAEP::Element.
 *
 * @remarks Note that __DAEP__Object__v1__model is declared,
 * -but that it isn't implemented. The generator implements
 * this method, and @a T inherits the implementation.
 *
 * The following code demonstrates use of @c DAEP::Elemental
 * by way of @c daeElemental (that's based on DAEP Elemental.)
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.C++}
 *	class domCOLLADA : public daeElemental<domCOLLADA>
 *	{
 *		<class definition goes here.>
 *	};
 *	DAEP::Model &DAEP::Elemental<domCollada>::__DAEP__Object__v1__model()const
 *	{
 *		<return daeModel by address.>
 *	}
 */
class Elemental : public Legacy
{
COLLADA_(public) //DAEP::Object methods

	/**PURE-OVERRIDE
	 * This __DAEP__Object__v1__model declation has to be defined.
	 */
	virtual DAEP::Model &__DAEP__Object__v1__model()const;	
				   
COLLADA_(public)
	/**
	 * Using to get the type of DAEP Element WRT a given class.
	 * @c __COLLADA__T is added to work around @c daeSmartRef.
	 */
	typedef T __COLLADA__T, __COLLADA__Element;

	/**
	 * This is used to shorten/secure DAEP Child/Value declarations.
	 */
	typedef char T::*_;

	/**
	 * @c Elemental::TOC is used inside @c __DAEP__Object__v1__model().
	 * It's a pointer-to-nowhere that can be passed to setup templates.
	 */
	typedef T *TOC;

COLLADA_(public) //UTILITIES
	/**HELPER,
	 * @struct @c COLLADA::DAEP::Elemental::Essentials
	 * 
	 * All elements have these features, although names differ
	 * according to how the generator is configured. Extracting
	 * this information means generators don't have to ouptut it.
	 *
	 * @todo This could be implemented by @c XS::Schema and 
	 * @c daeMetaElement where it's used. It's slightly better 
	 * organized to have here. SFINAE might be required if <_> is
	 * the name of one of some schema's children. 
	 */
	struct Essentials
	{
		daeOffset content_offset;
		daeFeatureID content_feature, union_feature;
		template<class CC> void set(CC* &_)
		{
			daeCTC<CC::_No==_->_N.name>();
			union_feature = _->_N.feature();
			content_feature = _->content.feature(); 
			content_offset = daeOffsetOf(CC,content);			
		}
		Essentials(){ typename T::_ *C4700; set(C4700); }		
	};

COLLADA_(public) //Elemental constructor
	/**
	 * Classic/Non-Constructor
	 *
	 * This is used to extract a "vptr."
	 * @remarks This is actually the original reason for DAEP Elemental.
	 * But, because there can be only one default-constructor, it's had
	 * to take a back seat.
	 */
	explicit Elemental(enum DAEP::VPTR){ /*NOP*/ }	

COLLADA_(protected) //Embodied constructor
	/**
	 * Default/Prototype Constructor 
	 *
	 * This is a special constructor that means the memory footprint
	 * is already initialized beneath the constructor, by memcpy'ing 
	 * a "prototype" image over it. The metadata includes prototypes.
	 *
	 * It has to be the default-constructor so that the compiler can
	 * automatically generate a default for generated classes. Where
	 * if it were not, generators would have to output a constructor.
	 */
	Elemental():COLLADA_SUPPRESS_C(4355)Legacy((const DAEP::Proto<>&)*this)
	{
		/*NOP*/ daeCTC<!T::__DAEP__Schema__g1__is_abstract>();
	}

COLLADA_(public) //One more for the road!
	/**
	 * The Prototypes' Constructor
	 *
	 * This is used by @c daeMetaElement to construct its prototypes.
	 * It can't use @c T::T() because that's the prototype constructor.
	 * @c DAEP::Element is an abstract class. So it can't be constructed.
	 */
	explicit Elemental(enum DAEP::PTYPE):COLLADA_SUPPRESS_C(4355)Legacy(this){}	
};

template<unsigned long long ULL=~0ULL> 
/** 
 * @class COLLADA::DAEP::Schema
 *
 * DAEP Schema statically advertises XML Schema (XSD) parameters.
 * @see @c DAEP::Schematic Doxygentation.
 */
class Schema
{
COLLADA_(public)
	/**
	 * Enum of boolean-like traits.
	 * These are nonzero if flagged.
	 * If @a Traits is left to default to "~0" it forms bitmasks.
	 * @see @c DAEP::Schematic Doxygentation.
	 */
	enum
	{
	__DAEP__Schema__Traits
	= ULL&0xFFFFFFFF,
	/** One of: empty, simple, complex, or mixed XSD content type. */
	__DAEP__Schema__g1__content_model
	= ULL&3,
	/** This means this is not a global type, within its XSD schema. */
	__DAEP__Schema__g1__is_local
	= ULL&4,
	/** This means this is pseudo type, like a wrapper of an xs:group. */
	__DAEP__Schema__g1__is_group
	= ULL&8,
	/** This means this is a placeholder for one or more concrete types. */
	__DAEP__Schema__g1__is_abstract
	= ULL&16,	
	/** This means that any kind of element can be a child of the element. */
	__DAEP__Schema__g1__allows_any
	= ULL&32,
	/** This means that any attribute is allowed and their types are opaque. */
	__DAEP__Schema__g1__allows_any_attribute
	= ULL&64,
	/** This means that the element uses the <xs:all> virtual content model. */
	__DAEP__Schema__g1__is_all
	= ULL&128,
	};

	/**
	 * This is a DAEP schema as a 64 bit code.
	 */
	static const unsigned long long __DAEP__Schema = ULL;

	/**
	 * Static __DAEP__Model__genus equivalent.
	 * 2.5 style generators emit an @c elementType constant, that
	 * cannot be represented by the ColladaDOM 3 style generators.
	 */
	static const int __DAEP__Schema__genus = 1+int(ULL>>48);

	/**
	 * Bits 32 through 38 count the number of built-in attributes.
	 * Value IDs greater-than-or-equal-to this value, are CONTENT.
	 */
	static const int __DAEP__Schema__extent_of_attributes = 0x3F&ULL>>32;
};
  
template<class T> 
/** 
 * @class COLLADA::DAEP::Schematic
 *
 * DAEP Schematic statically extracts DAEP Schema parameters.
 *
 * @note This class exposes unadvertised compile-time variables.
 * This is a necessary consequence of ceding the entire namespace
 * to the XML schema.
 *
 * VALIDATATION
 * ============
 * A Schematic represents what the generator believes a valid XML
 * -like document looks like. DAEP implementations that transform
 * the object-hierarchy in order to prepare a new "document" must
 * support/allow for "invalid" and/or intermediate configurations.
 */
class Schematic
{
COLLADA_(private)
	/**
	 * @note Assuming can ignore @c const qualifier.
	 * __COLLADA__T is defined by @c daeSmartRef<T>.
	 */
	typedef typename T::__COLLADA__T _;

COLLADA_(public)
	/**
	 * The generated class, accessed via pointers. 
	 */
	typedef typename _::__COLLADA__Element type;

	/**
	 * This is a way to form a C++ reference to a
	 * cursor-array inside of the contents-arrays
	 * for this @c type for a given name BUT ONLY
	 * if that name is non-singular.
	 */
	typedef dae_Array<type> content;	

	/**
	 * This is a type-ID for an element type, that is valid for
	 * a given generated schema. It's meaningless if types come
	 * from different generator sessions. It's provided because
	 * the @case claues of @c switch statements can't do better.
	 */
	static const int genus = _::__DAEP__Schema__genus;

	/** One of: empty (0), simple (1), complex (2), or mixed (3). */
	static const int XSD_content_model
	= _::__DAEP__Schema__g1__content_model;
	/** Empty models have only comments and processing-instructions. */
	static const bool is_of_empty_type
	= 0==_::__DAEP__Schema__g1__content_model;
	/** Simple has value-content after comments/processing-instructions. */
	static const bool is_of_simple_type
	= 1==_::__DAEP__Schema__g1__content_model;
	/** Complex has child-elements among comments/processing-instructions. */
	static const bool is_of_complex_type
	= 2==_::__DAEP__Schema__g1__content_model;
	/** Mixed has text-and-elements among comments/processing-instructions. */
	static const bool is_of_mixed_type
	= 3==_::__DAEP__Schema__g1__content_model;

	/** This means this is not a global type, within its XSD schema. */
	static const bool is_local
	= 0!=_::__DAEP__Schema__g1__is_local;
	/** This means this is pseudo type, like a wrapper of an xs:group. */
	static const bool is_group
	= 0!=_::__DAEP__Schema__g1__is_group;
	/** This means this is a placeholder for one or more concrete types. */
	static const bool is_abstract
	= 0!=_::__DAEP__Schema__g1__is_abstract;
	/** This means that any kind of element can be a child of the element. */
	static const bool allows_any
	= 0!=_::__DAEP__Schema__g1__allows_any;
	/** This means that any attribute is allowed and their types are opaque. */
	static const bool allows_any_attribute 
	= 0!=_::__DAEP__Schema__g1__allows_any_attribute;
	/** This means that the element uses the <xs:all> virtual content model. */
	static const bool is_all 
	= 0!=_::__DAEP__Schema__g1__is_all;
	/** This is a combinatorial statement to help write highly readable code. */
	static const bool is_plain_old_element
	//Should is_group be included? Can a group be a type-in-itself in other words?
	= !(is_group||is_abstract||allows_any||allows_any_attribute||is_all);
};
/**EXPERIMENTAL, TEMPLATE-SPECIALIZATION
 * @c xs::any (@c xsAny) requires this.
 */ 
template<> class Schematic<daeElement>
{ 
COLLADA_(public)

	/** The generated class, accessed via pointers. */
	typedef daeElement type;
	/** This means this is a placeholder for one or more concrete types. */
	static const bool is_abstract = true;	
};
/** Don't know if this is best, but it's simpler for now. */
template<class T> class Schematic<const T> : Schematic<T>{};

template<class SchemaType=class Undefined, int NoteID=-1> 
/**
 * @class COLLADA::DAEP::Note
 *
 * DAEP NOTE is used to bind compile-time-constant data
 * to the elements' children and attribute data members.
 * It's evolved out of a need to notify the database of
 * changes, but now it is a vector for arbitrary tacked
 * on values, like "notes" at the end of a book chapter.
 * Otherwise the values would have to appear within the
 * the DAEP Child/Value template parameters list, which
 * are already very baroque because of how C++ template
 * member-pointers demand so many additional parameters.
 * @see @c COLLADA_NOTE().
 */
struct Note
{
	/**
	 * These implement defaults that the specialization
	 * should inherit from in order to cover all of its
	 * bases. Note, @c concern can be any unusable type. 
	 */
	typedef enum{ is_fixed=0,has_default=0 }concern;
};

template<class Note=void, class Noted=Note, class Sig=signed>
/**
 * @class COLLADA::DAEP::Concern
 *
 * DAEP Concern enables or disables change-notification.
 *
 * @tparam Note must match Noted, or @c DAEP::Note will
 * find @c DAEP::Concern<>, and so the TRANSLATION-UNIT*
 * WILL BE built without notification of change support.
 * THIS CAN EASILY VIOLATE C++'s "ONE DEFINITION RULE."*
 * @tparam Noted See @a Note Doxygentation.
 * @tparam Sig exists for two reasons. DAEP Note always 
 * uses @c signed (or technically int.) Specializations
 * that are @c signed so assume primacy. Specialization
 * ceases once signed. Specialize based on @c Concern<> 
 * to disable/not build some TU with notification logic.
 * Alternatively @a Sig can be: typename Note::signed_x.
 * In this example, if "x" is not the @a Note namespace
 * the specialization isn't instantiated/does not apply.
 *
 * A "Note" has the following @c typedef members:
 * 1) schema. This is a generated class. Its name isn't
 * specified.
 * 2) context. This is the note's context. It is a name
 * of an element tag, and not a type of @c DAEP Element.
 * 3) signed_x. Here x is a placeholder for the name of
 * the namespace. The namespace is not determined until
 * the header files are @c #include. The C preprocessor
 * is the mechanism by which the TU owner controls this.
 *
 * @remarks This system is not an exact science. A note
 * is a C++ identifier, and there can be overlap. There
 * is not a distinction between attributes, content, or
 * elements. The system is designed to disable run-time
 * checks in most cases. False positives resulting in a
 * pointless check now and then are completely harmless.
 */
struct Concern{ typedef void VOID; };

template<class T, int N>
/**PARTIAL-TEMPLATE-SPECIALIZATION 
 * In the beginning the plan was to pass the "note" type 
 * directly to DAEP Concern. Now there is DAEP Note, and
 * @c DAEP::Note::concern is to be used for this purpose.
 * This is both a courtesy and a way to silently prevent
 * errors where there is a misunderstanding or accidents.
 */
struct Concern<DAEP::Note<T,N>>
:
public Concern<typename DAEP::Note<T,N>::concern>{};

template<class Note, class Unsigned>
/**PARTIAL-TEMPLATE-SPECIALIZATION 
 * This enables change-notices for attributes named "id."
 * (and techincally elements named "id" as well.) 
 */
struct Concern<Note,typename Note::context::id,Unsigned>{};

template<class Note, class Unsigned>
/**PARTIAL-TEMPLATE-SPECIALIZATION 
 * This enables change-notices for attributes named "sid."
 * (and techincally elements named "sid" as well.) 
 * @note This is for COLLADA's Scoped Identifiers (SIDs.)
 * @todo @a Note::schema::COLLADA can be used to rule out 
 * non-COLLADA schema? No big deal though.
 */
struct Concern<Note,typename Note::context::sid,Unsigned>{};

template<class Base, class Note, class Noted=Note, class Sig=signed>
/**
 * @class COLLADA::DAEP::Cognate
 *
 * DAEP Cognate converts lexical schema types into C++ @c typename.
 * @note If a type doesn't have a base, DAEP Cognate doesn't apply.
 * In that case the type must be defined before #including headers.
 * Possibly all simple types are based on xs:anySimpleType? A list
 * doesn't have a base. It will be a @c daeArray under the current
 * regime.
 *
 * DAEP Cognate builds on the principals of DAEP Concern; although,
 * -not to do with change-notices, but to discover the C++ type of
 * a local schema's simple-type (in XML Schema terms) given a base
 * type. Often the base type is sufficient, but a @c typedef isn't
 * a real C++ type that can be used to specialize a @c DAEP::Value
 * template; nor can a user/client type replace a type with a base
 * because the @c typedef to the base would otherwise collide with
 * the name of the user/client type.
 *
 * @see the daeDomTypes.h header file for examples of DAEP Cognate.
 */
struct Cognate{ COLLADA_(public) typedef Base type; };

template<class itemType, class Sig=signed>
/**
 * @class COLLADA::DAEP::Container
 *
 * DAEP Container takes a previously established DAEP Cognate type
 * and converts it into an <xs:list itemType> type. Historically a
 * @c daeString array used @c daeStringRef instead, when housed in
 * a @c daeArray. If you think about it, it makes sense, since the 
 * items require some unit of storage, whereas the external string
 * can live outside of the arrays as temporary objects.
 * (This practice runs some risk of accessing string-refs that are
 * no longer resident in memory, but is generally better for all.)
 *
 * @note The @c type @c typedef should be a @c daeArray, or it has
 * to be with the current generator, as <xs:restriction> types are
 * assumed to be based on @c daeArray.
 *
 * @remarks The purpose/future of DAEP Container here is left open.
 * In truth, it's designed to capture a storage-class, but for now
 * it needs to represent an XML Schema list first-and-foremost. To
 * capture the storage type, @c type needs to define @c value_type.
 * At some point DAEP InnerValue may extract the storage type this
 * way. For now it is necessary to specialize templates separately.
 * (This is arguably useful since strings become tokens in arrays.)
 * A secondary need is manipulating the @c size_on_stack parameter.
 */
struct Container{ typedef daeArray<itemType> type; };

template<class Unsigned>
/**PARTIAL-TEMPLATE-SPECIALIAZTION 
 * @note Using @c daeTokenRef, is not strictly equivalent to the
 * @c DAEP::Value specialization.
 * @see unspecialized @c DAEP::Itemize Doxygentation history notes.
 */
struct Container<daeString,Unsigned>{ typedef daeArray<daeTokenRef> type; };

/** 
 * @enum COLLADA::DAEP::ELEMENT
 *
 * @c elem->name(ELEMENT) expressly selects the DAEP Child "name." 
 */
enum ELEMENT{ ELEMENT=1 };	
/**
 * @enum COLLADA::DAEP::CONTENT
 *
 * This is to be limited to "contents" and "value" only. 
 * These are special members that are mutually exclusive.
 * (XSD "Complex-Types" use "contents;" "Simple-Types" use "value.") 
 */
enum CONTENT{ CONTENT=2 };
/**
 * @enum COLLADA::DAEP::ATTRIBUTE
 *
 * @c elem->name(ATTRIBUTE) expressly selects the DAEP Value "name." 
 * @see @c DAEP::CONTENT if "name" is "value" and not an attribute.
 */
enum ATTRIBUTE{ ATTRIBUTE=4 }; 

/**
 * @class COLLADA::DAEP::Nil
 *
 * DAEP Nil is an empty base class, that can be used when required.
 * Its members all serve to satisfy templates that expect elements.
 *
 * @remarks @c domAny could've been the dummy element, except that
 * compilers (and perhaps C++) want to instantiate @c domAny::_ in
 * order to evaluate the templates' default parameters; before the
 * templates themselves are instatiated; at which point, @c domAny
 * is not yet a "complete" type, as it depends on this header file.
 * STILL IT'S USEFUL to have a minimal example of what is expected
 * of procedural class generators (in DAEP Nil.)
 */
class Nil : public DAEP::Schema<>
{
COLLADA_(public) Nil(){} enum notestart{ _No };

	typedef char Nil::*_; typedef Nil __COLLADA__T;
};

template<class Note, class Type=void>
/**EXPERIMENTAL
 * Tells if there is NOT a specialization of DAEP Concern.
 */
struct NoConcern
{
	typedef char Yes; typedef long No;		
	template<class Note>
	static Yes Exists(...);
	template<class Note>
	static No Exists(typename DAEP::Concern<Note>::VOID*);
	enum{ value=sizeof(No)==sizeof(Exists<Note>(nullptr)) };
	typedef typename daeTypic<value,Type,const Type>::type type;
};

template<int,class,class CC,typename CC::_> class Child;
template<int,class,class CC,typename CC::_> class Value;
template<int ID, class T, class CC, typename CC::_ PtoM, class EBO=dae_Array<T,ID>>
/**
 * @class COLLADA::DAEP::InnerChild
 *
 * This class is an attempt to spare implementors of @c DAEP::Child
 * the torment and room for error in implementing their Child class.
 */
class InnerChild : public EBO
{	
COLLADA_(public) //MAYBE PORTABLE
	
	typedef DAEP::Child<ID,T,CC,PtoM> type;	

	friend class type;

	static const int name = ID;		
	static const bool is_element = true;
	static const bool is_content = false;			
	typedef DAEP::Schematic<T> schematic, XSD;
	static const bool is_child_nonplural = ID<0;
	static const bool is_children_plural = ID>0;
	typedef DAEP::Note<typename CC::notestart,CC::_No-ID+1> note;
													
COLLADA_(public) //PUBLIC UTILITIES		
	/** 
	 * Gets @c this child's object. Same as DAEP Value.
	 */
	inline CC &object(){ return daeOpaque(this)[-offset()]; }
	/**CONST-FORM
	 * Gets @c this child's object. Same as DAEP Value. 
	 */
	inline const CC &object()const{ return daeOpaque(this)[-offset()]; }

	/**
	 * Gets @c this child's offset. Same as DAEP Value.
	 */
	inline daeOffset offset()const{ return daeOffsetOf(CC::__COLLADA__T,*PtoM); }

	/**
	 * Extracts the model-feature identifier.
	 * @remarks There is a hole around 0 ever since the "unnamed" array took
	 * the place of the cursor. It could be filled by subtracting 1 when the
	 * ID is not negative, but it's not really worth complicating matters as
	 * there's already a hole where the nonplural bitfield is concerned, and
	 * the contents-array's feature ID would have to be adjusted as well, to
	 * justify filling the hole, since it's used as the total features count.
	 */
	inline daeFeatureID feature()const{ return daeFeatureID(-CC::_No-2+ID); }

COLLADA_(public) //GENERIC PROGRAMMING GUARANTEES	
	/** 
	 * Guaranteed ELEMENT selector. 
	 */
	inline type &operator()(enum DAEP::ELEMENT){ return (type&)*this; } 
	/**CONST-FORM
	 * Guaranteed ELEMENT selector. 
	 */
	inline const type &operator()(enum DAEP::ELEMENT)const{ return (type&)*this; } 
};

template<class NC> //Non-class
/**
 * @class COLLADA::DAEP::Class
 *
 * DAEP Class wraps a nonclass so DAEP InnerValue can be based on it.
 *
 * @note The RT package is using this in order to turn some C arrays 
 * into something that work inside of @c std::vector like containers.
 */
class Class
{ 	
COLLADA_(public)	
	/**
	 * Indicates @c this wraps "plain-old-data."
	 */
	typedef NC __COLLADA__POD;

	NC wrapped_value; 
	Class():wrapped_value(){}
	Class(const NC &v):wrapped_value(v){}		
	template<class Type> Class(const DAEP::Proto<Type>&)
	:wrapped_value(wrapped_value){ /*NOP*/ }
	NC &operator=(const NC &v){ return wrapped_value = v; }
	/** 
	 * This removes const-ness because A) it's not required by
	 * DAEP Value, and B) for C arrays it creates an ambiguity
	 * for Visual Studio 2010. It could be something about C++.
	 */
	operator NC&()const{ return const_cast<NC&>(wrapped_value); }
	//Seems alright to remove so RT::Matrix can be DAEP::Class.
	/**
	 * This exists so to convert the pointer to the type of NC.
	 */
	//NC *operator&()const{ return const_cast<NC*>(&wrapped_value); }		
};
template<class NC> //Non-class
/**PARTIAL-TEMPLATE SPECIALIZAITION
 * This is just a potentially bad way to extract @c wrapped_value if
 * a class is DAEP Class or is not.
 */
class Class<Class<NC>> : public Class<NC>{};

template<class T>
/**
 * This is used by @c DAEP::InnerValue::operator->*() to not convert
 * strings to @c daeStringRef types, because doing so would demand a
 * trip to the database's small string-table, or would result in the
 * system's string-table being used, which is a considerably greater
 * peril.
 */
class Default
{
	typedef const T type; 
};
template<class T, int N>
/**PARTIAL-TEMPLATE SPECIALIZATION 
 * Assuming this is a string-literal.
 */
class Default<T[N]>
{
	typedef const T *const type; 
};
template<class T, int N>
/**PARTIAL-TEMPLATE SPECIALIZATION 
 * Arrays should return a reference always--or fail. This also makes
 * the "size on stack" argument decay so that it is not a hinderance.
 * @todo Add something to cover @c daeBinary.
 */
class Default<daeArray<T,N>>
{
	typedef const daeArray<T> &type;
};
template<int ID, class T, class CC, typename CC::_ PtoM>
/**PARTIAL-TEMPLATE SPECIALIZATION 
 * If both arguments are DAEP Value then the underlying types should
 * match.
 */
class Default<DAEP::Value<ID,T,CC,PtoM>>
{	
	typedef const typename DAEP::Value<ID,T,CC,PtoM>::underlying_type &type;
};

template<int ID, class T, class CC, typename CC::_ PtoM, class EBO=
typename daeTypic<daeArrayAware<T>::is_class,T,DAEP::Class<T>>::type>
/**
 * @class COLLADA::DAEP::InnerValue
 *
 * This class is an attempt to spare implementors of @c DAEP::Value
 * the torment and room for error in implementing their Value class.
 */
class InnerValue
{
	/**
	 * This started out as a @c private base clase, but
	 * conversion operators against base classes' types
	 * had no influence.
	 */
	EBO value;

COLLADA_(public)	
	/** 
	 * This should be the equivalent Value. 
	 */
	typedef DAEP::Value<ID,T,CC,PtoM> type;	
	
	friend class type;
			
COLLADA_(public) //CONSTRUCTOR
	/**
	 * Default Constructor
	 *
	 * At this stage the object's memory footprint has 
	 * been initialized via a prototype (i.e. memcpy.)
	 * @c EBO may need to grab @c object() or if it is
	 * a container, it may need to replicate itself on
	 * top of itself. Admittedly this can get a little
	 * ugly; but this way the generated classes can do
	 * without explicitly defined constructors. It may
	 * or may not be "ultra efficient," but that's not
	 * especially important.
	 * @see @c prototype() Doxygentation.
	 *
	 * @note Containers that only copy a default value
	 * can avoid instantiating code in the constructor
	 * by conditioning on @c is_fixed & @c has_default.
	 */
	InnerValue():COLLADA_SUPPRESS_C(4355)
	value((const DAEP::Proto<type>&)*this)
	{	
		//Elements are not allowed to have subobjects
		//because the schema is in control of layouts.
		//Also, metadata registration uses the second
		//form of daeModel::addFeature().
		daeCTC<0==daeModel::__subobject_flag<EBO>::value>(); 
	}

COLLADA_(public) //MAYBE PORTABLE
	
	static const int name = ID;			
	static const bool is_element = false;	
	static const bool is_content = ID>=CC::__DAEP__Schema__extent_of_attributes;	
	//wrapped_type is only to specialize on DAEP::Class. It's needed right now 
	//to pick up the ColladaDOM 3 style enum datatypes. They are class wrapped.
	typedef EBO wrapped_type;
	typedef T advertised_type; typedef typename DAEP::Class<EBO>::__COLLADA__POD underlying_type;
	typedef DAEP::Note<typename CC::notestart,ID> note;
	typedef typename daeTypic<is_content,enum DAEP::CONTENT,enum DAEP::ATTRIBUTE>::type selector;	

COLLADA_(public) //PUBLIC UTILITIES
	/** 
	 * Gets @c this value's object. Same as DAEP Child.
	 */
	inline CC &object(){ return daeOpaque(this)[-offset()]; }
	/**CONST-FORM
	 * Gets @c this value's object. Same as DAEP Child. 
	 */
	inline const CC &object()const{ return daeOpaque(this)[-offset()]; }

	/**
	 * Gets @c this value's offset. Same as DAEP Child.
	 */
	inline daeOffset offset()const{ return daeOffsetOf(CC::__COLLADA__T,*PtoM); }

	/**
	 * Gets the model-feature identifier. 
	 */
	inline daeFeatureID feature()const{ return daeFeatureID(-ID-1); }

COLLADA_(public) //PROTOTYPE CONSTRUCTOR SUPPORT

  ////////////////////////////////////////////////////////////////////////////////////////
  //Note, this is academic. No classes depend on prototype(). User/client classes could.//
  ////////////////////////////////////////////////////////////////////////////////////////

	/**NOT-RECOMMENDED	 
	 * Technically this is here for when @a EBO cannot
	 * implement the "Prototype Constructor" self-copy logic. 
	 * This enables arbitrary user-defined data-types. Rather than
	 * self-copy, a type that must copy, can instead copy @c prototype().
	 */
	inline const EBO &prototype()const
	{
		return model()[feature()].getAllocThunk()->value; 
	}
	/**
	 * @todo If "__DAEP__Element__meta" is a local-copy of the model pointer, then:
	 * "return (DAEP::Model&)daeOpaque(object().__DAEP__Element__meta)[-sizeof(CC)];"
	 */
	inline DAEP::Model &model()const{ return object().__DAEP__Object__v1__model(); }

COLLADA_(public) //GENERIC PROGRAMMING GUARANTEES
	/**
	 * Select one of: ATTRIBUTE or CONTENT. 
	 */
	inline type &operator()(selector){ return (type&)*this; } 
	/**CONST-FORM
	 * Select one of: ATTRIBUTE or CONTENT. 
	 */
	inline const type &operator()(selector)const{ return (type&)*this; } 

COLLADA_(public) //CHANGE-NOTICE GUARANTEES	
	
	template<class,class> friend class InnerChange;	

	template<class S>
	/** It seems these must all be separate. */
	inline operator S()const{ return __to<S>(nullptr); } 	
	/** Implements @a S conversion operator. */
	template<class S> S __to(typename S::__COLLADA__Object*)const
	{
		//If this fails, user code is probably using a wrong type.
		//NOTE: This designed to convert daeURI, daeIDREF, and daeSIDREF.
		return S(value,(typename S::__COLLADA__Object*)&object());
	}
	/** Implements @a S conversion operator. C4927 wants explicity. */
	template<class S> S __to(...)const{ return S((const underlying_type&)value); }

	/**CONST-FORM
	 * This brings along with it non-assignment operators.
	 */
	inline operator const underlying_type&()const{ return value; } 		
	/**
	 * This brings along with it non-assignment operators.
	 */
	inline operator typename DAEP::NoConcern<note,underlying_type>::type&(){ return value; } 		

	/**
	 * @return Returns a @c const pointer to @c this. Its type is of @c value.
	 * @note Using -> is not consistent with pointer semantics. The usage has
	 * more to do with accessing methods and members the inner class may have.
	 */
	inline const underlying_type *operator->()const
	{
		return (underlying_type*)&value; //Removed operator &from DAEP::Class.
	}
	/**
	 * @return Returns a @c const pointer if this value should not be changed
	 * without notifice.
	 * @note Using -> is not consistent with pointer semantics. The usage has
	 * more to do with accessing methods and members the inner class may have.
	 */
	inline typename DAEP::NoConcern<note,underlying_type>::type *operator->()
	{
		return (underlying_type*)&value; //Removed operator &from DAEP::Class.
	}

	template<class T>
	/**
	 * Select between two DAEP Value given that the first is @c nullptr based.
	 * Or:
	 * Provide a default end to a chain @c const @c DAEP::Child::operator->().
	 *
	 * @todo A version that permits two DAEP Value arguments to be @c nullptr.
	 *
	 * @note This was created initially to simulate the "ternary operator" in
	 * the pickle that it cannot find a common type between two DAEP Value as
	 * they are templated wrappers, which confuses type deduction (Though the
	 * C++ standard is a little vague on if compilers should look for matched
	 * conversion to reference operators or not--MSCVC 2015 doesn't do that.)
	 * IT'S VALUE to extract a DAEP Value from potentially @c nullptr objects
	 * is a more common scenario, which should make the operator not uncommon.
	 */
	inline typename DAEP::Default<T>::type operator->*(const T &other)const
	{
		if(&object()==nullptr) return other; return value;
	}

	/**
	 * This is the type of __COLLADA__Atom. 
	 */
	typedef typename daeAtomOf<EBO>::type atom, __COLLADA__Atom;	

	/**
	 * This is for @c daeHashString. It's a common API, but isn't implemented
	 * unless @c underlying_type implements @c size(). Not for the time being.
	 */
	size_t size()const{ return value.size(); }
	/**
	 * This is for @c daeHashString. It's a common API, but isn't implemented
	 * unless @c underlying_type implements @c data(). Not for the time being.
	 */
	const atom *data()const{ return value.data(); }

	template<class I>
	/**CONST-FORM
	 * Gets @c underlying_type::operator[] or equivalent.
	 */
	inline const atom &operator[](const I &i)const
	{
		return value[i]; 
	} 
	template<class I>
	/**
	 * Gets @c underlying_type::operator[] or equivalent.
	 *
	 * @return Returns a @c const C++ reference if @c this DAEP Value
	 * must issues a notification-of-change. Item level notifications
	 * were implemented at first. It seemed like in hindsight that it
	 * was prone to misunderstandings that a loop would generate more
	 * than one notice; that it could be pathalogical on the database
	 * end; and a lot of duplicate code to maintain here, which would 
	 * just be more for users to learn for very little practical gain.
	 */
	inline typename DAEP::NoConcern<note,atom>::type &operator[](const I &i)
	{
		return value[i]; 
	} 

	template<class S> 
	/**
	 * All assignment operators are supported, defined outside of the 
	 * template, because that produces significantly smaller MSVC2015
	 * precompiled-header intermediary files. (This is a real problem
	 * that effects all versions of Visual Studio.)
	 * @note The difference was discovered while == and != were being
	 * added, in order to support the ColladaDOM 3 style scoped enums.
	 * @see @c COLLADA_DOM_GLOBAL_ASSIGNMENT_OPERATORS_NOT_INCLUDED &
	 * ColladaDOM.inl -=,+=,/=,%=,*=,<<=,>>=,&=,|=,^=,++,-- operators.
	 */
	inline type &operator=(const S &rvalue)\
	{
		struct _{ static void f(underlying_type &lv,const S &rv){ lv = rv; } };
		DAEP::InnerChange<type,S> cn(*(type*)this,_::f,rvalue); return *(type*)this;
	}
};

template<int ID, class T, class CC=DAEP::Nil, typename CC::_ PtoM=COLLADA_(nullptr)>
/**
 * @class COLLADA::DAEP::Child
 *
 * A DAEP Child implements a common name given to
 * the children of DAEP Element. Unlike InnerChild
 * DAEP Child is a base for template-specialization.
 *
 * SPECIALIZATION
 * ==============
 * Specializations of @c Child should inherit from
 * @c DAEP::InnerChild.
 *
 * @tparam PtoM is a pointer-to-member for this name.
 * @tparam CC is functionally identical to the parent.
 * @tparam T is a generated Elemental-based class name.
 * @tparam ID is assigned to children elements that bear
 * the same name. Names borne by more than one child have
 * lower IDs, because they are assigned physical addresses.
 */
class Child : public DAEP::InnerChild<ID,T,CC,PtoM>
{
COLLADA_(public) //using InnerChild::operator=; //-C2679-C2622

	/**C-PREPROCESSOR MACRO, C++98/03 SUPPORT
	 * This macro is used by @c DAEP::Child to work around the C++03
	 * restriction on @c operator= for members of unions. If clients 
	 * specialize @c DAEP::Child they will require this macro if the
	 * specialization is to be used by a C++98 compiler.
	 * @note "using InnerChild::operator=;" should do it after C++11.
	 */
	#define COLLADA__DAEP__Child__union__assignment_operator(T,ID) \
	template<class S>\
	/**\
	 * Assignment of smart-refs, arrays, and pointers.\
	 */\
	inline dae_Array<T,ID> &operator=(const S &cp)\
	{\
		this->__assign<S>(cp,nullptr); return *this;\
	}\
	/**\
	 * Removes this element from the contents-array by means\
	 * of converting it to an empty text-node. This will not\
	 * disrupt the array, and so is recommended. The removed\
	 * nodes can be cleaned out en masse as a maintenance op.\
	 */\
	inline daeString1 operator=(daeString1 empty_string_literal)\
	{\
		return this->operator[](0) = empty_string_literal;\
	}
	COLLADA__DAEP__Child__union__assignment_operator(T,ID)
};

template<int ID, class T, class CC=DAEP::Nil, typename CC::_ PtoM=COLLADA_(nullptr)>
/**
 * @class COLLADA::DAEP::Value
 *
 * A DAEP Value implements an attribute or simple
 * type value of a DAEP Element. Unlike InnerValue
 * DAEP Value is a base for template-specialization.
 *
 * SPECIALIZATION
 * ==============
 * Specializations of @c Value should inherit from
 * @c DAEP::InnerValue.
 *
 * @tparam PtoM is a pointer-to-member for the value.
 * @tparam CC is functionally identical to the parent.
 * @tparam T is the value's type before specialization.
 * @tparam ID has been assigned to this attribute/value. 
 */
class Value : public DAEP::InnerValue<ID,T,CC,PtoM>
{
COLLADA_(public) using InnerValue::operator=; //C2679
};

template<int ID, class CC, typename CC::_ PtoM>
/**TEMPLATE-SPECIALIZATION
 * Historically the library has exposed @c daeStringRef as @c daeString.
 * It might be more accommodating to specialize DAEP InnerValue instead,
 * -but it's hard to think of other uses for a plain C-string than this.
 */
class Value<ID,daeString,CC,PtoM> : public DAEP::InnerValue<ID,daeString,CC,PtoM,daeStringRef>
{
COLLADA_(public) using InnerValue::operator=; //C2679
};

/**
 * @class COLLADA::DAEP::Change
 *
 * When a change-notice is sent to a database it arrives
 * via an object of this type. The database can use this
 * window to examine the DAEP Object prior to the change.
 * Calling @c carry_out_change() should apply the change.
 * @see @c DAEP::Concern
 * @see @c DAEP::InnerChange
 * @see @c daeNoteChange()
 *
 * @remarks Presently changes are managed internally and
 * are triggered by C++'s assignment operators on values
 * whitelisted with @c DAEP::Concern.
 */
class Change
{
COLLADA_(public) 

	union
	{
	mutable daeObject *_object;
	/**
	 * This is the element that's changing.
	 */
	const DAEP::Element *element_of_change;	
	};

	/**ENUM
	 * One of:
	 * @c DAEP::ELEMENT:
	 * The element is moving.
	 * (A new element is created.)
	 * @c DAEP::CONTENT: 
	 * The value or mixed content is changing.
	 * @c DAEP::ATTRIBUTE:
	 * The attribute is changing.
	 * @see @c daeDB::_v1_note() via @c daeNoteChange().
	 */
	int kind_of_change;

	/**
	 * Non-Constructor
	 */
	Change(){ /*NOP*/ }
	/**
	 * Constructor
	 */
	explicit Change(int kof):kind_of_change(kof){}	
	/**
	 * Constructor
	 */
	Change(daeElement *e, int kof):_object((daeObject*)e),kind_of_change(kof){}	
	/**
	 * Constructor
	 */
	Change(DAEP::Element *e, int kof):element_of_change(e),kind_of_change(kof){}	
	/**
	 * Virtual Destructor
	 * This doesn't have to be virtual really, but
	 * it's no big deal, and C++11 forces it to be.
	 */
	virtual ~Change(){}

	/**
	 * This must be called to carry out the change.
	 * It's a mechanism for getting a before/after.
	 */
	virtual void carry_out_change()const{ assert(0); }
};

template<class S, class U>
/**
 * Formerly "DAEP::Notice."
 *
 * A InnerChange dispatches change-notices. It's mainly
 * an extension of DAEP Value. It should be ignored for
 * the most part. 
 *
 * First a compile-time check is done, so a translation 
 * unit can decide for itself to suppress notifications.
 * This is done by specializing DAEP Concern's template.
 */
class InnerChange : public DAEP::Change
{
COLLADA_(private) 
	/**SCHEDULED FOR REMOVAL, C++98 SUPPORT
	 * C++98 will not locally define template arguments.
	 */
	typedef void (*Operator)(typename S::underlying_type&,const U&);

	S &lvalue; Operator op; const U &rvalue; mutable bool b;

	template<class>
	/** Dispatches change-notice. */
	void _issue_change_if(...)
	{
		_object = (daeObject*)&lvalue.object();
		kind_of_change = S::is_content?DAEP::CONTENT:DAEP::ATTRIBUTE;
		daeMeta &m = dae(lvalue.object())->getMeta();
		daeAttribute &v = S::is_content?*m.getValue():m.getAttributes()[S::name];
		daeNoteChange(*this,v);		
	}
	template<class SFINAE>
	/** Suppresses change-notice. */
	void _issue_change_if(typename DAEP::Concern<SFINAE>::VOID*){}	
	
COLLADA_(public) 

	InnerChange(S &lv, Operator op, const U &rv)
	:lvalue(lv),op(op),rvalue(rv),b()
	{
		_issue_change_if<typename S::note::concern>(nullptr);
		if(!b) op(lvalue.value,rvalue);
	}

	virtual void carry_out_change()const{ op(lvalue.value,rvalue); b = true; }
};

/**
 * @class COLLADA::DAEP::Make
 *
 * The DAEP Make contains information that is common 
 * to all DAEP Models, such that all DAEP Objects can
 * be said to have a "Make & Model."
 */
class Make
{
COLLADA_(protected) //DATA-MEMBERS		
	/**
	 * This string should describe the individual or organization
	 * responsible for the executable model originating this Make.
	 */
	daeClientString __DAEP__Make__maker;

COLLADA_(public) //VIRTUAL METHOD TABLE
	/**
	 * Virtual Destructor
	 */
	virtual ~Make(){}
	/**
	 * Default Constructor
	 */
	Make():__DAEP__Make__maker(COLLADA_DOM_MAKER){}

	/**SEALED INTERFACE
	 * This is used to set up a DAEP Model. The models are
	 * then deleted by @c daeProcessShare::~daeProcessShare().
	 */
	virtual void *__DAEP__Make__v1__operator_new(size_t chars)const
	{
		return operator new(chars);
	}
	
	/**SEALED INTERFACE
	 * Delete @a obj from its orginating translation-unit.
	 * This is not supposed to happen at a high frequency.
	 *
	 * Perhaps there could be something more flexible here,
	 * but it's recommended to use @c new/delete for objects
	 * that do not belong to a database.
	 */
	virtual void __DAEP__Make__v1__delete(const DAEP::Object *obj)const
	{
		delete obj;
	}

COLLADA_(public) //OPERATORS

	/** Implicitly convert to "daeX" equivalents. */
	inline operator const XS::Schema&()const{ return (XS::Schema&)*this; }
	/** Implicitly convert to "daeX" equivalents. */
	inline operator daeProcessShare&()const{ return (daeProcessShare&)*this; }	

COLLADA_(private) //INHERITED NON-CONSTRUCTORS
	/**
	 * Disabled Copy Constructor & Assignment Operator
	 *
	 * As @c private, derived classes will error if copy-constructed.
	 */
	Make(const Make&); void operator=(const Make&);	
};

/**
 * @class COLLADA::DAEP::Model
 *
 * The DAEP Model class serves as a blueprint for a 
 * corresponding DAEP Object. Every Object has a Model,
 * and every Model has a DAEP Make/Maker.
 *
 * A @c Model* doesn't point at the beginning of the Model
 * object's memory. There is more of the data-structure that
 * begins at a lower, or adjusted address.
 *
 * This is to facilitate two kinds of direct lookup into
 * the table of information pertaining to the data-members
 * that makeup the Objects described by the Model.
 *
 * MEMORY LAYOUT
 * =============
 * The memory layout from the top of the Model down, matches
 * exactly the byte-by-byte layout of Objects it describes.
 * Its Objects all have the same invariant size and structure,
 * however, they are permitted to have a variable-length array
 * off the end of them, or even data that the object's reference
 * elsewhere, embedded within the objects. However, the Models
 * do not describe these. 
 *
 * The first few data-members of Model are overlaid precisely
 * with those of DAEP Object that are held in common. These 
 * parts of Objects are not considered interesting, and so are 
 * not described by the Model, and so can be repurposed. (There
 * is also likely a "vptr" to the virtual-method-table there, but
 * it is not considered, and it isn't necessary for the first few
 * members to overlap, they just cannot be greater than--that is
 * they cannot be greater than, on any system, ever, in theory.)
 *
 * The size of the Objects are @c __DAEP__Model__meta-this.
 * The bytes between @c this, and this member are a lookup table,
 * that has in each byte that is the start of a data-member of
 * the Objects, a member-ID. (There can be up-to 254 members, or
 * the IDs could also be 16-bits, with some restrictions placed 
 * on byte-sized members.) Technically "bytes" are @c char sized.
 *
 * These IDs can then be subtracted from @c (void*)this, to get
 * at object-member-descriptor pointers. These point back into
 * the body of the Model's layout. This layout is similiar to a
 * file of a given format, loaded into memory.
 */
class Model
{
COLLADA_(public) //DATA-MEMBERS

  /////////////////////////////////////////////////
  //There should be room for one more data-member//
  //Since DAEP Object's "vptr" must take up space//
  /////////////////////////////////////////////////

	/**
	 * The @c daeObjectType class's typeless enum.
	 *
	 * Here "genus" ties into COLLADA_DOM_GENERATION.
	 *
	 * @note The size must match @c __DAEP__Object__refs.
	 */
	const daeInt __DAEP__Model__genus;
	/**UNIMPLEMENTED
	 * If two models have the same "family", they 
	 * can be said to be binary and namespace compatible.
	 *
	 * Here "family" is one taxonomy above "genus."
	 * 
	 * @note The size must match @c __DAEP__Object__tags.
	 */
	const daeInt __DAEP__Model__family;

	union //COLLADA__VIZDEBUG
	{
	/**
	 * This pointer is most prominent when viewing this
	 * data-structure in the debugger's visualizaer.
	 * 
	 * @note The size must match @c __DAEP__Object__parent.
	 */
	const Make *const *const __DAEP__Model__make; 
	const daeMetaObject *const __DAEP__Model__meta;
	};
	daeMeta &__DAEP__Model__elements_meta()const
	{
		assert(__DAEP__Model__genus>=1); return (daeMetaElement&)*__DAEP__Model__meta;
	}

	/** Extracts the static "sizeof" the modeled object. */
	size_t __DAEP__Model__object_sizeof()const
	{
		size_t so = (char*)__DAEP__Model__meta-(char*)this;
		//maybe not the best place to do this
		assert(so>=sizeof(DAEP::Object)); return so;
	}

  ////////////////////////////////////////////////////////////////
  //Within this space there is a field of chars/shorts that ends//
  //where __DAEP__Model__meta begins. They hold feature IDs with//
  //which to extract the feature with only a data member address//
  //That's straightforward to do, if a feature ID is not on hand//
  //It's inadvisable to hand-code IDs if inheritance is involved//
  //Negative shorts are used when the features are not char-size//
  //Char-size features are limited to IDs ranging 1 to UCHAR_MAX//
  ////////////////////////////////////////////////////////////////
	
	template<class T>
	/** Extracts RTTI about a model feature. */
	inline const daeFeature &__DAEP__Model__feature(const DAEP::Object *object, const T *member)const
	{
		return operator[](__DAEP__Model__identify_feature(object,member)); 
	}
	template<class T>
	/** Extracts a model feature identifier. */
	inline daeFeatureID __DAEP__Model__identify_feature(const DAEP::Object *object, const T *member)const
	{
		//TODO: Should this fail: use a bitfield based class in place of (short&) down below.
		//This might fail for char-based objects. It's probably not possible for basic types.
		daeCTC<(sizeof(T)==1||sizeof(short)<=sizeof(T))>();
		daeOffset offset = (char*)member-(char*)object; 
		assert(offset<(daeOffset)__DAEP__Model__object_sizeof());
		int slot; if(1==sizeof(T))
		{
			//If there are char-sized members they must come before the UCHAR_MAX'th address.
			slot = -(int)((unsigned char*)this)[offset];
		}
		else slot = (short&)((char*)this)[offset]; assert(0!=slot); return (daeFeatureID)slot;	
	}	

COLLADA_(public) //OPERATORS

	/** Extracts RTTI about a model feature. */
	inline const daeFeature &operator[](daeFeatureID fID)const
	{	
		assert((int)fID<0&&fID>=__DAEP__Model__meta->rbeginFeatureID());
		return ((daeFeature*)this)[(int)fID];
	}

	/** Implicitly convert to "daeX" equivalents. */
	inline operator const daeModel&()const{ return (daeModel&)*this; }
	/** Implicitly convert to "daeX" equivalents. */
	inline operator daeMeta&()const{ return __DAEP__Model__elements_meta(); }

	/** @return Returns @c true if @a cmp is @c this model. */
	inline bool operator==(DAEP::Model &cmp)const{ return this==&cmp; }
	/** @return Returns @c false if @a cmp is @c this model. */
	inline bool operator!=(DAEP::Model &cmp)const{ return this!=&cmp; }
					 
COLLADA_(private) //INHERITED NON-CONSTRUCTORS
	/**
	 * Disabled Copy Constructor & Assignment Operator
	 *
	 * As @c private, derived classes will error if copy-constructed.
	 */
	void operator=(const Model&); Model(const Model&);
};

//-------.
	}//<-'
}

#endif //__COLLADA_DOM__DAEP_H__
/*C1071*/