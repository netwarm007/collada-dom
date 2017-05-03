/*
 * Copyright 2006 Sony Computer Entertainment Inc.
 *
 * Licensed under the MIT Open Source License, for details please see license.txt or the website
 * http://www.opensource.org/licenses/mit-license.php
 *
 */

#ifndef __COLLADA_DOM__DAE_META_ATTRIBUTE_H__
#define __COLLADA_DOM__DAE_META_ATTRIBUTE_H__

#include "DAEP.h"
#include "daeAtomicType.h"

COLLADA_(namespace)
{//-.
//<-'

/**
 * The @c daeValue class describes a C++ COLLADA-DOM element's value.
 * @c XS::Attribute is based on @c daeValue.
 */
class daeValue
{
	friend class daeElement;
	friend class daeMetaElement;

COLLADA_(protected) //XML Schema relationships	
	/**
	 * The element that this value is modeling.
	 */
	daeMeta *_meta;
	/**
	 * This is used to destruct the prototypes.
	 */
	void (*_destructor)(void*);

COLLADA_(protected) //XML Schema values	
	/**
	 * Default, or fixed string if @c _fixed!=0.
	 */
	daeHashString _default;	
	/**
	 * Not exposed by @c daeValue, but here
	 * so @c XS::Attribute::getName() can see it.
	 * @c _attribute_name==nullptr should be meaningful.
	 */
	daePseudonym _attribute_name;
	/**BITFIELD
	 * @c getNextID() uses this flag.
	 * @remarks 8 was going to be 1, but it had relied on
	 * reordering the attributes, except that the generated
	 * compile-time-constant attribute indices realy ought to
	 * accurately reflect the ordering of the static attributes.
	 */
	unsigned int _next_attribute_is_ID:8,_this_attribute_is_ID:1;
	/**BITFIELD-CONTINUED
	 * Both <xs:attribute> & <xs:element> can be fixed.
	 * In which case @c _default becomes the fixed string.
	 * This should not effect @c getDefaultString().
	 */
	unsigned int _fixed:1;
	/**BITFIELD-CONTINUED
	 * Various <xs:attribute> attributes/options.
	 */
	unsigned int _attribute_use_required:1, _attribute_use_prohibited:1;

COLLADA_(protected) //Implementation details	
	/**
	 * Address of the value where @c this==nullptr.
	 * @note The value is not necessarily inside of the object
	 * described by @c _meta. It can be anywhere on the system.
	 */
	daeOffset _offset;
	/**WARNING, VARIABLE-LENGTH
	 * @warning Positioning this member last, in case it grows.
	 * Houses default value, RTTI, typename, and min/maxLength.
	 */
	daePrototype _type;	
	/**COURTESY
	 * Not used internally. 
	 * Equivalent to @c getMeta()->getSchema()->findType(getType().alias).
	 */
	const XS::SimpleType *_simpletype;

#ifdef BUILDING_COLLADA_DOM

COLLADA_(protected) //INVISIBLE

	//ALL MEMBERS ARE VISIBLE FOR THE MAIDEN VOYAGE. _type MAY GROW.	

COLLADA_(public)
	/**
	 * Default Constructor
	 */
	daeValue(){ /*NOP*/ }

#endif //BUILDING_COLLADA_DOM

COLLADA_(public) //OPERATORS

	COLLADA_DOM_OBJECT_OPERATORS(daeValue)
	/**
	 * These are the same class under the hood. 
	 */
	inline operator daeAttribute&()const{ return *(daeAttribute*)this; }

COLLADA_(public) 
	/**
	 * This is a roundabout way to determine if the value is an <xs:attribute>.
	 * @note "isAttribute" reads somewhat funny amidst XS::Attribute's members.
	 */
	inline daeXS getXS()const
	{
		return 0==_attribute_name.extent?(daeXS)0:XS::ATTRIBUTE; 
	}

	/**LEGACY
	 * Gets the @c sizeof(char) offset (from @ this) where this value's storage is
	 * found in its container element class.
	 * @return Returns the @c sizeof(char) offset from the element's @c this pointer.
	 */
	inline daeOffset getOffset()const{ return _offset; }

	/**WARNING, LEGACY
	 * Gets the number of bytes for this attribute.
	 * @return Returns the number of bytes in the C++ COLLADA DOM element for this
	 * attribute.
	 * @warning There was a bug, in so far as the array version of this class hadn't
	 * overridden the old implementation; That is like @c getType()->getAtomicSize().
	 */
	inline daeSize getSize()const{ return _type->getSize(); }

	/**LEGACY
	 * Previously "isArrayAttribute."
	 * Tells if this value is a @c daeArray contained value.
	 * @return Returns @c true if this value is an array type.
	 */
	inline bool isArrayValue()const{ return _type==_type->per<daeArray>(); }
	
	/**LEGACY
	 * Gets the @c daeTypewriter used by this value.
	 * Post-2.5 @c daePrototype converts into @c daeTypewriter.
	 * @return Returns the @c daeTypewriter that this value uses for its
	 * implementation.
	 */
	inline const daePrototype &getType()const{ return _type; }

	/**COURTESY
	 * Gets the @c XS::SimpleType used by this value.
	 * The library doesn't use this. It's provided if user/client code does.
	 */
	inline const XS::SimpleType &getSimpleType()const{ return *_simpletype; }

	/**LEGACY	 
	 * Gets the "default" for this value as a string.
	 * @return Returns @c nullptr if no default string is provided.
	 */
	inline const daeHashString &getDefaultString()const{ return _default; }

	#ifndef COLLADA_NODEPRECATED
	COLLADA_DEPRECATED("Post-2.5: END-OF-SUPPORT\n\
	Sorry, it's too easy to confuse with getDefaultString(). Use getType().value.")
	/**
	 * Gets the default for this value as a memory value.
	 * @return Returns a @c daeOpaque representing the default value.
	 */
	void getDefaultValue()const;
	COLLADA_DEPRECATED("Post-2.5: END-OF-SUPPORT\nUse getMeta()")
	/**NOT-IMPLEMENTING Use @c getMeta(). */
	void getContainer()const;
	#endif //COLLADA_NODEPRECATED

	/**
	 * Gets the containing @c daeMetaElement for this value.
	 * @return Returns the @c daeMetaElement to which this @c daeValue belongs.
	 */
	inline daeMeta &getMeta()const{ return *_meta; }	

COLLADA_(public) //GENERATOR-SIDE APIs

	template<int N>
	/**GENERATOR-SIDE API, LEGACY
	 * Sets the default for this attribute via a string.
	 * @param defaultVal @c daeString representing the default value.
	 */
	inline void setDefaultString(const daeStringCP (&def)[N])
	{
		//This API has to disable the prototype data-bit temporarily.
		//That could be avoided if there is a dedicated finalization
		//API that the generator must call. It doesn't seem worth it
		//to add such an API just for this, although addContentModel
		//could call it, so it would only be needed for simple-types.
		_setDefaultString(def);
	}
	/** Implements setDefaulString(). */
	COLLADA_DOM_LINKAGE void _setDefaultString(daeHashString);

COLLADA_(public) //"WRT" APIs. (With Respect To.)

  /////////////////////////////////////////////////////////////////////////////////////////
  //IDEALLY THESE APIs WOULD BE private/friend TO daeElement. ELEMENT/METDATA MUST AGREE.//
  /////////////////////////////////////////////////////////////////////////////////////////

	template<class T> //daeElement
	/**LEGACY, NOT-RECOMMENDED
	 * Gets the value's memory pointer from containing element @a e.
	 * @param e Element from which to get the value.
	 * @return Returns the memory pointer corresponding to this value out of parent element @a e.
	 */
	inline typename daeConstOf<T,daeOpaque>::type getWRT(T &e)const
	{
		const daeElement &upcast = dae(*e); return daeOpaque(e)[getOffset()]; 
	}
	template<class This> //daeElement
	/**LEGACY, NOT-RECOMMENDED
	 * Gets the value's memory pointer from containing element @a e.
	 * This overload is just to receive @c this pointers/coceivably other rvalues.
	 * @param e Element from which to get the value.
	 * @return Returns the memory pointer corresponding to this value out of parent element @a e.
	 */
	inline typename daeConstOf<This,daeOpaque>::type getWRT(This *e)const
	{
		const daeElement &upcast = dae(*e); return daeOpaque(e)[getOffset()]; 
	}
	
	template<class Change> //DAEP::Change
	/**
	 * Send note-of-change to database if certain conditions are met.
	 */
	inline Change &noteChangeWRT(Change &note)const
	{ 
		#ifdef NDEBUG
		#error Need a mechanism for suppressing notification.
		#endif
		bool notify = true;
		if(notify) daeNoteChange(note,(XS::Attribute*)this); return note;
	}

	/**LEGACY
	 * Copies the value of this value from fromElement into toElement.
	 * @param toElement Pointer to a @c daeElement to copy this value to.
	 * @param fromElement Pointer to a @c daeElement to copy this value from.
	 * CHANGE-NOTICES
	 * Do "getType()->copy(getWRT(from),getWRT(to));" to bypass @c noteChangeWRT().
	 */
	inline void copyWRT(daeElement *toElement, const daeElement *fromElement)const
	{
		_operation<> op(toElement,this,getWRT(fromElement)); return !noteChangeWRT(op);
	}
	/**LEGACY
	 * Copies the default value of this value to the element
	 * @param toElement Pointer to a @c daeElement to copy the default value to.
	 * CHANGE-NOTICES
	 * Do "getType()->copy(getType().value,getWRT(element));" to bypass @c noteChangeWRT().
	 */
	inline void copyDefaultWRT(daeElement *toElement)const
	{
		_operation<> op(toElement,this,getType().value); return !noteChangeWRT(op);
	}

	/**LEGACY
	 * Compares the value of this value in the given elements.
	 * @param elt1 The first element whose value value should be compared.
	 * @param elt2 The second element whose value value should be compared.
	 * @return Returns a positive integer if value1 > value2, a negative integer if 
	 * value1 < value2, and 0 if value1 == value2.
	 */
	inline int compareWRT(const daeElement *elt1, const daeElement *elt2)const
	{
		return getType()->compare(getWRT(elt1),getWRT(elt2));
	}
	/**LEGACY
	 * Compares the value of this value from the given element to the default value
	 * of this value (if one exists).
	 * @param e The element whose value should be compared to the default value.
	 * @return Returns a positive integer if value > default, a negative integer if 
	 * value < default, and 0 if value == default.
	 */
	inline int compareToDefaultWRT(const daeElement *e)const
	{
		return getType()->compare(getWRT(e),getType().value);
	}

	/**LEGACY
	 * Converts a string to a memory value in the specified element.
	 * @param src Source string, shorter than 128 characters.
	 * (This is to prevent really long @c strlen calls for potentially short values.)
	 * @see daeTypewriter::stringToMemory().
	 * CHANGE-NOTICES
	 * Do "getType()->stringToMemory("xyz",getWRT(e));" to bypass @c noteChangeWRT().
	 */
	inline daeOK stringToMemoryWRT(daeElement *e, daeString src)const
	{
		_operation<daeOpaque> op(e,this,src,getWRT(e)); return noteChangeWRT(op);
	}
	template<class T> //const daeStringCP* or int
	/**LEGACY, OVERLOAD
	 * Converts a string to a memory value in the specified element.
	 * @see daeTypewriter::stringToMemory().
	 * CHANGE-NOTICES
	 * Do "getType()->stringToMemory("xyz",getWRT(e));" to bypass @c noteChangeWRT().
	 */
	inline daeOK stringToMemoryWRT(daeElement *e, const daeStringCP *src, T len_or_end)const
	{
		_operation<T,daeOpaque> op(e,this,src,len_or_end,getWRT(e)); return noteChangeWRT(op);
	}
	/**OVERLOAD, NEW/MAY HAVE ISSUES, LEGACY-SUPPORT
	 * Converts a string to a memory value in the specified element.
	 * @see daeTypewriter::stringToMemory().
	 * CHANGE-NOTICES
	 * Do "getType()->stringToMemory("xyz",getWRT(e));" to bypass @c noteChangeWRT().
	 */
	inline daeOK stringToMemoryWRT(daeElement *e, const daeHashString &src)const
	{
		_operation<daeString,daeOpaque> op(e,this,src,src+src.extent,getWRT(e)); return noteChangeWRT(op);
	}

	/**LEGACY
	 * Converts an element's attribute value to a string.
	 * @c dst.data() is 0 terminated. @c dst.size() is not.
	 */
	inline daeOK memoryToStringWRT(const daeElement *e, daeArray<daeStringCP> &dst)const
	{
		return getType()->memoryToString(getWRT(e),dst);
	}

	template<class S=void, class T=int>
	/**EXPERIMENTAL, SEMI-INTERNAL
	 * Implements change-notice logic. 
	 */
	class _operation : public DAEP::Change
	{	
		S s; T t; 		
		daeTypewriter *tw; daeString src;		
		mutable daeOK OK; mutable bool b; 		
		virtual void carry_out_change()const
		{
			b = true; OK = tw->stringToMemory(src,s,t);
		}public:
		_operation(daeElement *e, const daeValue *v, daeString src, S s, T t=0)			
		//Reminder: getXS here is intended to be provisional.
		:Change(e,v->getXS()==0?DAEP::CONTENT:DAEP::ATTRIBUTE) 
		,tw(v->getType()),src(src),s(s),t(t),b(){}
		inline operator daeOK(){ if(!b) carry_out_change(); return OK; } 		
	};
	/**TEMPLATE-SPECIALIZATION
	 * Implements @c daeTypeWriter::copy(). 
	 */
	template<> class _operation<> : public DAEP::Change
	{	
		daeOpaque s,t;
		daeTypewriter *tw; mutable bool b; 		
		virtual void carry_out_change()const
		{
			b = true; tw->copy(s,t);
		}public:
		_operation(daeElement *e, const daeValue *v, const daeOpaque src)			
		//Reminder: getXS here is intended to be provisional.
		:Change(e,v->getXS()==0?DAEP::CONTENT:DAEP::ATTRIBUTE) 
		,tw(v->getType()),s(src),t(v->getWRT(e)),b(){}
		//operator! works surprisingly well for this purpose.
		inline void operator!(){ if(!b) carry_out_change(); } 		
	};
};

//-------------------.
	namespace XS //<-'
	{//-.
//<-----'

/**
 * @c XS::Attribute describes C++ COLLADA-DOM elements' attributes.
 * @c XS::Attribute is based on @c daeValue.
 */
class Attribute : public daeValue
{
	//This is so daeMetaElement's arrays store objects v. pointers.
	enum{ __size_on_stack=24*sizeof(void*) };
	char __reserved_for_daeValue[__size_on_stack-sizeof(daeValue)];

#ifdef BUILDING_COLLADA_DOM

COLLADA_(public) //INVISIBLE

	Attribute(){ daeCTC<__size_on_stack*2/3>=sizeof(daeValue)>(); }

#endif //BUILDING_COLLADA_DOM

COLLADA_(public) //daeArray traits

	typedef void __COLLADA__atomize;

COLLADA_(public) //OPERATORS

	COLLADA_DOM_OBJECT_OPERATORS(XS::Attribute)

COLLADA_(public) //ACCESSORS
	/**LEGACY
	 * Gets the name of this attribute.
	 * @return Returns the name of this attribute.
	 */
	inline const daePseudonym &getName()const{ return _attribute_name; }

	/**
	 * Gets the next ID attribute. IDs can be anything. The user provides
	 * the list on a per @c XS::Schema basis. COLLADA uses "id" and "sid."
	 * @see @c daeMetaElement::getFirstID()
	 *
	 * @note This can reach up to 255 attributes away, but if it must be
	 * stretched further, it may return a non-ID attribute to make up the
	 * difference.
	 */
	inline daeAttribute *getNextID()const
	{
		return _next_attribute_is_ID==0?nullptr:this+_next_attribute_is_ID; 
	}
	/**WARNING
	 * @warning This is probably not @c nullptr for an "id" attribute, but
	 * it isn't necessarily one. See @c XS::Schema::getIDs().
	 *
	 * Tells if this attribute is an ID attribute. This is test if @c this
	 * is part of the @c getNextID() chain given an arbitrary @c daeAttribute.
	 * There isn't a backward iterator at this time.
	 * @see @c daeMetaElement::getFirstID()
	 */
	inline daeAttribute *getThisID()const
	{
		return _this_attribute_is_ID==0?nullptr:this; 
	}

	/**LEGACY
	 * Tells if the schema indicates that this is a required attribute.
	 * @return Returns true if this is a required attribute, false if not.
	 */
	inline bool getIsRequired()const{ return _attribute_use_required!=0; }

COLLADA_(public) //GENERATOR-SIDE APIs

	/**GENERATOR-SIDE API, LEGACY
	 * Sets the value that indicates that this attribute is required by the schema. If set, the attribute
	 * will always be exported by the API regardless of its value.
	 * @param isRequired Indicates if the schema says this attribute is required, true if it is, false if not.
	 */
	inline void setIsRequired(bool required=true){ _attribute_use_required = required?1:0; }
};

//-------.
	}//<-'
}

#endif //__COLLADA_DOM__DAE_META_ATTRIBUTE_H__
/*C1071*/