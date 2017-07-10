/*
 * Copyright 2006 Sony Computer Entertainment Inc.
 *
 * Licensed under the MIT Open Source License, for details please see license.txt or the website
 * http://www.opensource.org/licenses/mit-license.php
 *
 */
#ifndef __COLLADA_DOM__DOM_ANY_H__
#define __COLLADA_DOM__DOM_ANY_H__

#include "daeElement.h"
#include "daeDomTypes.h"
  
COLLADA_(namespace)
{//-.
//<-'

	/////////////////////////////////////////////
	//REMINDER: domAny IS COLLADA-DOM 2.x STYLE//
	/////////////////////////////////////////////

//TODO: IT WOULD BE	NICE IF THE CONTENTS-ARRAYS WERE EMBEDDABLE PER USUAL.
/**LEGACY, VARIABLE-LENGTH
 * The @c domAny class allows for weakly typed XML elements. This class is 
 * used anywhere in the COLLADA schema where <xs:any> elements appear. 
 *
 * Post-2.5 the only thing unique about @c domAny is its attributes. This is
 * more or less as it ever was. Really there just needs to be a solution for
 * <xs:anyAttribute> use cases. @c domAny is exclusive to COLLADA-DOM 2 code.
 * IOW: There shouldn't be a ColladaDOM 3 side to @c domAny. It can wait for
 * <xs:anyAttribute> support.
 * The pre-2.5 implementation relied on a @c virtual setAttribute() method. 
 * Post-2.5 waits until @c getAttribute() fails, checks if the element is a
 * @c domAny, and adds the attribute if so. The difference to this approach
 * is the attribute is added just by looking for it. This however seems more
 * correct, since a failed "get" communicates the attribute is not supported.
 * (@c setAttribute() uses @c getAttribute() to set the attribute.)
 * NOTE: This mechanism only works for @c domAny. It works by making a copy
 * of the the @c daeMeta record for every instance of @c domAny. This is how
 * it's always worked. It's not a good solution; escpecially just for to get
 * the attributes to work.
 *
 * @note The APIs removed from @c domAny were all deprecated. The notes that 
 * said as much were just to be found in the daeElement.h file instead. 2.5
 * removes all of the APIs said to be deprecated. 
 *
 * @remarks @c domAny instances do not have a proper @c daeMetaElement. Each 
 * instance creates and maintains its own. STILL, in static scenarios, there 
 * is a regular/dummy @c daeMetaElement, and so the class more or less works
 * identically. All @c domAny elements belong to the library's process-share.
 */
class domAny : public daeElemental<domAny>, public DAEP::Schema<64+32+3>
{
COLLADA_(public) //COLLADA-DOM 2
	/**
	 * These were deprecated; but are kept for switch-cases.
	 */
	enum{ elementType=daeObjectType::ANY };

COLLADA_(public) //Parameters

	typedef struct:Elemental,Schema
	{	DAEP::Value<0,xsAnySimpleType>
	_0; COLLADA_WORD_ALIGN
		COLLADA_DOM_Z(0,0)
	DAEP::Value<1,dae_Array<>> _Z; enum{ _No=1 };
	DAEP::Value<2,daeContents> content; typedef void notestart;
	}_;
						
COLLADA_(public) //Content
	/**WARNING
	 * The @c xsAnySimpleType value of the text data of this element. 
	 * @warning Post-2.5 "domAny" objects report a mixed-content-model.
	 * In which case, @c value should not rightly be. Historically this is how it
	 * has been done. It's probably not going anywhere.
	 * IT MAY BE AN EITHER/OR SITUATION AT SOME JUNCTURE.
	 */
	DAEP::Value<0,xsAnySimpleType,_,(_::_)&_::_0> value;

	COLLADA_WORD_ALIGN
	COLLADA_DOM_Z(0,0) 
	/**NO-NAMES
	 * These elements are invalid according to the schema. They may be user-defined 
	 * additions and substitutes.
	 * @note This name uses the 2.4 convention because @c domAny is a legacy object.
	 */
	DAEP::Child<1,xsAny,_,(_::_)&_::_Z> elemAny_et_cetera__unnamed;
	/**
	 * Children, mixed-text, comments & processing-instructions.
	 */
	DAEP::Value<2,daeContents,_,(_::_)&_::content> content;

COLLADA_(public) //Accessors and Mutators
	/**
	 * Gets the contents-array.
	 * @return Returns a reference to the content element array.
	 */
	daeContents &getContents(){ return content; }
	/**
	 * Gets the contents-array.
	 * @return Returns a constant reference to the content element array.
	 */
	const daeContents &getContents()const{ return content; }

	#ifdef NDEBUG
	#error Technically "domAny" is_of_mixed_type.
	#error daeStringRef? Should this be housed in content? Or typed?
	#error IT WOULD BE NICE IF THE CONTENTS-ARRAYS WERE EMBEDDABLE PER USUAL.
	#endif
	/**WARNING
	 * Gets the value of this element.
	 * @return Returns a @c xsAnySimpleType of the value.
	 * @warning Post-2.5 "domAny" objects report a mixed-content-model.
	 * In which case, @c value should not rightly be. Historically this is how it
	 * has been done. It's probably not going anywhere.
	 * IT MAY BE AN EITHER/OR SITUATION AT SOME JUNCTURE.
	 */
	xsAnySimpleType getValue()const{ return value; }
	/**WARNING
	 * Sets the value of this element.
	 * @param val The new value for this element.
	 * @warning Post-2.5 "domAny" objects report a mixed-content-model.
	 * In which case, @c value should not rightly be. Historically this is how it
	 * has been done. It's probably not going anywhere.
	 * IT MAY BE AN EITHER/OR SITUATION AT SOME JUNCTURE.
	 */
	void setValue(xsAnySimpleType val){ value = val; }
 	   
  ///////////// END OF GENERATION (1) OUTPUT /////////////
												
#ifdef BUILDING_COLLADA_DOM

	union //SHEDULED FOR REMOVAL (AND AS FOR _domAny_safe_model?)
	{
		void *_ALIGNER; char _[sizeof(daeMeta)];
		daeMetaElement *operator->(){ return (daeMetaElement*)_; }
	/** Copy of @c domAny::_master_meta with own attributes-array. */
	}_meta;

COLLADA_(public) 
		/**
		 * Implements @c domAny::_master.
		 */
		struct _Master
		{
			Elemental::TOC toc;
			daeMeta &meta; DAEP::Model &model; _Master(XS::Schema&);
		};
		/**
		 * This is an empty "meta" record, that is used to bridge
		 * the difference between regular static metadata and the 
		 * manifold @c domAny metadata. 
		 * @c XS::Any::findChild() use it for processContents="lax".
		 */
		static _Master _master;

COLLADA_(private) 

		friend daeMetaElement;
		/**
		 * Prototype Constructor
		 */
		domAny()
		{
			(daeMeta*&)__DAEP__Element__data.meta = (daeMeta*)&_meta;
		}
		/**
		 * Virtual Destructor
		 */
		virtual ~domAny();
					
		friend daeElement;
		//EXPERIMENTAL
		//This framework needs to evolve into <xs:anyAttribute> support.
		/**
		 * This needs to be persistent memory. @c std::deque
		 * would do if its minimum size wasn't measured in KBs.		 
		 * (Also deque would not be embedded.)
		 * The number of refs in the last bucket is the remainder
		 * of the attribute count divided by @c size.
		 *
		 * @note It doesn't strictly have to be persistent, although
		 * attributes probably should have permanent addresses. Really
		 * the only trouble is if the addresses change, the metadata has
		 * to be updated; and metadata addresses wouldn't normally change.
		 * (Alternatively the values could be in the reserved attribute
		 * metadata; then all addresses change with the attribute array.)		 
		 */
		struct AttributeBucket
		{
			void _self_destruct(int);

			enum{ size=16 }; daeString refs[size]; AttributeBucket *next;
		};
		/**
		 * The array of @c daeString to hold attribute data for this element.
		 * @todo These could be @c daeTypewriter/variant pairs, according to
		 * if the string looks like a number or a string.
		 */
		AttributeBucket _attribs;

#endif //BUILDING_COLLADA_DOM
	
COLLADA_(private)

	friend class DAEP::Elemental<domAny>;
	/**
	 * Reminder: This lets "math:math" default to @c domAny.
	 */
	COLLADA_DOM_LINKAGE static DAEP::Model &_master_model()
	COLLADA_DOM_SNIPPET( return _master.model; )
};

namespace DAEP //GCC refuses to disable this (erroneous) warning
{
	template<>
	/**
	 * Gets a @c domAny model for the purpose of making a new domAny/model.
	 * @remarks This was added so @c DAEP::Schematic<domAny> can be of use.
	 */
	inline DAEP::Model &DAEP::Elemental<domAny>::__DAEP__Object__v1__model()const
	{
		return domAny::_master_model();
	}
}

//---.
}//<-'

#endif //__COLLADA_DOM__DOM_ANY_H__
/*C1071*/
