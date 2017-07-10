/*
 * Copyright 2006 Sony Computer Entertainment Inc.
 *
 * Licensed under the MIT Open Source License, for details please see license.txt or the website
 * http://www.opensource.org/licenses/mit-license.php
 *
 */

#ifndef __COLLADA_DOM__DAE_META_ELEMENT_H__
#define __COLLADA_DOM__DAE_META_ELEMENT_H__

#include "daeMetaAttribute.h"
#include "daeMetaElementAttribute.h"

COLLADA_(namespace)
{//-.
//<-'

#include "../LINKAGE.HPP" //#define LINKAGE

/**SCOPED ENUM */
struct daeContentModel
{
	/**
	 * These values are returned by @c daeMetaElement::getContentModel().
	 */
	enum{ EMPTY=0,SIMPLE,COMPLEX,MIXED };
};

/**
 * Each instance of the @c daeMetaElement class describes a C++ COLLADA DOM
 * element type.
 * @par
 * The meta information in @c daeMetaElement is a combination of the information
 * required to create and maintain C++ object instances and
 * the information necessary to parse and construct a hierarchy of COLLADA
 * elements.
 * @par
 * @c daeMetaElement objects also act as factories for C++ COLLADA dom classes where
 * each @c daeElement is capable of creating an instance of the class it describes.
 * Further, each @c daeMetaElement contains references to other @c daeMetaElements
 * for potential XML children elements.  This enables this system to easily
 * create @c daeElements of the appropriate type while navigating through XML
 * recursive parse.
 * @par
 * See @c daeElement for information about the functionality that every @c daeElement implements.
 */
class daeMetaElement : public daeMetaObject
{
COLLADA_(public) //OPERATORS

	COLLADA_DOM_OBJECT_OPERATORS(daeMetaElement)

	bool operator==(daeMeta &cmp)const{ return this==&cmp; }
	bool operator!=(daeMeta &cmp)const{ return this!=&cmp; }

COLLADA_(public) //ACCESSORS
	/**
	 * 2.5: holds schema namespace and "atomic" types.
	 */
	inline const XS::Schema &getSchema()const{ return *_schema; }
																   
	/**
	 * Gets the daeElement::getElementType() constant.
	 */
	inline int getElementType()const{ return getModel().__DAEP__Model__genus; }

	/**
	 * Gets the copied @c DAEP::Schema::__DAEP__Schema__Traits.
	 */
	NOALIAS_LINKAGE int _getDSTs()const
	SNIPPET( return (int&)_DAEP_Schema; )

	/**
	 * Gets the XML Schema content-model type.
	 * @return Returns one of the 4 @c daeContentModel @c enum, from 0 to 3.
	 */
	inline int getContentModel()const
	{	
		return _getDSTs()&DAEP::Schema<>::__DAEP__Schema__g1__content_model; 
	}
	/**LEGACY
	 * Previously "getIsInnerClass."
	 * Tells if elements of this type use an inner class (a local XSD type.)
	 */
	inline bool getIsLocal()const
	{
		return 0!=(_getDSTs()&DAEP::Schema<>::__DAEP__Schema__g1__is_local); 
	}
	/**LEGACY
	 * Tells if elements of this type can be placed in the object model.
	 */
	inline bool getIsAbstract()const
	{
		return 0!=(_getDSTs()&DAEP::Schema<>::__DAEP__Schema__g1__is_abstract); 
	}
	/**LEGACY
	 * Previously "getIsTransparent."
	 * Tells if elements of this type should have an element tag printed when saving.
	 */
	inline bool getIsGroup()const
	{ 
		return 0!=(_getDSTs()&DAEP::Schema<>::__DAEP__Schema__g1__is_group); 
	}
	/**LEGACY
	 * Tells if elements of this type allow for any element as a child.
	 */
	inline bool getAllowsAny()const
	{
		return 0!=(_getDSTs()&DAEP::Schema<>::__DAEP__Schema__g1__allows_any); 
	}
	/**LEGACY-SUPPORT
	 * Tells if elements of this type allow for any attribute.
	 */
	inline bool getAllowsAnyAttribute()const
	{
		return 0!=(_getDSTs()&DAEP::Schema<>::__DAEP__Schema__g1__allows_any_attribute); 
	}
	/**LEGACY-SUPPORT
	 * Tell if elements of this type use the <xs:all> virtual content model.
	 */
	inline bool getIsAll()const
	{
		return 0!=(_getDSTs()&DAEP::Schema<>::__DAEP__Schema__g1__is_all); 
	}

	template<class T>
	/**LEGACY-SUPPORT
	 * Previously "getMetaCMPolicy::findChild."
	 * Gets the daeMetaElement of an acceptable child of this 
	 * content-model object.	 
	 * @note The library doesn't (yet) support QNames properly.
	 * Some children names are NCNames with a colon, or a pseudo QName.
	 */
	inline daeMeta *findChild(const T &pseudonym)const
	{
		return _findChild2(daeBoundaryString2<T>::type(pseudonym));
	}		
	/** Implements @c findChild(). */
	NOALIAS_LINKAGE daeMeta *_findChild2(daeString)const;
	/** Implements @c findChild(). */
	NOALIAS_LINKAGE daeMeta *_findChild2(const daeHashString&)const;	 
	/** Implements @c findChild2(). */
	template<class T> inline daeMeta *_findChild3(const T&)const;

	/**LEGACY
	 * Previously "getValueAttribute".
	 * Gets the @c XS::Attribute for the non-element contents of a @c daeElement.
	 * @return Returns the @c XS::Attribute pointer for the non-element contents of
	 * this element type.
	 */
	NOALIAS_LINKAGE daeCharData *getValue()const
	SNIPPET( return _value; )
	
	/**LEGACY
	 * Previously "getIDAttribute."
	 * Gets the @c XS::Attribute for the "id" attribute of a @c daeElement.
	 * @return Returns the "id" @c XS::Attribute, or @c nullptr if the element type
	 * does not have an "id" attribute.
	 */
	NOALIAS_LINKAGE daeAttribute *getID_id()const 
	SNIPPET( return _IDs_id; )

	/**LEGACY-SUPPORT
	 * This is a new, equal opportunity system, post-2.5.
	 * @see @c daeMetaAttribute::getNextID()
	 * @see @c XS::Schema::getIDs();
	 */
	NOALIAS_LINKAGE daeAttribute *getFirstID()const 
	SNIPPET( return _IDs; )

	/**
	 * Previously "getMetaAttributes."
	 * Gets the array of all known attributes on this element type.
	 * This includes all meta attributes except those describing child
	 * elements. It does include the value element.
	 * @return Returns the array of @c daeMetaAttributeRefs.
	 */
	NOALIAS_LINKAGE const daeArray<daeAttribute> &getAttributes()const
	SNIPPET( return (const daeArray<daeAttribute>&)_attribs; )

	template<class T>
	/**LEGACY
	 * Previously "getMetaAttribute."
	 * Gets the attribute named @a NCName. 
	 * @note The library doesn't (yet) support QNames properly.
	 * Some attribute names are NCNames with a colon, or a pseudo QName.
	 */
	inline daeAttribute *getAttribute(const T &pseudonym)const
	{
		return _getAttribute(typename daeBoundaryString2<T>::type(pseudonym));
	} 
	/** Implements @c getAttribute(). */
	NOALIAS_LINKAGE daeAttribute *_getAttribute(daeString pseudonym)const
	SNIPPET( return getAttribute(daeHashString(pseudonym)); )
	/** Implements @c getAttribute(). */
	NOALIAS_LINKAGE daeAttribute *_getAttribute(const daeHashString &pseudonym)const;	

	/**LEGACY, NOT-RECOMMENDED
	 * THERE'S NO REASON TO USE THIS RIGHT NOW. IT COULD BE USED TO WALK THE
	 * SCHEMA; BUT THERE'S NEVER BEEN A WALKING API.
	 * Gets the root of the content-model policy tree.
	 * @return Returns @c *jumpIntoTOC(0) if there is no content-model per se.
	 * (This semi-dummy @c daeCM is always available.)
	 */
	NOALIAS_LINKAGE const daeCM &getCMRoot()const
	SNIPPET( return *_CMRoot; )
	/**LEGACY-SUPPORT
	 * @return Returns a CM node that may eliminate redundant nodes surrounding
	 * the root returned by @c getCMRoot(). In the case of <xs:group> this can
	 * even be a CM node from the group's ref's CM graph.
	 */
	NOALIAS_LINKAGE const daeCM &getCMEntree()const
	SNIPPET( return *_CMEntree; )

COLLADA_(public) //CONTENT-MODEL	
	/**
	 * Gets the "Table-of-Contents."
	 * The child-IDs are in ascending order.
	 * These are positioned in front of the contents-arrays.
	 *
	 * This is a @c daeArray of @c XS::Element. It isn't necessarily all
	 * of the xs:elements. There is only one per unique child-ID, or the
	 * names of the element tags that make up the content-model metadata.
	 * Tags can also be unnamed, in which case the 0 child-ID is held in
	 * common. This is all to do with COLLADA-DOM's databinding facility.
	 */
	NOALIAS_LINKAGE daeTOC &getTOC()const
	SNIPPET( return (const daeArray<const XS::Element>&)_elems; )

	//TODO: THIS SHOULD BE INLINE IF IT'S GOING TO GET USED.
	//TODO: THIS SHOULD BE INLINE IF IT'S GOING TO GET USED.
	//TODO: THIS SHOULD BE INLINE IF IT'S GOING TO GET USED.
	/**
	 * @c jumpIntoTOC() returns the "TOC" array element having the 0th child-ID.
	 * All elements have a 0th child ID, even though it typically won't see use.
	 * No elements have a 1st child ID, but if there is a 2nd, then there's a 1.
	 * The idea is to index TOC with static @c DAEP::Child and @c dae_Array IDs.
	 */
	NOALIAS_LINKAGE XS::Element &_jumpIntoTOC(daeOffset i)
	SNIPPET( XS::Element *o = _elem0+i; _elems[o-_elems.begin()]; return *o; ) //assert(in-bounds).
	template<class T>
	/**OVERLOAD
	 * @c jumpIntoTOC() returns the "TOC" array element having the 0th child-ID.
	 * This overload offsets the pointer to produce a child ID with a "name" greater-or-less-than-0.
	 */
	inline daeTOC::Atom &jumpIntoTOC(const T &i)const
	{
		return const_cast<daeMetaElement*>(this)->_jumpIntoTOC(i); 
	}
	//template<> //Not worth moving into namespace for GCC/C++.
	/**TEMPLATE-SPECIALIZATION, OVERLOAD
	 * In case there is any confusion, this specialization explicitly converts a child-ID into its name.
	 */
	inline daeTOC::Atom &jumpIntoTOC(const daeChildID &i)const
	{
		return const_cast<daeMetaElement*>(this)->_jumpIntoTOC(i.getName()); 
	}

	/**
	 * Gets the @c sizeof(char) offset (from @ this) where the contents-array's storage is
	 * found in its container element class.
	 * @return Returns the @c sizeof(char) offset from the element's @c this pointer.
	 */
	NOALIAS_LINKAGE daeOffset getContentsOffset()const
	SNIPPET( return _content_offset; )

COLLADA_(public) //daeContent.h "WID" APIs (With-ID)	
	/**
	 * Adds an element to the contents-array. It will be constructed according to the
	 * ID, and its name will be that of the ID's. (It will be a default in every way.)
	 */
	LINKAGE static const daeChildRef<> &_addWID(daeChildID,daeContents&,daeContent*_=nullptr);

	template<class U> //See daeContents::__cursorize().
	/**
	 * Adds an element to the contents-array. This is identical to @c _addWID(), except
	 * that an element is provided rather than constructed from nothing. The provided element
	 * may already belong to a contents-array or may be @c nullptr.
	 * @see @c _addWID() Doxygentation.
	 */
	inline static const daeChildRef<> &_setWID(daeChildID ID, const U &child, daeContents &c, daeContent *it=nullptr)
	{
		//& is used to invoke the DAEP::Element conversion
		//to a daeElement& in order to behave like xs::any.
		const daeElement &upcast = *child; 
		void *_ = daeContents::__cursorize(child);
		return _==&upcast?_setWID2(ID,(daeElement*)&upcast,c,it):_setWID2(ID,*(daeContent*)_,c,it);
	}
	/** Implements @c _setWID(). */
	LINKAGE static const daeChildRef<> &_setWID2(daeChildID,daeElement*,daeContents&,daeContent*);
	/** Implements @c _setWID(). */
	LINKAGE static const daeChildRef<> &_setWID2(daeChildID,daeContent&,daeContents&,daeContent*);
	/** Implements @c _setWID2(). */
	template<class S> static inline void _setWID3(daeChildID,S&,daeContents&,daeContent*&);

	/**
	 * Removes a child by converting it into an empty text-node. Setting a child to be
	 * @c nullptr does not remove it. That is a transient state. Setting it to "" does
	 * remove it, although it leaves behind a degenerate text-node. These can be taken
	 * care of together as a post-step, or left behind. It doesn't particularly matter.
	 * If left behind, there is no natural process that will recycle the "" text nodes.
	 */
	LINKAGE static void _removeWID(daeContents&,daeContent*);

	template<bool Branch>
	/**
	 * Sets the number of elements named @a ID.getName() to @a ID.getIndex().
	 * This can be thought of as either adding placeholders to pad the count,
	 * -or removing high-index elements in order to truncate the count.
	 */
	inline static void _resizeWID(daeChildID ID, daeContents &c, daeCounter size)
	{
		if(!Branch||ID.getIndex()!=size) _resizeWID2(c,ID,daeChildID(ID.getName(),size));
	}
	/** Implements _resizeWID(). */
	LINKAGE static void _resizeWID2(daeContents&,daeChildID::POD,daeChildID::POD);

COLLADA_(public) //daeElement "WRT" APIs (with-respect-to)
	/**WARNING
	 * 2.5 adds this API that is geared toward document loading code.
	 * It may come in other variations, but in its basic form, it is
	 * designed to add the element to the back of the children, with
	 * an appropriate ordinal. If there is no such ordinal, then the
	 * ordinal 0 is used. 
	 *
	 * If @c DAE_ORDER_IS_NOT_PRESERVED would be returned, a comment
	 * or text on the back of the contents-array will be moved so it
	 * is in front of the child, as if the child was placed onto the
	 * back.
	 *
	 * @note The "back" is not necessarily the last child, since the
	 * back must be ceded to 0 ordinal children. The prior paragraph
	 * also explains how @c DAE_ORDER_IS_NOT_PRESERVED can result in
	 * non-back placements. 
	 *
	 * @return Returns a nonzero child-ref.
	 *
	 * @warning This API alone assigns extra-schema elements unnamed
	 * status. This marks these elements as invalid schema-wise. The
	 * other APIs don't do this; either because they are legacy with
	 * hard error requirements, or are required to fill @c dae_Array.
	 * (@c dae_Array indices must always produce identical results.)
	 */
	LINKAGE const daeChildRef<> &pushBackWRT(daePseudoElement*, const daePseudonym&)const;

	/**WARNING, LEGACY
	 * Previously "create," second overload form.
	 * POST-2.5 THIS WON'T CREATE INSTANCES OF @c this @c daeMetaElement.
	 * POST-2.5 @a parent becomes the parent of the created element, but
	 * it's not yet among the children. This avoids parenting to the DOM.
	 * 
	 * Looks through the list of potential child elements
	 * for this metadata finding the corresponding metadata; if metadata
	 * is found, return an instance of it.
	 * Typically "place" is called after "create(child_element_name)."
	 *
	 * @warning This is the 2.5 entrypoint to the legacy creation/placement
	 * APIs, including: @c placeWRT(), @c placeAfterWRT() & @c placeBeforeWRT(),
	 * -all below.
	 * First the element is created (via the parent) and then it is placed.
	 * This is not identical to the pre-2.5 arrangement, but it's very close.
	 * (Typically, client code would have been heavily reliant on the string-
	 * based, @c daeElement::add() family of APIs, that call on these by proxy.)
	 *
	 * @param pseudonym is the child's name. Historically it's not really a QName.
	 * (And presently) the library doesn't differentiate between NCNames and QNames. 
	 * (COLLADA 1.5.0 has at least one QName: <formula><technique_common><math:math>.)
	 *
	 * @return Historically this API returns @c nullptr if @a pseudonym is not matched.
	 * Therefore it cannot be used to create unnamed children, unless @c getAllowsAny().
	 */
	LINKAGE daeElementRef createWRT(daePseudoElement *parent, const daePseudonym &pseudonym)const;
					 
	template<class U> //See daeContents::__cursorize().
	/**WARNING, LEGACY
	 * Previously "place."
	 * Places a child element into the @a parent element where the
	 * calling object is the @c daeMetaElement for the parent element.
	 *
	 * @param parent Element to act as the container.
	 * @param child Child element to place in the parent.
	 * @return Returns true if the operation was successful, false otherwise.
	 * @warning @a child CANNOT BE A NULLPTR-PLACEHOLDER NOR NULLPTR-POINTER. 
	 */
	inline daeOK placeWRT(daePseudoElement *parent, const U &child)const
	{
		daeElement *upcast = *child; 
		void *_ = daeContents::__cursorize(child);
		return _==upcast?_placeWRT(*parent,*upcast):_placeWRT(*parent,*(daeContent*)_);
	}		
	/**LEGACY-SUPPORT
	 * Implements @c placeWRT().
	 */
	LINKAGE daeError _placeWRT(daeElement&,daeElement&)const;	
	/**LEGACY-SUPPORT
	 * Implements @c placeWRT().
	 */
	LINKAGE daeError _placeWRT(daeElement&,daeContent&)const;

	template<class T, class U> //See daeContents::__cursorize().
	/**WARNING, LEGACY
	 * Previously "placeAfter."
	 * Places a child element into @a parent not before @c after.
	 *
	 * @warning <xs:all> returns @c DAE_ORDER_IS_NOT_PRESERVED, which 
	 * should be handled by the caller. @c daeOK won't convert this to 
	 * @c true. @c daeOK is not @c daeError to support legacy user code.
	 *
	 * @warning @a child CANNOT BE A NULLPTR-PLACEHOLDER NOR NULLPTR-POINTER. 
	 *
	 * @param after The element location in the contents array to insert after.
	 * @param parent Element to act as the container.
	 * @param child Child element to place in the parent.
	 * @return Returns @c DAE_ORDER_IS_NOT_PRESERVED or @c DAE_OK if successful.
	 */
	inline daeOK placeAfterWRT(const T &after, daePseudoElement *parent, const U &child)const
	{
		//Note: may want to implement with _placeAfterWRT() at some point.
		return placeBetweenWRT(parent,after,child,-1);
	}

	template<class T, class U> //See daeContents::__cursorize().
	/**WARNING, LEGACY
	 * Previously "placeBefore."
	 * Places a child element into @a parent not after @c before.
	 *
	 * @warning <xs:all> returns @c DAE_ORDER_IS_NOT_PRESERVED, which 
	 * should be handled by the caller. @c daeOK won't convert this to 
	 * @c true. @c daeOK is not @c daeError to support legacy user code.
	 *
	 * @warning @a child CANNOT BE A NULLPTR-PLACEHOLDER NOR NULLPTR-POINTER. 
	 *
	 * @param before The element location in the contents array to insert before.
	 * @param parent Element to act as the container.
	 * @param child Child element to place in the parent.
	 * @return Returns @c DAE_ORDER_IS_NOT_PRESERVED or @c DAE_OK if successful.
	 */
	inline daeOK placeBeforeWRT(const T &before, daePseudoElement *parent, const U &child)const
	{
		//Note: may want to implement with _placeBeforeWRT() at some point.
		return placeBetweenWRT(parent,-1,child,before);
	}

	template<class S, class T, class U> //See daeContents::__cursorize().
	/**WARNING, LEGACY-SUPPORT
	 * @warning Care must be taken with @a before and @a after so not
	 * to shift the contents-array as a side-effect of obtaining them.
	 *
	 * @warning <xs:all> returns @c DAE_ORDER_IS_NOT_PRESERVED, which 
	 * should be handled by the caller. @c daeOK won't convert this to 
	 * @c true. @c daeOK is used only to be consistent with legacy APIs.
	 *
	 * @warning @a child CANNOT BE A NULLPTR-PLACEHOLDER NOR NULLPTR-POINTER. 
	 *
	 * @note: This API is recommended for working with large/complex content.
	 * The user/client just has to "know" where their schema is likely to be
	 * slow going.
	 *
	 * @note: In the editor context, clients should use this API exclusively.
	 * It restricts the insertion to be between two existing elements and/or
	 * text nodes. 
	 *
	 * Places @a child into @a parent not before @c after, nor after @c before.
	 * @return Returns @c DAE_ORDER_IS_NOT_PRESERVED or @c DAE_OK if successful.
	 */
	inline daeOK placeBetweenWRT(daePseudoElement *parent, const S &after, const U &child, const T &before)const
	{
		daeElement *upcast = *child; 
		void *_ = daeContents::__cursorize(child);
		void *a = daeContents::__cursorize(after), *b = daeContents::__cursorize(before);
		return _==upcast?_placeBetweenWRT(*parent,a,*upcast,b):_placeBetweenWRT(*parent,a,*(daeContent*)_,b);
	}
	/**LEGACY-SUPPORT
	 * Implements @c placeBetweenWRT(), @c placeAfterWRT() & @c placeBeforeWRT().
	 */
	LINKAGE daeError _placeBetweenWRT(daeElement&,void*,daeElement&,void*)const;
	/**LEGACY-SUPPORT
	 * Implements @c placeBetweenWRT(), @c placeAfterWRT() & @c placeBeforeWRT().
	 */
	LINKAGE daeError _placeBetweenWRT(daeElement&,void*,daeContent&,void*)const;

	template<class U> //See daeContents::__cursorize().
	/**WARNING, LEGACY
	 * Previously "remove."
	 * Removes a child element from its parent element.
	 * @param parent Element That is the parent.
	 * @param child Child element to remove.
	 *
	 * @return Returns @c DAE_ORDER_IS_NOT_PRESERVED or @c DAE_OK if successful.
	 * @warning THIS API WILL ACCEPT NULLPTR-PLACEHOLDERS, AND REMOVE THEM FROM
	 * THE CONTENTS-ARRAY; STILL THIS PRACTICE IS DISCOURAGED FOR ITS ASYMMETRY.
	 *
	 * @remarks The library does not currently envision constrained removals of
	 * <xs:choice> based children. This API is unconstrained in any case. Where
	 * code expects a @c bool return type, it will report @c false if reordered.
	 */
	inline daeOK removeWRT(daePseudoElement *parent, const U &child)const
	{
		daeElement *upcast = *child; 
		void *_ = daeContents::__cursorize(child);
		return _==upcast?_removeWRT(*parent,*upcast):_removeWRT(*parent,*(daeContent*)_);
	}	
	/**LEGACY-SUPPORT
	 * Implements @c removeWRT().
	 */
	LINKAGE daeError _removeWRT(daeElement&,daeElement&)const;
	/**LEGACY-SUPPORT
	 * Implements @c removeWRT().
	 */
	LINKAGE daeError _removeWRT(daeElement&,daeContent&)const;
	
	template<class T> //DAEP::Element
	/**
	 * Previously "getContents."
	 * Gets the contents-array of @a e.
	 */
	inline typename daeConstOf<T,daeContents>::type &getContentsWRT(T &e)const
	{
		const daePseudoElement *upcast = e;	(void)upcast;
		return daeOpaque(e)[getContentsOffset()];
	}
	template<class This> //DAEP::Element
	/**OVERLOAD
	 * Previously "getContents."
	 * Gets the contents-array of @a e.
	 * This overload is just to receive @c this pointers/coceivably other rvalues.
	 */
	inline typename daeConstOf<This,daeContents>::type &getContentsWRT(This *e)const
	{
		const daePseudoElement *upcast = e; (void)upcast;
		return daeOpaque(e)[getContentsOffset()];
	}

	template<class E>
	/**C++98/03-SUPPORT
	 * Implements @c getChildrenWRT().	
	 */
	struct _getChildrenWRT_f
	{	
		daeArray<daeSmartRef<E>> &result;
		void operator()(E *ch){ if(ch!=nullptr) result.push_back(ch); } 
	};
	/**LEGACY
	 * Previously "getChildren."
	 * Gets all of the children from an element of this type.
	 * @param parent The element that you want to get the children from.
	 * @param array The return value.  An elementref array to append this element's children to.
	 */
	inline daeArray<daeElementRef> &getChildrenWRT(daePseudoElement *parent, daeArray<daeElementRef> &result)const
	{
		_getChildrenWRT_f<daeElement> f = {result};		
		getContentsWRT(parent).for_each_child(f); return result;
	}
	/**LEGACY, CONST-PROPOGATING-FORM
	 * Previously "getChildren."
	 * Gets all of the children from an element of this type.
	 * @param parent The element that you want to get the children from.
	 * @param array The return value.  An elementref array to append this element's children to.
	 */
	inline daeArray<const_daeElementRef> &getChildrenWRT(const daePseudoElement *parent, daeArray<const_daeElementRef> &result)const
	{
		_getChildrenWRT_f<const daeElement> f = {result};
		getContentsWRT(parent).for_each_child(f); return result;
	}

	/**C++98/03-SUPPORT 
	 * Implements @c getChildrenCountWRT().	
	 */
	struct _getChildrenCountWRT_f
	{	
		size_t count;
		void operator()(const daeElement *ch){ if(ch!=nullptr) count++; }
	};
	/**LEGACY-SUPPORT
	 * @return Returns what @c getChildrenWRT().size() would be.
	 */
	inline size_t getChildrenCountWRT(const daePseudoElement *parent)const
	{
		_getChildrenCountWRT_f f = {0};
		getContentsWRT(parent).for_each_child(f); return f.count;
	}
	
COLLADA_(private) //GENERATOR-SIDE APIs
	
	friend class domAny;
	template<class T, class> friend class DAEP::Elemental;

	template<class T, int M, int N> 
	/**GENERATOR-SIDE API
	 * Appends a @c XS::Attribute that represents a field corresponding to an
	 * XML attribute to the C++ version of this element type.
	 */
	inline XS::Attribute &addAttribute(T &nul, daeClientStringCP (&typeQName)[M], daeClientStringCP (&pseudonym)[N])
	{
		typedef typename T::underlying_type VT;
		//Not pretty. Quick fix to build with GCC/C++.
		//typedef typename XS::List::Item<VT>::type atom;
		typedef COLLADA_NCOMPLETE(N) XS::List::template Item<VT>::type atom;
		typedef daeAlloc<> &(*lt_void)(daeAllocThunk&);
		typedef daeAlloc<atom> &(*lt_atom)(daeAllocThunk&);
		XS::Attribute &o = _addAttribute(nul.feature(),nul.offset(),
		(lt_void)static_cast<lt_atom> //Want to be sure lt/ltT are correct.
		(daeAlloc<atom>::locateThunk),typeQName,N<=1?daeHashString(nullptr,0):pseudonym);
		//new(o.getType().value) VT();
		_maybe_prototype<VT>(o); return o;
	}
	template<class T, int M> 
	/**GENERATOR-SIDE API, SHADOWING
	 * Appends a @c daeValue that represents a field corresponding to an
	 * XML complexType's "value" pseduo-attribute to the C++ version of this element type.
	 */
	inline daeValue &addValue(T &nul, daeClientStringCP (&typeQName)[M]){ return addAttribute(nul,typeQName,""); }

	template<class T> //XS::Any, XS::Choice, XS::Element, XS::Group, or XS::Sequence
	/**GENERATOR API
	 * This is so @c new is not used by generators to construct @c daeCM_base based classes.
	 *
	 * @param parentCM is subtly important to pay attention to. If @c nullptr then the new CM
	 * will be returned by @c getCMRoot(). Or IOW it's the root of the content-model. Otherwise,
	 * -@a parentCM must be an xs:sequence or xs:choice type, as they are the only list-based CMs.
	 * (There's also xs:all; however the library doesn't implement "XS::All." It will at some point.)
	 *
	 * @note In practice the default arguments are pointless, as this is called by generators.
	 * (They were historically part of the "daeMetaCMPolicy" based classes, now called @c daeCM.)
	 */	
	T &addCM(daeCM* &push, daeCounter ordSubtrahend, int minOccurs=1, int maxOccurs=1)
	{
		daeParentCM *pop = (daeParentCM*)push;		
		T &out = static_cast<T&>(_addCM(T::__daeCM__XS_enum,pop,ordSubtrahend,minOccurs,maxOccurs));
		if(T::__daeCM__XS_enum!=XS::ELEMENT&&T::__daeCM__XS_enum!=XS::ANY)
		push = &out; return out;
		//UNVETTED
		//Before removing this, double-check that pushBackWRT() handles DAE_ORDER_IS_NOT_PRESERVED
		//and vet XS::All (just in general.)
		daeCTC<T::__daeCM__XS_enum!=XS::ALL>();		
	}

	template<class T> //XS::Any, XS::Choice, XS::Element, XS::Group, or XS::Sequence
	/**GENERATOR API, TODO
	 * PartialCMs are top-level choices, that are generated to aid the intelligent insertion
	 * APIs. COLLADA's schema do not require any. The generator requires some work also, but
	 * it should complain if its inputs require partials to be outputted.
	 *
	 * @todo THIS API SHOULD PROBABLY RAISE A "_partial" FLAG ON THE OUTGOING @c daeCM. THIS
	 * WAY IF THE USER EVER WALKS THE CONTENT-MODEL, THEY CAN TELL WHICH NODES ARE SYNTHETIC.
	 */	
	T &addPartialCM(daeCM* &push, daeCounter ordSubtrahend, int minOccurs=1, int maxOccurs=1)
	{
		T &out = addCM<T>(push,ordSubtrahend,minOccurs,maxOccurs); 
		/*out._partial = true;*/ return out;
	}
	
	//This is merged into the "pop" function so generators can ouptut less text.
	template<unsigned long long N, unsigned maxO>
	/**GENERATOR-SIDE API, OVERLOAD
	 * Previously "daeCMPolicy::setMaxOrdinal()."
	 *
	 * @warning THIS IS FOR INNER "COMPOSITORS." OUTER COMPOSITORS SHOULD USE THE 
	 * @c addContentModel() API THAT RECEIVES @c DAEP::Elemental::TOC.
	 *
	 * Sets the maximum ordinal count of this policy object's children. 
	 * @tparam N The maximum ordinal count for this content-model object.
	 * @tparam maxO The maxOccurs attribute to be multiplied by @a ord to determine
	 * if the resulting ordinals will fit into the 24-bit limit. T
	 * This also promotes the numbers to 64-bit so that the product cannot rollover.
	 *
	 * @note the 24-bit limit has to do with @c daeContent. If higher, then the
	 * content is taken to be part of a text-node.

	 * @remark This actually sets a count of ordinals, which is 1 more than the
	 * the maximum ordinal. The final ordinal is reserved for unnamed child IDs.
	 */
	inline void popCM(daeCM* &pop)
	{
		_popCM<N,maxO,maxO>(pop); //FYI: C++98 cannot defeault the 3rd parameter.
	}
	template<unsigned long long N, unsigned maxO, class T> //DAEP::Elemental TOC
	/**GENERATOR-SIDE API, OVERLOAD
	 * Previously "daeCMPolicy::setMaxOrdinal()."
	 * In addition to calling @c popCM() this API precomputes placement algorithm 
	 * data-points.
	 *
	 * @warning THIS IS FOR OUTER "COMPOSITORS." INNER COMPOSITORS SHOULD USE THE 
	 * OTHER OVERLOAD. THIS FORM RELAXES THE ORDINAL REQUIREMENTS. METADATA WOULD
	 * BE MALFORMED IF THIS FORM IS USED FOR INNER COMPOSITORS.
	 *
	 * @remarks It might be useful to have a template for overriding the unbounded
	 * ordinals via specialization. It's not clear how to do that with the non-TOC
	 * form. The outer compositor doesn't have to fit into the ordinal value space
	 * since it just runs to the end. Except for groups, which must be transcluded 
	 * into other elements. A value (100) is used to ensure that the ordinal space
	 * isn't too tight of a fit for CMs that seem to have a manually set maxOccurs
	 * value that isn't just arbitrarily hight, or at least there is room for some
	 * content, up to 100 times.
	 * IN GENERAL, IF ELEMENTS EVER EXCEED THE ORDINAL-SPACE, THE LIBRARY NEEDS TO
	 * FLAG THE DOCUMENT, SO TO PREVENT IT FROM BEING OVERWRITTEN BY AN INCOMPLETE
	 * VERSION OF ITSELF.
	 */
	inline void addContentModel(daeCM* &x, T *toc)
	{
		enum{ relaxO=maxO>100&&toc->__DAEP__Schema__g1__is_group==0?100:maxO };
		daeCM *root = x; _popCM<N,maxO,relaxO>(x); _addContentModel(root);
	}	
	template<unsigned long long N, unsigned maxO, unsigned _relax_maxO_CTC/*=maxO*/>
	/**Implements @c popCM() & @c addContentModel()  */
	inline void _popCM(daeCM* &pop)
	{
		//If this isn't compiling, there has to be a discussion about how to support 
		//such a schema. Unfortunately "unbounded" is hard to deal with with ordinals.
		pop->_maxOrdinals = N; daeCTC<(N*_relax_maxO_CTC<=daeOrdinals)>();
		pop->_maxOrdinals_x_maxOccurs = std::min<daeCounter>(daeOrdinals,N*maxO);
		pop = pop->_parentCM;
	}

COLLADA_(private) //MODEL SETUP IMPLEMENTATION

	template<class VT, class T> 
	/**OVERLOAD Implements newPrototype(). */
	inline void _maybe_prototype2(XS::Attribute &o, daeArray<T,0> *a)
	{
		/*NOP*/ //Don't overwrite the offsetted thunk.
		if(sizeof(VT)!=sizeof(void*)) //Derived/unembedded?
		{
			new((void*)&o.getType().value) VT(*a->getAU()); 
		}
	}
	template<class VT, class T, int N> 
	/**OVERLOAD Implements newPrototype(). */
	inline void _maybe_prototype2(XS::Attribute &o, daeArray<T,N> *a)
	{
		const char *value = &o.getType().value;
		const char *prototype = value-o.getOffset()-daeOffsetOf(VT,getAU());
		new((void*)value) VT((daeObject*)prototype);
		((daeAllocThunk*)((VT*)value)->getAU())->_prototype = &o.getType(); //SKETCHY
	}
	template<class> 
	/**OVERLOAD Implements newPrototype(). */
	inline void _maybe_prototype2(XS::Attribute&,daeStringRef*)
	{
		/*NOP*/ //Don't call the LEGACY nullptr default-constructor.
	}
	template<class VT> 
	/**OVERLOAD Implements newPrototype(). */
	inline void _maybe_prototype2(XS::Attribute &o,...)
	{
		//This is initializing _prototype. This constructor is not object aware.
		new((void*)&o.getType().value) VT(); 
	}
	template<class VT> 
	/**
	 * Filters out @c daeArray & @c daeStringRef based values in @c _addAttribute(). 
	 */
	inline void _maybe_prototype(XS::Attribute &o)
	{
		_maybe_prototype2<VT>(o,(VT*)nullptr); 
		if(!daeArrayAware<VT>::is_plain)
		o._destructor = __prototype_destruct<VT>;
	}	
	template<class T> 
	/**
	 * This is for destructing the prototypes, since there's no clean way to
	 * call the full constructor on them, in order to install the destructor.
	 */
	static void __prototype_destruct(void *a){ ((T*)a)->~T(); }

	template<class T> 
	/**
	 * Instantiate the compiler's generated default constructor as function pointer.
	 */
	static void __placement_new(DAEP::Object *e)
	{			
		new(e) typename daeTypic<T::__DAEP__Schema__g1__is_abstract!=0,__abstract_1,T>::type;
	}
	/**C++98/03 support */
	struct __abstract_1{ __abstract_1(){ assert(!"Constructing abstract type?!"); } };
	
	friend class XS::Schema; template<class T>
	/** Stage 2 of initialization-sequence. */
	T *_continue_XS_Schema_addElement(typename T::Essentials &ee)
	{	
		T *pt = (T*)_continue_XS_Schema_addElement2
		(__placement_new<T>,ee.union_feature,ee.content_offset);
		assert(getElementType()==T::__DAEP__Schema__genus
		&&getAttributes().size()==T::__DAEP__Schema__extent_of_attributes);		
		//This cannot get the virtual-destructor, but it's
		//initializing the process-share, and gets the model.
		//Extracting the destructor is a messy internal detail. 
		new(pt) DAEP::Elemental<T>(DAEP::PTYPE); return pt;
	}	
	/** Stage 3 of initialization-sequence; Returning @c _prototype. */
	LINKAGE void *_continue_XS_Schema_addElement2(void(DAEP::Object*),daeFeatureID,daeOffset);

	/** Implements @c addCM(). */	
	LINKAGE daeCM &_addCM(daeXS,daeParentCM*,daeCounter,int,int);
	/** Implements @c addContentModel(). */	
	LINKAGE void _addContentModel(const daeCM*);

	friend class XS::Group;
	friend class XS::Element; 
	/** Implements @c XS::Element::setChild() & XS::Group::addChild(). */	
	XS::Element &_addChild(const XS::Element&,daeFeatureID,daeOffset,daePseudonym);
	/** Adds the default/empty CM child. */	
	XS::Element &_addChild2(XS::Element *out, int i, daeOffset os);

	/** Implements @c addAttribute(). */	
	LINKAGE XS::Attribute &_addAttribute(daeFeatureID,daeOffset,daeAlloc<>&(daeAllocThunk&),daeHashString,daePseudonym);
	
#if defined(BUILDING_COLLADA_DOM) || defined(__INTELLISENSE__)

		/** @c addAttribute() and @c daeElement::_cloneAnyAttribute() share this. */	
		void _addAttribute_maybe_addID(daeAttribute &maybe_ID, const daeElement *prototype_or_domAny);
		//SCHEDULED FOR REMOVAL
		//This must remap _IDs and _IDs_id if the _attribs vector is reallocated.
		XS::Attribute &_anyAttribute_maybe_addID(const daeElement *prototype_or_domAny);

		friend class daeDOM;
		friend class daeModel;
		friend class daeElement;

COLLADA_(protected) //INVISIBLE
		/**
		 * Features above this are considered trivial.
		 * This is for elements, since features after the attributes
		 * and value can be safely ignored.
		 */
		daeFeatureID _feature_completeID;

		/**
		 * Bitwise flags/fields reserved for use by elements.
		 */
		unsigned long long _DAEP_Schema;
		  													
		/**
		 * This is a function-pointer that placement-new constructs the
		 * object. These are statically generated during model creation.
		 */
		void (*_constructor)(DAEP::Object*);
		/**
		 * Constructs/initiailizes via @c _prototype and @c _constructor.
		 * @note Constructors of data with ownership semantics must copy
		 * the data supplied by @c _prototype into their current context.
		 */
		void _construct(DAEP::Object *e, DAEP::Object &parent, daeDOM&)const;
		/**
		 * This is a subroutine of this class's methods that call for a
		 * new child.
		 */
		daeElement *_construct_new(daePseudoElement &parent)const;

		size_t _clearTOC_offset; //Scheduled for removal?
		/** 
		 * Should be combined with clearing the contents-array. 
		 * This should copy @c _prototype over the "TOC" region of @a e.
		 * There could be an all-purporse "clear" method, but there's no need.
		 */
		void _clearTOC(daeElement *e)const;
		
		/**INVISIBLE
		 * These are subroutines of the placeWRT() methods.
		 */
		struct _place_operation : daeCM_Placement<>, daeOK
		{
			void prolog(daeMeta&,daeElement&,daeElement&); 
			daeOrdinal a,b; void uncursorize(void*,void*);
			daeCursor _a,_b; void reach_insertion_point();
			void epilog(daeElement&), epilog(daeContent&);
		};

		template<class T> 
		/** These members begin their life as zeroed memory. */
		T &_bring_to_life(T &t)
		{
			new(&t) T(); return t;
		}
		daeArray<XS::Attribute> _attribs;						
		daeAttribute *_IDs_id,*_IDs;
		daeValue *_value;
		const daeCM *_CMRoot, *_CMEntree;
		XS::Element *_elem0;		
		daeArray<XS::Element> _elems;
		//TODO: THIS MAP SHOULD NOT USE <int> AND IT SHOULD 
		//INCLUDE ATTRIBUTES ALONGSIDE THE ELEMENTS. BUT IT
		//CAN'T BE DONE THIS WAY UNTIL domAny IS SORTED OUT.
		daeStringMap<int> _elem_names;	
		//CreateWRT->PlaceWRT Cache???
		//Don't know if writing to this memory can be worse 
		//than doing the lookup or not?
		mutable XS::Element *_elem_names_cache;		
		//_content_thunk.offset-daeOffsetOf(daeContents,_au).
		daeOffset _content_offset;
		//NOTE: for now the partition is actually daeContent.
		//Together these form a dummy AU w/ hidden partition.
		daeAllocThunk _content_thunk; daeContent _content_0_terminator; 		

		/** Implements daeMetaElement_self_destruct(). */
		void _self_destruct();
		/** This is the @c daeObjectMeta::_destructor_ptr. */
		friend void daeMetaElement_self_destruct(daeMetaObject*); 
		/**
		 * Disabled Destructor
		 * @c daeMetaObject::_self_destruct() is used instead.
		 * @note This is called by @c _self_destruct().
		 */
		~daeMetaElement(){ assert(_domAny_safe_prototype==_prototype); }

		//SKETCH/EXPERIMENTAL
		friend class daeDocument;
		/**LEGACY-SUPPORT
		 * It's tempting to put this in the prototype, so it will set a
		 * flag in __DAEP__Element__data when memcpy'ed, but in practice
		 * that creates too many complications, requiring lots of boolean
		 * logic to unravel. 
		 */
		mutable bool _daeDocument_typeLookup_enabled;
		/**LEGACY-SUPPORT
		 * This tries to determine whether to take the typeLookup() path.
		 * If the element has content it's generally assumed that the graph
		 * must be traversed, unless the source/destination documents are the
		 * same, or a global flag that says typeLookup() has never been called.
		 */
		static bool _typeLookup_unless(daeElement*);

		/**SCHEDULED FOR REMOVAL
		 * Historically each @c domAny object has their own copy of the 
		 * master-metadata record. Implementing <xs:anyAttribute> should
		 * make that practice obsolete. In the meantime this can be a way
		 * to visualize @c _prototype in debugging sessions.
		 */
		daeElement *_domAny_safe_prototype;

		friend class daeValue;
		friend class XS::Schema;		
		/**VARIABLE-LENGTH
		 * THIS IS THE FINAL DATA-MEMBER. 
		 *
		 * @remarks THIS IS STRICTLY FOR PROTOTYPE-CONSTRUCTION. 
		 * @c domAny::_meta DOESN'T HAVE ANYTHING IN THIS PLACE.
		 */
		union _Prototype
		{
			void *_ALIGNER;
			//daeElement is not yet defined.			
			operator daeElement*()const{ return (daeElement*)this; }
			daeElement *operator->()const{ return (daeElement*)this; }			
		}_prototype;
	  //}_prototype;
	  //}_prototype;

	////^NO MORE MEMBERS BEYOND THIS POINT^////

#endif //BUILDING_COLLADA_DOM
};

#include "../LINKAGE.HPP" //#undef LINKAGE

//---.
}//<-'

#endif //__COLLADA_DOM__DAE_META_ELEMENT_H__
/*C1071*/
