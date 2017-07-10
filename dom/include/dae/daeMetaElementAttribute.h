/*
 * Copyright 2006 Sony Computer Entertainment Inc.
 *
 * Licensed under the MIT Open Source License, for details please see license.txt or the website
 * http://www.opensource.org/licenses/mit-license.php
 *
 */

#ifndef __COLLADA_DOM__DAE_META_ELEMENT_ATTRIBUTE_H__
#define __COLLADA_DOM__DAE_META_ELEMENT_ATTRIBUTE_H__

#include "daeContent.h"
#include "daePlatonic.h"
#include "daeMetaCMPolicy.h"

COLLADA_(namespace)
{
	/**INTERNAL, SEALED
	 * @c XS::Any and @c XS::Element are derived from this class. It is more-or-less
	 * a "leaf node" in the graph, associated with an actionable ordinal.
	 *
	 * @remarks Having this class simplifies padding and lets @c __place() be shared.
	 * IT SHOULDN'T BE ADDED TO. IT WILL BREAK MINOR RELEASES IF SO.
	 */
	class daeElementCM : public daeCM
	{
	COLLADA_(protected) //PADDING AND ONLY DATA-MEMBER

		daeElementCM(){ _maxOrdinals_x_maxOccurs = 1; }

		enum
		{
		__size_of_CM=12*sizeof(void*), 	
		__reserved_for_daeCM=__size_of_CM-sizeof(daeCM)
		};
		char __pad1[__reserved_for_daeCM];

		friend class daeCM;
		friend class daeMetaElement;
		/**
		 * This is the same value as _maxOccurs, unless there are substitution-groups
		 * involved, in which case, it is 0.
		 * @remarks  This isn't assigned to prior to @c daeCM::_prepareContentModel().
		 * @note SINCE maxOccurs WOULDN'T NORMALLY BE 0, IT'S PROBABLY SAFE TO ASSUME
		 * THAT A NONZERO VALUE CAN BE USED TO GENERICALLY QUERY WHETHER OR NOT THESE
		 * ELEMENTS (WITH THIS NAME) ARE SUBJECT TO SUBSTITUTIONS.	
		 * @see @c XS::Element::hasSubstitutes().
		 */
		daeCounter _substitute_maxOccurs;

		friend class XS::All;
		template<class C>
		/** 
		 * Called by @c daeCM::__placeXS(). 
		 */
		inline daeError __place(daeCM_Placement<C> &c, daeOrdinal o)const;

		/**CIRCULAR-DEPENDENCY
		 * This is to represent the pseudo-element content model.
		 */
		inline daeMeta &getOnlyChild()const;
	};

	namespace XS
	{//-.
//<-----'
		
/**
 * The COLLADA::XS::All class defines the behavior of an xs:all content-model.
 */
class All : public daeParentCM
{	
COLLADA_(public)

	COLLADA_DOM_OBJECT_OPERATORS(XS::All)

	//addCM<T>() still uses this to validate T.
	static const daeXS __daeCM__XS_enum = XS::ALL;

#ifdef BUILDING_COLLADA_DOM

	friend class COLLADA::daeCM;

COLLADA_(protected) //daeCM methods		
	/**INVISIBLE
	 * @return Returns @c true, meaning that order is specified.
	 */
	static bool __placed_between(...){ return true; }
	/**INVISIBLE
	 * @return Returns @c false, meaning that order is unspecified.
	 */
	static bool __placed_between(void*,void*){ return false; };
	
	//C should only be daeContents, but is required to be syntactically compatible.
	template<class A, class B, class C>
	/**INVISIBLE 
	 * Called by @c daeCM::__placeXS(). 
	 *
	 * HOW <xs:all> IS IMPLEMENTED
	 * XS::All acts like XS::Sequence with any insertion barriers forcibly removed.
	 * This means the library will corrupt the order of the elements: but they are
	 * still valid, because <xs:all> doesn't have any order. The library is unique
	 * in how it allows placements into sequences that are incomplete, and so this
	 * isn't an issue. Comments may require special consideration. COLLADA doesn't
	 * have any <xs:all> elements among its schemas.	
	 *
	 * @return Returns @c DAE_ORDER_IS_NOT_PRESERVED if @a A or @ B isn't @c void*
	 * and the ordinal returned via @a p.ordinal is not between @a a and @a b. The
	 * caller is responsible for taking an appropriate course of action concerning
	 * text. As documents are loaded and saved they will become ordered. This will
	 * work this way indefinitely, but schemas can include hints to preserve their
	 * order. Those will have to be implemented, naturally.
	 */
	inline daeError __place(A a, daeCM_Placement<C> &p, B b, daeOrdinal o)const;	

#endif //BUILDING_COLLADA_DOM
};

#ifdef NDEBUG
#error Add some features here, so ambiguity concerns can at least be detected.
#endif
/**TODO
 * The COLLADA::XS::Any class defines the behavior of an xs:any content-model.
 *
 * @todo The current implementation treats all <xs:any> elements as wildcards.
 * This makes them equivalent. They are the only automatic source of unnameds.
 * Therefore there is no ambiguity--by defect. If not for this, then the APIs
 * would be incomplete, as @c daeCM_Placement & @c daeCM::_placeElement() are
 * ambiguous where it comes to unnamed sources. Unnamed insertions should use
 * a special path: e.g. "_placeUnnamedElement" in order to consider ambiguity.
 * Of course, before this is done, support for <xs:any> features must improve.
 */
class Any : public daeElementCM
{	
COLLADA_(private) //XML Schema members

	int _processContents;

	daeHashString _namespace;

COLLADA_(public)

	COLLADA_DOM_OBJECT_OPERATORS(XS::Any)

	//addCM<T>() still uses this to validate T.
	static const daeXS __daeCM__XS_enum = XS::ANY;

COLLADA_(public)
	/**UNUSED
	 * Get the "namespace" attribute as set by the generator.
	 */
	const daeHashString &getNamespaceString()const
	{
		return _namespace;
	}

	/**C++98 SCOPED ENUM 
	 * Possible values returned by @c getProcessContents().
	 */
	struct processContents{ enum{ lax=3,skip=4,strict=6 }; };
	/**UNUSED
	 * Get the "processContents" attribute as set by the generator.
	 */
	inline int getProcessContents()const{ return _processContents; }

COLLADA_(public) //GENERATOR-SIDE APIs
	
	template<int N>
	/**WARNING, UNUSED
	 * This is just being used to detect if schemas require
	 * a feature that is not implemented.
	 * @warning Currently the library assumes "##any" values.
	 */
	inline XS::Any &setNamespaceString(const daeStringCP (&ns)[N])
	{
		_namespace = ns; return *this;
	}

	template<int N>
	/**WARNING, UNUSED
	 * This is just being used to detect if schemas require
	 * a feature that is not implemented.
	 * @warning Currently the library assumes "lax" values.
	 */
	inline XS::Any &setProcessContents(const daeStringCP (&pc)[N])
	{
		_processContents = N-1; return *this;
	}

#ifdef BUILDING_COLLADA_DOM

COLLADA_(public) //INVISIBLE
	/**
	 * Default Constructor
	 */
	Any():_processContents(processContents::strict),_namespace("##any")
	{
		_maxOrdinals = 1; 
	}

#endif //BUILDING_COLLADA_DOM
};

/**
 * The COLLADA::XS::Choice class defines the behavior of an xs:choice content-model.
 */
class Choice : public daeParentCM
{	
COLLADA_(public)

	COLLADA_DOM_OBJECT_OPERATORS(XS::Choice)

	//addCM<T>() still uses this to validate T.
	static const daeXS __daeCM__XS_enum = XS::CHOICE;

#ifdef BUILDING_COLLADA_DOM
	
	static struct _solution //INVISIBLE
	{	
		size_t keysN; const daeCM *to,*fro; 		
		daeOrdinal *keys;
		daeOrdinal *ordered_mapping, *ordered_places;
		daeOrdinal *unordered_mapping, *unordered_places;
		
		inline operator daeContents_like<_solution>&()
		{
			assert(ordered_mapping!=nullptr);
			assert(ordered_mapping[keysN]==0);
			return *(daeContents_like<_solution>*)this;
		}struct value_type:daeText_less<daeOrdinal_only>{};
		typedef const value_type *iterator, *const_iterator;		
		iterator begin()const{ return (iterator)ordered_mapping; } 
		iterator end()const{ return (iterator)ordered_mapping+keysN; }
		daeCursor_less<iterator> cursor()const{ return ordered_mapping; }

		template<class A, class B> bool requires_rearrange(A,B)const;
		template<class A, class B> daeOrdinal place(A,int,B,daeOrdinal)const;
		void rearrange(daeContent*,const daeContent*,daeContents&,daeOrdinal)const;

	}_solution_cache_null/* = {0,0,0}*/;
	/**
	 * Virtual Destructor
	 */
	~Choice(){ for(size_t i=0;i<_solutions.size();i++) delete[] _solutions[i].keys; }

	friend class COLLADA::daeCM;

COLLADA_(protected) //daeCM methods		
	template<class A, class B, class C>
	/**INVISIBLE
	 * Called by @c daeCM::__placeT<XS::All>() for @c XS::ALL_LIKE_CHOICE_X.
	 * Called by @c daeCM::__placeXS() for @c XS::ALL and @c XS::ALL_LIKE_CHOICE.
	 * @note The COLLADA schema often use <xs:choice> where it seems <xs:all>
	 * would do. This may be a common practice among schema. 
	 * The difference between the two here, is that a choice can have maxOccurs
	 * greater than 1. Whereas XML Schema restricts an all to 1 or 0 occurrences.
	 */
	inline daeError __placeOccurrence(A,daeCM_Placement<C>&,B,daeOrdinal)const;	
	template<class A, class B>
	/**UNDOCUMENTED This is a branch on the C template parameter. */
	inline daeError __placeOccurrence2(A,daeCM_Placement<>&,B,daeOrdinal,daeOrdinal,daeContent*)const;	
	template<class A, class B> 
	/**UNDOCUMENTED This is a branch on the C template parameter. */
	inline daeError __placeOccurrence2(A,daeCM_Placement<_solution>&,B,daeOrdinal,daeOrdinal,...)const;	

	template<class A, class B>
	/**
	 * Tries to promote a choice to a new choice. This can fail if there
	 * are complex relationships between the names, deep within the model.
	 * It's just lost work when that happens.
	 */
	daeError __tryCM(daeContent*,daeContent*,size_t,size_t,A,daeCM_Placement<>&,B,daeOrdinal)const;

	/**
	 * @c daeCM::_chooseWithout() calls this on an @c XS::Choice to pass
	 * the operation along; It does so recursively, "bubbling up" as the
	 * scope of the complex choice decision tree broadens.
	 */
	void __tryWithout(daeContent*,size_t,daeCM_Demotion<>&,daeOrdinal)const; 
							 
COLLADA_(protected) //INVISIBLE
	/**
	 * These are stored on the back of @c daeParentCM::_deepCM.
	 * They are a list of every choice that is a subset of the 
	 * choice corresponding to @c daeParentCM::_CM, all sorted 
	 * according to which are subsets of each other.
	 * 
	 * This relates to @c daeCM::__claim() only for purpose of
	 * promotion. Promotion is tried on the basis of the names
	 * of the children, and nothing else.
	 */
	_DeepCM_string *_proclaimCM;

	mutable const _solution *_solution_cache;
	/**
	 * This converts a new solution into a permanent solution.
	 */
	const daeContent *_solve(_solution&,const daeContent*,size_t,size_t,daeOrdinal)const;
	/**
	 * These save the solutions to ordinal promotion problems,
	 * -so that they can be reused once they're required once.	 
	 */
	mutable std::vector<_solution> _solutions;

	friend class COLLADA::daeParentCM;
	/**
	 * Called by @c daeCM::_prepareContentModel().
	 */
	void __preparePromotionStrategy();

#endif //BUILDING_COLLADA_DOM
};
	  
/**
 * The @c COLLADA::XS::Element class represents a content-model object that is an element.
 * HISTORICAL-NOTE
 * This class (previously "daeMetaElementAttribute") was never really exposed by the APIs.
 */
class Element : public daeElementCM
{
COLLADA_(private)

	friend class XS::Schema;
	friend class XS::Choice;
	friend class COLLADA::domAny;
	friend class COLLADA::daeDocument;
	friend class COLLADA::daeMetaElement;
	template<class>
	friend class COLLADA::daeCM_Demotion;
	//This is so daeMetaElement's arrays store objects v. pointers.
	//NOTE: ALL OF XS::Element's MEMBERS SHOULD GO INSIDE _Element.
	struct _Element 
	{
	/**
	 * Address of the child-group where @c this==nullptr.
	 * The offset can be shared by multiple @c XS::Element,
	 * -either if they are unioned 0-or-1 elements, or if a
	 * content-model has the same name in two-or-more places.
	 */
	daeOffset offset;
	/**USEFUL?
	 * Negative offset from the contents-array.
	 */
	daeOffset content_offset;
	/** 
	 * The @c daeMetaElement that describes the element metadata for this child.
	 */
	daeMeta *child;
	/** 
	 * The namefellow group assigned to the @c DAEP::Child in @c daeChildID form.
	 */
	daeChildID childID;
	/**
	 * This is the name of the children, of which there may be more than 1, or 0.
	 * Probably @c name.string is a @c daeClientString.
	 * @c daeHashString isn't a hash or anything; it just has an @c extent member.
	 * The "extent" is used to slightly accelerate look-up.
	 */
	daePseudonym name; 
	/**
	 * @c namefellows has two purposes:
	 * 1) It's used during metadata setup to act like temporary working memory.
	 * 2) In @c daeMetaElement::getTOC() it's a chain of same-name <xs:element>.
	 *
	 * On count 1 @c daeMetaElement::addCM<XS::Element>() is drawing memory from
	 * @c daeMetaElement::_elems, but it can't know which one until @c setChild().
	 * If @c namefellows==this then @c this is said to be temporary working memory.
	 */
	XS::Element *namefellows;
	/**BITFIELD FLAGS
	 * This is an @c int sized bitfield. It's not a mask, the library just reserves
	 * the ability to convert this into a bitfield if need be.
	 * INTERNAL
	 * @c namefellows_demote is calculated by @c addContentModel().
	 * It means that these elements' can shift the ordinals arround them if removed.
	 *
	 * @remarks IT'D BE HELPFUL IF THE GENERATOR PRODUCED THIS INFO.
	 * ON ONE HAND, IT CAN WAIT FOR A FULLY INLINE MODE OF INSERTION.
	 * ON THE OTHER HAND, REMOVAL IS PRETTY STRAIGHTFORAWRD TO INLINE.
	 */
	int namefellows_demote;	
	};	
	enum
	{
	__size_of_XS_Element=24*sizeof(void*),
	__reserved_for_XS_Element=__size_of_XS_Element-sizeof(_Element)-sizeof(daeElementCM)
	};
	/** 
	 * Contains members of XS::Element that are not padding nor part of @c daeElementCM.
	 */
	_Element _element;	
	char __pad2[__reserved_for_XS_Element];

COLLADA_(public) //daeArray traits

	typedef void __COLLADA__atomize;

COLLADA_(public) //OPERATORS
	
	COLLADA_DOM_OBJECT_OPERATORS(XS::Element)

COLLADA_(public) //Previously of XS::Attribute
	/**
	 * Previously "isArrayAttribute," although the meaning is
	 * slightly different this time.
	 * @see @c isArrayChildID() for a use-case that is closer
	 * to the old "isArrayAttribute."
	 */
	inline bool canReoccur()const{ return getMaxOccurs()>1; }

	/**LEGACY-SUPPORT
	 * Previoiusly "getIsRequired."
	 * Tells if the schema indicates that this is a required attribute.
	 * @return Returns true if this is a required attribute, false if not.
	 */
	inline bool isRequired()const{ return getMinOccurs()>0; }

	/**
	 * Tells if this element is part of a substitution-group.
	 * Its siblings will have equal values for @c daeCM::getSubtrahend()
	 * and will be adjacent in the CM node list of @c getParentCM().
	 * @note At the time of writing this, there are not APIs for walking
	 * the CM node graph.
	 */
	inline bool hasSubstitutes()const{ return 0==_substitute_maxOccurs; }

	/**
	 * Tells if this element is transcluded via <xs:group ref>.
	 * @note These elements should never come up by traversing
	 * a CM node graph. They can be found in a @c daeTOC array.
	 * Other APIs can be used to discern the group they belong
	 * to, but it's not possible to distinguish between graphs
	 * that have multiple instances of a global <xs:group> ref.
	 * @remark Grouplicated is a portmanteau: group-duplicated.
	 */
	inline bool isGrouplicated()const{ return _meta!=&_parentCM->getMeta(); }

	/**LEGACY
	 * Gets the @c sizeof(char) offset (from @ this) where this value's storage is
	 * found in its container element class.
	 * @return Returns the @c sizeof(char) offset from the element's @c this pointer.
	 */
	inline daeOffset getOffset()const{ return _element.offset; }
	/**LEGACY-SUPPORT
	 * Gets the negative offset from the contents-array's offset.
	 */
	inline daeOffset getCAOffset()const{ return _element.content_offset;  }

	/**LEGACY
	 * Gets the name of this child element set.
	 * @return Returns the name of this child element set.
	 */
	inline const daePseudonym &getName()const{ return _element.name; }

	/**WARNING
	 * @warning This ID can belong to more than one @c XS::Element.
	 * @return Returns the index to first child in the contents-array.
	 */
	inline daeChildID getChildID()const{ return _element.childID; }

	#ifndef COLLADA_NODEPRECATED
	COLLADA_DEPRECATED("Post-2.5: END-OF-SUPPORT\n\
	Use isArrayChildID or possibly canReoccur, depending on intent.")
	inline void isArrayAttribute()const;
	#endif //COLLADA_NODEPRECATED
	/**WARNING, LEGACY
	 * Previously "isArrayAttribute."
	 * Tells if this child is a @c dae_Array contained child.
	 * @return Returns @c true if this child shares an array.
	 * @warning The array in question can include other @c XS::Element.
	 */
	inline bool isArrayChildID()const{ return getChildID().isArray(); }	

	/**
	 * @return Returns @c true if this element has pre-generated metadata.
	 * This indicates that it's embedded in the body of the element object.
	 * @note All elements have a counter for unnamed children, although not
	 * one that is able to function as an array.
	 * @warning On some level this API is nonsensical; except that there are
	 * "holes" in @c daeMetaElement::getTOC() where this API returns @c false.
	 */
	inline bool hasNamedChildID()const{ return getChildID().isNamed(); }

	/**LEGACY
	 * Previously "getElementType."
	 * Gets the shared metadata for the children that share this schema element.
	 * @remark The name "getChild" is used to be in keeping with @c findChild().	 
	 */
	inline daeMeta &getChild()const{ return *_element.child; }

COLLADA_(public) //GENERATOR-SIDE APIs

  ///////////////////////////////////////////////////////////////////////////////
  //WARNING: Before new accessory "setX" APIs can be used, _setChild2() must be//
  //changed, so that it saves the result of _setChild3() before daeGetMeta() is//
  //called, and afterward, it must repair _element.namefellows by assigning the//
  //value; The "setX" API must then use _element.namefellows instead of "this."//
  ///////////////////////////////////////////////////////////////////////////////

	template<class T, int N> //DAEP::Child
	/**GENERATOR-SIDE API	 
	 * Sets the element type for the element that the children point to.
	 */
	inline void setChild(T &nul, daeClientStringCP (&name)[N])
	{
		//Perhaps _setChild2() should have its body put here?
		//(For a short while substitution-groups had their own implementation.)
		_setChild2<typename T::XSD::type>(nul,name);		
	}

COLLADA_(private) //DELICATE-MACHINERY

	template<class S, class T> //S was a substitution-group parameter.
	/**
	 * Implements setChild().
	 * Calls @c _setChild3(), which returns the real child by address.
	 * The child is set to @c daeGetMeta<S>(), which may be recursive.
	 * @c see @c _setChild3()
	 */
	inline void _setChild2(T &nul, daePseudonym name)
	{
		XS::Element &real = _setChild3(nul.feature(),nul.offset(),name);
		real._element.child = daeGetMeta<S>();
		//Uncomment this if something like setX() is added.
		//And then do setX(X x){ _element.namefellow->_element.x = x; }
		//_element.namefellow = real;
		real._assert_id(nul);
	}	
	friend class XS::Group; template<class T> inline void _assert_id(T &nul)
	{
		//Check that the generator/alignment is on target & daeMetaElement::_addChild().
		assert(nul==getWRT(&(daeContents&)nul.object().content));
		assert(nul==getWRT(&nul.object()));
		assert(nul.name==getChildID().getName());
	} 
	/**
	 * Implements @c _setChild2(). 
	 * @return Returns the real element, as @c this is a token object.
	 * (@c this is actually @c XS::Schema::_temporary_working_memory.)
	 * @see the daeMetaElement.cpp TU's definition. Perhaps it should
	 * be "daeMetaElement::_addChild()" instead.
	 */
	COLLADA_DOM_LINKAGE XS::Element &_setChild3(daeFeatureID,daeOffset,daePseudonym)const;

COLLADA_(public) //"WRT" APIs. (With Respect To.)

	template<class T> //DAEP::Element* or daeContents*
	/**WARNING, LEGACY, NOT-RECOMMENDED
	 * @warning Post-2.5 the pointer is not an element-ref. It's a @c dae_Array.
	 * Gets the child's memory pointer from containing element @a e.
	 * @param e Element from which to get the value.
	 * @return Returns the memory pointer corresponding to this value out of parent element @a e.
	 */
	inline typename daeConstOf<T,daeOpaque>::type getWRT(T &t)const
	{
		return daeOpaque(t)[_getWRT(t)];
	}
	template<class This> //DAEP::Element* or daeContents*
	/**WARNING, LEGACY, NOT-RECOMMENDED
	 * @warning Post-2.5 the pointer is not an element-ref. It's a @c dae_Array.
	 * Gets the child's memory pointer from containing element @a e.
	 * This overload is just to receive @c this pointers/coceivably other rvalues.
	 * @param e Element from which to get the value.
	 * @return Returns the memory pointer corresponding to this value out of parent element @a e.
	 */
	inline typename daeConstOf<This,daeOpaque>::type getWRT(This *t)const
	{
		return daeOpaque(t)[_getWRT(t)];		
	}
	/** Implements @c getWRT(). */
	inline daeOffset _getWRT(const daeElement*)const{ return _element.offset; }
	/** Implements @c getWRT(). */
	inline daeOffset _getWRT(const DAEP::Element*)const{ return _element.offset; }
	/** Implements @c getWRT(). */
	inline daeOffset _getWRT(const daeContents*)const{ return _element.content_offset; }	
		
	//LIBRARIANS SHOULD PREFER daeContents_base::__plural_or_non_size()
	//TO getCountWRT(). 
	template<class T> //DAEP::Element* or daeContents*
	/**WARNING, LEGACY
	 * @warning This count includes refs that may be @c nullptr.
	 * @return Returns the number of elements-refs in this child-ID group WRT to parent element @a e.
	 * This bypasses the contents-array. It returns 0 for unnamed child IDs, and will reports 0-or-1
	 * for single instance children, even though technically there can be more in the contents-array.
	 */
	inline daeCounter getCountWRT(const T &t)const
	{
		daeChildID::POD Name = _element.childID.getName();
		return Name>0?((daeChildID&)getWRT(t)).getIndex():
		//[sizeof(daeCounter)] should cancel out, in theory.
		((daeContents&)getWRT(t)[sizeof(daeCounter)]).__nonplural_size(Name);
	}

COLLADA_(public) 

	//addCM<T>() still uses this to validate T.
	static const daeXS __daeCM__XS_enum = XS::ELEMENT;

#ifdef BUILDING_COLLADA_DOM

	friend class COLLADA::daeCM;

COLLADA_(protected) //daeCM methods		

	template<class CM>
	/** 
	 * Used to prioritize choices. 
	 */
	inline bool __subsetof(const CM&,int,int)const;

COLLADA_(public) //INVISIBLE
	/**
	 * Cross-access of this comes up often.
	 */
	inline bool __names(int name)const
	{
		return name==_element.childID.getName();
	}

COLLADA_(public) //INVISIBLE
	/**
	 * Default Constructor
	 *
	 * This is used to @c intialize XS::Schema::_temporary_working_memory.
	 * Which is returned by @c daeMetaElement::addCM() to be resolved once
	 * @c setChild() is called. offset is used to detect a first namefellow.
	 */
	Element(){ _maxOrdinals = 1; _element.offset = 0; }

#endif //BUILDING_COLLADA_DOM
};

/**VARIABLE-LENGTH
 * The COLLADA::XS::Group class defines the behavior of an xs:group ref content-model.
 */
class Group : public daeCM
{
COLLADA_(public)

	COLLADA_DOM_OBJECT_OPERATORS(XS::Group)

	//addCM<T>() still uses this to validate T.
	static const daeXS __daeCM__XS_enum = XS::GROUP;

COLLADA_(public) //GENERATOR-SIDE APIs

	template<class S> //DAEP::Element based class
	/**GENERATOR-SIDE API
	 * Sets the element type for the element that the children point to.	 
	 * @return Returns @c *this so @c setGroup().addChild() can be done.
	 */
	inline XS::Group &setGroup(){ _setGroup(daeGetMeta<S>()); return *this; }

	template<class T, int N>
	/**GENERATOR-SIDE API
	 * @return Returns @c *this so @c addChild().addChild() can be done.
	 */
	inline XS::Group &addChild(T &nul, daeClientStringCP (&pseudonym)[N])
	{
		//hit: Sometimes registration must be deferred due to dependencies.
		XS::Element *hit = _addChild(nul.feature(),nul.offset(),pseudonym);
		if(hit!=nullptr) hit->_assert_id(nul); return *this;
	}

COLLADA_(private) //Implementation details 

	friend class COLLADA::daeMetaElement;
	/** Implements setGroup(). */
	COLLADA_DOM_LINKAGE void _setGroup(daeMeta&);
	/** Implements addChild(). */
	COLLADA_DOM_LINKAGE XS::Element *_addChild(daeFeatureID,daeOffset,daePseudonym);

COLLADA_(public) //This is really not required.

	COLLADA_NOALIAS
	/**LEGACY
	 * Previously "getElementType" or "getChild."
	 * Gets the shared metadata for the <xs:group> "ref" attribute.
	 */
	COLLADA_DOM_LINKAGE daeMeta &getGroup()const
	COLLADA_DOM_SNIPPET( return _groupCM->getMeta(); )

#ifdef BUILDING_COLLADA_DOM

COLLADA_(protected) 

	friend class XS::Choice;
	/**
	 * This is the group's CM graph.
	 */
	const daeCM *_groupCM;
	/**
	 * This is a jump-table that maps local names to the group's namespace.
	 * @note The actual memory is located at @c this+1. The pointer is not
	 * to @c this+1, but somewhere inside. Only the valid names are set up.
	 */
	short *_groupNames;

friend class daeCM;
COLLADA_(protected) //daeCM methods	

	template<class A, class B, class C>
	/**
	 * Called by @c daeCM::__placeT().
	 */
	inline daeError __placeOccurrence(A,daeCM_Placement<C>&,B,daeOrdinal)const;

	template<class CM>
	/** 
	 * Used to prioritize choices. 
	 */
	inline bool __subsetof(const CM&,int,int)const;

COLLADA_(public) //INVISIBLE

	enum{ _groupNames_not_included=SHRT_MAX };
	/**
	 * Cross-access of this comes up often.
	 */
	inline bool __names(int name)const
	{
		return _groupNames[name]!=_groupNames_not_included;
	}

COLLADA_(public) //INVISIBLE
	/**
	 * Dummy Constructor
	 * @c daeMetaElement::_addCM() initializes CMs.
	 */
	Group(short *jt):
	#ifdef _DEBUG
	_groupCM(),
	#endif
	_groupNames(jt){}

#endif //BUILDING_COLLADA_DOM
};

/**
 * The COLLADA::XS::Sequence class defines the behavior of an xs:sequence content-model in the COLLADA Schema.
 */
class Sequence : public daeParentCM
{
COLLADA_(public)

	COLLADA_DOM_OBJECT_OPERATORS(XS::Sequence)

	//addCM<T>() still uses this to validate T.
	static const daeXS __daeCM__XS_enum = XS::SEQUENCE;

#ifdef BUILDING_COLLADA_DOM

	friend class COLLADA::daeCM;

COLLADA_(protected) //daeCM methods

	template<class A, class B, class C>
	/**
	 * Called by @c daeCM::__placeT().
	 */
	inline daeError __placeOccurrence(A,daeCM_Placement<C>&,B,daeOrdinal)const;

#endif //BUILDING_COLLADA_DOM
};		  

//-------.
	}//<-'
}

#endif //__COLLADA_DOM__DAE_META_ELEMENT_ATTRIBUTE_H__
/*C1071*/
