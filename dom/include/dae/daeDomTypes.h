/*
 * Copyright 2006 Sony Computer Entertainment Inc.
 *
 * Licensed under the MIT Open Source License, for details please see license.txt or the website
 * http://www.opensource.org/licenses/mit-license.php
 *
 */

#ifndef __COLLADA_DOM__DAE_DOM_TYPES__
#define __COLLADA_DOM__DAE_DOM_TYPES__

#include "daeURI.h"
#include "daeIDRef.h"
#include "daeSIDResolver.h"

/**C-PREPROCESSOR MACRO
 * This is a C++ "rule of 3" wrapper for ColladaDOM 3 C++ types.
 */
#define COLLADA_DOM_3(x,y,...) COLLADA_DOM_3__##y##__##__VA_ARGS__(x)
#ifndef COLLADA_DOM_3__enum__
#define COLLADA_DOM_3__enum__(x)\
operator __enum__()const{ return __value__; }\
__enum__ operator=(__enum__ v){ return __value__ = v; }\
x(__enum__ v=(__enum__)0):__value__(v){} typedef daeEnumeration __COLLADA__POD;
#endif
#ifndef COLLADA_DOM_3__struct__daeSmartRef
#define COLLADA_DOM_3__struct__daeSmartRef(x) \
template<class T> x &operator=(T &cp){ daeSmartRef::operator=(cp); return *this; }\
template<class T> x &operator=(T *cp){ daeSmartRef::operator=(cp); return *this; }\
template<class T> x(T &cp):daeSmartRef(cp){}\
template<class T> x(T *cp):daeSmartRef(cp){}\
x(){}  x(__COLLADA__T *cp):daeSmartRef(cp){} //nullptr magic
#endif

/**C-PREPROCESSOR MACRO
 * This just cuts out a lot of repetitive code in the generator output. 
 * @c __NB__ and @c __NS__ are just how the first generator to use this
 * happened to have been written. This is just to shape generator output.
 */
#define COLLADA_DOM_NOTE(N,context_concern,...) \
template<> struct __NS__<N>:DAEP::Note<>\
{ typedef __NB__::context_concern concern; ##__VA_ARGS__ };

COLLADA_(namespace)
{	
	namespace xs
	{
		//These are defined last below.
		//DAEP::xs needs to using this.
	};
	//Refs default to the "ref" attribute.
	namespace xml
	{
		typedef daeURI base; //xml:base
	}
	namespace math
	{
		/////////////////////////////////////////////
		//REMINDER: domAny IS COLLADA-DOM 2.x STYLE//
		/////////////////////////////////////////////
		typedef domAny math; //math:math default type
	}
	namespace DAEP
	{
		namespace math //math:math default type
		{
			typedef domAnyRef math; 
			typedef const_domAnyRef const_math; 
		}
		namespace xs //xs:any abstract type
		{
			using namespace ::COLLADA::xs;
			typedef daeElementRef any;
			typedef const_daeElementRef const_any;
		}

		template<class Note, class Unsigned>
		/**TEMPLATE-SPECIALIZATION, BREAKING-CHANGE
		 * Don't know if this is helpful or not. It breaks client code, 
		 * -but seems like the logical progression of SID support.
		 */
		struct Cognate<daeString,Note,typename Note::schema::sidref_type,Unsigned>
		{
			typedef daeSIDREF type; 
		};
	
		template<class Note, class Unsigned>
		/**TEMPLATE-SPECIALIZATION, LEGACY-SUPPORT
		 * Historically the library treats urifragment_type as a URI.
		 * @note URIFragmentType is the name taken from the 1.4.1 COLLADA schema.
		 */
		struct Cognate<daeString,Note,typename Note::schema::URIFragmentType,Unsigned>
		{
			typedef daeURI type; 
		};
		template<class Note, class Unsigned>
		/**TEMPLATE-SPECIALIZATION, LEGACY-SUPPORT
		 * Historically the library treats urifragment_type as a URI.
		 * @note urifragment_type is the name taken from the 1.5.0 COLLADA schema.
		 */
		struct Cognate<daeString,Note,typename Note::schema::urifragment_type,Unsigned>
		{
			typedef daeURI type; 
		};

		#if COLLADA_UPTR_MAX==0xFFFFFFFF || defined(COLLADA_DOM_INT32_long_long)
		template<class Note, class Unsigned>
		/**TEMPLATE-SPECIALIZATION, LEGACY-SUPPORT
		 * 64 bit values should not be used on 32 bit systems. If a value requires
		 * 64 bits this specialization can still be overriden.
		 * @note 64 bit @c size_t is standard on 64 bit systems, for better or worse.
		 */
		struct Cognate<signed long long,Note,Note,Unsigned>
		{
			typedef signed COLLADA_DOM_INT32 type; 
		};
		template<class Note, class Unsigned>
		/**TEMPLATE-SPECIALIZATION, LEGACY-SUPPORT
		 * 64 bit values should not be used on 32 bit systems. If a value requires
		 * 64 bits this specialization can still be overriden.
		 * @note 64 bit @c size_t is standard on 64 bit systems, for better or worse.
		 */
		struct Cognate<unsigned long long,Note,Note,Unsigned>
		{
			typedef unsigned COLLADA_DOM_INT32 type; 
		};
		#else
		template<class Unsigned>
		/**PARTIAL-TEMPLATE-SPECIALIAZTION 
		 * COLLADA is ridiculous for using 64-bit indices. There's never a reason
		 * to do that, as datasets can always be partitioned if they truly needed
		 * BILLIONS of datapoints!
		 */
		struct Container<long long,Unsigned>
		{
			typedef signed COLLADA_DOM_INT32 type; 
		};
		template<class Unsigned>
		/**PARTIAL-TEMPLATE-SPECIALIAZTION 
		 * COLLADA is ridiculous for using 64-bit indices. There's never a reason
		 * to do that, as datasets can always be partitioned if they truly needed
		 * BILLIONS of datapoints!
		 */
		struct Container<unsigned long long,Unsigned>
		{
			typedef unsigned COLLADA_DOM_INT32 type; 
		};
		#endif

		template<int N, class Unsigned>
		/**PARTIAL-TEMPLATE-SPECIALIAZTION 
		 * For some misbegotten reason COLLADA 1.4.1 and 1.5.0 have spaces
		 * in their "list_of_hex_binary_type" types. They really shouldn't
		 * have been list-based, but the spefication is what it is for now.
		 * It's possible the schema could be retroactively changed, making
		 * documents with spaces invalid, but either way, this specializes
		 * a plain xs:hexBinary list so that the first item is an embedded
		 * allocation-unit. If there are more than one, the memory is lost.
		 * Because hexBinary is an array, this way the first dimension can
		 * not be dynamically allocated. Since it's binary data, if it has
		 * units, that should be stored in a header at the top of the data.
		 * @tparam N applies to base64Binary also, though COLLADA has none.
		 *
		 * @remark "sizeof(void*)" is to avoid allocating dummy allocation
		 * units if a multi-dimensional array is indeed used. 1 would work
		 * as well, but it may as well be rounded up to the structure size.
		 */
		struct Container<daeBinary<N>,Unsigned>
		{
			typedef daeArray<daeBinary<N,sizeof(void*)>,1> type; 
		};

		template<class Unsigned>
		/**PARTIAL-TEMPLATE-SPECIALIAZTION 
		 * Post-2.5 references are based on @c DAEP::Object, and so cannot
		 * be housed in @c daeArray since smart-refs cannot form permanent
		 * pointers into the array. There are other reasons, but this is a
		 * strong rationale.
		 */
		struct Container<daeURI,Unsigned>{ typedef daeTokenRef type; };
		template<class Unsigned>
		/**PARTIAL-TEMPLATE-SPECIALIAZTION 
		 * @see @c daeURI specialization's Doxygentation.
		 */
		struct Container<daeIDREF,Unsigned>{ typedef daeTokenRef type; };
		template<class Unsigned>
		/**PARTIAL-TEMPLATE-SPECIALIAZTION 
		 * @see @c daeURI specialization's Doxygentation.
		 */
		struct Container<daeSIDREF,Unsigned>{ typedef daeTokenRef type; };

		template<int ID, class CC, typename CC::_ PtoM>
		/**TEMPLATE-SPECIALIZATION
		 * @see the @c daeURI specialization of DAEP Container Doxygentation.
		 */
		class Value<ID,daeURI,CC,PtoM> : public DAEP::InnerValue<ID,daeURI,CC,PtoM,daeStringRef>
		{
		COLLADA_(public) using InnerValue::operator=; //C2679
		};
		template<int ID, class CC, typename CC::_ PtoM>
		/**TEMPLATE-SPECIALIZATION
		 * @see the @c daeURI specialization of DAEP Container Doxygentation.
		 */
		class Value<ID,daeIDREF,CC,PtoM> : public DAEP::InnerValue<ID,daeIDREF,CC,PtoM,daeTokenRef>
		{
		COLLADA_(public) using InnerValue::operator=; //C2679
		};
		template<int ID, class CC, typename CC::_ PtoM>
		/**TEMPLATE-SPECIALIZATION
		 * @see the @c daeURI specialization of DAEP Container Doxygentation.
		 */
		class Value<ID,daeSIDREF,CC,PtoM> : public DAEP::InnerValue<ID,daeSIDREF,CC,PtoM,daeTokenRef>
		{
		COLLADA_(public) using InnerValue::operator=; //C2679
		};
	}
	//IT SEEMS USING DAEP::Container BELOW MAKES THESE DEPENDENT ON THE ABOVE SPECIALIZATIONS
	namespace xs
	{
	//EXPERIMENTAL
	typedef daeElement any; //DAEP::Element

	typedef daeString anySimpleType;
	typedef daeURI anyURI;	
	typedef daeString duration;
	typedef daeString dateTime;	
	typedef daeString fate, time;	
	typedef daeString gYear, gMonth, gDay;	
	typedef daeString gYearMonth, gMonthDay;	
	typedef daeBinary<16> hexBinary;
	typedef daeBinary<64> base64Binary;	
	typedef daeString QName;	
	typedef daeString NOTATION;	
	typedef daeString ID;	
	typedef daeIDREF IDREF;
	typedef daeArray<DAEP::Container<daeIDREF>::type> IDREFS;
	typedef daeString ENTITY;
	typedef daeArray<DAEP::Container<ENTITY>::type> ENTITIES;
	typedef daeString NCName;	
	typedef daeString NMTOKEN;
	typedef daeArray<DAEP::Container<NMTOKEN>::type> NMTOKENS;
	typedef daeString Name;	
	typedef daeString token;	
	typedef daeString string;	
	typedef daeString normalizedString;
	typedef daeBoolean boolean;	
	typedef daeFloat float__alias;	
	typedef daeDouble double__alias;	
	typedef daeDouble decimal;	
	typedef daeLong integer;	
	typedef daeULong nonNegativeInteger;
	typedef daeULong positiveInteger;
	typedef daeLong nonPositiveInteger;
	typedef daeLong negativeInteger;	
	typedef daeLong long__alias;	
	typedef daeULong unsignedLong;	
	typedef daeInt int__alias;
	typedef daeUInt unsignedInt;	
	typedef daeShort short__alias;	
	typedef daeUShort unsignedShort;	
	typedef daeByte byte;	
	typedef daeUByte unsignedByte;
	}

#if 2 == COLLADA_DOM || !defined(COLLADA_NOLEGACY)

	 //RESERVED
	//This is reserved for a type that sets its length
	//to 0 instead of calling strlen() in order to not
	//inject strlen calls into user code. It's used to
	//fix the 2.x setX() mutator APIs so setting a URI
	//is not ambiguous and doesn't trigger conversions
	//on top of conversions--involving buffers/exports.
	//daeStringRef would then call strlen() if it must.
	//THE QUESTION IS, IS LEGACY-SUPPORT EVEN WORTH IT?
	typedef daeHashString daeHashString2;

	//2.5 refs default to the "ref" attribute.
	typedef daeURI xmlBase; //xml:base
	typedef domAny mathMath; //math:math default type
	typedef daeSmartRef<domAny> mathMathRef;

	//use this if you want to
	typedef daeElement domElement;

	//legacy: seems harmless
	#define daeTSmartRef daeSmartRef
	//2.5: same deal as above
	#define daeTArray daeArray

	//EXPERIMENTAL
	typedef daeElement xsAny;
	typedef daeElementRef xsAnyRef;
	typedef const_daeElementRef const_xsAnyRef;
	typedef daeArray<daeElementRef> xsAny_Array;
	typedef daeArray<const_daeElementRef> const_xsAny_Array;

	typedef daeString xsAnySimpleType;
	typedef daeURI xsAnyURI;
	typedef daeIDREF xsIDREF;
	typedef daeString xsString,
	xsName, xsToken, xsNormalizedString, 
	xsDuration, xsDateTime, xsDate, xsTime, 
	xsGYear, xsGMonth, xsGDay, xsGYearMonth, xsGMonthDay,
	xsQName, xsNOTATION, xsENTITY, xsNCName, xsNMTOKEN, 
	xsID;
	typedef daeArray<daeTokenRef> xsStringArray, 
	xsNameArray, xsNMTOKENS, xsNMTOKENArray, xsNormalizedStringArray, 
	xsDurationArray, xsDateTimeArray, xsDateArray, xsTimeArray,
	xsGYearArray, xsGMonthArray, xsGDayArray, xsGYearMonthArray, xsGMonthDayArray, 	
	xsQNameArray, xsNOTATIONArray, xsENTITIES, xsENTITYArray, xsNCNameArray, xsTokenArray, 
	xsIDArray, xsIDREFS, xsIDREFArray, xsAnyURIArray;
	typedef daeBinary<16> xsHexBinary;	
	typedef daeArray<xsHexBinary> xsHexBinaryArray; //A		
	typedef daeBinary<64> xsBase64Binary;	
	typedef daeArray<xsBase64Binary> xsBase64BinaryArray; //A
	typedef daeBoolean xsBoolean;
	typedef daeArray<daeBoolean> xsBooleanArray; //A
	typedef daeFloat xsFloat;
	typedef daeArray<daeFloat> xsFloatArray; //A
	typedef daeDouble xsDouble;
	typedef daeArray<daeDouble> xsDoubleArray; //A
	typedef daeDouble xsDecimal;
	typedef daeArray<daeDouble> xsDecimalArray; //A
	typedef daeLong xsInteger;
	typedef daeArray<daeLong> xsIntegerArray; //A
	typedef daeULong xsNonNegativeInteger;
	typedef daeArray<daeULong> xsNonNegativeIntegerArray; //A
	typedef daeULong xsPositiveInteger;
	typedef daeArray<daeULong> xsPositiveIntegerArray; //A
	typedef daeLong xsNonPositiveInteger;
	typedef daeArray<daeLong> xsNonPositiveIntegerArray; //A
	typedef daeLong xsNegativeInteger;
	typedef daeArray<daeLong> xsNegativeIntegerArray; //A
	typedef daeLong xsLong;
	typedef daeArray<daeLong> xsLongArray; //A
	typedef daeULong xsUnsignedLong;
	typedef daeArray<daeULong> xsUnsignedLongArray; //A
	typedef daeInt xsInt;
	typedef daeArray<daeInt> xsIntArray; //A
	typedef daeUInt xsUnsignedInt;
	typedef daeArray<daeUInt> xsUnsignedIntArray; //A
	typedef daeShort xsShort;
	typedef daeArray<daeShort> xsShortArray; //A
	typedef daeUShort xsUnsignedShort;
	typedef daeArray<daeUShort> xsUnsignedShortArray; //A
	typedef daeByte xsByte;
	typedef daeArray<daeByte> xsByteArray; //A
	typedef daeUByte xsUnsignedByte;
	typedef daeArray<daeUByte> xsUnsignedByteArray; //A

#else //domAny requires xsAny/xsAnySimpleType.

		typedef daeElement xsAny;
		typedef daeString xsAnySimpleType;
#endif
}

#endif //__COLLADA_DOM__DAE_DOM_TYPES__
/*C1071*/