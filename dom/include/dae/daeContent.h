/*
 * Copyright 2006 Sony Computer Entertainment Inc.
 *
 * Licensed under the MIT Open Source License, for details please see license.txt or the website
 * http://www.opensource.org/licenses/mit-license.php
 *
 */

#ifndef __COLLADA_DOM__DAE_CONTENT_H__
#define __COLLADA_DOM__DAE_CONTENT_H__

#include "daeSmartRef.h"

COLLADA_(namespace)
{//-.
//<-'

template<int> class daeContents_size;
/**
 * Special class that is situated at the end of generated element classes.
 * It cannot exist on its own, and requires precise setup, with relation to
 * itself, and its cursor, and the @c dae_Array_base<true/false> classes that
 * surround it; including a bitfield prior to the cursor, that is used to count
 * children that are expected to be only 1 or 0 in number (although technically
 * there can be more: allowing users to go against the XSD schema in question.)
 */
typedef daeContents_size<0> daeContents;

/**
 * Ordinals have historically been used
 * to implement the XML Schema based CM,
 * -or Content-Model. 
 *
 * @remarks AT SOME POINT THERE'S GOING
 * TO HAVE TO BE A 64BIT-ORDINAL OPTION.
 */
typedef unsigned daeOrdinal;
/**INTERNAL
 * This is 0xFFFFFF, or a 24-bit maximum.
 * The remaining 8-bits are used by the text
 * span counter. Because of this, text nodes are
 * always greater than @c daeOrdinals.
 *
 * @remarks This is also the first ordinal, as
 * the ordinal-space is inverted, counting down
 * to 0. 0 is reserved for unordered content. It
 * is forced to the back of the contents-array.
 * Inversion is to let forward-traversal skip over
 * text-nodes without special logic.
 * There's also a dummy 0 ordinal at the end of 
 * the contents-arrays, so it's unnecessary to test
 * @c end().
 */
enum{ daeOrdinals=16777215 };
/**INTERNAL
 * This is used a default B (or Before) parameter.
 * There are currently two modes, where mixed types
 * are not implemented. (That would be four modes.)
 * Once mixed types are implemented, this will be a
 * non-daeOrdinal type sentinel.
 *
 * @remarks This is also the last ordinal, as
 * the ordinal-space is inverted, counting down
 * to 0. 0 is reserved for unordered content.
 */
static const daeOrdinal daeOrdinal0 = 0;

/**
 * Normally 32-bit unsigned types
 * the same size as @c daeChildID.
 * @see @c daeChildID::daeChildID().
 */
typedef unsigned daeCounter;
//INTERNAL
//WARNING: daeCursor is no longer int.
//But this is still used to pass -1 as
//default to the bounded placement APIs.
typedef int signed_daeCursor;
typedef ptrdiff_t signed_daeCursor_ptr;
static daeCTC<sizeof(ptrdiff_t)==sizeof(void*)> daeCursor_ptr_check;
	
/**
 * @c daeChildID is a 16-bit index, plus 
 * a 16-bit array identifier (bits 17~32.)
 * It's designed to align with @c daeOrdinal
 * such that together they make a 64-bit unit.
 * (@c daeCounter lines up with @c daeChildID.) 
 */
class daeChildID
{	
COLLADA_(public) 

	typedef int POD; //"Plain-Old-Data"
			
COLLADA_(private)

	POD _id;

COLLADA_(public) //CONSTRUCTORS
	/** 
	 * IDs less-than 65536 are "unnamed," and 
	 * Can be any combination of different kinds
	 * of elements. Named IDs are of a single kind. 
	 */
	daeChildID(POD id):_id(id){}
	/**
	 * If name is 0, use @c daeChildID(index) instead. 
	 */
	daeChildID(POD name, POD index):_id(index|name<<16)
	{	
		assert((daeCounter)index<=0xFFFF);
		assert((daeCounter)name>=1&&std::abs(name)<65536);		
	}
	/**
	 * Non-Constructor (It does a compile-time-check.) 
	 */
	daeChildID()
	{
		//Previously POD was daeInt, but it
		//should be compatible with INT_MAX.
		daeCTC<(sizeof(POD)*CHAR_BIT>=32)>();
		daeCTC<sizeof(daeCounter)==sizeof(POD)>(); 
	}

COLLADA_(public) 
	/** 
	 * Extracts the child-ID's name.
	 * Names 0 and 1 are not valid names.
	 * 0 is unnamed, and must be watched for.
	 * 1 is illegal, and SHOULD not be returned.
	 */
	inline POD getName()const
	{	
		//~0>>1==~0 is true for most systems.
		if(~0>>1==~0||_id>=0) return _id>>16;
		return _id>>16|~POD(0)<<16; 
	}

	/** 
	 * Gets this child-ID's subscript. 
	 */
	inline POD getIndex()const{ return _id&0xFFFF; }
	/**
	 * Sets this child-ID's index to 0. 
	 */
	inline void clearIndex(){ _id&=0xFFFF0000; }
	  
	/**
	 * Gets at the compound child-ID. 
	 */
	inline operator POD&(){ return _id; } 
	/**CONST-FORM
	 * Gets the compound child-ID.
	 */
	inline operator POD()const{ return _id; } 
	
	/**
	 * This is true if the name is negative. 
	 */
	inline bool isAlone()const{ return _id<0; }
	/**
	 * This is true if the name is greater-than 0. 
	 */
	inline bool isArray()const{ return _id>=0x10000; }
	/**
	 * This is true if the name is not equal to 0. 
	 */
	inline bool isNamed()const{ return (daeCounter&)_id<=0x1FFFF; }			   
};

template<class T>
/**
 * This class shouldn't be copyable	nor assignable.
 * It identifies smart-refs housed in contents-arrays.
 */
class daeChildRef : public daeSmartRef<T>
{
COLLADA_(private) //Disable copy/construction/everything.

	//Should there be a public method for extracting a daeContent*?
	~daeChildRef(); daeChildRef(); daeChildRef(const daeChildRef&);

COLLADA_(public) //These APIs want daeChildRef to be C++ references.
	/**
	 * Access the @c daeContent object containing this child-ref.
	 */	
	inline typename daeConstOf<T,daeContent>::type &content()const
	{
		daeOffset CTC = -daeOffsetOf(daeContent,_child.ref);
		return (daeContent&)*((char*)this+CTC);
	}
	/**
	 * Gets the child-ref's ordinal in the contents-array.
	 * @return Returns 0 if the child-ref is "unordered."
	 * (Unordered elements are schema-violations, or indications
	 * that there is no schema. 0 is the last-ordinal. In the back.)
	 */	
	inline daeOrdinal ordinal()const{ return content()._ordinal(); }
	/**
	 * Gets the child-ref's child-ID in the contents-array.
	 */	
	inline daeChildID childID()const{ return content()._childID(); }
	/**
	 * Gets the child-ref's child-ID's name.
	 * A negative name means there can only be one child of its
	 * kind. 0 shouldn't occur. It's not a valid name. It can be 
	 * used as a sentinel value by algorithms.
	 * @return Returns 1 if the child-ref is "unnamed."
	 * (Unnamed elements are schema-violations, or indications
	 * that there is an <xs:any> element within the content-model.
	 * In theory they can also be substitution-group elements not
	 * mentioned in the schema.)
	 */	
	inline daeChildID::POD name()const{ return childID().getName(); }
	/**
	 * Gets the child-ref's child-ID index-or-subscript-or-cursor-stop.
	 * Any of those terms is accurate. This is the positive number used
	 * to access C-like arrays; usually via @c operator[]. If @c name()
	 * is equal, the child-refs belong to the same cursor-array.
	 */	
	inline daeChildID::POD index()const{ return childID().getIndex(); }
};
 
/**SCOPED-ENUM 
 * @see @c daeText::kind(). 
 */
struct daeKindOfText
{
	/**
	 * @c DEBRIS is deleted text. Theoretically it could be anything?
	 * @c COMMENT is an XML like comment.
	 * @c MIXED is ostensibly for "mixed text" content models, but it
	 * may be required wherever untagged text is desired.
	 * @c PI_LIKE was "PROCESSING_INSTRUCTION" but that's long winded.
	 * The "_LIKE" bit is to not be "PI" and because <?xml ?> is said
	 * to not be a "processing-instruction."
	 * @c DTD is reserved for <!DOCTYPE> and <!ENTITY> and so on. The
	 * contents-array cannot insert these. Only @c daeDocument should.
	 * @note This is probably a complete list, because 8 is the limit.
	 * To add more would mean not being able to represent them with a
	 * bitmask.
	 * @see @c daeText::kind(). 
	 */
	enum{ DEBRIS=0, COMMENT=1, MIXED=2, PI_LIKE=4, DTD=8 /*UNUSED*/ };
};

/**VARIABLE-LENGTH
 * This is the text of the XML-like document--except for simple-type
 * elements, that have only a single value.
 * THE STRING IS SEGMENTED AND SEGMENTS ARE NOT-NOT-NOT 0-TERMINATED.
 * @see @c size() for 0-terminator remarks.
 * @c daeText is an alt-view of a @c daeContent that is the 0th text
 * node in a span of text nodes. Spans are segmented after the 255th.
 * (A full segment is at least 4060*8 bits.)
 * @see @c daeContent.
 */
class daeText
{
	friend union daeContent;
	friend class daeContents_base;
	template<int>
	friend class daeContents_size;
	struct _Header
	{	  
		/** 
		 * The hole is used to rewind to the beginning,
		 * -and so must be a @c char (or optimally it is.)
		 * @see @c daeContent::getText(). 
		 */
		unsigned hole:8;
		/** 
		 * Returned by @c size() indicating printable text.
		 * @c extent==0 if @c kind()==0||kind()==5.
		 */
		unsigned extent:12;
		/**
		 * Courtesy flags governing broken up text. 
		 */
		unsigned continued:1, continues:1;
		/**
		 * Can this be used to note <![CDATA[]]> text? 
		 */
		unsigned reserved:2;
		/** 
		 * The length in context-array position units. 
		 * This must be bit 24~32, and is used to tell if
		 * the position is a text, or a child-element ref. */
		unsigned span:8;
	}_;
	/**
	 * @c _hole is used to rewind a span of text, back to
	 * its beginning where the hole is. @c _hole is always 
	 * an illegal XML code-point/unit that's less than '\t'.
	 *
	 * @c _text is the C-string. ([1] is not its real size.)
	 */
	daeStringCP _text[1];

COLLADA_(public) //ACCESSORS
	/**
	 * @see @c daeKindOfText.
	 * @return Returns one of;
	 * '\1': SOH, or start-of-heading. Used for <!--comment-->.
	 * '\2': STX, or start-of-text. Used for XSD Mixed Content text.
	 * '\4': EOT, or end-of-transmission. Used for <?processing-instruction?>.
	 * '\5': ENQ, or enquiry/inquiry. This is a simple-type value range.
	 * '\6' through '\x08' are reserved.
	 * '\0' is debris. Deleted content.
	 */
	inline int kind(){ return _.hole; }

	/** 
	 * Gets the length of this text in units of
	 * @c daeContent, for forward-iteration purposes.
	 * (It happens to also be the memory that
	 * is checked by @c daeContent::hasText().)
	 */
	inline daeUStringCP span(){ return _.span; }

	/**WARNING 
	 *
	 * Gets the UN-0-terminated size of the string.
	 *
	 * @warning It's not possible to terminate these due
	 * to ambiguity if the end coincided with the "hole"
	 * marker that lets algorithms scan for the front or
	 * back of the span. (Whether that ability is useful
	 * or not is debatable. It's main purpose is to make
	 * the union-style @c daeContent data-structure more
	 * straightforward by "show-not-tell.") 
	 * @see @c daeContent::getKnownText().
	 */
	inline size_t size(){ return _.extent; }
	/**
	 * The library provides this for all string-like types.
	 */
	inline daeString data(){ return _text; }
	/**
	 * The library provides this for all string-like types.
	 */
	inline bool empty(){ return _.extent==0; }
	
	//WHAT CAN BE DONE?
	#ifdef NDEBUG
	#error is/hasX is confusing and looks like isDebris() etc.
	#endif
	/** 
	 * @return Returns @c true if this text continues a previous. 
	 * Text is broken up so rewinding the text takes fewer cycles.
	 * This is mainly informational to code that wants to print out
	 * a complete unit of text. (Back-to-back units should belong to
	 * each-other regardless.)
	 */
	inline bool isMoreText(){ return _.continues==1; }
	/*
	 * @return Returns @c true if this text continues in the next. 
	 * Text is broken up so rewinding the text takes fewer cycles.
	 * This is mainly informational to code that wants to print out
	 * a complete unit of text. (Back-to-back units should belong to
	 * each-other regardless.)
	 */
	inline bool hasMoreText(){ return _.continued==1; }

	/**
	 * @return Returns @c true if excised text. */
	inline bool isDebris(){ return _.hole==daeKindOfText::DEBRIS; /*'\0'*/ }  
	/**
	 * @return Returns @c true if <!--comment-->. */
	inline bool isComment(){ return _.hole==daeKindOfText::COMMENT; /*'\1'*/ }  
	/** 
	 * Indicates regular text-content. 
	 * Simple XSD types do not have contents-arrays, so the 
	 * parent element must have a "mixed" content-model.
	 * @return Returns @c true XSD Mixed Content text.
	 */
	inline bool isMixedText(){ return _.hole==daeKindOfText::MIXED; /*'\2'*/ }
	/**
	 * @return Returns @c true if <?processing instruction?>. 
	 * @remarks The "XML declaration" is covered by "_like."
	 */
	inline bool isPI_like(){ return _.hole==daeKindOfText::PI_LIKE; /*'\4'*/ }
	/**PROPOSAL
	 * This doesn't have a corresponding @c daeKindOfText flag as it's not text
	 * and there's only room for four such flags.
	 * 
	 * This is designed to preserve location of the following sort of text data.
	 *
	 * <lookat> 
     * 2.0  0.0  3.0  <!-- eye position (X,Y,Z)       --> 
     * 0.0  0.0  0.0  <!-- interest position (X,Y,Z)  --> 
     * 0.0  1.0  0.0  <!-- up-vector position (X,Y,Z) --> 
	 * </lookat> 
	 *
	 * This text-node doesn't store text. It stores a range of values to insert
	 * at this point in the text. If the value is a list, the range is in terms
	 * of subscripts. If the value is text it is in terms of control-points. If
	 * the value is other, there is a single value and its range is 0-0.
	 */
	inline bool isValueRange(){ return _.hole=='\5'; }

	#ifdef NDEBUG
	#error This still needs to be implemented/tested.
	#endif
	/**PROPOSAL
	 * This type is a 32-bit inclusive range.
	 */
	typedef unsigned ValueRange[2];
	/**WARNING
	 * If @c isValueRange()==true this returns the range of values to insert at
	 * this point into the simple-type content.
	 *
	 * @warning The range is inclusive. It may be corrupted if code changed the
	 * value without considering text-nodes. The caller must clamp the returned
	 * range to the end of the value's container.
	 */
	inline const ValueRange &getKnownValueRange()
	{
		ValueRange &out = *(ValueRange*)&_text; 
		assert(_.hole=='\5'&&_.extent==0&&out[1]>=out[0]); return out;
	}

COLLADA_(public) //text extraction

	template<class I> //int or daeCursor or daeContent pointer to this.
	/**
	 * Gets the text, including continued text.
	 * @param i is incremented by the number ot text-nodes represented.
	 * @see getTag_increment().
	 */
	daeArray<daeStringCP> &getText_increment(I &i, daeArray<daeStringCP> &io, enum dae_clear clear=dae_clear)
	{
		daeCursor CTC = (I)0; //If I is a pointer i==this should be so.
		if(clear) io.clear();
		union{ daeCursor p; daeText *t; }; t = this; for(;;)
		{
			i+=t->span(); io.append_and_0_terminate(t->_text,t->_.extent);			
			if(t->hasMoreText()) p+=t->span(); else break;
		}return io;
	}
	/**
	 * Gets the text, including continued text.
	 * @see getTag().
	 */
	daeArray<daeStringCP> &getText(daeArray<daeStringCP> &io, enum dae_clear clear=dae_clear)
	{
		unsigned i; return getText_increment(i,io,clear);
	}

	template<class I> //int or daeCursor or daeContent pointer to this.
	/**	 
	 * Gets the text with surrounding tags. If the text is not surrounded by tags
	 * then the text will be got.
	 * @param i is incremented by the number ot text-nodes represented.
	 * @see getText_increment().
	 */
	daeArray<daeStringCP> &getTag_increment(I &i, daeArray<daeStringCP> &io, enum dae_clear clear=dae_clear)
	{
		if(clear) io.clear(); switch(_.hole)
		{
		case daeKindOfText::COMMENT: io.append("<!--",4); break;
		case daeKindOfText::PI_LIKE: io.append("<?",2); break;
		}
		getText_increment(i,io,dae_append); switch(_.hole)
		{
		case daeKindOfText::COMMENT: io.append_and_0_terminate("-->",3); break;
		case daeKindOfText::PI_LIKE: io.append_and_0_terminate("?>",2); break;
		}
		return io;
	}
	/**
	 * Gets the text with surrounding tags. If the text is not surrounded by tags
	 * then the text will be got.
	 * @see getText().
	 */
	daeArray<daeStringCP> &getTag(daeArray<daeStringCP> &io, enum dae_clear clear=dae_clear)
	{
		unsigned i; return getTag_increment(i,io,clear);
	}
};

/** 
 * COLLADA-DOM class that is XML-like text, comments & child-elements.
 *
 * ===================================================================
 * PREFACE: a @c daeContent is EITHER a "child", OR some "text". Still,
 * -to keep with the language of @c daeObject, and to avoid exposing its
 * underlying dual nature, instead of "isChild": language like "hasChild"
 * is preferred. Where "asChild" might do: "getChild" is said instead. The
 * other tack has a feeling of uneasiness about it--even though this way is
 * necessarily concealing.
 * =======================
 * 
 * The contents of elements include text & comments, and child-elements.
 * Text and comments are embedded directly into the contents-array, along
 * with CM (content-model) data. Everything in daeContent.h is new in 2.5.
 *
 * Pre-2.5, contents was a list of element-refs, and text/comments were missing.
 * The old approach had three arrays, one for ordinals, one for the refs, and yet
 * another for <xs:choice> decisions. The latter was actually an array-of-arrays,
 * -one array per choice.
 *
 * In 2.5 these are all combinded into a single vector, and furthermore, the other
 * ref arrays which duplicated the references in the contents-arrays--causing there
 * to be more than one reference belonging to the containing element--are ambitiously
 * replaced by a specialization of @c daeArray; designed to walk the contents-arrays,
 * -and not take up space.
 */
union daeContent
{	
	friend class XS::Choice;
	friend class daeElement;	
	friend class daeMetaElement;
	friend class daeContents_base;	
	template<class> friend class daeChildRef;
	template<class,int> friend class dae_Array;
	template<int> friend class daeContents_size;
	template<class> friend class daeContents_like;

COLLADA_(private)
	/**NOTATION
	 * Union with @c _child.
	 * @c see _ordinal_magic().
	 */
	daeOrdinal _magic_ordinal;
	/**
	 * Union with @c _textview.
	 */
	struct _ChildSize
	{
		/**UNDOCUMENTED
		 * @see @c daeOrdinal.
		 * @see @c _magic_ordinal.
		 */
		daeOrdinal ordinal;
		/**UNDOCUMENTED
		 * @see @c daeChildID.
		 */
		daeChildID::POD ID;
		union
		{
		/**
		* @see @c _ref().
		* @note @c this may be "text"-
		* -content, in which case @c _element
		* is not a pointer (or an element-ref.)
		*/
		daeElement *ref; const DAEP::Element *DAEP;
		};

	}_child;	
	/**COLLADA__VIZDEBUG
	 * Union with @c _child.
	 */
	daeStringCP _textview
	[(sizeof(_ChildSize)<16?16:sizeof(_ChildSize))/sizeof(daeStringCP)];	
	/**COLLADA__VIZDEBUG
	 * Union with @c _child.
	 */
	mutable daeText _SOTextview;

	/**
	 * Constructor used by @c daeContents_base::_insert_operation.
	 */
	daeContent(daeOrdinal ord, daeChildID ID, const DAEP::Element *e)
	{
		_child.ordinal = ord; _child.ID = ID; _child.DAEP = e; 
	}
	/**
	 * Disabled Constructors & Assignment Operator
	 *
	 * These are disabled so @c daeArray::setCount() will refuse. 
	 *
	 * Classes that work with content use @c setInternalCounter().
	 * Client code should obviously not attempt to do this. Having
	 * setCount disabled hints to users that they're doing it wrong.
	 */
	daeContent(); daeContent(const daeContent&); void operator=(const daeContent &);

COLLADA_(public) //INTERNAL NON-TEXT APIs
	/**
	 * Gets @c _child.ordinal. Assuming not-text, or doing == or != test.
	 */
	inline daeOrdinal _ordinal()const{ return _child.ordinal; }
	/**NOTATION
	 * Gets @c _child.ordinal as @c _magic_ordinal, in order to 
	 * bring attention to the fact that code understands that text
	 * units by way of aliasing "report" ordinals that are larger than
	 * than the high end of the ordinal-space. 
	 * This is a "magic" trick to do a single comparison when using
	 * <, <=, >, and >= operators in a loop. If the convention is ever
	 * changed, the offending code using @c _ordinal_magic() can be found
	 * out and corrected; however it's not likely to change, even
	 * though an argument can be made that this arrangement favors reverse
	 * iteration, and forward might be preferrable, using 0~255 for
	 * text would be impossible, because text cannot have 0s in this field.
	 * ON THE OTHER HAND, one possible change is to invert the ordinal
	 * space. The only thing standing in the way of that is it is unnatural.
	 */
	inline daeOrdinal _ordinal_magic()const{ return _magic_ordinal; }

	/**
	 * Gets @c _child.ID as a @c daeChildID. ASSUMING NOT TEXT.
	 */
	inline const daeChildID &_childID()const
	{
		return (daeChildID&)_child.ID;
	}

	/**
	 * Gets @c _child.element as a @c daeSmartRef. ASSUMING NOT TEXT.
	 * @remark This was "_ref" but that looks like it's adding a ref.
	 */
	inline const daeChildRef<> &_child_ref()const
	{
		assert(hasChild()); return (daeChildRef<>&)_child.ref;
	}
	template<class T>
	/**
	 * Gets @c _child.element as a @c daeSmartRef. ASSUMING NOT TEXT.
	 * @remark This was "_ref" but that looks like it's adding a ref.
	 */
	inline const daeChildRef<T> &_child_ref()const
	{
		assert(hasChild()); return (daeChildRef<T>&)_child.ref;
	}

COLLADA_(private) //daeArray grow implementation
		
	//__COLLADA__move() was implemented below until it was necessary to
	//implement it via @c daeArray::_grow2<daeContent>() so to maintain
	//the cursor-iterator (It's desirable to let users grow the array.)
	typedef void __COLLADA__atomize;

COLLADA_(public)
	/**INTERNAL
	 * Destructor
	 *
	 * Unlike construction, destruction needs to work anywhere-at-anytime.
	 *
	 * Technically it should be safe if users to clear the contents-array.
	 *
	 * (Growing it should also be safe. As @c daeArray::_move has a special-
	 * -form for @c daeContent. It could be beneficial to reserve capacity.)
	 */
	~daeContent(){ assert(getChild()==nullptr); }

COLLADA_(public) //LOW-LEVEL CONTENTS-ARRAYS TRAVERSAL METHODS
	/**
	 * Indicates @c this is a child-element. 
	 * @note Otherwise it is a form of text-content.
	 * @remarks @c hasChild() may still be a @c nullptr.
	 * PROCEDE WITH CAUTION.
	 */
	inline bool hasChild()const
	{			
		//Test bits 24 through 31.
		//0==((daeText*)this)->_.span;
		return _magic_ordinal<=0xFFFFFF;
	}

	/**
	 * Indicates @c this is text-content. 
	 * @note Otherwise it is a child-element.
	 * @remarks Text spans multiple array-elements.
	 */
	inline bool hasText()const
	{
		//Test bits 24 through 31.
		//0!=((daeText*)this)->_.span;
		return _magic_ordinal>0xFFFFFF;
	}	

	/**
	 * Gets the @daeElement pointer. 
	 * @note This pointer can be @c nullptr.
	 * @remarks Children occupy one array-element.
	 */
	inline daeElement *getChild()
	{
		return hasChild()?_child.ref:nullptr; 
	}	
	/**CONST-PROPOGATING-FORM
	 * Gets the @daeElement pointer. 
	 * @note This pointer can be @c nullptr.
	 * @remarks Children occupy one array-element.
	 */
	inline const daeElement *getChild()const
	{
		return hasChild()?_child.ref:nullptr; 
	}	
	/**CAUTION
	 * Get the element-ref, knowingly, as a reference.
	 */
	inline const daeChildRef<> &getKnownChild()
	{
		return (daeChildRef<>&)_child.ref;
	}	
	/**CAUTION, CONST-PROPOGATING-FORM
	 * Get the element-ref, knowingly, as a reference.
	 */
	inline const const_daeChildRef &getKnownChild()const
	{
		return (const_daeChildRef&)_child.ref;
	}	

	//These are for daeContents_size::for_each_child(). 
	//MSVC2013 seems to have a bug that hides the non-const
	//forms if implemented as a template, even though I can't 
	//reproduce the bug with a mocked up class??? It takes a 
	//daeContent--plain as day--and will only use the const-this 
	//form, all else being equal. It's very strange.
	//ANYWAY! It's really not that many permutations. Just too many
	//to document in much detail. It's impractical to get a tooltip 
	//for the conversion operators anyway.
	/** These are for daeContents::for_each_child(). */
	inline operator daeElement*(){ return getKnownChild(); }
	/** These are for daeContents::for_each_child(). */
	inline operator const daeElement*()const{ return getKnownChild(); }
	/** These are for daeContents::for_each_child(). */
	inline operator daeElement&(){ return *getKnownChild(); }
	/** These are for daeContents::for_each_child(). */
	inline operator const daeElement&()const{ return *getKnownChild(); }
	/** These are for daeContents::for_each_child(). */
	inline operator const daeChildRef<>&(){ return getKnownChild(); }
	/** These are for daeContents::for_each_child(). */
	inline operator const const_daeChildRef&()const{ return getKnownChild(); }
	/** These are for symmetry with the getKnownChild() operators. */
	inline operator daeText&()const{ return getKnownStartOfText(); }
	/**These are for symmetry with the getKnownChild() operators. */
	inline operator daeEOText&()const{ return getKnownEndOfText(); }
					
	template<class T>
	/**
	 * Comparison operators must check that that node is
	 * not text in order to work with @c daeArray::find().
	 */
	inline bool operator==(const T &cmp)const
	{
		return _child.ref==cmp&&!hasText();
	}
	template<class T>
	/**
	 * Comparison operators must check that that node is
	 * not text in order to work with @c daeArray::find().
	 */
	inline bool operator!=(const T &cmp)const
	{
		return _child.ref!=cmp||hasText();
	}

	/**
	 * Gets the @daeText pointer.
	 * Text-content cannot be @c nullptr,
	 * -unless @c hasChild() is @c true, or,
	 * -unless @c hasText() is @c false.
	 * HOWEVER, it can be a comment or text
	 * that does not belong in the document.
	 * (I.e. internal content-model data.)
	 * @see @c daeText
	 */
	inline daeText *getText()const
	{
		return hasChild()?nullptr:&getKnownText();
	}	
	/**CAUTION
	 * Get the text, knowingly, as a reference.
	 */
	inline daeText &getKnownText()const
	{
		assert(hasText());
		const daeContent *p = this; 
		while(p->_SOTextview._.hole>='\t') p--; 
		return p->_SOTextview;
	}	
	/**CAUTION
	 * Get @c this, as @c daeText text, knowingly, as a refrence.
	 */
	inline daeText &getKnownStartOfText()const
	{
		assert(hasText()&&_SOTextview._.hole<'\t');
		return _SOTextview;
	}	

	/**CAUTION
	 * This is solely for reverse-iteration.
	 * @see @c daeEOText::counterspan(). 
	 */
	inline daeEOText &getKnownEndOfText()const
	{
		assert(hasText());
		if(this[1].hasText()) this[1].getKnownStartOfText();
		return *(daeEOText*)this;
	}
};

/** 
 * Simple class for enabling reverse-iteration through text.
 * (Likely required to do efficient binary-insertion-search.)
 */
class daeEOText
{
	friend class daeContents_base;
	enum{ _textsize=sizeof(daeContent)/sizeof(daeStringCP) };
	enum{ _cspan = _textsize-1 };
	daeUStringCP _textview[_textsize]; //COLLADA__VIZDEBUG

COLLADA_(public)
	/** 
	 * Gets the unsigned-length of this text in units 
	 * of @c daeContent, for reverse-iteration purposes.
	 * @note Logically this is negative, but it it IS NOT.
	 * It must be subtracted IOW. (Unsigned sizes are just
	 * too prevalent in the C++ world, and -= is clearer.)
	 */
	inline daeUStringCP counterspan()
	{
		daeUStringCP out = _textview[_cspan]; assert(out!=0);
		return out; 
	}
};

/**
 * This was originally meant to be a 32-bit offset,
 * -but it seems more natural to make it a pointer.
 */
typedef const daeContent *daeCursor;

template<class T>
/**EXPERIMENTAL
 * Implements a dummy cursor for @c daeContents_like.
 */
struct daeCursor_less
{
	/** The offset based @c daeCursor was so much simpler. */ 
	T _; operator T&(){ return _; } 	
	/** @a _ must be an iterator owing to being pointer based. */
	daeCursor_less(const void *_):_((T)_){} 		
}; 
template<class EBO>
/**EXPERIMENTAL
 * Implements imposter of @c daeContent for @c daeContents_like.
 */
struct daeText_less : EBO
{
	inline bool hasText()const{ return false; }
	daeText &getKnownStartOfText()const{ return *(daeText*)nullptr; }
	daeEOText &getKnownEndOfText()const{ return *(daeEOText*)nullptr; }	
};
/**EXPERIMENTAL
 * Implements imposter of @c daeContent for @c daeContents_like.
 */
struct daeOrdinal_only
{
	struct _Child{ daeOrdinal ordinal; };
	union{ _Child _child; daeOrdinal _magic_ordinal; };
	inline daeOrdinal _ordinal()const{ return _child.ordinal; }			
	inline daeOrdinal _ordinal_magic()const{ return _magic_ordinal; }	
};

template<class C=daeContents>
/**EXPERIMENTAL
 * This class implements a common interface for the @c daeCM classes.
 * @see @c XS::Choice::_solution.
 */
class daeContents_like : public C
{								 
COLLADA_(public) //EXPERIMENTAL

	#ifdef NDEBUG
	#error Eventually these should be written out longhand. C++!
	#endif
	#define _(f,args,args2) \
	inline typename C::const_iterator f args const\
	{ return const_cast<daeContents_like<C>*>(this)->f args2; }\
	inline typename C::iterator f args
	/**WARNING
	 * @warning This is a linear-search algorithm, owing to the cursor
	 * like nature of the contents-array. 
	 * Furthermore, it ISN'T a strict lower-bound. If the cursor is on
	 * an ordinal that is equal to @a o, it doesn't rewind to the first
	 * position that is equal to @a o. 
	 *
	 * Finds the first ordered content equal-or-greater-than @a o.
	 * @param oN should be eliminated by the compiler with optimization
	 * enabled. If supplied the return value is anywhere betweeen @c o,o+oN.
	 */
	_(_fuzzy_lower_bound,(daeOrdinal o, daeOrdinal oN=0),(o,oN))	
	{
		C::const_iterator _ = C::cursor();
		C::iterator it = C::begin(); assert(!_->hasText());
		if(_->_child.ordinal>=o) it = (C::iterator)_; return _fuzzy_lower_bound(it,o,oN); 
	}
	template<class IT> static inline IT _fuzzy_lower_bound(IT it, daeOrdinal o, daeOrdinal oN=0)
	{
		//Code-elimination is viable if oN=0.
		if(o-it->_magic_ordinal<oN) return it;
		while(o<it->_magic_ordinal) it++; return it;
	}				
	/** Rewinds @c _fuzzy_lower_bound(). */
	_(_lower_bound_with_text,(daeOrdinal o),(o))
	{
		return _rewind_ordinal(_fuzzy_lower_bound(o),o);	
	}	
	/** Rewinds @c _fuzzy_lower_bound(), then skips ahead of text. */
	_(_lower_bound_omit_text,(daeOrdinal o),(o))
	{
		return _find_ordinal(_lower_bound_with_text(o));
	}
	/** Rewinds @c _fuzzy_lower_bound(o-1). */
	_(_upper_bound_with_text,(daeOrdinal o),(o))
	{
		assert(o!=0); return _rewind_ordinal(_fuzzy_lower_bound(o-1),o-1);	
	}	
	template<class IT> inline IT _rewind_ordinal(IT rit, daeOrdinal o)const
	{
		assert(!rit->hasText());
		for(rit-=1;;)
		{
			//Using >= instead of == can't hurt, and provides more value.
			while(o>=rit->_child.ordinal&&rit>=C::begin()) 
			rit--;
			if(rit->hasText()&&rit>=C::begin())
			rit-=rit->getKnownEndOfText().counterspan();
			else break;
		}
		return rit+1;
	}
	template<class IT> inline IT _rewind_ordinal(IT it)const
	{
		return _rewind_ordinal(it,it->_child.ordinal);
	}
	template<class IT> static inline IT _find_ordinal(IT it)
	{
		while(it->hasText()) it+=it->getKnownStartOfText().span(); return it;
	}
	#undef _ //TEMPORARY
};

template<class C=daeContents>
/**INTERNAL
 * Previously "daeCM::_Placement."
 * @remarks This class was internal to @c daeCM.
 * It's here so @c daeContents_base::_operation
 * can use it to implement the DAEP Change note.
 */
class daeCM_Placement
{
COLLADA_(public)
	/**
	 * This should be used for unordered placement only.
	 */
	inline void default_insertion_point_to_back()
	{
		if(insertion_point==nullptr) insertion_point = 
		ordinal==0?content.end():content._upper_bound_with_text(ordinal);
	}
	/**
	 * Does default behavior for extra-schema placement.
	 */
	inline void set_insertion_point_et_cetera_to_extra()
	{
		ordinal = 0; insertion_point = content.end();

		if(name!=1) //FYI: Not overriding the name this time. 
		{
			#ifdef NDEBUG
			#error Emit a warning via the error-handler.
			#endif
			//This forces the element into the unnamed group.
			/*THIS, WHILE NICE, SEEMS TREACHEROUS:
			//If the index used to add a child changes, then
			//code that expects to find it in the same place
			//may keep adding new children instead. That can
			//loop endlessly, eating memory until it crashes.
			//NOTE: The new pushBackWRT() DOES work this way.
			name = 1;*/
		}
	}

	/**
	 * Explicit Constructor
	 * This was for @c daeMetaElement::pushBackWRT() to use.
	 */
	explicit daeCM_Placement(C &c):content(c){}
	/**
	 * Constructor
	 * @param count is an optimization only. It can be 0
	 * to avoid counting ordinal instances, or "-1" to force
	 * counting. This guarantees @c insertion_point will be set.
	 */
	daeCM_Placement(int name, daeCounter count, C &c)
	:name(name),count(count),content(c){}
	
COLLADA_(protected) daeCM_Placement():content(content){}
COLLADA_(public)
	/**
	 * These are inputs.
	 */
	int name; daeCounter count; daeContents_like<C> &content;
	/**
	 * These are outputs.
	 */
	daeOrdinal ordinal; typename C::const_iterator insertion_point;
};	
template<class LAZY=daeContents>
/**INTERNAL
 * This class is designed to work with @c daeCM::_chooseWithout().
 */
class daeCM_Demotion : public daeCM_Placement<LAZY>
{
	daeMeta &_meta;	typedef daeCM_Placement<LAZY> _P;

COLLADA_(public)
	/**
	 * _remove_operation Constructor
	 */	
	daeCM_Demotion(daeMeta &meta, LAZY &c, daeContent &ctr)
	:_P(ctr._childID().getName(),_P::count,c),_meta(meta){ reset(ctr); }
	/**
	 * If @c maybe_demote() is called (e.g. by _remove_operation)
	 * then @c count needs to be reset to 0. Although pure remove
	 * operations should always succeed, and set the count.
	 */	
	void reset(daeContent &ctr)
	{
		_P::ordinal = ctr._ordinal(); _P::insertion_point = &ctr;		
	}

	/**CIRCULAR-DEPENDENCY
	 * Maybe calls @c _meta.getCMRoot()._chooseWithout().
	 */	
	inline void maybe_demote(); daeOK maybe_demote_ORDER_IS_NOT_PRESERVED;
};

template<int size_on_stack>
/**
 * Implements @c daeContents.
 */
class daeContents_size 
: 
/*REMINDER: daeContainerObject SUGGESTS THAT IT'S NOT JUST friend.*/
//BECAUSE daeContents_base PREVIOUSLY APPEARS IN friend-DECLARATIONS
//MSVC2013 REFUSES TO USE IT AS A BASE CLASS. PROBABLY THIS IS A BUG.
//IF THIS ISN'T PORTABLE, THERE'LL BE A COUPLE CIRCULAR-DEPENDENCIES.
public daeContents_base_MSVC2013, public daeArray<daeContent,size_on_stack>
{	
	friend class daeContents_base;
	typedef daeContents_base base;

COLLADA_(public) //C++ annoyances

	#define _(x) using __::x;
	#define _2(x) using typename __::x;
	typedef daeArray<daeContent,size_on_stack>__;
	_2(iterator)_2(const_iterator)_(begin)_(end)_(data)_(size)_(_au)
	#undef _2
	#undef _
	
COLLADA_(public) //daeContents_like implementation
	/**WARNING
	 * @warning In some INTERNAL contexts this can return @c nullptr.
	 * In those contexts, it shouldn't be necessary to call this API.
	 *
	 * Gets the element containing this contents-array. 
	 * @todo This can be optimized by passing along DAEP Child's template 
	 * parameters through @c dae_Array.
	 */
	inline daePseudoElement &getElement()
	{
		return (daePseudoElement&)*daeArray::getObject(); 
	}
	/**CONST-FORM
	 * @warning In some INTERNAL contexts this can return @c nullptr.
	 * In those contexts, it shouldn't be necessary to call this API.
	 *
	 * Gets the element containing this contents-array. 
	 * @todo This can be optimized by passing along DAEP Child's template 
	 * parameters through @c dae_Array.
	 */
	inline const daePseudoElement &getElement()const
	{
		return (daePseudoElement&)*daeArray::getObject(); 
	}

COLLADA_(private) //Disabled Copy Construct & Assignment Operator
	/**DISABLED
	 * This replaces the @c daeArray constructor since @c daeContent 
	 * is non-copyable.
	 */
	daeContents_size(const daeContents_size&){ assert(0); }
	/**DISABLED
	 * Likewise. See copy-constructor Doxygentation.
	 */
	void operator=(const daeContents_size&){ assert(0); }

COLLADA_(public) //CONSTRUCTORS
	/**
	 * Default Constructor 
	 */
	daeContents_size()
	{ 									   
		//Dread if multi-inheritance order is reversed.
		assert(0==daeOffsetOf(daeContents,_padding_reserved));

		//"daeContents_in_a_box" is no more.
		if(size_on_stack>0) _0_fill();
	} 
	/** Old "daeContents_in_a_box" Constructor */
	explicit daeContents_size(size_t real_size_on_stack)
	:
	daeArray<daeContent,size_on_stack>(nullptr,real_size_on_stack)
	{ 
		daeCTC<(size_on_stack>0)>(); _0_fill();
	}
	/**Old "daeContents_in_a_box" API */
	inline void _0_fill(size_t slice=0)
	{
		assert(slice<_au->getCapacity()); 
		_au->setInternalCounter(slice);
		base::cursor() = data()+slice; daeCTC<(size_on_stack>0)>();
		memset(end(),0x00,sizeof(daeContent)*(_au->getCapacity()-slice));
	}
	/**Old "daeContents_in_a_box" API
	 * This reallocates without calling @c _grow2<daeContent>() so that that
	 * pathway doesn't have to handle the @c getObject()==nullptr case.
	 */
	inline void _element_less_grow(size_t newT)
	{
		assert(newT>_au->getCapacity());
		((daeObject*)nullptr)->reAlloc(_au,newT,_element_less_mover);
		_0_fill(size()); 
	}static void _element_less_mover(const daeObject*, 
	daeAlloc<daeContent> &dst, daeAlloc<daeContent> &src)
	{
		memcpy(dst._varray,src._varray,src.getCount()*sizeof(daeContent));
	}

	template<class Type> //DAEP::Value<...>
	/**
	 * Prototype Constructor
	 */
	explicit daeContents_size(const DAEP::Proto<Type> &pt)
	//This instantiates daeContent::daeContent() on XML Schema's default/fixed.
	/**:daeArray<daeContent,size_on_stack>(pt){}*/
	:daeArray<daeContent,0>(*_au){ assert(pt->empty()); }

COLLADA_(public) //Alternative to old "getChildren" APIs 
	/**
	 * @c daeElement::getChild() uses exceptions to break out of 
	 * @c for_each_child(). This is to prevent clients from disabling
	 * exceptions. If this is a problem, a macro can be used, and it can disable
	 * APIs that require exceptions. Or another macro can enable C++11 lambda support.
	 * @note THE LIBRARY DOESN'T USE EXCEPTIONS, EXCEPT POSSIBLY LOCALLY & INTERNALLY.
	 */
	inline void _exceptions_2016(){ throw 2016; }
	/**
	 * Enumerates the contents-array (skipping text.)
	 */
	template<class F> void for_each_child(F &f)
	{
		iterator it = begin(), itt = end();
		while(it!=itt) it->hasChild()?f(*it++):it+=it->getKnownStartOfText().span();
	}
	/**CONST-PROPOGATING-FORM
	 * Enumerates the contents-array (skipping text.)
	 */
	template<class F> void for_each_child(F &f)const
	{
		const_iterator it = begin(), itt = end();
		while(it!=itt) it->hasChild()?f(*it++):it+=it->getKnownStartOfText().span();
	}

	/**EXPERIMENTAL
	 * This void form is only useful if only one child
	 * is expected to be present.
	 */
	inline iterator find_first_child()
	{
		iterator it = begin(), itt = end();
		for(;it!=itt;it+=it->getKnownStartOfText().span())
		if(it->hasChild()) return it; return nullptr;
	}	
	/**EXPERIMENTAL, CONST-FORM
	 * This void form is only useful if only one child
	 * is expected to be present.
	 */
	inline const_iterator find_first_child()const
	{
		return const_cast<daeContents*>(this)->find_first_child();
	}	

	/**WARNING
	 * @warning "text" includes all non-child nodes. This
	 * includes comments, processing-instructions, an XML
	 * declaration, maybe DTDs?? and non simple-type text.
	 * There are unresolved issues around values vs. text.
	 *
	 * Removes "text" by marking it as "debris."
	 * @see @c clear_of_debris().
	 *
	 * @return Reterns @c nullptr if @a text is not first
	 * in a series of text-spans all comprising a logical
	 * unit of text; in which case the text is not erased.
	 * If not @c nullptr, it is the node after the erased
	 * spans, thatcan be anything, including a terminator.
	 */
	inline const_iterator erase_text(const_iterator text)
	{
		assert(text>=data()&&text<=end());
		if(text->hasText()&&!text->getKnownStartOfText().isMoreText()) do 
		{
			daeText &t = *text; t._.extent = t._.hole = 0; text+=t.span();
		}while(text->hasText()&&text->getKnownStartOfText().isMoreText());
		else return nullptr; return text;
	}
	/**UNIMPLEMENTED
	 * This is a maintenance task to cleanse removed text
	 * and children from the contents-array. When a child
	 * is removed, it's converted to an empty/degenerate
	 * text node. Those hang around until they are lifted
	 * out. IT MAY ALSO be useful to mark text as removed.
	 * Probably such text will be marked as kind-of-text-
	 * 0. The text's span shouldn't matter. Its size will
	 * be 0.
	 */
	inline void clear_of_debris()
	{
		//TODO? It might be a good idea to offer this 
		//alongside a childID-index untangling process.
		assert(0); //unimplemented: technically harmless

		//REMINDER. THIS MUST 0-FILL THE BACK OF THE BUFFER	
		//(See daeElement::__clear2().)
	}
};
typedef class daeContents_base daeContents_base_MSVC2013;
/**
 * This class is a visualizer for the contents-array.
 * It must be ahead of the @c daeArray part, in case
 * the database opts to embed the contents-array.
 *
 * @todo There needs to be members that are set when text is put
 * into the contents-array. They should describe how much of the
 * array is text versus children. Algorithms should consult them.
 * @note A relatively small amount of text can dominate an array.
 */
class daeContents_base
{
	template<int> friend class daeContents_size;

	//Reminder: this is first so it can be 0-filled on clear().
	/**RESERVED, INTERNAL
	 * This is required to make a nonzero sized @c daeContents
	 * have its AU pointer located on an 8 byte aligned system.
	 * Text statistics are going here at some point either way.
	 */
	size_t _padding_reserved;

COLLADA_(private)

	/**MUTABLE, INTERNAL
	 * This is a union of nonplural children: only-childs. They
	 * are represented by a bitfield, of which this is the first.
	 * This is housed outside of the contents-array.
	 */
	daeCounter &_nonplurals()const{ return ((daeCounter*)this)[-2]; }

COLLADA_(public)
	/**
	 * This is always present in case there are unnamed children.
	 * This is housed outside of the contents-array.
	 */
	inline dae_Array<daeElement,1> &unnamed()
	{
		return (dae_Array<daeElement,1>&)((daeCounter*)this)[-1]; 
	}
	/**CONST-FORM
	 * This is always present in case there are unnamed children.
	 * This is housed outside of the contents-array.
	 */
	inline const dae_Array<daeElement,1> &unnamed()const 
	{
		return (dae_Array<daeElement,1>&)((daeCounter*)this)[-1]; 
	}

	/**MUTABLE, INTERNAL
	 * @history This was in the @c unnamed() slot for a time.
	 * Gets the built-in cursor that all contents-arrays have. 
	 * @remarks There are two considerations with the built-in
	 * cursor:
	 * 1) It cannot be greater than the content-array's size().
	 * 2) It cannot be left in a text node. It can be positioned
	 * on a text-node prior to inserting a child-node at the same
	 * position (for example; these are largely internal details.)
	 */
	mutable daeContent *_; inline daeCursor &cursor()const
	{
		return const_cast<daeCursor&>(_); //"loses qualifiers"
	}

	//DON'T ADD A "push_front." insert("",0) is enough. insert("",-1) could 
	//probably work in place of push_back() but it is not pretty to look at.
	template<int kind_of_text>
	/**
	 * Inserts text nodes at @c daeContents_size::end().
	 * @tparam kind_of_text is '!' or '?' or ' ' for comment, processing-instruction,
	 * -or mixed-text respectively.
	 */
	inline daeCursor push_back(const daeHashString &text)
	{
		return insert<kind_of_text>(text,_this().end());
	}
	template<int kind_of_text> 
	/**WARNING
	 * Inserts text nodes at @c cursor().
	 *
	 * @warning @c insert() moves @c cursor() to the first non-text node after the
	 * text-nodes that it inserts. This is because it can't rest on non-text nodes.
	 * It was "_cursor" in the beginning. It's hard to know what moves @c cursor()
	 * and what doesn't. Insertions do, but only if they actually insert the child.
	 * Accessing the 0th child does. But the random-access algorithm tries to keep
	 * @c cursor() on the 0th child so it can access the children like a regular C
	 * array, more or less, for better or worse.
	 * 
	 * @tparam kind_of_text is '!' or '?' or ' ' for comment, processing-instruction,
	 * -or mixed-text respectively.
	 */
	inline daeCursor insert(const daeHashString &text)
	{
		return insert<kind_of_text>(text,cursor());
	}
	template<int kind_of_text, class U> //__cursorize()
	/**
	 * Inserts text nodes at @a U.
	 * @tparam kind_of_text is '!' or '?' or ' ' for comment, processing-instruction,
	 * -or mixed-text respectively. 
	 */
	inline daeCursor insert(const daeHashString &text, const U &_)
	{
		switch(kind_of_text)
		{
		case '!': return _comment(text.string,text.extent,__cursorize(_));
		case '?': return _instruct(text.string,text.extent,__cursorize(_));		
		default: return _mix(text.string,text.extent,__cursorize(_));
		}
		daeCTC<kind_of_text=='!'||kind_of_text=='?'||kind_of_text==' '>();
	}		
	/**EXPORTED Inserts a comment. */
	COLLADA_DOM_LINKAGE daeCursor _comment(daeString,size_t,const void*_);
	/**EXPORTED Inserts mixed-text. */
	COLLADA_DOM_LINKAGE daeCursor _mix(daeString,size_t,const void*_);
	/**EXPORTED Inserts a processing-instruction. */
	COLLADA_DOM_LINKAGE daeCursor _instruct(daeString,size_t,const void*_);	
	#ifdef BUILDING_COLLADA_DOM
	/** Implements @c insert(). */	
	inline daeCursor __insert(int,daeString,size_t,const void*);
	#endif //BUILDING_COLLADA_DOM

	#ifdef COLLADA__VIZDEBUG
	daeCounter (&__vizArrays)[16];
	#endif	
	/**
	 * Constructor
	 */
	daeContents_base()
	#ifdef COLLADA__VIZDEBUG
	:__vizArrays((daeOpaque(this)[-sizeof(__vizArrays)])
	#endif
	{}

COLLADA_(public) //OPERATORS
	
	/**EXPERIMENTAL
	  * Gets a @c daeContents_like version of @c this.
	  */
	inline operator daeContents_like<daeContents>&()
	{
		return *(daeContents_like<daeContents>*)this;
	}

COLLADA_(public) //INTERNALS
	/**
	 * Extract the contents-array from @c this.
	 */
	daeContents &_this()const{ return *(daeContents*)this; }

  ////TODO////////////////////////////////////////////
  //These should return structures with compile-time//
  //-constant tags 3-1: 3 for int, 2 for daeContent*//
  //-and 1 for daeElement*.//////////////////////////                         
  //////////////////////////
		
	static void *__cursorize(signed_daeCursor _) //Can be text.
	{
		#ifdef NDEBUG
		#error The contents-array should not exceed 65535.
		#error Systems can build a higher limit if safe to.
		#endif
		//__uncursorize() is using 0xFFFF instead of c.size().
		assert(_<65535); return (void*)signed_daeCursor_ptr(_);
	}
	static void *__cursorize(const daeElement *_)
	{
		assert((size_t)_>65535||_==nullptr); return (void*)_;
	}
	static void *__cursorize(const DAEP::Element *_)
	{
		assert((size_t)_>65535||_==nullptr); return (void*)_;
	}
	static void *__cursorize(daeCursor _) //const daeContent *
	{
		assert((size_t)_>65535); return (void*)_;
	}
	/*Unlike the others this doesn't have a *operator, which is
	currently being used to convert DAEP::Element to daeElement.
	static void *__cursorize(const daeContent &_) //Can be text.
	{
		assert((size_t)&_>65535); return (void*)&_;
	}*/
	template<class T> static void *__cursorize(const dae_<T> &child)
	{
		return __cursorize(child._hit());
	}	
	template<class T, int N> void *__cursorize(const dae_Array<T,N> &child)
	{
		return __cursorize(child.operator[](0));
	}
	template<class T> static void *__cursorize(const daeChildRef<T> &child)
	{
		assert((size_t)&child-1>65535); //-1 is permitting 0.
		return (void*)&child.content();
	}
	/**INTERNAL
	 * The goal here is to unpack this on the library's side,
	 * -so that client code doesn't have to inline this logic.
	 * (Alternatively there could be an exported API for every
	 * combination; but that's maintenance/Doxygentation hell.)
	 */
	inline daeCursor __uncursorize(const void *_)
	{
		daeContents &c = _this();
		//This returns values in the range of -1 to c.size()-1.
		daeCTC<sizeof(size_t)==sizeof(void*)>();
		//WARNING: This can be ~0, and so will be beyond end().
		if((size_t)_+1<65536) return c.data()+(daeOffset)_;		
		//return __uncursorize2(_); //daeContent*
		//This returns daeContent pointers to inside the array.
		if(_>=c.begin()&&_<=c.end()) return (daeCursor)_;
		//return __uncursorize1(_); //daeElement*
		//This is a slightly better scan algorithm that
		//hits cursor() first, which is the most likely
		//place to find the element. Storing an ordinal
		//or index in the element data seems like a bad
		//idea, since even ordinals change with Choices.
		//if(c.find((daeElement*)_,(size_t&)_)!=DAE_OK) 
		daeCursor it = cursor(), itt = c.end();
		while(it<itt) if(*it++==(daeElement*)_) return it;
		it = c.begin(); itt = cursor();
		while(it<itt) if(*it++==(daeElement*)_) return it;		
		//If you are hitting this, it's possible that arguments were
		//passed that have since moved along with the contents-array.
		//(This is an error similar to a std iterator being invalid.)
		assert(0); return c.end();
	}	

	/**INTERNAL
	 * These centralize calculation of the nonplural child
	 * bitfields. 
	 */
	struct __nonplural_bitfield
	{
		const int bit; daeCounter &field;
		__nonplural_bitfield(int Name, daeCounter &First)
		:bit(1<<(Name+1)%-int(sizeof(daeCounter)*CHAR_BIT))
		,field((&First)[Name/int(sizeof(daeCounter)*CHAR_BIT)])
		{ assert(Name<0); }
		bool empty()const{ return 0==(field&bit); }
	};
	inline void __increment_internal_counter(int Name)
	{
		if(Name<0)
		{
			//This shouldn't override the schema.
			__nonplural_bitfield bf(Name,_nonplurals());
			assert(bf.empty()); bf.field|=bf.bit;
		}
		else //Assert no overflow nor misapplied.
		{
			daeChildID &ID = ((daeChildID*)this)[-Name];
			ID++; assert(ID.getName()==Name);
		}
	}
	inline daeChildID __decrement_internal_counter(int Name)
	{
		if(Name<0)
		{
			//This shouldn't override the schema.
			__nonplural_bitfield bf(Name,_nonplurals());
			assert(!bf.empty()); bf.field&=~bf.bit; return Name<<16; //0
		}
		else //Assert no underflow nor misapplied.
		{
			daeChildID &ID = ((daeChildID*)this)[-Name];
			ID--; assert(ID.getName()==Name); return ID;
		}
	}
	inline daeCounter __nonplural_size(int Name)const
	{ 
		assert(Name<0);
		return __nonplural_bitfield(Name,_nonplurals()).empty()?0:1;
	}
	inline daeCounter __plural_size(int Name)const
	{ 
		assert(Name>0); return ((daeChildID*)this)[-Name].getIndex();
	}
	inline daeCounter __plural_or_non_size(int Name)const
	{ 
		return Name>0?__plural_size(Name):__nonplural_size(Name);
	}

COLLADA_(private) //INTERNALS

	friend class XS::Choice;	
	friend class daeMetaElement;		
	template<int grow>
	/**
	 * As long as the strategy is to 0-fill the back of the contents
	 * arrays, all that's required to increase their size is to move
	 * the end-pointer along. This can only increase the size by one.
	 */
	daeContent *_advance_0_terminator()
	{
		daeContents &c = _this(); size_t size = c.size()+1;		
		if(grow==+1) c.grow(size); 		
		else assert(grow==0&&c.getCapacity()>size);
		daeContent *o = c.end(); 
		c.getAU()->setInternalCounter(size); return o;
	}
	template<int grow>
	/**
	 * Returns a completely uninitialized child-node, which must be 
	 * placement-new constructed.
	 */
	daeContent *_append_child()
	{
		//_append_child() should not set the cursor.
		//This family of APIs is designed to do the 
		//minimal amount of work possible.
		return _advance_0_terminator<grow>();
	}
	template<int grow>
	/**
	 * Returns a completely uninitialized child-node, which must be 
	 * placement-new constructed.
	 */
	daeContent *_insert_child(daeCursor &insertion_point)
	{
		daeContents &c = _this();
		//Reminder: not trying to maintain the cursor state if the
		//contents-array is being modified. The cursor algorithms
		//are more for read-only scenarios: that are most common
		//and tend to make up the bulk of high-throughput needs.
		//ALTHOUGH, WHEN ORDINALS HAVE TO BE COUNTED, COUNTING 
		//BACKWARD IN daeElementCM::__place() TAKES MORE WORK.
		cursor() = insertion_point; 
		assert(_>=c.begin()&&_<=c.end());
		_advance_0_terminator<grow>();
		memmove(_+1,_,(c.end()-_-1)*sizeof(daeContent));
		insertion_point = _; return _;
	}
	/**
	 * Removes a child node from the contents-array. 
	 * This converts the child to an empty text-node. The built-in
	 * cursor must be moved if it's on the child.
	 */
	void _remove_child(const daeCursor child_to_remove)
	{	
		daeContents &c = _this();
		assert(child_to_remove>=c.begin()&&child_to_remove<c.end());
		assert(!child_to_remove->hasText());	
		//Convert into a 0-terminated, empty text node.
		((daeContent*)child_to_remove)->_magic_ordinal = daeOrdinals+1;
		((daeText*)child_to_remove)->_text[0] = '\0';
		((daeEOText*)child_to_remove)->_textview[daeEOText::_cspan] = 1;
			//Removing this would be fine. It might touch the
			//cache for no reason if removed.
			if(child_to_remove==_)
		//The built-in cursor cannot rest on text.
		while(_->hasText()) _+=_->getKnownText().span();
	}

	friend class daeElement;
	friend class daeMetaElement;
	//Ch-Ch-Ch-Changes, notices of:
	//
	// TODO: Probably the way these default to calling carry_out_change()
	// should go out of style. It kind of works for DAEP::Note, but here
	// it's just distracting. It should be fixed after the notifications
	// suppression stuff is ironed out.
	//	
	struct _operation : DAEP::Change //ABSTRACT
	{	
		mutable bool b; bool typeLookup;				
		_operation():Change(DAEP::ELEMENT),b(),typeLookup(){}
		~_operation(){ if(!b) carry_out_change(); } 		
		static void typeLookup_insert(daeElement*); //maybe()?
		static void typeLookup_remove(daeElement*); //maybe()?
		inline void maybe(void op(daeElement*))const //LEGACY
		{
			//MIGHT WANT TO DISABLE typeLookup(). BUT NOT YET.
			//#ifdef COLLADA_NOLEGACY
			if(typeLookup) op((daeElement*)element_of_change);
			//#endif
		}		
		#ifdef NDEBUG
		#error 1 is as if the document has no xmlns. This needs work.
		#endif		
		enum{ o=0,l=1 }; //getElementTags().namespaceTag values.		
	};		
	class _remove_operation : virtual public _operation
	{	protected:
		daeCM_Demotion<> &d;		
		virtual void carry_out_change()const
		{
			b = true; remove();			
			//HACK: This can't be the only reference.
			assert(!_object->_compact()); 
			((daeSmartRef<>&)element_of_change).~daeSmartRef();
			if(_object->_getClassTag()--!=l) assert(0);
		}
		daeError remove()const
		{ 
			maybe(typeLookup_remove); //LEGACY
			daeChildID ID = d.insertion_point->_child.ID; 
			daeChildID N = d.content.__decrement_internal_counter(d.name);
			//Reminder: _chooseWithout() looks for 0.
			//_insert_move_operation checks this too.
			d.count = N.getIndex();
			//This could branch on ID==N, but it should 
			//return child_to_remove shortly when ID==N.
			daeCursor _ = d.insertion_point+(N-ID);
			d.content._get<-int(!0)>(_,N)->_child.ID = ID;
			d.content._remove_child(d.insertion_point);
			return ID==N?DAE_OK:DAE_ORDER_IS_NOT_PRESERVED;
		}public:
		_remove_operation(daeCM_Demotion<> &d):d(d)
		{ 
			element_of_change = d.insertion_point->_child.DAEP; 
		}
		inline ~_remove_operation()
		{
			//Should ~daeCM_Demotion() do this?? Regardless, the
			//goal is to not have to do this manually whenever a
			//(re)move is called for. Removes aren't a fast path.
			if(!b) carry_out_change(); d.maybe_demote(); 
		}
	};
	class _insert_operation : virtual public _operation
	{	protected:
		mutable daeError e;
		daeCM_Placement<> &p; 		
		virtual void carry_out_change()const
		{
			b = true; if(DAE_OK!=reparent()) return;			
			insert(); new(&_object) daeSmartRef<>(_object);
			if(_object->_getClassTag()++!=o) assert(0);
		}
		daeError reparent()const
		{
			return e = _object->_reparent_element_to_element(*p.content.getObject()); 
		}
		void insert()const
		{
			maybe(typeLookup_insert); //LEGACY
			p.content.__increment_internal_counter(p.name); daeChildID ID(p.name,p.count);			
			new(p.content._insert_child<+1>(p.insertion_point)) daeContent(p.ordinal,ID,element_of_change);
		}public:		
		_insert_operation(daeCM_Placement<> &p, const DAEP::Element &e):p(p){ element_of_change = &e; }
		operator daeError(){ if(!b) carry_out_change(); return e; } 
	};		
	class _insert_move_operation : _remove_operation, public _insert_operation
	{	protected:									
		virtual void carry_out_change()const
		{
			//Remove converts the CM node to an empty text
			//node, so that the array doesn't shift. So it
			//is done first, as the arrays may be the same.
			//It should also be done first so the bitfield
			//counters don't overflow if they are the same.
			//typeLookup_remove precedes typeLookup_insert.
			b = true; if(reparent()==DAE_OK)
			{
				if(remove()==DAE_ORDER_IS_NOT_PRESERVED)
				if(&_insert_operation::p.content
				 ==&_remove_operation::d.content)
				{
					_insert_operation::p.count--; //EDGE-CASE

					#ifdef NDEBUG
					#error The library must take a stance on if it's
					#error cool to skip subscripts (go over N) or not.
					#endif
					//This should indicate p.count>d.count
					//meaning the subscript is greater than
					//N. Maybe some dae_Array asserts should
					//have fired already???
					assert(p.count==d.count);
				}
				insert(); 
			}			
			assert(l==_object->_getClassTag());
		}public:
		_insert_move_operation(daeCM_Demotion<> &d, daeCM_Placement<> &p)
		:_remove_operation(d),_insert_operation(p,*element_of_change){}		
	};	
	class _insert_new_operation : virtual public _operation
	{	protected:
		daeCM_Placement<> &p; 
		virtual void carry_out_change()const
		{
			maybe(typeLookup_insert); //LEGACY
			b = true; new(&_object) daeSmartRef<>(_object); daeChildID ID(p.name,p.count);
			p.content.__increment_internal_counter(p.name); if(_object->_getClassTag()++!=o) assert(0);
			new(p.content._insert_child<+1>(p.insertion_point)) daeContent(p.ordinal,ID,element_of_change);					
		}public:		
		_insert_new_operation(daeCM_Placement<> &p, const DAEP::Element &e):p(p){ element_of_change = &e; }		
	};
	class _vacate_operation : virtual public _operation
	{	protected:
		daeElement* &child_to_vacate;
		virtual void carry_out_change()const
		{
			maybe(typeLookup_remove); //LEGACY
			b = true; child_to_vacate = nullptr;
			//HACK: This can't be the only reference.
			assert(!_object->_compact()); 
			((daeSmartRef<>&)_object).~daeSmartRef();
			if(_object->_getClassTag()--!=l) assert(0);			
		}public:
		_vacate_operation(daeContent &ctv):child_to_vacate(ctv._child.ref)
		{ element_of_change = ctv._child.DAEP; }		
	};
	class _populate_new_operation : virtual public _operation
	{	protected:
		static const daeError e = DAE_OK;
		daeElement* &vacancy;
		virtual void carry_out_change()const
		{
			b = true; populate();
			if(_object->_getClassTag()++!=o) assert(0);
		}
		void populate()const
		{
			maybe(typeLookup_insert); //LEGACY
			assert(nullptr==vacancy); 
			new(&vacancy) daeSmartRef<>(_object);

		}public:		
		_populate_new_operation(daeContent &v, const DAEP::Element &e)
		:vacancy(v._child.ref){ element_of_change = &e; }
	};		
	class _populate_operation : public _populate_new_operation
	{	protected:		
		daeObject &parent;
		mutable daeError e;
		virtual void carry_out_change()const
		{
			b = true; if(DAE_OK!=reparent()) return;
			
			_populate_new_operation::carry_out_change();
		}
		daeError reparent()const
		{
			return e = _object->_reparent_element_to_element(parent); 
		}public:		
		_populate_operation(daeContent &v, daeElement &p, const DAEP::Element &e)
		:_populate_new_operation(v,e),parent((daeObject&)p){}
	};		
	class _populate_move_operation : _remove_operation, public _populate_operation
	{	protected:
		virtual void carry_out_change()const
		{
			//One should be nullptr and one should not,
			//-so it should be impossible for them to be
			//the same, so at least there's no worry there.
			//NOTE: THERE ISN'T AN OPERATION THAT ALLOWS AN
			//ELEMENT TO BE REASSIGNED TO ITSELF. THAT CASE
			//MUST BE DETECTED AND HANDLED IN ADVANCE AS-IS.
			assert(&vacancy!=&d.insertion_point->_child.ref);

			//Remove converts the CM node to an empty text
			//node, so that the array doesn't shift. So it
			//is done first, as the arrays may be the same.
			//It should also be done first so the bitfield
			//counters don't overflow if they are the same.
			b = true; if(reparent()==DAE_OK)
			{	
				if(remove()==DAE_ORDER_IS_NOT_PRESERVED)
				{
					//There's nothing to do in this case.
					//The subscript may change, and users
					//just have to live with it, or not do
					//this.
				}
				populate();
			}			
			assert(l==_object->_getClassTag());
		}public:
		_populate_move_operation(daeCM_Demotion<> &d, daeContent &v, daeElement &p)
		:_remove_operation(d),_populate_operation(v,p,*element_of_change){}		
	};
	template<class Op> class __rename : public Op
	{
		daePseudonym name;
		virtual void carry_out_change()const
		{
			Op::carry_out_change();
			if(Op::e==DAE_OK)
			((daeElement*)Change::_object)->getNCName() = name;			
		}public:
		template<class A, class B>
		__rename(A &a, B &b, daePseudonym name)
		:Op(a,b),name(name){}
		template<class A, class B, class C>
		__rename(A &a, B &b, C &c, daePseudonym name)
		:Op(a,b,c),name(name){}
	};	
	typedef __rename<_insert_operation> _insert_rename_operation;
	typedef __rename<_insert_move_operation> _insert_move_rename_operation;
	typedef __rename<_populate_operation> _populate_rename_operation;		
	typedef __rename<_populate_move_operation> _populate_move_rename_operation;	
	
COLLADA_(public) //SEMI-INTERNAL _get METHOD (_ can be read as "cursor.")
	
	template<class Type, int Name> 
	/**NON-CONST-ONLY
	 * Gets a @c dae_ (hit or miss.) This form uses the provided "cursor."
	 */	
	inline dae_<Type> &_get(daeCursor &cursor, dae_<> &id, bool miss=false)
	{
		return id._return<Type>(this,miss?nullptr:_get<Name>(cursor,id._id));
	}
	template<class Type, int Name> 
	/**NON-CONST-ONLY
	 * Gets a @c dae_ (hit or miss.) This form uses the built-in (shared) "cursor."
	 */	
	inline dae_<Type> &_get(dae_<> &id, bool miss=false)
	{
		return id._return<Type>(this,miss?nullptr:_get<Name>(cursor(),id._id));
	}
	template<int Name>
	/**
	 * Gets a child. This form uses the built-in (shared) "cursor."
	 * @see the two argument form's Doxygentation.
	 */
	inline daeContent *_get(daeChildID::POD id)
	{
		return _get<Name>(cursor(),id);
	}
	template<int Name>
	/**CONST-FORM
	 * Gets a child. This form uses the built-in (shared) "cursor."
	 * @see the two argument form's Doxygentation.
	 */
	inline const daeContent *_get(daeChildID::POD id)const
	{
		return _get<Name>(cursor(),id);
	}		
	template<int Name> 
	/**CONST-FORM
	 * Gets a child. This form uses the built-in (shared) "cursor."
	 * @see non-const form's Doxygentation.
	 */
	inline const daeContent *_get(daeCursor &cursor, daeChildID::POD id)const
	{
		return const_cast<daeContents_base*>(this)->_get<Name>(cursor,id);
	}
	template<int Name> 
	/**
	 * Gets a child, or, @c nullptr if the child-ID does not exist.
	 * @todo Can this be optimized by passing DAEP Child's template 
	 * parameters via @c dae_Array?
	 *
	 * Algorithm
	 * ==========
	 * If cursor where Name<0 (alone) try current position, and set to the next position.
	 * If cursor where Name>=0 (array) try to do random-access, and failing that:
	 * -If names differ, set the cursor to the 0th subscript, readying for random-access.
	 * (And in all cases, if all else fails, find the ID, and ready at the next position.)
	 */
	inline daeContent *_get(daeCursor &cursor, daeChildID::POD id)
	{
		daeContents &c = _this();
		//NEW: All cursors are held to the same requirements as the
		//built-in cursor, since the algorithms don't differentiate.
		//It's recommended to initialize a cursor end() unless it's
		//known to not be a non-child/text node.
		assert(size_t(cursor-c.data())<=c.size()&&cursor->hasChild()&&id!=0);
		return (daeContent*)((Name<0?__get1:__get2)(cursor,id,c.data(),c.end()));
	}

COLLADA_(private) //BASIC (text nodes are not skimmed) CURSOR ALGORITHM

	////////////////////////////////////////////////////////////////////////
	//NOTICE: These are amortized for worst case scenarios given maximally//
	//limited input schema-wise. IOW this is best-case starting point wise//
	////////////////////////////////////////////////////////////////////////

	static daeCursor __get1(daeCursor &_, daeChildID ID, daeCursor d, daeCursor e)
	{
		daeCursor o = _->_child.ID==ID?_:__get1_missed(_,ID,d,e);
		for(_++;_->hasText();_+=_->getKnownStartOfText().span()); return o;
	}	
	static daeCursor __get2(daeCursor &_, daeChildID ID, daeCursor d, daeCursor e)
	{	
		//This is implementing a random-access algorithm for better or worse.
		//Pre-2.5 arrays were random-access, and it can only help with large
		//blocks of like-elements; A small array/block is trivial either way.
		daeCursor o = _+ID.getIndex(); 
		return o<e&&ID==o->_child.ID&&o->hasChild()?o:__get2_missed(_,ID,d,e);
	}	
	COLLADA_NOINLINE
	static daeCursor __get1_missed(daeCursor &_, daeChildID ID, daeCursor d, daeCursor e)
	{
		//_--; counteracts __get1's ++ and keeps _ in place if ID isn't found.
		if(!__reset_cursor<0>(_,ID,d,e)){ _--; return nullptr; }else return _;
	}
	COLLADA_NOINLINE
	static daeCursor __get2_missed(daeCursor &_, daeChildID ID, daeCursor d, daeCursor e)
	{
		if(ID.getName()!=_->_childID().getName())		
		return __reset_cursor<0>(_,ID&~0xFFFF,d,e)?__get2(_,ID,d,e):nullptr;
		return __get1(_,ID,d,e);		
	}
	template<int Inner>
	static bool __reset_cursor(daeCursor &_, daeChildID::POD ID, daeCursor d, daeCursor e)
	{
		for(daeCursor o=_;o<e;o++)
		if(o->_child.ID==ID&&o->hasChild()){ _ = o; return true; }
		if(!Inner)
		if(__reset_cursor<1>(d,ID,d,e=_)){ _ = d; return true; } return false;
	}
}; 

/**TEMPLATE-SPECIALIZATION, AGGREGATE
 * @c dae_<> is @c dae_<void>.
 * @c dae_ is privately based on @c dae_<>.
 * It can have operators, but its methods must be underscored.
 * 
 * @note @c dae_<> may seem unnecessary, but it's designed so
 * it can be reused in theory. It makes operators of @c dae_<T>
 * easier to reason about. Especially when there are variations.
 *
 * @remarks The library can do better than this, but it requires
 * more compile-time-constant variables to do so. It will be some
 * work to get that kind of system into place. This implements the
 * DAEP logic in the meantime.
 */
template<> class dae_<>
{
COLLADA_(public) //AGGREGATE MEMBERS
	/**
	 * This has evolved mid-development. Typically @c _id is 
	 * initialized via an aggregate construction. It might be
	 * a good idea to initialize @c _content also. The pointer
	 * members had been a @c union, and there was a hit-or-miss
	 * state. But these are temporary-objects for the most part,
	 * -and so compilers should just inline them into surrounding
	 * code. It's left to compilers to "optimize-away" its quirks.
	 */
	daeChildID::POD _id; daeContents *_content; daeContent *_it;
	/**
	 * This calculates if there is a viable non-placeholder pointer
	 * or not. It implements the + and -> semantics. The older system
	 * worked well until the ="" operator was added. "" couldn't easily
	 * distinguish between the two kinds of misses.
	 */
	inline bool _missed(){ return _it==nullptr||_it->_child_ref()==nullptr; }

COLLADA_(public) //daeContents uses this.

	template<class T> 
	/** 
	 * This is written to be convenient to @c daeContents. 
	 * @a c may be thought of as an anachronism, but in theory
	 * the user could switch between contents-arrays.
	 */
	inline dae_<T> &_return(daeContents_base *c, daeContent *got)
	{
		const daeElement *upcast = dae((T*)nullptr);
		_content = (daeContents*)c; _it = got; return (dae_<T>&)*this;
	}	

COLLADA_(public) //These are used by _dae<T>.

	template<class LAZY> //CIRCULAR-DEPENDENCY
	inline const daeChildRef<LAZY> &_maybe_add()
	{
		return _missed()?_add<LAZY>():_it->_child_ref<LAZY>(); 
	}	

	template<class LAZY> //CIRCULAR-DEPENDENCY
	/** 
	 * Performs -> and + semantics. 
	 */
	inline const daeChildRef<LAZY> &_add()
	{			
		assert(_missed());
		return (daeChildRef<LAZY>&)daeMeta::_addWID(_id,*_content,_it);
	}

	template<class LAZY, class U> //CIRCULAR-DEPENDENCY
	/**
	 * Performs assignment-operator semantics. 
	 */
	inline const daeChildRef<LAZY> &_set(const U &seat)
	{	
		return (daeChildRef<LAZY>&)daeMeta::_setWID(_id,seat,*_content,_it);
	}
	template<class LAZY> //CIRCULAR-DEPENDENCY
	/**
	 * Performs assignment-operator semantics; Assignment to "".
	 */
	inline void _set()
	{
		//This assert is designed to do boundary checks for array-based removals.
		//Note that removals in the middle of the array shifts higher subscripts.
		assert(_it!=nullptr||0==(_id&0xFFFF)); daeMeta::_removeWID(*_content,_it); 
	}
};
  
/**AGGREGATE
 * _ is pronounced "cursor." (It looks like a cursor.)
 * @tparam T is expected to be a generated class based on @c DAEP::Element.
 *
 * This is a short-lived class that opens a window to use various operators
 * to do fancy non-standard things with, that closes immediately. It should
 * not live beyond one crack at it. There are parallels to each operator in
 * @c dae_Array, that apply to the first element only, must like -> can be
 * used with C-arrays.
 *
 * @remarks At present there's no reason to use dae_ for const element-refs.
 * Therefore all methods are non-const. 
 * In addition methods should be limited to operators. Non-operator methods
 * are reserved by @c daeSmartRef.
 */
template<class T> class dae_ : public dae_<>
{						
	friend class daeContents_base;

	/** Prevent operator T* as C-array. */
	inline void operator[](int)const{}
	/** Not-allowing, for const-propogation reasons. */
	inline void operator=(const daeSmartRef<const T>&){}

	template<class,int> friend class dae_Array;
	/**
	 * Disabled Constructors & Assignment Operator
	 *
	 * This is intended to discourage the C++11 @c auto keyword.
	 * @c dae_<> still works.
	 */
	dae_(); dae_(const dae_&cp):dae_<>(cp){} void operator=(const dae_&)const;

COLLADA_(public) //OPERATORS (This class is never const.)
	/**
	 * @return Returns @c nullptr if no placeholder exists.
	 * @note This is the only non return-by-reference operator.
	 * @see deprecated @c cast().
	 */
	inline operator T*()
	{
		return _it==nullptr?nullptr:_it->_child_ref<T>(); 
	}

	/**
	 * This is for interfaces that must treat const and
	 * non-const versions of @c dae_Array consistently.
	 */
	inline const daeChildRef<T> &_hit()const
	{
		assert(!_missed());	return _it->_child_ref<T>(); 
	}

	/** 
	 * Adds an element when one had not previously existed. 
	 * @return Returns @c T& which can be used to write less
	 * complicated template deduction than @c operator+(), or
	 * get a pointer with &* without writing @c .operator->().
	 * @remarks This is in line with std::unique_ptr semantics.
	 */
	inline T &operator*(){ return *(T*&)_maybe_add<T>(); }
	
	/** 
	 * Adds an element when one had not previously existed. 
	 */
	inline const daeChildRef<T> &operator+(){ return _maybe_add<T>(); }

	/** 
	 * Adds an element when one had not previously existed. 
	 * (Caller is obliged to do something with the dangling @c ->.)
	 * @see @c operator+ for a non-dangling @c -> alternative.
	 */
	inline T *operator->(){ return (T*&)_maybe_add<T>(); }
	
	/**
	 * Removes this element from the contents-array by means
	 * of converting it to an empty text-node. This will not
	 * disrupt the array, and so is recommended. The removed
	 * nodes can be cleaned out en masse as a maintenance op. 
	 */
	inline daeString1 operator=(daeString1 empty_string_literal)
	{	
		assert(empty_string_literal[0]=='\0'); _set<T>();
		return empty_string_literal;
	}
	template<class S>
	/**
	 * Assigns a new element-pointer to this position in the
	 * contents-array. 
	 */
	inline const daeChildRef<S> &operator=(S *seat)
	{
		_upcast<T>C2082(seat); return _set<S>(seat);
	}	
	template<class S>
	/** Implements @c operator=() for const-pointer types. */
	inline void operator=(const S *seat)
	{
		daeCTC<0>("Assigning const-pointer to non-const array.");
	}
	template<template<class> class R, class S>
	/**
	 * Assigns a new element-pointer to this position in the
	 * contents-array. 
	 */
	inline const daeChildRef<S> &operator=(const R<S> &seat)
	{	
		_upcast<T>C2082(seat); return _set<S>(seat);
	}
	//daeElement not being a DAEP::Element complicates this.
	template<class T> struct _upcast{ _upcast(const T*){} };
	template<> struct _upcast<daeElement>
	{ _upcast(const daeElement*){} _upcast(const DAEP::Element*){} };
	template<> struct _upcast<DAEP::Element>
	{ _upcast(const daeElement*){} _upcast(const DAEP::Element*){} };	

#ifdef COLLADA_NODEPRECATED

COLLADA_(public) //daeSmartRef compatibility layer

	COLLADA_DEPRECATED("operator T*")
	/**@see daeSmartRef::cast()
	 * Function that returns a pointer to object being reference counted.
	 * @return the object being reference counted.
	 */
	inline T *cast()const{ return operator T*(); }

#endif //COLLADA_NODEPRECATED
};

template<class Type,int Name, bool alone=(Name<0)> 
class dae_Array_base;

template<class Type, int Name>
/**
 * COLLADA C++ child-element array class that mimics @c daeArray.
 *
 * _ is pronounced "cursor." (It looks like a cursor.)
 * @tparam Type is expected to be a generated class based on @c daeElement.
 * Typically each class has a typedef that defines a corresponding @c dae_Array.
 *
 * @c dae_Array<Type,Name> can decay into @c dae_Array<Type>; A SPECIALIZATION that 
 * can live outside of its @c DAEP::Child wrapper, in order to function as a classic
 * @c daeArray-like class. This is a back-compatibility measure--but also a courtesy,
 * -as code may often wish to tear-off an array by C++ reference. Which is difficult
 * to do here, without the C++11 @c auto keyword. (Not knowing what @a Name is, will
 * instantiate slightly suboptimal code--but this shouldn't be considered whatsoever!)
 */
class dae_Array : public dae_Array_base<Type,Name>
{ 
	using dae_Array_base<Type,Name>::_i;
	using dae_Array_base<Type,Name>::_content;

COLLADA_(public) //ACCESSORS & MUTATORS

	using dae_Array_base<Type,Name>::size;

	/**
	 * @return Returns @c true if @c 0==size().
	 */
	inline bool empty()const{ return 0==size(); }

	/**
	 * Gets the first element in the array.
	 * Here @c size() CAN be 0. The returned cursor is assignable.
	 * @return Returns a cursor that is always assignable,
	 * -although, if a compatible element is not assigned, or all bets are off.
	 */
	inline dae_<Type> front(){ return operator[](0); }	
	/**CONST-FORM
	 * Gets the first element in the array.
	 */
	inline const daeChildRef<const Type> &front()const{ return operator[](0); }	

	/**
	 * Gets the last element in the array.
	 * Here @c size() CAN be 0. The returned cursor is assignable.
	 * @return Returns a cursor that is always assignable,
	 * -although, if a compatible element is not assigned, or all bets are off.
	 */
	inline dae_<Type> back(){ daeCounter n = Name<0?0:size(); return operator[](n==0?0:n-1); }	
	/**CONST-FORM
	 * Gets the last element in the array.
	 */
	inline const daeChildRef<const Type> &back()const{ return operator[](Name<0?0:size()-1); }

	/**WARNING
	 * Post 2.5 @c push_back() should be used instead of the old @c append() API
	 * for single argument uses.
	 */
	inline void push_back(Type *ref){ assert(Name>=0||0==size()); operator[](size()) = ref; }

	/**
	 * Complements @c push_back(). (Removes last item.)
	 */
	inline void pop_back(){ back() = ""; }

#if 2 == COLLADA_DOM

COLLADA_(public) //ACCESSORS & MUTATORS (2)
	/** 
	 * Oldstyle method name. 
	 * @see @c dae_Array_base::size().
	 */
	inline daeCounter getCount()const{ return size(); }
	/** 
	 * Oldstyle method name. 
	 * @see @c dae_Array_base::resize().
	 */
	inline void setCount(daeCounter nElements){ resize(nElements); }

	/**
	 * Sets a specific index in the @c dae_Array, growing the array if necessary.
	 * @param index Index of the object to set.
	 * @param value Value to store at index in the array.
	 */
	inline void set(daeCounter index, const Type *val){ operator[](index) = val; }
	/**@see operator[].
	 * Gets the object at a specific index in the @c dae_Array.
	 * @param index Index of the object to get. If @a Index exceeds
	 * @c size(), the count is extended IF the cursor is assigned to.
	 * @return Returns an assignable cursor at @c index.
	 */
	inline dae_<Type> get(daeCounter index){ return operator[](index); }
	/**CONST-FORM
	 * Gets the object at a specific index in the @c dae_Array.
	 * @return Returns the object at index.
	 */
	inline const daeChildRef<const Type> &get(daeCounter index)const
	{ 
		return operator[](index); 
	}

#endif //2 == COLLADA_DOM

COLLADA_(public) //OPERATORS	
	/**
	 * Converts into the first element.
	 * @return Returns @c nullptr if @c empty().
	 */
	inline operator Type*()
	{
		if(empty()) return nullptr;
		const daeContent *got = _content()._get<Name>(_i(0));
		return got==nullptr?nullptr:(Type*)got->_child.ref;
	}	
	/**INTERNAL
	 * This might be sligtly better done by @c DAEP::Child.
	 */
	inline bool __inoperable_ptr()const
	{
		return (char*)this-(char*)nullptr<65536&&(Name<0||Name>1);
	}	
	/**WARNING, CONST-FORM
	 * Converts into the first element.
	 * @return Returns @c nullptr if @c empty().
	 * @warning This works with @c operator->() to produce
	 * a @c nullptr result if the source/parent was itself
	 * a @c nullptr result. 
	 * This does not apply @c Name is 0 or 1.
	 * @see @c const variety of operator->().
	 */
	inline operator const Type*()const
	{
		//FOR BETTER OR WORSE
		//This seems to facilitate better coding practices.
		if(__inoperable_ptr()) 
		return nullptr;
		return const_cast<dae_Array*>(this)->operator Type*();
	}	

	/**
	 * Adds an element when one had not previously existed. 
	 * @return Returns @c T& which can be used to write less
	 * complicated template deduction than @c operator+(), or
	 * get a pointer with &* without writing @c .operator->().
	 * @remarks This is in line with std::unique_ptr semantics.
	 */
	inline Type &operator*(){ return *operator[](0); }
	/**WARNING, CONST-FORM
	 * Dereferences a the first instance of this child, which 
	 * must exist.
	 * @warning This operator does not enjoy deferred pointer
	 * chain dereferencing semantics; as @c operator->() does.
	 */
	inline const Type &operator*()const
	{
		return *operator[](0); //return *operator const Type*(); 
	}

	/** 
	 * Adds an element when one had not previously existed. 
	 */	
	inline const daeChildRef<Type> &operator+(){ return +operator[](0); }	

	/** 
	 * Adds an element when one had not previously existed. 
	 * (Caller is obliged to do something with the dangling @c ->.)
	 * @see @c operator+ for a non-dangling @c -> alternative.
	 */
	inline Type *operator->(){ return +operator[](0); }	
	/**WARNING, CONST-FORM
	 * Gets the first element in the array. Caveats follow.
	 *
	 * @warning This is the @c const form of this operator.
	 * At first the idea was to @c assert(!empty()). Still
	 * code often has to check for @c nullptr not just for
	 * the element it is interested in, but also every one
	 * of its parents along the chain. Because the library
	 * is high-level by nature, it seems best to allow for
	 * @c nullptr descedents, but only for @c operator->()
	 * and only for @c const @c dae_Array, and only if the
	 * @c Name parameter is not 0 and not 1. This behavior
	 * chains until @c operator const Type*() completes it.
	 * @see @c operator const Type*().
	 */
	inline const Type *operator->()const
	{
		//FOR BETTER OR WORSE
		//This seems to facilitate better coding practices.
		if(__inoperable_ptr()||empty()) 
		return nullptr;	
		//assert(!empty());
		const daeContent *got = _content()._get<Name>(_i(0));
		assert(got!=nullptr); 
		return (const Type*)got->_child.ref;
	}
	
	template<class I> //I defeats operator Type*().
	/**
	 * Gets the object at a specific index in the @c dae_Array.
	 * @param index Index of the object to get. If @a Index exceeds
	 * @c size(), the count is extended IF the cursor is assigned to.
	 * @return Returns an assignable cursor at @c index.
	 */
	inline dae_<Type> operator[](I indexIn)
	{									  
		daeCounter index = indexIn;
		//This is enforcing a contiguity requirement, meaning
		//that user code shouldn't skip subscripts, because it's not
		//clear what the effects are (at this time it's not studied and is
		//almost guaranteed to crash the library one way or another.)
		//Going forward it's probably unworth affording special treatement.
		if(Name>=0) assert(index<=size());

		dae_<> _ = { dae_Array_base::_i(index) }; 
		return _content()._get<Type,Name>(_,index>=size());
	}	
	template<class I> //I defeats operator Type*().
	/**CONST-FORM
	 * Gets the object at a specific index in the @c dae_Array.
	 * @return Returns the object at index.
	 */
	inline const daeChildRef<const Type> &operator[](I indexIn)const
	{
		daeCounter index = indexIn;
		//This assert should support overloading the "alone" arrays
		assert((Name<0?0:index)<size());
		const daeContent *got = _content()._get<Name>(_i(index));
		assert(got!=nullptr); 
		return (daeChildRef<const Type>&)got->_child.ref;
	}	
	  
COLLADA_(public) //operator=

	//C++98/03 SUPPORT
	//DAEP::Child defines this.
	//inline dae_Array &operator=(const S &cp)
	//inline daeString1 operator=(daeString1 empty_string_literal)
	COLLADA__DAEP__Child__union__assignment_operator(Type,Name)
	template<class S>
	/** Implements @c operator=() for pointer types. */
	void __assign(const S &cp,...)
	{
		operator[](0) = cp;
	}
	template<class> struct __assign_if_class{};
	template<class S>
	/** Implements @c operator=() for class types. */
	void __assign(const S &cp, __assign_if_class<S S::*>*)
	{
		__assign_class<S>(cp,nullptr);
	}
	template<class S>
	/** Implements @c operator=() for @c daeSmartRef types. */
	void __assign_class(const S &cp, typename S::__COLLADA__T*)
	{
		operator[](0) = cp;
	}	
	template<class S>
	/**LEGACY, OVERLOAD, ***CAUTION!*** 
	 *
	 * CAUTION: Pre 2.5 this would have allowed a 
	 * child to have multiple parents. 
	 * In 2.5 assigning to the contents-array moves 
	 * a child from parent to parent.
	 *
	 * Overloaded assignment operator.
	 * @param other A reference to the array to copy
	 * @return A reference to this object.
	 *
	 * @remarks, this does pointer assignment only.
	 * It may not be meaningful, but must mimic @c daeArray.
	 */
	void __assign_class(const S &cp,...)
	{
		daeChildID::POD iN = cp.size(), i = 0;
		resize(iN);									
		dae_<> id = { _i(0) };
		daeContents &ci = _content();
		daeCursor i_ = ci.end();
		for(iN+=id._id;id._id<iN;id._id++) 
		{
			ci._get<Type,Name>(i_,id) = cp[i++];
		}
	}

#if 2 == COLLADA_DOM

COLLADA_(public) //QUESTIONABLE LEGACY OPERATORS (2)

	#ifndef COLLADA_NODEPRECATED
	template<class S>
	COLLADA_DEPRECATED("== crashes on text or null-content.")
	/**WARNING, LEGACY
	 * Overloaded equality operator.
	 * @param other A reference to the other array.
	 * @return true if the arrays are equal, false otherwise.
	 * @remarks, this does pointer comparison only.
	 * It may not be meaningful, but must mimic @c daeArray.
	 *
	 * @warning THIS CRASHES IF THERE IS TEXT OR PLACEHOLDERS.
	 * NOT SUPPORTING 0 PLACEHOLDERS IS A BREAKING CHANGE. TO
	 * DO SO A NEW DEREFERENCING CHECK WOULD HAVE TO BE ADDED.
	 */
	inline bool operator==(const S &cmp)const
	{
		daeCounter iN = size();
		if(iN!=cmp.size()) return false;		
		daeChildID id = _i(0);
		const daeContents &ci = _content();
		daeCursor i_ = ci.end();
		for(daeCounter i=0;i<iN;i++)		
		if(ci._get<Name>(i_,id++)->child.ref!=cmp[i]) 
		return false; return true;
	} 
	#endif

#endif //2 == COLLADA_DOM
};
					
template<class Type, int Name>
/**INTERNAL, PARTIAL-SPECIALIZATION
 * This specialization implements arrays that can have more than one child.
 */
class dae_Array_base<Type,Name,false> 
{
	template<class,int> friend class dae_Array;

COLLADA_(public) //++ OPERATOR
	/** 
	 * Creates a new element on the back of the array.
	 */	
	inline const daeChildRef<Type> &operator++()
	{
		return +((dae_Array<Type,Name>*)this)->operator[](size());
	}	

COLLADA_(private) 
	/** 
	 * Rather than an index, this holds a count
	 * instead. The @c Name==0 form gets "Name"
	 * from @c _counter as @a Name is not known.
	 */
	daeChildID::POD _counter;
	 
	/** Locate the contents-array. */
	inline daeContents &_content()
	{
		int os = Name==0?_counter>>16:Name;
		return *(daeContents*)(this+os);
	}
	/** Locate the contents-array. CONST-FORM */
	inline const daeContents &_content()const
	{
		return const_cast<dae_Array_base*>(this)->_content();
	}

	/**
	 * Construct child-ID from @a index and @a Name.
	 * If @a Name is 0, the name ID must be pulled from
	 * @c _counter's bits 16-through-32.
	 */
	inline daeChildID _i(daeCounter index)const
	{
		assert(index<0xFFFF);
		return index|(Name==0?_counter&0xFFFF0000:Name<<16);
	}
	
COLLADA_(public) //ACCESSORS & MUTATORS
	/**
	 * Gets the number of elements name @c Name in the contents-array.
	 * @note @c nullptr placeholders are included in the final count.
	 * @return Returns the number of items stored in this @c dae_Array.
	 */
	inline daeCounter size()const
	{ 
		return (daeCounter&)_counter&0xFFFF; 
	}
	/**
	 * Sets the number of placeholders for children name @c Name in
	 * the contents array.
	 * If the array increases in size, the refs will be @c nullptr.
	 * @param nElements The new size of the array.
	 */
	inline void resize(daeCounter nElements)
	{
		daeMeta::_resizeWID<!'Y'>(_i(nElements),_content(),size());
		assert(size()==nElements);
	}	

COLLADA_(public) //DOWN-CONVERSION OPERATOR
	/** 
	 * Tears off an array by address of this type, without knowing its @a Name. 
	 * @note that operating without @a Name is sub-optimal, but is possible, and
	 * is implemented to be backward compatible with the 2.x style.
	 */
	inline operator dae_Array<Type,0>&(){ return (dae_Array<Type,0>&)*this; }
	/**CONST-FORM
	 * Tears off an array by address of this type, without knowing its @a Name. 
	 * @note that operating without @a Name is sub-optimal, but is possible, and
	 * is implemented to be backward compatible with the 2.x style.
	 */
	inline operator const dae_Array<Type,0>&()const{ return (dae_Array<Type,0>&)*this; }
};

template<class Type, int Name>
/**INTERNAL, PARTIAL-SPECIALIZATION
 * This specialization implements arrays that can have only one child.*
 * *Still these children can be accessed like arrays, with the [] operator.
 * Users may want to disable this ability, however it's provided in case
 * users wish to, or need to, deviate from the XSD schema that was used 
 * to generate their classes--and so code can treat them as-if arrays.
 */
class dae_Array_base<Type,Name,true> 
{	
	template<class,int> friend class dae_Array;

COLLADA_(private)
	/**UNION
	 * This WAS the contents-array's cursor. It isn't modified.
	 * @remark Calling _counter for Microsoft's natvis product.
	 */
	daeCounter _counter;
	
	/** Construct child-ID from @a index and @a Name. */
	inline daeChildID _i(daeCounter i)const
	{
		return daeChildID(Name,i); daeCTC<(Name<0)>();
	}

	/** Locate the contents-array. */
	inline daeContents &_content()
	{
		return (daeContents&)*(this+1);
	}
	/** Locate the contents-array. CONST-FORM */
	inline const daeContents &_content()const
	{
		return (const daeContents&)*(this+1);
	}

COLLADA_(public) //CONSTRUCTORS

	//There are no constructors. This class must work w/ @c union.

COLLADA_(public) //ACCESSORS & MUTATORS	
	/**
	 * Sets the number of placeholders for children name @c Name in
	 * the contents array.
	 * If the array increases in size, the refs will be @c nullptr.
	 * @param nElements The new size of the array.
	 */
	inline void resize(daeCounter nElements)
	{
		//The stance here, is if nElements exceeds 1 then the overflow
		//would have to be assigned to daeOrdinal0. The generator code
		//is calling "unnamed" elements "extra-schema" and so just for
		//symmetry, it seems like overflow would go against the schema,
		//-and so should also be called "extra-schema" for consistency.
		assert(nElements<=1);

		daeMeta::_resizeWID<!'Y'>(_i(nElements),_content(),size());
		assert(size()==(nElements>0?1:0));
	}
	/**
	 * Retrieves 0 or 1 count from a bit-field. 
	 */
	inline daeCounter size()const
	{ 
		daeCTC<(Name<0)>();
		return ((daeContents_base*)(this+1))->__nonplural_size(Name);
	}	
};

//SCHEDULED FOR REMOVAL
/**C-PREPROCESSOR-MACRO
 * TODO: EVENTUALLY THIS SHOULD BE REPLACED
 * BY SOME ATTRIBUTE DATA-MEMBERS THAT WILL 
 * GET THE ALIGNMENT ON TRACK. IF THERE ARE
 * NO ATTRIBUTES, ALIGNMENT ISN'T NECESSARY
 */
#define COLLADA_WORD_ALIGN COLLADA_ALIGN(COLLADA_PTR_CHAR)
//__declspec(align(X)) doesn't accept sizeof().
#if UINT_MAX < COLLADA_UPTR_MAX	
#define COLLADA_PTR_CHAR 8
#else
#define COLLADA_PTR_CHAR 4
#endif
static daeCTC<COLLADA_PTR_CHAR==sizeof(void*)> COLLADA_PTR_CHAR_check;
/**C-PREPROCESSOR-MACRO
 * This macro is used to align the cursor-arrays with the contents-array.
 * @param align is 0 if there is a soft/odd number of 32-bit words in all.
 * If 0, then 64-bit builds add a padding word.
 * 0 is used because 0,0 is less conspicuous. It's 1 if there are plurals.
 * In which case, an odd value is hard/required to find the contents-array.
 * @param words is the number of nonplural cursor-arrays divided by 32, so
 * that they are divided into groups of 32, each bit being a 0-or-1 counter.
 *
 * @remarks The name comes from the last/shared pointer-to-member called "_N."
 * The need comes from packing the 32-bit values on 64-bit builds. Originally,
 * -the cursor-arrays would be behind the contents-array, but in order to have
 * an embedded contents-array, they must be placed in the front, so they aren't
 * in the way. Using 64-bit counters would make the alignment problem go way, at
 * the cost of doubling memory for plural cursor-arrays.
 * @note Each 32-bit counter is really a 16-bit counter. The other 16-bits are an
 * ID. This is for backward-compatibility mainly. The ID is an arbitrary template
 * parameter. By storing it, client code can get a reference without using the ID
 * or the C++11 @c auto keyword.
 */
#define COLLADA_DOM_N(align,words) COLLADA_DOM_N_(align,words)
//If check1 fails, generators must know what else to divide by.
//If check2 fails, there will have to be a manual fallback strategy.
static daeCTC<32==sizeof(daeCounter)*CHAR_BIT> COLLADA_DOM_N_check1;
static daeCTC<0==sizeof(void*)%sizeof(daeCounter)> COLLADA_DOM_N_check2;
#if UINT_MAX < COLLADA_UPTR_MAX	
#define COLLADA_DOM_N_0 daeCounter __padding__;
#define COLLADA_DOM_N_1
#define COLLADA_DOM_N_(align,words) COLLADA_DOM_N_##align COLLADA_DOM_N_FILL_##words
#else
#define COLLADA_DOM_N_(align,words) COLLADA_DOM_N_FILL_##words
#endif
#define COLLADA_DOM_N_FILL_0 //Just add more upon request.
#define COLLADA_DOM_N_FILL_1 daeCounter __nonplurals__[1];
#define COLLADA_DOM_N_FILL_2 daeCounter __nonplurals__[2];
#define COLLADA_DOM_N_FILL_3 daeCounter __nonplurals__[3];
#define COLLADA_DOM_N_FILL_4 daeCounter __nonplurals__[4];
#define COLLADA_DOM_N_FILL_5 daeCounter __nonplurals__[5];
#define COLLADA_DOM_N_FILL_6 daeCounter __nonplurals__[6];
#define COLLADA_DOM_N_FILL_7 daeCounter __nonplurals__[7];
#define COLLADA_DOM_N_FILL_8 daeCounter __nonplurals__[8];

//---.
}//<-'

#endif //__COLLADA_DOM__DAE_CONTENT_H__
/*C1071*/