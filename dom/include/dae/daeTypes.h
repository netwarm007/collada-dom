/*
 * Copyright 2006 Sony Computer Entertainment Inc.
 *
 * Licensed under the MIT Open Source License, for details please see license.txt or the website
 * http://www.opensource.org/licenses/mit-license.php
 *
 */

#ifndef __COLLADA_DOM__DAE_TYPES_H__
#define __COLLADA_DOM__DAE_TYPES_H__
				
#include "daeError.h"

#ifndef COLLADA_DOM_USERDATA
#define COLLADA_DOM_USERDATA void //LEGACY
#endif 

COLLADA_(namespace)
{
	/** daeObject tags */
	typedef unsigned char daeTag;
	/**SINGLETON
	 * @c COLLADA::DOM_process_share is wrapped up in
	 * this class so it's auto-initialized at startup.
	 */
	extern struct daePShare
	{
		daeTag tag; daeOK OK;

		operator daeTag&(){ return tag; }
		
		COLLADA_DOM_LINKAGE 
		/**NOT-THREAD-SAFE
		 * This is more or less a @c private "init" API. 
		 * It was "COLLADA::DOM_grant_process_share()."
		 */
		daeOK grant(size_t=sizeof(daePShare));
		
		/**
		 * Dynamic clients must define: 
		 * ~~~~~~~~~~~~~~~~~~~~~~{.C++}
		 * //Initialize the process-share.
		 * extern COLLADA::daePShare COLLADA::DOM_process_share = 0;
		 *
		 * @param _extern_ is just so C++ lets the @c extern keyword
		 * decorate the declaration. It is ignored.
		 */
		daePShare(int _extern_=0):tag(),OK(grant())
		{
			//There's a limit of 127 shares. That's a lot of modules.
			assert(OK==DAE_OK); (void)_extern_;
		}
		/** 
		 * The library will use this to free up a process-share slot.
		 */
		~daePShare(){ grant(0); }

	/**SINGLETON
	 * @see @c COLLADA::DAEP::Object::Object().	 
	 * User/client code must add this to a translation-unit.
	 *
	 * @todo Probably as long as the user is required to define
	 * this, the structure could be grown to include whatever's
	 * available to inline more of the library's infrastructure.
	 */
	}DOM_process_share;	

	//Here "CP" stands for "code-point."
	//NOTE: Historically the library doesn't commit to UTF8.
	//There had been a "Latin-1" option available via daeDOM,
	//but it was only implemented by the legacy LIBXML plugin.
	//Now it's up to the user provided platform; daeDOM is now
	//agnostic. The settings are now found in the legacy plugin.
	typedef char daeStringCP;	
	typedef const daeStringCP *daeString;
	typedef const daeStringCP (&daeString1)[1];
	//daeStringCP may be unsigned as well
	typedef unsigned char daeUStringCP;
	//"client" here must be interpreted to mean
	//a string literal or functional equivalent
	typedef const daeStringCP *daeClientString; 	
	typedef const daeStringCP daeClientStringCP; 

	#ifndef COLLADA_DOM_MAKER
	/**
	 * This is used by @c __DAEP__Make__maker to visually identify
	 * the process-share. It's a default. The address is available
	 * regardless. It's recommended to define @c COLLADA_DOM_MAKER
	 * to a string-literal that describes the output of the linker.
	 */
	#define COLLADA_DOM_MAKER _COLLADA_DOM_MAKER()
	inline daeClientString _COLLADA_DOM_MAKER()
	{
		//Reminder: %p is non-portable.
		static daeStringCP address[2+16+1] = "0x";
		if('\0'==address[2]) COLLADA_SUPPRESS_C(4996)			
		if(sprintf(address+2,"%p",&DOM_process_share)>=sizeof(address)) assert(0);
		return address;
	}
	#endif

	//see daeStringRef.h (unrelated to daeString)
	class daeStringRef; class daeTokenRef;

	typedef bool daeBoolean; //previously daeBool
	typedef int daeEnumeration; //previously daeEnum 	

	//MOVING AWAY FROM daeDomTypes.h.
	//Still these are related constructs.
	//These represent the enum itself; both ways.
	struct daeClientEnum
	{
		daeClientString const string_constant; 
		daeEnumeration const integral_constant;  
	};
	
  ////REMINDER/////////////////////////////////////////////////
  //These need to be macros to work around access-controllers//
  /////////////////////////////////////////////////////////////

	//assigned to daeAllocThunk
	typedef ptrdiff_t daeOffset;
	//alternative to C offsetof macro
	//(Probably to suppress warnings.)
	#define daeOffsetOf(class,member) \
	((daeOffset)(((char*)&(((class*)0x0100)->member))-(char*)0x0100))
	typedef size_t daeSize;
	#define daeSizeOf(class,member) ((daeSize)sizeof(((class*)0x100)->member))
	  
	//predeclaration
	//NOTE: typedef types are not 
	//defined here, so that Visual Studio
	//will associate the comment text with the typedef.
	//Their deprecated forms must be postponed as well.	
	namespace DAEP
	{
		template<class> class Proto;
		class Object; class Element; class Model;	
	}	
	class daeAllocThunk;
	template<class,int=0> class daeArray;
	//4*4 is anything to avoid 0. It's 3D matrix sized.
	template<class=void,int=4*4> class daeAlloc;
	class daeObject;	
	class daeText;
	class daeEOText;
	union daeContent;	
	template<class=void> class dae_;
	template<class=daeElement,int=0> class dae_Array;
	class daeElement;
	//can be daeDocument::_PseudoElement
	typedef daeElement daePseudoElement;	
	class daeDoc;
	class daeDocument;
	class daeDOM;
	class daeAtlas;
	class daeArchive;
	template<class T=void> struct daeDocRoot;
	class daeDatabase;	
	class daePlatform;	
	class daeIOPlugin;
	class daeIORequest;
	class daeAtom{};
	class daeTypewriter;
	template<int=0,int=0> class daeBinary;	
	class daeModel;	
	class daeFeature;
	//EXPERIMENTAL INTERNAL
	//These are non-integer operator[] subscripts. 
	enum daeFeatureID{}; 
	//EXPERIMENTAL This affords limited iteration.
	inline daeFeatureID operator++(daeFeatureID &ID,int)
	{
		daeFeatureID old = ID; ID = (daeFeatureID)(int(ID)+1); return old;
	}	
	//EXPERIMENTAL This affords limited iteration.
	inline daeFeatureID operator+(daeFeatureID a, int b)
	{
		return daeFeatureID((int)a+b);
	}	
	class daeMetaObject;
	typedef const class daeMetaElement daeMeta;
	class daeProcessShare;		
	namespace XS
	{
		typedef const class SimpleType List; 
		class Schema; class Attribute; class Restriction; class Enumeration;
	}				
	class daeCM;
	namespace XS //This is to differentiate the "content-model" from "meta."
	{	
		class Choice; class Element; class Group; 
		
		//#ifndef COLLADA_NODEPRECATE typedef
		class All; class Any; class Sequence;
	
		enum __daeXS //XS::All is not implemented.
		{			
			ALL=-3,CHOICE,SEQUENCE, //negative for daeCM::isParentCM().
			ANY,ELEMENT,GROUP, //0~2, etc.
			SCHEMA=30,ATTRIBUTE, //miscellaneous
		};
	}
	typedef XS::__daeXS daeXS;
	//These three elevate low-level concepts.	
	typedef const class daeValue daeCharData;
	typedef const XS::Attribute daeAttribute;	
	typedef const daeArray<const XS::Element> daeTOC;
	//Pseudonym is neither a proper NCName nor QName.
	typedef class daeHashString daePseudonym,daeName;
	//Trying to use this when a 0-terminator is used.
	typedef daeName daeName_0;
	class daeRef;
	class daeRefRequest;
	class daeRefResolver;
	class daeRefResolverList;
	class daeContainedObject;
	class daeDefaultURIResolver;
	class daeDefaultIDREFResolver;
	class daeDefaultSIDREFResolver;
	class daeQuietErrorHandler;
	class daeStandardErrorHandler;		
	#ifndef COLLADA_NODEPRECATED
	COLLADA_DEPRECATED("daeDOM")
	typedef daeDOM DAE;
	COLLADA_DEPRECATED("daeAttribute")
	typedef daeAttribute daeMetaAttribute;
	COLLADA_DEPRECATED("const daeCM")
	typedef const daeCM daeMetaCMPolicy;
	COLLADA_DEPRECATED("const XS::Any")
	typedef const XS::Any daeMetaAny;
	COLLADA_DEPRECATED("const XS::Choice")
	typedef const XS::Choice daeMetaChoice;
	COLLADA_DEPRECATED("const XS::Element")
	typedef const XS::Element daeMetaElementAttribute;
	COLLADA_DEPRECATED("const XS::Group")
	typedef const XS::Group daeMetaGroup;
	COLLADA_DEPRECATED("const XS::Sequence")
	typedef const XS::Sequence daeMetaSequence;
	//daeAtomicType has a new meaning.
	//COLLADA_DEPRECATED("daeTypewriter")
	//typedef daeTypewriter daeAtomicType;
	COLLADA_DEPRECATED("daeRefResolver")
	typedef daeRefResolver daeURIResolver;
	COLLADA_DEPRECATED("daeRefResolverList")
	typedef daeRefResolverList daeURIResolverList;	
	COLLADA_DEPRECATED("daeDefaultURIResolver")
	typedef daeDefaultURIResolver daeStandardURIResolver;
	COLLADA_DEPRECATED("daeDefaultIDREFResolver")
	typedef daeDefaultIDREFResolver daeDefaultIDRefResolver;
	COLLADA_DEPRECATED("daeQuietErrorHandler")
	typedef daeQuietErrorHandler quietErrorHandler;
	COLLADA_DEPRECATED("daeStandardErrorHandler")
	typedef daeStandardErrorHandler stdErrPlugin;
	#endif //COLLADA_NODEPRECATED

	//This default is a limited shorthand only.
	template<class=daeObject> class daeSmartRef;	
	template<class=daeElement> class daeChildRef;
	typedef daeSmartRef<daeObject> daeObjectRef;
	typedef daeSmartRef<daeElement> daeElementRef;
	typedef daeSmartRef<daeDoc> daeDocRef;
	typedef daeSmartRef<daeDOM> daeDOMRef;
	typedef daeSmartRef<daeArchive> daeArchiveRef;
	typedef daeSmartRef<daeDocument> daeDocumentRef;
	typedef daeSmartRef<daeRef> daeRefRef;	
	typedef daeSmartRef<const daeDOM> const_daeDOMRef;
	typedef daeSmartRef<const daeDoc> const_daeDocRef;
	typedef daeSmartRef<const daeObject> const_daeObjectRef;
	typedef daeChildRef<const daeElement> const_daeChildRef;
	typedef daeSmartRef<const daeElement> const_daeElementRef;
	typedef daeSmartRef<const daeArchive> const_daeArchiveRef;
	typedef daeSmartRef<const daeDocument> const_daeDocumentRef;

	#ifndef COLLADA_NOLEGACY
	//these are legacy for the most part
	typedef daeArray<daeInt> daeIntArray; //xsIntegerArray
	typedef daeArray<daeUInt> daeUIntArray;
	typedef daeArray<daeFloat> daeFloatArray; //xsFloatArray
	typedef daeArray<daeEnumeration> daeEnumerationArray;
	typedef daeArray<daeString> daeStringArray; //NOT StringRef
	typedef daeArray<daeByte> daeByteArray; //xsHexBinaryArray
	typedef daeArray<daeBoolean> daeBooleanArray; //xsBooleanArray
	typedef daeArray<daeDouble> daeDoubleArray; //xsDoubleArray
	typedef daeArray<daeLong> daeLongArray; //xsLongArray
	typedef daeArray<daeShort> daeShortArray; //xsShortArray
	typedef daeArray<daeStringRef> daeStringRefArray; //xsNameArray 
	typedef daeArray<daeElementRef> daeElementRefArray;
	//THESE are thought to be ill-formed.
	#ifndef COLLADA_NODEPRECATED
	COLLADA_DEPRECATED("daeByte")
	typedef daeByte daeChar; 
	COLLADA_DEPRECATED("daeUByte")
	typedef daeUByte daeUChar;
	COLLADA_DEPRECATED("daeBoolean")
	typedef daeBoolean daeBool; 
	COLLADA_DEPRECATED("daeEnumeration")
	typedef daeEnumeration daeEnum; 
	COLLADA_DEPRECATED("daeArray<daeEnumeration>")
	typedef daeArray<daeEnumeration> daeEnumArray;
	COLLADA_DEPRECATED("daeArray<daeByte>")
	typedef daeArray<daeByte> daeCharArray;
	#endif //COLLADA_NODEPRECATED
	#endif //COLLADA_NOLEGACY	   
	class domAny; //built-in element class
	typedef daeSmartRef<domAny> domAnyRef;		
	typedef daeSmartRef<const domAny> const_domAnyRef;		
	#ifndef COLLADA_NODEPRECATED	
	COLLADA_DEPRECATED("Post-2.5: END-OF-SUPPORT\n\
	Use daeArray<domAnyRef> instead if you must.\n\
	(domAny_Array is rightly dae_Array<domAny>.)")
	//Post-2.5: END-OF-SUPPORT
	//Use daeArray<domAnyRef> instead if you must.
	//(domAny_Array is rightly dae_Array<domAny>.)	
	typedef void domAny_Array;
	#endif //COLLADA_NODEPRECATED
	//This is used to catch client element classes 
	//that were generated with a different generator
	//than the one the DOM implementation is expecting.
	//IT'S ALSO HERE BESIDE domAny TO REMIND MAINTAINERS
	//TO SEE IF domAny.h/cpp REFLECT THE GENERATED CLASSES. 
	#define COLLADA_DOM_GENERATION 1	
}

COLLADA_(namespace) //maybe a separate support header someday
{//-.
//<-'

template<class> class daeElemental;
/**
 * This is pretty crazy. This is introduced after @c daeElement
 * was made to not inherit @c DAEP::Element. Still it's desired
 * to retain both @c DAEP::Elemental and @c daeElemental. To do
 * this, a "Legacy" paramter is added. The element class is not
 * yet defined initially, so @c daeElemental specifies "Legacy."
 * The "Legacy" parameter cannot be mismatched and so it should
 * be allowed to default to @c daeLegacy<T>::type going forward. 
 */
template<class T> struct daeLegacyOf
{
	typedef char Yes; typedef long No;	
	static Yes DAEP(...);
	template<typename S> static No DAEP(daeElemental<S>*);
	template<int> struct Type{ typedef daeElement type; };
	template<> struct Type<sizeof(Yes)>{ typedef DAEP::Element type; };
	typedef typename Type<sizeof(DAEP((T*)nullptr))>::type type;
};

template<class T>
/** Detect presence/support for operator[] like semantics. */
struct daeAtomOf
{
	//Currently only explicit __COLLADA__Atom is implemented.
	typedef char Yes; typedef long No;	
	template<typename S> static Yes Array(typename S::__COLLADA__Atom*);
	template<typename S> static No Array(...);
	template<class,int>
	struct Atom{ typedef class undefined type; };
	template<class S>
	struct Atom<S,sizeof(Yes)>{ typedef typename S::__COLLADA__Atom type; };
	typedef typename Atom<T,sizeof(Array<T>(nullptr))>::type type;
};
template<class T> struct daeAtomOf<T*>{ typedef T type; };
template<class T> struct daeAtomOf<const T*>{ typedef T type; };
template<class T, int N> struct daeAtomOf<T[N]>{ typedef T type; };
template<class T, int N> struct daeAtomOf<const T[N]>{ typedef T type; };

/** Copies the constness of @a S. This is used by "WRT" APIs. */
template<class S, class T> struct daeConstOf{ typedef T type; };
template<class S, class T> struct daeConstOf<const S,T>{ typedef const T type; };
template<class S, class T> struct daeConstOf<const S*,T>{ typedef const T type; };
//EXPERIMENTAL These are required to go the other direction.
template<class S, class T> struct daeConstOf<S,const T>{ typedef T type; };
template<class S, class T> struct daeConstOf<const S,const T>{ typedef const T type; };

//std containers were being passed cross-module
//COLLADA-DOM 2.5 uses these forms with LINKAGE
struct daeBoundaryStringIn
{
	const daeString c_str;
	inline operator daeString()const{ return c_str; }
	daeBoundaryStringIn(daeString c_str):c_str(c_str){}	
	daeBoundaryStringIn(int o):c_str(0){ assert(o==0); }	
	template<class T, int I> //daeStringCP
	daeBoundaryStringIn(const daeArray<T,I> &c_str):c_str(c_str.data()){}	
	template<class A, class B, class C>
	daeBoundaryStringIn(const std::basic_string<A,B,C> &str):c_str(str.c_str()){}
	template<class T> //See WORKAROUND
	daeBoundaryStringIn(const T &operator_char_):c_str(operator_char_){}
	template<int> friend class daeURI_size; //(Just predeclaring.)
	//WORKAROUND: MSVC2010 wants the above constructor to be valid
	//when matching overloads of other APIs! specifically newDoc<void>
	//which isn't ambiguous on the second argument, but is for the first.
	template<int N> daeBoundaryStringIn(const daeURI_size<N> &URI)
	//daeCTC<0> is putting a hold on this, because it's not obvious
	//which form of the URI the caller means. Use getURI_baseless().
	//In general it's undesirable. URI.data() was to avoid problems.
	:c_str(URI.data()){ daeCTC<0>("getURI_baseless() or getURI()?"); }
};
//This form is used when daeHashString is desired.
template<class T> struct daeBoundaryString2
{	//Anything larger than a pointer, is assumed to
	//have its length encoded somewhere inside of it.
	template<int N> struct extended{ typedef daeHashString type; };
	template<> struct extended<sizeof(void*)>{ typedef daeString type; };
	typedef typename extended<sizeof(T)>::type type;
};//These are all sizeof(void*).
template<> struct daeBoundaryString2<daeStringRef>
{ typedef daeHashString type; };
template<> struct daeBoundaryString2<daeArray<daeStringCP>>
{ typedef daeHashString type; };
//The non (&) forms are just for completeness sake.
template<class T, int N> struct daeBoundaryString2<T[N]>
{ typedef daeHashString type; };
template<class T, int N> struct daeBoundaryString2<T (&)[N]>
{ typedef daeHashString type; };
template<class T, int N> struct daeBoundaryString2<const T[N]>
{ typedef daeHashString type; };
template<class T, int N> struct daeBoundaryString2<const T (&)[N]>
{ typedef daeHashString type; };

//ColladaDOM.inl templates
template<class S> 
inline bool daeUnsafe(const daeElement*);
template<template<class> class S>
inline bool daeUnsafe(const daeElement*);
template<class S, class T>
inline typename daeConstOf
<T,typename S::__COLLADA__T>::type *daeSafeCast(T*);
//PHASING OUT? THESE WERE BYPASSING VIRTUAL-INHERITANCE. BUT IT'S NOT USED.
//These workaround that C++ requires virtual-base-classes use dynamic_cast.
//Note: dynamic_cast cannot be of use here, as the types may be incomplete.
 /**WARNING @warning CONST-CAST. */
inline daeModel *dae(const DAEP::Model *p){ return (daeModel*)p; }
 /**WARNING @warning CONST-CAST. */
inline daeModel &dae(const DAEP::Model &r){ return (daeModel&)r; }
 /**WARNING @warning CONST-CAST. */
inline daeObject *dae(const DAEP::Object *p){ return (daeObject*)p; }
 /**WARNING @warning CONST-CAST. */
inline daeObject &dae(const DAEP::Object &r){ return (daeObject&)r; }
//These 2 are since daeElement is no longer based on DAEP::Element.
 /**WARNING @warning CONST-CAST. */
inline daeElement *dae(const daeElement *p){ return (daeElement*)p; }
 /**WARNING @warning CONST-CAST. */
inline daeElement &dae(const daeElement &r){ return (daeElement&)r; }
 /**WARNING @warning CONST-CAST. */
inline daeElement *dae(const DAEP::Element *p){ return (daeElement*)p; }
 /**WARNING @warning CONST-CAST. */
inline daeElement &dae(const DAEP::Element &r){ return (daeElement&)r; }
template<class T>
/**WARNING @warning CONST-CAST. */
inline daeElement *dae(const daeSmartRef<T> &r){ T *p = r; return dae(p); }
	
//Many legacy APIs take it upon themselves to clear (or not clear) arrays.
//Rather than doubling the number of APIs, this enum is added as a default.
enum dae_clear{ dae_default=0,dae_append=0,dae_clear };

//icky template-meta-programming
template<int,class,class,class=void> struct daeTypic;
//this form is most likely unused
template<int N, class A, class B, class C> struct daeTypic
{
	template<int> struct _choice; 
	template<> struct _choice<0>{ typedef A type; };
	template<> struct _choice<1>{ typedef B type; };
	template<> struct _choice<2>{ typedef C type; };
	typedef typename _choice<N>::type type;
};
//flip the sense around if only 2
template<int N, class B, class A> struct daeTypic<N,B,A,void>
{
	template<int> struct _choice; 
	template<> struct _choice<0>{ typedef A type; };
	template<> struct _choice<1>{ typedef B type; };	
	typedef typename _choice<N!=0>::type type;
};

/**NULLABLE
 * Added for @c daeURI_base::isSameDocumentReference(). 
 */
struct dae3ool
{
	enum State{ is_not=-1,is_neither,is }state;	
	
	dae3ool(State state):state(state){}
	dae3ool(bool l):state(l?is:is_not){}
	inline operator bool(){ return state==is; }	
	inline bool operator!()const{ return state==is_not; }	
	inline bool operator==(dae3ool cmp)const{ return state==cmp.state; }
	inline bool operator!=(dae3ool cmp)const{ return state!=cmp.state; }	
};

//---.
}//<-'
   
/**PREPROCESSOR MACRO
 * COLLADA_DOM_OBJECT_OPERATORS serves as a compatibility layer
 * for pre-2.5 APIs that inconsistently produced pointers and/or
 * C++ references. Post-2.5 APIs seem inconsistent as well, in so
 * far as they return mixed types, depending on relationships; But
 * the relationships are consistent:
 * - Local (lockable) pointers are returned by reference.
 * - Non-local (far) pointers are returned via @a daeSmartRef.
 * - Local, Nullable pointers are returned via pointer.
 *
 * These conversions are not considered a legacy feature, because it
 * is thought convenient that all returns should "decay" to a pointer.
 * It also means references can do "obj->a<daeX>()" v. "obj.a<daeX>()."
 *
 * REMINDER
 * @c COLLADA_DOM_OBJECT_OPERATORS is not limited to @c daeObject based
 * classes. It's used also by classes in/related to the @c XS namespace.
 */
#define COLLADA_DOM_OBJECT_OPERATORS(daeObj) \
/** COLLADA_DOM_OBJECT_OPERATORS Common interface, -> operator. */\
inline daeObj *operator->(){ return (daeObj*)this; }\
/** COLLADA_DOM_OBJECT_OPERATORS Common interface, const-> operator. */\
inline const daeObj *operator->()const{ return (daeObj*)this; }\
template<class T>\
/** COLLADA_DOM_OBJECT_OPERATORS Weak conversion to pointer. */\
inline operator T*(){ T *upcast = (daeObj*)nullptr; return (T*)this; }\
template<class T>\
/** COLLADA_DOM_OBJECT_OPERATORS Weak conversion to const-pointer. */\
inline operator const T*()const{ T *upcast = (daeObj*)nullptr; return (T*)this; }

#endif //__COLLADA_DOM__DAE_TYPES_H__
/*C1071*/