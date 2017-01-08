/*
 * Copyright 2006 Sony Computer Entertainment Inc.
 *
 * Licensed under the MIT Open Source License, for details please see license.txt or the website
 * http://www.opensource.org/licenses/mit-license.php
 *
 */

#ifndef __COLLADA_DOM__DAE_ELEMENT_H__
#define __COLLADA_DOM__DAE_ELEMENT_H__

#include "daeMetaElement.h"

COLLADA_(namespace)
{//-.
//<-'

#ifndef COLLADA_NODEPRECATED
/**LEGACY-SUPPORT, NOT-RECOMMENDED
 * @c getAttributes() uses @c attr to form its key/value pairs. 
 */
struct daeAttributeKeyValuePair
{
	typedef void __COLLADA__atomize;
	daeName name; daeArray<daeStringCP,96> value; 
};
//Just for getDocumentURI() API.
template<int> class daeURI_size;
typedef daeSmartRef<const daeURI_size<260>> const_daeURIRef;
#endif //COLLADA_NODEPRECATED

/**PLAIN-OLD-DATA
 * 32-bit structure that overlays with @c daeObject::_tags.
 *
 * If two elements have equal tags, then you can assume several
 * things about them:
 *
 * A) They belong to the same modular XML namespace.
 * B) They have the same : based XML prefix: Eg. <xs:element>.
 * C) The virtual-table layout is identical. This one's for librarians.
 * D) They hail from the same client module; That is hosting the space.
 *
 * IMPORTANT!
 * None of this holds true unless the entire 32-bit value is identical.
 * In other words, do not compare individual tags. That is meaningless.
 */
struct daeElementTags
{
	inline operator daeInt()const{ return (daeInt&)*this; }

	/**ABCD
	 * interfaceTag corresponds to daeObjectLayout::_vtab. 
	 * moduleTag is always odd if owned by a daeDatabase, &
	 * moduleTag is always 0 if not owned by a daeDatabase.
	 * nameTag will lookup a global XML colonized prefix, but
	 * namespaceTag must be provided to the lookup procedure, in
	 * case it's necessary to expand the table beyond 255 entries.
	 * When nameTag is 0, namespaceTag is the the default namespace.
	 * When namespaceTag is 0, the element is not in a contents-array. 
	 * (This is to work w/ @c daeMetaElement::createWRT()->placeWRT().)
	 * When namespaceTag is 1, it is an undefined, document namespace. 
	 */
	const daeTag namespaceTag, nameTag, interfaceTag, moduleTag;
};

template<class T>
/**
 * COLLADA C++ class that implements an instance of @c daeElement.
 *
 * Generators have the option of basing classes off @c daeElement
 * (COLLADA-DOM 2.x) or @c DAEP::Element (ColladaDOM 3) depending
 * on if @c daeElemental is inherited from, or @c DAEP::Elemental
 * is inherited from. 
 * @see @c DAEP::Elemental template in the DAEP.h header.
 * @note Generators must implement @c __DAEP__Object__v1__model()
 * for all classes based on @c daeElemental.
 *
 * @remarks There are significant wrinkles introduced by the Legacy
 * parameter. Initially the hope was that virtual-inheritance could
 * be used to share the data/virtual-layout, but it didn't work out.
 */
class daeElemental : public DAEP::Elemental<T,daeElement>
{
	//Microsoft's Natvis can't seem to find namespaces???
	typedef DAEP::Elemental<T,daeElement> __super_natvis;

COLLADA_(public) //These may not work.
	/**
	 * Static version of @c daeObject::getObjectType().
	 */
	static int getObjectType(){ return getElementType(); }
	/**
	 * Static version of @c daeElement::getElementType().
	 */
	static int getElementType()
	{
		return T::elementType; 
		//This is just checking the generator's work.		
		daeCTC<elementType==T::__DAEP__Schema__genus>();
	}
};

#include "../LINKAGE.HPP" //#define LINKAGE

/**ABSTRACT BASE-CLASS
 *
 * 2016 Changes
 * ============
 * 2.5 NOTES: Classes can only be indirectly based on @c daeElement
 * by way of @c daeElemental<T>, where "T" is the class itself.
 * Loaders should use @c daeMetaElement::pushBackWRT() to fill out
 * the element's contents.
 * 2.5 adds text-nodes (comments, etc.) but does not add APIs to this
 * legacy interface, because COLLADA itself doesn't really require it,
 * -and because @c getContents() is added and can be called on to get
 * at these new APIs; just as @c getContents().clear() can clear them.
 * End 2.5 NOTES
 * =============
 * The @c daeElement class represents an instance of a COLLADA "Element";
 * it is the main base class for the COLLADA DOM.
 * Features of this class include:
 * - Uses factory concepts defined via @c daeMetaElement.
 * - Composed of attributes, content elements and content values.
 * - Reference counted via daeSmartRef.
 * - Contains information for XML base URI, and XML containing element. 
 */
class daeElement : public daeObject
{	
	friend class daeDOM;
	friend class domAny;
	friend class daeObject;
	friend class daeDocument;
	friend class daeMetaElement;
	//DAEP Element DATA-MEMBERS & CONSTRUCTORS
	COLLADA_(protected)
	//VIRTUAL-INHERITANCE DOESN'T LINE UP WITH 
	//DAEP::Element, AND COULD ENTAIL OVERHEAD.
	DAEP::Element::__DAEP__Element__Data __DAEP__Element__data;
	char _reserved[daeSizeOf(DAEP::Element,__DAEP__Element__data_reserved)];
	//REPLICATING @c DAEP::Element CONSTRUCTOR
	daeElement()
	{ /*NOP*/ }	
	//REPLICATING @c DAEP::Element CONSTRUCTOR
	explicit daeElement(const DAEP::Proto<> &pt):daeObject(pt)
	{ /*NOP*/ }
	//REPLICATING @c DAEP::Element CONSTRUCTOR
	explicit daeElement(const DAEP::Object *_this):daeObject(_this)
	{ __DAEP__Object__unembed(1); }
	friend class DAEP::Object;
	friend class DAEP::Element;
	/** These ferry the document downstream (for @c daeDocument::typeLookup().) */
	void __atomize2(daeMeta&);
	void __clear2(daeContents&,daeMeta&,const daeDocument*);
	COLLADA_(public)
	/**CIRCULAR-DEPENDENCY @c DAEP::Element::__COLLADA__atomize() is exported. */
	inline void __COLLADA__atomize(){ ((DAEP::Element*)this)->__COLLADA__atomize(); }

COLLADA_(public) //2.5 "daeSafeCast" shorthand

	COLLADA_DOM_OBJECT_OPERATORS(daeElement)
	/** These sometimes comes in handy since no longer inheriting. */
	inline operator DAEP::Element*(){ return (DAEP::Element*)this; }	
	/** These sometimes comes in handy since no longer inheriting. */
	operator const DAEP::Element*()const{ return (DAEP::Element*)this; }
	/** These sometimes comes in handy since no longer inheriting. */
	inline operator DAEP::Element&(){ return *(DAEP::Element*)this; }	
	/** These sometimes comes in handy since no longer inheriting. */
	operator const DAEP::Element&()const{ return *(DAEP::Element*)this; }

	template<class T> 
	/**WARNING
	 * @warning 2.5 adds an @c assert() to @c daeSafeCast(). This was
	 * originally to be a shorter alternative, but eventually similar
	 * APIs were added to the other objects, which are designed to do
	 * filtering. For consistency the @c assert() function is removed.
	 *
	 * Here is an example; below "->" is pronounced "to."
	 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.C++}
	 * domCOLLADA *COLLADA = p->a<domCOLLADA>();
	 */	
	inline typename T *a()
	{
		return daeUnsafe<T>(this)?nullptr:(T*)this; 
	}		
	template<class T> 
	/**CONST-FORM 
	 * @see non-const Doxygentation. 
	 */
	const T *a()const
	{
		return daeUnsafe<daeConstOf<int,T>::type>(this)?nullptr:(T*)this; 
	}	
	template<class T> 
	/**ALT-SPELLING
	 * Just because some words use "an" instead of "a" in English. 
	 * @see @c a().
	 */
	inline T *an()
	{
		return daeUnsafe<T>(this)?nullptr:(T*)this; 
	}		
	template<class T> 
	/**CONST-FORM, ALT-SPELLING
	 * @see @c a().
	 */
	const T *an()const
	{
		return daeUnsafe<daeConstOf<int,T>::type>(this)?nullptr:(T*)this; 
	}	

COLLADA_(public)	
	/**LEGACY
	 * @remarks This method predates 2.5, and so is repurposed.
	 * @return Returns the daeObjectType class's typeless enum.
	 */
	inline int getElementType()const{ return getMeta()->getElementType(); }

	/**WARNING, LEGACY-SUPPORT
	 * Gets this element's NCName.
	 * @note The library doesn't (yet) support QNames properly.
	 * Some child names are NCNames with a colon, or a pseudo QName.
	 * @return Returns the @c daePseudonym for the NCName.
	 * @remarks @c getTypeName() gets the COLLADA schema type name
	 * @warning A STRING-LITERAL OR PERMA-STRING MUST BE USED TO SET THIS.
	 */
	inline daePseudonym &getNCName(){ return __DAEP__Element__data.NCName; }
	/**CONST-FORM, LEGACY-SUPPORT
	 * Gets this element's NCName.
	 * @note The library doesn't (yet) support QNames properly.
	 * Some child names are NCNames with a colon, or a pseudo QName.
	 * @return Returns the @c daePseudonym for the NCName.
	 * @remarks @c getTypeName() gets the COLLADA schema type name
	 * Use caution when using this function since you can easily create invalid documents.
	 */
	inline const daePseudonym &getNCName()const{ return __DAEP__Element__data.NCName; }	

	/**LEGACY
	 * Gets the associated Meta information for this element. This
	 * Meta also acts as a factory. See @c daeMetaElement Doxygentation for more
	 * information.
	 * @return Returns the associated meta information.
	 */
	inline daeMeta &getMeta()const{ return *__DAEP__Element__data.meta; }
	
	//Exposing to client on daeStringRef prototype-constructor's grounds.
	//Reminder: it's not just for efficiency. Since the prototypes don't
	//have a DOM parent, they only have DOM set to daeStringRef_protoDOM.
	//#ifdef BUILDING_COLLADA_DOM //INVISIBLE
	/**INVISIBLE, SHADOWING @c daeObject::getDOM().
	 */
	inline const daeDOM *getDOM()const{ return __DAEP__Element__data.DOM; }	
	//#endif
	/**CIRCULAR-DEPENDENCY
	 * This is newly added in case it's decided to store a copy of @c daeDOM::_database in
	 * every element, alongside a copy of the @c daeMetaElement pointer. They are constant
	 * for the entirety of the element's life.
	 *
	 * NOTE: BECAUSE OF _databaseRejoinder, IT SEEMS MORE LIKE A DOM POINTER WOULD BE USED.
	 * ALSO: A _DOM WOULD HAVE TO WORK TRANSPARENTLY VIS-A-VIS daeDocument::_PseudoElement.
	 */
	inline daeDatabase &getDatabase()const;	

	/**MICRO-OPTIMIZATION
	 * This is an optimization mainly for DAEP::ELEMENT change-notices.
	 */
	inline bool getOptimizationBit_has_registered_ID_attribute()const
	{
		return 1==__DAEP__Element__data.getMeta_getFirstID_is_nonzero;
	}
	//NOTE: THIS MAY BE USEFUL IF NOTICES ARE DISABLED, BECAUSE
	//IF THERE'S A GRAPH, THE NOTICE IS PROBABLY UNAVOIDABLE, AS
	//THERE SHOULD ONLY BE ONE GRAPH-TRAVERSER: I.E. THE DATABASE.
	/**MICRO-OPTIMIZATION
	 * This is an optimization mainly for DAEP::ELEMENT change-notices.
	 *
	 * As its wording suggests, it means the element cannot have content,
	 * -or that it cannot be a "graph." It doesn't mean that it is a graph.
	 * By knowing that it's not a graph, code can avoid accessing a metadata
	 * record, in order to access a contents-array, in order to determine that
	 * the array is empty or not.
	 *
	 * Very often elements are inserted into the document, and then their content
	 * is added afterward. So on the optimum route, this returns @c false. In isn't
	 * really important in the grand scheme of things.
	 */
	inline bool getOptimizationBit_is_definitely_not_a_graph()const
	{
		return 0==__DAEP__Element__data.daeContents_capacity_is_nonzero;
	}

	/**STATIC OVERRIDE
	 * Tags provide a mechanism for extensibility. 
	 * @remarks At some point getTags might be added to daeObject; Override it here if so.
	 * @return Returns @c (daeElementTags&)daeObject::_tags by value.
	 */ 
	inline daeElementTags getTags()const
	{
		daeCTC<_sizeof__DAEP__Object__tags==sizeof(daeElementTags)>(); 
		return (daeElementTags&)_getClassTag(); 
	}
	/** @see getTags--Just explicit--and just in case of name-clash. */
	inline daeElementTags getElementTags()const
	{
		daeCTC<_sizeof__DAEP__Object__tags==sizeof(daeElementTags)>();
		return (daeElementTags&)_getClassTag(); 
	}

	template<class U> //See daeContents::__cursorize().
	/**WARNING, LEGACY, OVERLOAD 
	 * Adds @a child to @c this element.
	 *
	 * @remarks This API had a cursor-like argument before. It wasn't
	 * really appropriate, since it only applied to the old contents-
	 * arrays, which not every element had.	In any case, it's removed.
	 *
	 * @warning @a child CANNOT BE A NULLPTR-PLACEHOLDER NOR NULLPTR-POINTER.
	 */
	inline daeElement *add(const U &child)
	{
		return getMeta()->placeWRT(this,child)>=DAE_OK?child:nullptr; 
	}
	/**LEGACY, OVERLOAD
	 * @param list_of_names is a space separated list of nested child 
	 * names. For example: @c COLLADA->add("asset contributor");
	 * @note The library doesn't (yet) support QNames properly.
	 * Some children names are NCNames with a colon, or a pseudo QName.
	 *
	 * @note This is the pre-2.5 way. It's not C++ style, because a
	 * string is weakly typed. It's better to assign directly to the 
	 * arrays when possible. The "ColladaDOM 3" generator will output
	 * prefix/suffix free member names.
	 * ColladaDOM 3 alternative: @c +COLLADA->asset->contributor.
	 *
	 * @remarks This API had a cursor-like argument before. It wasn't
	 * really appropriate, since it only applied to the old contents-
	 * arrays, which not every element had.	In any case, it's removed.
	 */
	LINKAGE daeElement *add(daeString list_of_names);	

	template<class S, class T, class U> //See daeContents::__cursorize().
	/**WARNING, LEGACY-SUPPORT
	 *
	 * Adds @a child to @c this element, not before @a after, nor after @c before.
	 *
	 * @warning Care must be taken with @a before and @a after so not
	 * to shift the contents-array as a side-effect of obtaining them.
	 * @warning <xs:all> returns @c DAE_ORDER_IS_NOT_PRESERVED, which 
	 * appears to be a success as viewed by this API.
	 *
	 * @warning @a child CANNOT BE A NULLPTR-PLACEHOLDER NOR NULLPTR-POINTER.
	 */
	inline daeElement *addBetween(const S &after, const U &child, const T &before)
	{
		return getMeta()->placeBetweenWRT(this,after,child,before)>=DAE_OK?child:nullptr; 
	}

	template<class T, class U> //See daeContents::__cursorize().
	/**WARNING, LEGACY 
	 *
	 * IF IMPLEMENTING A LOADER, 2.5 ADDS @daeMeta::pushBackWRT().
	 *
	 * Adds @a child to @c this element, not before @a after.
	 *
	 * @warning <xs:all> returns @c DAE_ORDER_IS_NOT_PRESERVED, which 
	 * appears to be a success as viewed by this API.	 
	 *
	 * @warning @a child CANNOT BE A NULLPTR-PLACEHOLDER NOR NULLPTR-POINTER. 
	 */
	inline daeElement *addAfter(const U &child, const T &after)
	{
		return getMeta()->placeAfterWRT(after,this,child)>=DAE_OK?child:nullptr;
	}

	template<class T, class U> //See daeContents::__cursorize().
	/**WARNING, LEGACY
	 * Adds @a child to @c this element, not after @a before.
	 *
	 * @warning <xs:all> returns @c DAE_ORDER_IS_NOT_PRESERVED, which 
	 * appears to be a success as viewed by this API.
	 *
	 * @warning @a child CANNOT BE A NULLPTR-PLACEHOLDER NOR NULLPTR-POINTER. 
	 */
	inline daeElement *addBefore(const U &child, const T &before)
	{
		return getMeta()->placeBeforeWRT(before,this,child)>=DAE_OK?child:nullptr;
	}

	template<class U> //See daeContents::__cursorize().
	/**WARNING, LEGACY
	 * Removes the @a element from its parent, the @c this element.
	 * @param element Element to be removed in the @c this container.
	 *
	 * @return Returns @c DAE_ORDER_IS_NOT_PRESERVED or @c DAE_OK if successful.
	 * @warning THIS API WILL ACCEPT NULLPTR-PLACEHOLDERS, AND REMOVE THEM FROM
	 * THE CONTENTS-ARRAY; STILL THIS PRACTICE IS DISCOURAGED FOR ITS ASYMMETRY.
	 *
	 * @remarks The library does not currently envision constrained removals of
	 * <xs:choice> based children. This API is unconstrained in any case. Where
	 * code expects a @c bool return type, it will report @c false if reordered.
	 */
	inline daeOK removeChildElement(const U &element)
	{
		return getMeta()->removeWRT(this,element);
	}

	template<class U> //See daeContents::__cursorize().
	/**LEGACY
	 * Removes @a element element from its parent element.
	 * The API finds the parent itself, and is a @c static method,
	 * since removing the element from its parent may result in its deletion.
	 * (IT'S NOT CLEAR WHAT'S DELETED. Pre-2.5 childless groups were deleted.)
	 * If the element has no parent, nothing is done.
	 *
	 * @return Returns @c DAE_ORDER_IS_NOT_PRESERVED or @c DAE_OK if successful.
	 *
	 * @remarks The library does not currently envision constrained removals of
	 * <xs:choice> based children. This API is unconstrained in any case. Where
	 * code expects a @c bool return type, it will report @c false if reordered.
	 */
	static daeOK removeFromParent(const U &element)
	{
		if(const daeElement*p=element->getParentElement()) 
		return const_cast<daeElement*>(p)->removeChildElement(element); 
		return DAE_ERROR;
	};

	/**LEGACY
	 * Returns the number of attributes in this element.
	 * @return The number of attributes this element has.
	 * @remarks Returns the highest getAttributeIndex + 1.
	 */
	inline size_t getAttributeCount()const{ return getMeta()->getAttributes().size(); }

	#ifdef NDEBUG
	#error NOALIAS_LINKAGE is a problem for domAny,
	#error -as can be holding onto daeAttribute pointers.
	#error There has to be an anyAttribute bucket-based system.
	#endif	
	/**LEGACY-SUPPORT
	 * @param NCName Name of the got index's attribute.
	 * @note The library doesn't (yet) support QNames properly.
	 * Some attribute names are NCNames with a colon, or a pseudo QName.
	 * @return Returns -1 if the attribute @a name is not present.
	 *
	 * @remarks 2.5 exposes this API. Previously many APIs received
	 * this index. ALL APIs have been reorganized to flow through this
	 * API. Unfortunately that sometimes means calling two exported APIs.
	 * On the other hand, it's more flexible and easier to maintain. 
	 *
	 * At present @c getAttributeIndex() is required to implement @c domAny.
	 * Eventually this will be obsolete, and can route through @c daeMetaElement.
	 * At that point, none of this will matter.
	 */
	template<class T> inline int getAttributeIndex(const T &pseudonym)const
	{
		return _getAttributeIndex(daeBoundaryString2<T>::type(pseudonym));
	}
	#define _(x) template<> int getAttributeIndex<x>(const x &i)const{ return i; }
	COLLADA_SUPPRESS_C(4244) //This is only for size_t. short/char will be added upon request.
	_(signed)_(unsigned)_(signed long)_(unsigned long)_(signed long long)_(unsigned long long)
	#undef _
	/** Implements @c getAttributeIndex(). */
	NOALIAS_LINKAGE int _getAttributeIndex(daeString pseudonym)const
	SNIPPET( return _getAttributeIndex(daeHashString(pseudonym)); )
	/** Implements @c getAttributeIndex(). */
	NOALIAS_LINKAGE int _getAttributeIndex(const daeHashString&)const;
	
	//JUST FOR THE RECORD, THIS CALLS 2 EXPORTED APIS AS REVISED.
	template<class T>
	/**LEGACY	 
	 * @param i_or_NCName Name or index of the got attribute.
	 * @note The library doesn't (yet) support QNames properly.
	 * Some attribute names are NCNames with a colon, or a pseudo QName.
	 * @return Returns the corresponding @c daeAttribute or @c nullptr if this element
	 * doesn't have the specified attribute.
	 */
	inline daeAttribute *getAttributeObject(const T &i_or_NCName)const
	{
		//WARNING//WARNING//WARNING
		//This differs slightly from getMeta()->getAttribute(QName)
		//because it can trigger _addAnyAttribute() to expand domAny.
		//It may also need to support <xs:anyAttribute> and buckets.
		return _getAttributeObject(getAttributeIndex(i_or_NCName));
	}
	/** Implements @c getAttributeObject(). */
	NOALIAS_LINKAGE daeAttribute *_getAttributeObject(size_t)const;

	template<class T>
	/**LEGACY
	 * Checks if this element can have the attribute specified.
	 * @param NCName The name of the attribute to look for.
	 * @note The library doesn't (yet) support QNames properly.
	 * Some attribute names are NCNames with a colon, or a pseudo QName.
	 * @return Returns true is this element can have an attribute with the name specified. False otherwise.
	 */
	inline bool hasAttribute(const T &NCName)const{ return nullptr!=getAttributeObject(NCName); }

	/**LEGACY
	 * Returns the name of the attribute at the specified index.
	 * @param i The index of the attribute whose name should be retrieved.
	 * @return Returns the name of the attribute, or "" if the index is out of range.
	 */
	inline daePseudonym getAttributeName(size_t i)const
	{
		daeAttribute *a = getAttributeObject(i); return a==nullptr?a->getName():"";
	}
	
	//JUST FOR THE RECORD, THIS CALLS 2 EXPORTED APIS AS REVISED.
	template<class T>
	/**LEGACY-SUPPORT
	 * @return Returns @a value.
	 * @param i_or_NCName Name or index of the got attribute.
	 * @note The library doesn't (yet) support QNames properly.
	 * Some attribute names are NCNames with a colon, or a pseudo QName.
	 * @param A string in which to store the value of the attribute. 
	 * This will be set to "" if this element doesn't have the specified attribute.
	 * @remarks The order is reversed because MSVC2010 resolves template arguments 
	 * from left to right, and will crash on the single-argument form.
	 */
	inline daeArray<daeStringCP> &getAttribute(daeArray<daeStringCP> &value, const T &i_or_NCName)const
	{
		return _getAttribute(value,getAttributeIndex(i_or_NCName));
	}
	template<class T>
	/**LEGACY-SUPPORT
	 * This form creates a new array on the stack in the caller's context.
	 * It's constant, and quite large.
	 * @see 2 argument overload form's Doxygentation.
	 */
	inline const daeArray<daeStringCP> &getAttribute(const T &i_or_NCName, 
	class undefined*_=0,const daeArray<daeStringCP>&def=daeArray<daeStringCP,96>())const
	{
		getAttribute<T>(const_cast<daeArray<daeStringCP>&>(def),i_or_NCName); return def;
	}	
	/**Implements @c getAttribute(). */
	LINKAGE daeArray<daeStringCP> &_getAttribute(daeArray<daeStringCP> &value, size_t i)const;
	  	
	#ifndef COLLADA_NODEPRECATED
	/**LEGACY-SUPPORT, NOT-RECOMMENDED
	 * @c getAttributes() uses @c attr to form its key/value pairs. 
	 * @note "attr" must come from the prefix of the 2.x generated attribute member names.
	 */
	typedef daeAttributeKeyValuePair attr;
	/**LEGACY-SUPPORT, NOT-RECOMMENDED
	 * @return Returns @a attrs.
	 * @param attrs Array of @c daeElement::attr (@c daeAttributeKeyValuePair) objects to 
	 * return.
	 */
	inline daeArray<attr> &getAttributes(daeArray<attr> &attrs)const
	{
		const daeArray<daeAttribute> &attribs = getMeta()->getAttributes();
		attrs.setCount(attribs.size());
		for(size_t i=0,iN=attrs.size();i<iN;i++)
		{
			attrs[i].name = attribs[i].getName();
			attribs[i].memoryToStringWRT(this,attrs[i].value);
		}
		return attrs;
	}
	/**WARNING, LEGACY-SUPPORT, NOT-RECOMMENDED
	 * Gets a temporary array of attribute key/value pairs by reference.
	 * @note The array will be destructed at the end of the calling scope.
	 * @warning THE EMBEDDED-ARRAY IS CONSIDERABLY LARGE ON THE STACK: ~2KB.
	 */
	inline const daeArray<daeElement::attr> &getAttributes
	(class undefined*_=0,const daeArray<daeElement::attr>&def=daeArray<daeElement::attr,20>())const
	{
		getAttributes(const_cast<daeArray<daeElement::attr>&>(def)); return def;
	}
	#endif //COLLADA_NODEPRECATED
	
	//JUST FOR THE RECORD, THIS CALLS 2 EXPORTED APIS AS REVISED.
	template<class S, class T>
	/**LEGACY
	 * Sets the attribute to the specified value.
	 * @param i_or_NCName Name or index of the set attribute.
	 * @note The library doesn't (yet) support QNames properly.
	 * Some attribute names are NCNames with a colon, or a pseudo QName.
	 * @param value Value to apply to the attribute.
	 * @return Returns true if the attribute was found and the value was set, false otherwise.
	 */
	inline daeOK setAttribute(const S &i_or_NCName, const T &value)
	{
		return _setAttribute(getAttributeIndex(i_or_NCName),daeBoundaryString2<T>::type(value));
	}
	/** Implements @c setAttribute(). */
	LINKAGE daeOK _setAttribute(size_t,daeString);
	/** Implements @c setAttribute(). */
	LINKAGE daeOK _setAttribute(size_t,const daeHashString&);

	/**LEGACY
	 * Previously "getID."
	 * Gets the element "id" if it exists.
	 * @remarks The string--converted to a non-nullptr lvalue--can be @c (daeStringRef&) cast.
	 * @return Returns @c nullptr if the "id" attribute does not exist. OTHERWISE SOME STRING
	 * WITH AN UNSPECIFIED LIFETIME IS RETURNED. 
	 */
	NOALIAS_LINKAGE daeString getID_id()const
	SNIPPET( daeAttribute *a = getMeta()->getID_id(); return a==nullptr?nullptr:(const daeString&)a->getWRT(this); )
	
	/**LEGACY
	 * Returns the @c daeValue object corresponding to the character data for this element.
	 * @return Returns a @c daeValue object or @c nullptr if this element doesn't have
	 * character data.
	 */
	inline daeCharData *getCharDataObject()const{ return getMeta()->getValue(); }

	/**LEGACY-SUPPORT, NOT-RECOMMENDED
	 * Checks if this element can have character data.
	 * @return Returns true if this element can have character data, false otherwise.
	 */
	inline bool hasCharData()const{ return getCharDataObject()!=nullptr; }

	/**LEGACY
	 * @return Returns @a inout.
	 * @param data The string to be filled with this element's character content. The
	 * string is set to an empty string if this element can't have character data.
	 */
	LINKAGE daeArray<daeStringCP> &getCharData(daeArray<daeStringCP> &inout)const;
	/**LEGACY-SUPPORT
	 * This form creates a new array on the stack in the caller's context. It's constant, and quite large.
	 * "daeAtom" is bogus. (It's for overload resolution.)
	 * The stack-allocated buffer is slightly larger than @c daeGetAttribute() since such data is often so.
	 * Still, for very large data buffers, use/reuse a fully dynamic buffer with the other form, reserving
	 * space comparable to the amount of data if possible.
	 */
	inline const daeArray<daeStringCP> &getCharData
	(class undefined*_=0,const daeArray<daeStringCP>&def=daeArray<daeStringCP,256>())const
	{
		getCharData(const_cast<daeArray<daeStringCP>&>(def)); return def;
	}
	template<class T>
	/**LEGACY
	 * Sets this element's character data.
	 * @param data The new character data of this element.
	 * @return Returns true if this element can have character data and the character data
	 * was successfully changed, false otherwise.
	 */
	inline daeOK setCharData(const T &data)
	{
		return _setCharData(daeBoundaryString2<T>::type(data)); 
	}
	/** Implements @c setCharData(). */
	LINKAGE daeOK _setCharData(daeString value)
	SNIPPET( return _setCharData(daeHashString(value)); )
	/** Implements @c setCharData(). */
	LINKAGE daeOK _setCharData(const daeHashString&);
	
	/**LEGACY
	 * Gets the container element for @c this element.
	 * If @c createAndPlace() was used to create the element, its parent is the the caller of @c createAndPlace().
	 * @return Returns the parent element, if @c this is not the top level element.
	 * @remarks Post-2.5 returns @c const types for upstream "getter" APIs. 
	 */
	inline const daeElement *getParentElement()const 
	{
		const daeElement &c = static_cast<const daeElement&>(getParentObject());
		//0x1000000 screens out documents and DOMs. (Its only possible parents.)
		if(c.getElementTags()<=0x1000000) return nullptr;
		else assert(c._isElement()); return c; 
	}	

	/**LEGACY-SUPPORT
	 * Tells if the element is part of a document, as opposed to simply having a
	 * @c daeDocument object parent.
	 */
	inline bool isContent()const{ return 0!=getElementTags().namespaceTag; }
	/**WARNING, LEGACY
	 * Gets the container document for @c this element. 
	 * @return Returns a @c const pointer. Post-2.5 upstream pointers are @c const.
	 * @warning This is not @c getDoc(). If @c isContent()==false, it's @c nullptr.
	 */
	inline const daeDocument *getDocument()const
	{
		//The isContent() check is really required for routines that try to determine
		//if an element was removed from a contents-array, because more often than not
		//that does not change the element's parent object.
		return !isContent()?nullptr:(daeDocument*)getDoc(); 
	}

	#ifndef COLLADA_NODEPRECATED
	/**LEGACY, DEPRECATED
	 * (@c getDocument()->getBaseURI() should do.)
	 * Gets the URI of the document containing this element, note that this is NOT the URI of the element.
	 * @return Returns a pointer to the daeURI of the document containing this element.
	 */
	NOALIAS_LINKAGE const_daeURIRef getDocumentURI()const;
	#endif //COLLADA_NODEPRECATED
				
	//These are helper structures to let the XML hierarchy search functions know when we've
	//found a match. You can implement a custom matcher by inheriting from this structure,
	//just like matchName and matchType.
	struct matchElement
	{
		virtual ~matchElement(){}
		virtual bool operator()(const daeElement*)const = 0;		
	};	
	/**LEGACY-SUPPORT
	 * Matches an element by its schema metadata.
	 */
	struct matchMeta : public matchElement
	{
		daeMeta *meta;
		matchMeta(daeMeta *m):meta(m){}		
		virtual bool operator()(const daeElement *e)const{ return meta==e->getMeta(); }
	};
	/**LEGACY
	 * Matches an element by name.
	 */
	struct matchName : public matchElement
	{
		daePseudonym name; //std::string
		matchName(const daePseudonym &n):name(n){}
		virtual bool operator()(const daeElement *e)const{ return name==e->getElementName(); }		
	};	
	/**WARNING, LEGACY
	 * Matches an element by schema type.
	 * @warning mismatched namespaces can result in false positives
	 */
	struct matchType : public matchElement
	{
		int DAEP_genus;
		matchType(int g):DAEP_genus(g){}		
		virtual bool operator()(const daeElement *e)const{ return DAEP_genus==e->getElementType(); }
	};
	/**LEGACY
	 * Returns a matching child element. By "child", I mean one hierarchy level beneath the
	 * current element. This function is basically the same as getDescendant, except that it
	 * only goes one level deep.
	 */
	NOALIAS_LINKAGE daeElement *getChild(const matchElement &matcher);
	/**LEGACY
	 * Returns a matching child element. By "child", I mean one hierarchy level beneath the
	 * current element. This function is basically the same as getDescendant, except that it
	 * only goes one level deep.
	 */
	inline daeElement *getChild(const daePseudonym &named)
	{
		return getChild(matchName(named)); 
	}
	/**CONST-PROPOGATING-FORM, LEGACY
	 * Returns a matching child element. By "child", I mean one hierarchy level beneath the
	 * current element. This function is basically the same as getDescendant, except that it
	 * only goes one level deep.
	 */
	inline const daeElement *getChild(const matchElement &matcher)const
	{
		return const_cast<daeElement*>(this)->getChild(matcher); 
	}
	/**CONST-PROPOGATING-FORM, LEGACY
	 * Returns a matching child element. By "child", I mean one hierarchy level beneath the
	 * current element. This function is basically the same as getDescendant, except that it
	 * only goes one level deep.
	 */
	inline const daeElement *getChild(const daePseudonym &named)const
	{
		return getChild(matchName(named)); 
	}
	/**LEGACY
	 * Performs a breadth-first search and returns a matching descendant element. A "descendant
	 * element" is an element beneath the current element in the xml hierarchy.
	 */
	NOALIAS_LINKAGE daeElementRef getDescendant(const matchElement &matcher);									
	/**LEGACY
	 * Performs a breadth-first search and returns a matching descendant element. A "descendant
	 * element" is an element beneath the current element in the xml hierarchy.
	 */
	inline daeElementRef getDescendant(const daePseudonym &named)
	{
		return getDescendant(matchName(named)); 
	}	
	/**CONST-PROPOGATING-FORM, LEGACY
	 * Performs a breadth-first search and returns a matching descendant element. A "descendant
	 * element" is an element beneath the current element in the xml hierarchy.
	 */
	inline const daeElementRef getDescendant(const matchElement &matcher)const
	{
		return const_cast<daeElement*>(this)->getDescendant(matcher);
	}
	/**CONST-PROPOGATING-FORM, LEGACY
	 * Performs a breadth-first search and returns a matching descendant element. A "descendant
	 * element" is an element beneath the current element in the xml hierarchy.
	 */
	inline const_daeElementRef getDescendant(const daePseudonym &named)const
	{
		return getDescendant(matchName(named)); 
	}	
	/**LEGACY
	 * Searches up through the XML hiearchy and returns a matching element.
	 * @remarks Post-2.5 returns @c const types for upstream "getter" APIs. 
	 */
	NOALIAS_LINKAGE const_daeElementRef getAncestor(const matchElement &matcher);
	/**LEGACY
	 * Searches up through the XML hiearchy and returns a matching element.
	 * @remarks Post-2.5 returns @c const types for upstream "getter" APIs. 
	 */
	inline const_daeElementRef getAncestor(const daePseudonym &named)
	{
		return getAncestor(matchName(named)); 
	}
	/**LEGACY
	 * Returns the parent element.
	 * @remarks Post-2.5 returns @c const types for upstream "getter" APIs. 
	 */
	inline const daeElement *getParent()const{ return getParentElement(); }

	/**WARNING, LEGACY
	 * SOFT-DEPRECATED: Prefer @c getNCName().	 
	 * @warning: QNames are not properly implemented. (They are NCNames with colons.)
	 * Gets this element's QName.
	 * @return Returns the string for the name.
	 * @remarks @c getTypeName() gets the COLLADA schema type name
	 */
	inline const daePseudonym &getElementName()const
	{
		return __DAEP__Element__data.NCName; 
	}		
	template<class T>
	/**WARNING, LEGACY
	 * SOFT-DEPRECATED: Prefer @c getNCName().
	 * @warning: QNames are not properly implemented. (They are NCNames with colons.)
	 * Sets this element's QName.
	 * @param QName Specifies the string to use as the element's name.
	 * @remarks This legacy API adds you string to the system string-table to be safe.
	 * Use caution when using this function since you can easily create invalid documents.
	 */
	inline void setElementName(const T &QName)
	{
		//TODO: QNAMES ARE NOT ACTUALLY IMPLEMENTED. COLONS SHOULD BE LOOKED FOR HERE.
		daeStringRef ref(QName); __DAEP__Element__data.NCName = ref;
		//IF LARGER STRINGS ARE REQUIRED (UNICODE MAYBE?) MANAGE THEM BY ANOTHER MEANS.
		assert(ref.size()<64);
	}	
	/**LEGACY
	 * Gets the COLLADA schema type name.
	 */
	inline const daePseudonym &getTypeName()const{ return getMeta()->getName(); }
	
	/**KISS
	 * Implements @c getContents().grow(). It's needed to adjust the iterator.
	 */
	LINKAGE void __grow(daeArray<daeContent> &contents_of_this, size_t minCapacity);
	/**KISS
	 * Implements @c getContents().clear().
	 * @remarks "e->__clear(e->getContents())" is technically
	 * more efficent, but stylistically, getContents().clear() is recommended.
	 * @note @c this==nullptr is accepted, in case of freestanding contents-arrays.	 
	 */
	LINKAGE void __clear(daeArray<daeContent> &contents_of_this)
	SNIPPET( __clear2(static_cast<daeContents&>(contents_of_this),getMeta(),getDocument()); )
	
	/**LEGACY-SUPPORT
	 * Gets the contents-array.
	 * @remarks Pre-2.5 not all elements had contents-arrays.
	 */
	inline daeContents &getContents(){ return getMeta().getContentsWRT(this); }
	/**CONST-FORM, LEGACY-SUPPORT
	 * Gets the contents-array.
	 * @remarks Pre-2.5 not all elements had contents-arrays.
	 */
	inline const daeContents &getContents()const{ return getMeta()->getContentsWRT(this); }		

	/**
	 * @return Returns what @c getChildren().size() would be.
	 */
	NOALIAS_LINKAGE size_t getChildrenCount()const
	SNIPPET( return getMeta()->getChildrenCountWRT(this); )

	/**WARNING, LEGACY 
	 * @warning historically, does NOT clear the input array!
	 * BREAKING-CHANGES: children nodes are added to the results
	 * whether-or-not they appear more than once. Logically this should not be!
	 * And where there are duplicates--if ever--this change preserves them/will not obscure them.
	 *
	 * @param array The return value. An elementref array to append this element's children to.
	 * @return Returns @a result.
	 */
	LINKAGE daeArray<daeElementRef> &getChildren(daeArray<daeElementRef> &result)
	SNIPPET( return getMeta()->getChildrenWRT(this,result); )
	/**CONST-FORM */
	inline daeArray<const_daeElementRef> &getChildren(daeArray<const_daeElementRef> &result)const
	{
		const_cast<daeElement*>(this)->getChildren((daeArray<daeElementRef>&)result); return result;
	}
	
	template<int recursive=0, class T=daeArray<daeElementRef>>
	/**C++98/03-SUPPORT
	 * Implements @c getChildrenByType().	
	 */
	struct _getChildrenByType_f
	{	
		T &mc; daeMeta *meta;		
		template<int> void op(daeElement *ch);
		template<> void op<0>(daeElement *ch){ if(ch->getMeta()==meta) mc.push_back(ch); }
		template<> void op<1>(daeElement *ch){ op<0>(ch); ch->getContents().for_each_child(*this); }
		void operator()(daeElement *ch){ op<recursive>(ch); }
	};	
	template<int op, typename T>
	/** Implements @c getChildrenByType(), etc. */	
	inline void _getChildrenByType_op(daeArray<daeSmartRef<T>> &got)const
	{
		_getChildrenByType_f<op> f = { (daeArray<daeElementRef>&)got,daeGetMeta<T>() };
		const_cast<daeElement*>(this)->getContents().for_each_child(f); 
	}	
	template<typename T>
	/**WARNING, LEGACY, NOT-RECOMMENDED
	 * @warning historically, clears the input array!
	 * @warning mismatched namespaces can result in false positives
	 * @return Returns @a matchingChildren.
	 */	
	inline daeArray<daeSmartRef<T>> &getChildrenByType(daeArray<daeSmartRef<T>> &matchingChildren, enum dae_clear clear=dae_clear)
	{
		if(clear) matchingChildren.clear(); _getChildrenByType_op<0>(matchingChildren); return matchingChildren;
	}	
	template<typename T>
	/**LEGACY-SUPPORT 
	 * Use this if you really want to. It's adapted from @c daeDocument::getElementsByType().
	 * @return Returns a recursive version of @c getChildrenByType() in an unspecified order.
	 */	
	inline daeArray<daeSmartRef<T>> &getDescendantsByType(daeArray<daeSmartRef<T>> &matchingElements, enum dae_clear clear=dae_clear)const
	{
		if(clear) matchingChildren.clear(); _getChildrenByType_op<1>(matchingChildren); return matchingChildren;
	}	
	
	/**LEGACY-SUPPORT */
	struct clone_Suffix : daeArray<daeStringCP,96>
	{
		clone_Suffix():id(nullptr,0),name(nullptr,0){}
		daeHashString id,name; COLLADA_NOINLINE void _suffix(daeElement*);		
	};
	/**LEGACY
	 * Clones copies @c this @c daeElement and all of its descendents.
	 * @param DOM performs cross-DOM copy. Cross-copy is no different.
	 * @return Returns a @c daeElementRef of the copy of this element.
	 * @param idSuffix A string to append to the copied element's ID, if one exists.
	 * @param nameSuffix A string to append to the copied element's name, if one exists.
	 */
	NOALIAS_LINKAGE daeElementRef clone(daeDOM &DOM, clone_Suffix *suffix=nullptr)const;
	/**LEGACY-SUPPORT
	 * Clones copies @c this @c daeElement and all of its descendents.
	 * @return Returns a @c daeElementRef of the copy of this element.
	 * @param idSuffix A string to append to the copied element's ID, if one exists.
	 * @param nameSuffix A string to append to the copied element's name, if one exists.
	 */
	inline daeElementRef clone(clone_Suffix *suffix=nullptr)const
	{
		return clone(*const_cast<daeDOM*>(getDOM()),suffix);
	}	

	/**
	 * Class for reporting info about element comparisons
	 */
	typedef struct compare_Result
	{
		int compareValue; //> 0 if elt1 > elt2,
		//< 0 if elt1 < elt2,
		//= 0 if elt1 = elt2
		const daeElement *elt1,*elt2;
		bool nameMismatch; //true if the names didn't match
		//2.5: std::string was unsafe
		daePseudonym attrMismatch; //The name of the mismatched attribute, or "" if there was no attr mismatch
		bool charDataMismatch; //true if the char data didn't match
		bool childCountMismatch; //true if the number of children didn't match

		daeElement::compare_Result::compare_Result()
		:compareValue(),elt1(),elt2(),nameMismatch(),attrMismatch(""),
		charDataMismatch(),childCountMismatch(){}

		//Write to a string. Use approximately daeArray<daeStringCP,1024>.
		LINKAGE daeArray<daeStringCP> &format(daeArray<daeStringCP> &out);

	}compareResult; //LEGACY
	  
	/**LEGACY
	 * Function for doing a generic, recursive comparison of two XML elements. It
	 * also provides a full element ordering, so that you could store elements in
	 * a map or a set. Return val is > 0 if elt1 > elt2, < 0 if elt1 < elt2, and 0
	 * if elt1 == elt2.
	 */
	static int compare(const daeElement &elt1, const daeElement &elt2)
	{
		return compareWithFullResult(elt1,elt2).compareValue;
	} 
	/**LEGACY
	 * Same as the previous function, but returns a full compare_Result object.
	 */
	NOALIAS_LINKAGE static compare_Result compareWithFullResult(const daeElement &elt1, const daeElement &elt2);

#ifdef BUILDING_COLLADA_DOM

COLLADA_(protected) //domAny.cpp
	
		//2.5: Fall back to domAny if getAttribute fails.
		//TODO: daeMetaElement requires an xs:anyAttribute indicator.
		//TODO: change enum{ xs_anyAttribute_is_still_not_implemented=1 } to 0.
		bool _addAnyAttribute(const daePseudonym&)const;
		//daeElement::clone() calls this instead.
		void _cloneAnyAttribute(const daePseudonym&)const;

#endif //BUILDING_COLLADA_DOM
COLLADA_(public)
	enum{ xs_anyAttribute_is_still_not_implemented=1 };
};

#include "../LINKAGE.HPP" //#undef LINKAGE

//---.
}//<-'

#endif //__COLLADA_DOM__DAE_ELEMENT_H__
/*C1071*/