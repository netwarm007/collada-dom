/*
 * Copyright 2006 Sony Computer Entertainment Inc.
 *
 * Licensed under the MIT Open Source License, for details please see license.txt or the website
 * http://www.opensource.org/licenses/mit-license.php
 *
 */
											 
#include <ColladaDOM.inl> //PCH

COLLADA_(namespace)
{//-.
//<-'

//This is conceptually the first thing
//that is initialized. The rest follow.
extern daePShare DOM_process_share = 0;

/**128-1-1
 * Arbitrary figure, divisible by 8, 
 * minus the #, and minus the 0-terminator.
 * @todo "COLLADA_DOM_LARGE_SYSTEM_STRING"?
 */
enum{ daeStringRef_large=126 }; 

static daeSmallStringTable<> daeStringRef_table(0);

extern daeString daeStringRef_empty = daeStringRef_table.allocString("",0);

daeString daeStringRef_system(daeString cp, size_t extent)
{
	daeString out;
	bool frag = '#'==cp[0]; 
	if(frag){ cp++; extent--; }
	if(extent>=daeStringRef_large-1) //1 is added to len.
	{
		void *large = operator new
		(daeOffsetOf(daeDBaseString,fragment[1+extent+1]));
		out = *new(large)daeDBaseString(0,1,cp,extent)+1;		
	}
	else out = daeStringRef_table.allocString(cp,extent); 
	return out+(frag?-1:0);
}

//pool registration
static std::vector<daeStringPool<>*> daeStringRef_pools;
static std::vector<unsigned short> daeStringRef_poolstack;
static struct daeStringRef_0 : daeStringPool<> //system pool
{	
	//daeStringPool methods
	virtual void release(String string)
	{
		if(string->fragmentN>=daeStringRef_large) delete string;
	}
	virtual daeString ref(daeString cp, size_t extent)
	{
		return daeStringRef_system(cp,extent);
	}
	virtual void rollover(String &string, String &source, int factor)
	{
		//Just handle the unlikely case that a large-string has 32768 refs.
		if(0==string->refs&&string->fragmentN<daeStringRef_large)
		{
			#ifdef NDEBUG
			#error string.string should be added to a list with a link to
			#error its rollover string, so its rollover can be reassigned.
			#error It may also be a good idea to emit a graphical warning.
			#endif
			string->refs-=2; 
			string.string = daeStringRef_system(string.string,string.size);
			string->refs+=1;
			source.string = string.string; 			
		}
	}
}daeStringRef_0; //The constructor registers it...
daeStringPool_pool::
daeStringPool_pool():pool(daeStringRef_poolstack.empty()?
daeStringRef_pools.size():daeStringRef_poolstack.back())
{
	if(pool<daeStringRef_pools.size())
	{
		daeStringRef_poolstack.pop_back();
		daeStringRef_pools[pool] = (daeStringPool<>*)this;	
	}
	else daeStringRef_pools.push_back((daeStringPool<>*)this);
}
daeStringPool_pool::~daeStringPool_pool()
{
	daeStringRef_pools[pool] = nullptr;
	daeStringRef_poolstack.push_back((unsigned short)pool);
}

//pool API
void daeStringRef::_release() 
{
	daeStringRef_pools[_ptr->pool]->release(_string);
}
void daeStringRef::_ref_and_release(daeString cp)
{
	_ref_and_release(cp,strlen(cp));
}
void daeStringRef::_ref_and_release(const daeStringRef &cp)
{
	_ref_and_release(cp,cp.size());
}
void daeStringRef::_ref_and_release(daeString cp, size_t extent)
{
	daeStringPool<> *pool = daeStringRef_pools[_ptr->pool];
	daeString string = _string; _string = pool->ref(cp,extent);	
	pool->release(string);
}
void daeStringRef::_ref_and_release(const daeHashString &cp)
{
	_ref_and_release(cp.string,cp.extent);
}
void daeStringRef::_rollover(daeStringRef &source, int factor) 
{
	daeStringRef_pools[_ptr->pool]->rollover
	((daeStringPool<>::String&)_string
	,(daeStringPool<>::String&)source,factor);
}

//prototype-constructor
void daeStringRef::_ref(const daeDOM &DOM)
{
	DOM._database->_prototype(*this,DOM._databaseRejoinder);
}

//array-emplacement
void daeStringRef::_ref(const daeObject *c, daeString cp) 
{
	_ref(c,cp,strlen(cp));
}
void daeStringRef::_ref(const daeObject *c, daeString cp, size_t len) 
{
	assert(cp!=nullptr);
	const daeDOM *DOM = c==nullptr?nullptr:c->getDOM();
	if(DOM!=nullptr) _ref(*DOM,cp,len); else _ref(cp,len);
}
void daeStringRef::_ref(const daeObject *c, const daeStringRef &cp) 
{
	const daeDOM *DOM = c==nullptr?nullptr:c->getDOM();
	if(DOM!=nullptr) _ref(*DOM,cp); else _ref(cp);
}
void daeStringRef::_ref(const daeObject *c, const daeHashString &cp)
{
	_ref(c,cp.string,cp.extent);
}

//constructor
void daeStringRef::_ref(const DAEP::Object &c, daeString cp) 
{
	_ref(c,cp,strlen(cp));
}
void daeStringRef::_ref(const DAEP::Object &c, daeString cp, size_t len) 
{
	assert(cp!=nullptr);
	const daeDOM *DOM = dae(c).getDOM();
	if(DOM!=nullptr) _ref(*DOM,cp,len); else _ref(cp,len);
}
void daeStringRef::_ref(const DAEP::Object &c, const daeHashString &cp)
{
	_ref(c,cp.string,cp.extent);
}
//constructor/subroutine
void daeStringRef::_ref(const daeDOM &DOM, daeString cp, size_t len) 
{
	_string = DOM._database->_ref(cp,len,DOM._databaseRejoinder);
}
void daeStringRef::_ref(const daeDOM &DOM, const daeHashString &cp) 
{
	_ref(DOM,cp.string,cp.extent);
}
//system-pool/subroutine
void daeStringRef::_ref(const daeString cp)
{
	_string = daeStringRef_system(cp,strlen(cp));
}
void daeStringRef::_ref(const daeString cp, size_t len)
{
	_string = daeStringRef_system(cp,len);
}
void daeStringRef::_ref(const daeHashString &cp) 
{
	_string = daeStringRef_system(cp.string,cp.extent);
}

//constructor
void daeStringRef::_ref(const DAEP::Object &c, const daeStringRef &cp) 
{
	const daeDOM *DOM = dae(c).getDOM();
	if(DOM!=nullptr) _ref(*DOM,cp); else _ref(cp);
}
//constructor/subroutine
void daeStringRef::_ref(const daeDOM &DOM, const daeStringRef &cp) 
{
	//paranoia: Shouldn't happen. Not even for a prototype.
	assert(&DOM!=nullptr); 

	daeString empty = DOM.getEmptyURI().data();
	if(cp._ptr->pool!=((daeStringRef&)empty)._ptr->pool)
	_string = DOM._database->_ref(cp,cp.size(),DOM._databaseRejoinder);
	else new(this) daeStringRef(cp);
}
//system-pool/subroutine
void daeStringRef::_ref(const daeStringRef &cp)
{
	_string = 0==cp._ptr->pool?cp:daeStringRef_system(cp,cp.size());
}

//system-pool
void daeStringRef::_ref(){ _string = daeStringRef_empty; }


////////////////////////////////////////////////////////////////////////
// These are globals that depend on the system string pool having been//
// initialized. (daeStringRef.cpp may or may not be initialized first)//
////////////////////////////////////////////////////////////////////////

template<class T>
static daeType<T> &xs_best_fit()
{
	static daeType<T> out; return out; 
}
//THESE MUST AGREE WITH daeDomTypes.h
//* technically should be string
#define xs_(x,y) \
extern daeTypewriter &daeStringRef_xs_##y = xs_best_fit<x>();
xs_(daeStringRef,anySimpleType)
xs_(daeStringRef,string)
xs_(daeStringRef,normalizedString)
xs_(daeStringRef,duration)
xs_(daeStringRef,dateTime) 
xs_(daeStringRef,date) 
xs_(daeStringRef,time)
xs_(daeStringRef,gYear) 
xs_(daeStringRef,gYearMonth)
xs_(daeStringRef,gMonth) 
xs_(daeStringRef,gMonthDay)
xs_(daeStringRef,gDay)
xs_(daeBinary<16>,hexBinary)
xs_(daeBinary<64>,base64Binary) 
xs_(daeStringRef,anyURI)
xs_(daeStringRef,QName)
xs_(daeStringRef,NOTATION)
xs_(daeBoolean,boolean) 
xs_(daeFloat,float) 
xs_(daeDouble,double) 
xs_(daeDouble,decimal) //*
xs_(daeLong,long)
xs_(daeLong,integer) //*		
xs_(daeLong,nonPositiveInteger) //*
xs_(daeLong,negativeInteger) //*	
xs_(daeInt,int)
xs_(daeShort,short)
xs_(daeByte,byte)
xs_(daeULong,unsignedLong)
xs_(daeULong,nonNegativeInteger) //*
xs_(daeULong,positiveInteger) //*
xs_(daeUInt,unsignedInt)
xs_(daeUShort,unsignedShort)
xs_(daeUByte,unsignedByte)
xs_(daeTokenRef,token)
xs_(daeTokenRef,language)
xs_(daeTokenRef,Name)
xs_(daeTokenRef,NCName) 
xs_(daeTokenRef,NMTOKEN)
xs_(daeTokenRef,NMTOKENS)
xs_(daeTokenRef,ID)
xs_(daeTokenRef,IDREF)
xs_(daeTokenRef,IDREFS)		
xs_(daeTokenRef,ENTITY)
xs_(daeTokenRef,ENTITIES) 
#undef xs_
//A content-typewriter is a big side project.
//(It'd basically be an XML document "writer.")
extern daeAlloc<daeCounter,0> daeStringRef_counterLT(0);
extern daeTypewriter &daeStringRef_counterTW = xs_best_fit<daeCounter>();
static XS::Schema domAny_xs(0); XS::Schema::Schema(int ps)
{
	__XS__Schema__construct(false, //__invisible,
	COLLADA_DOM_PHILOSOPHY,COLLADA_DOM_PRODUCTION,COLLADA_DOM_GENERATION,"","");
	//HACK: Assign the special 0 process-share, or: why this constructor exists.
	_ps = ps; assert(ps==0&&this==&domAny_xs);
}
domAny::_Master domAny::_master(domAny_xs);
//This must be destructed absolutely last.
static struct daeStringRef_protoDOM : daeDoc
{
	daeStringRef_protoDOM():COLLADA_SUPPRESS_C(4355)daeDoc((daeDOM*)this,daeDocType::ARCHIVE)
	{
		(&_getClassTag())[1] = 2; assert(_isDOM()); 
	}
}protoDOM;
//This struct is just needed for an empty URI for daeStringRef.
//It could be a daeDOM, but that has a rather larger footprint.
extern daeDOM &daeStringRef_protoDOM = *(daeDOM*)&protoDOM;

extern daeTypewriter *daeStringRef_xs_(daeName name)
{
	if(name[0]!='x'||name[1]!='s'||name[2]!=':')
	return nullptr;
	name.string+=3; name.extent-=3;
	#define _(x) if(#x==name) return &daeStringRef_xs_##x;
	switch(name[0])
	{
	case 'a':_(anyURI)_(anySimpleType) 
	break;
	case 'b':_(boolean)_(base64Binary)_(byte)
	break;
	case 'd':_(double)_(dateTime)_(date)_(decimal)_(duration)
	break;
	case 'E':_(ENTITY)_(ENTITIES)
	break;
	case 'f':_(float)
	break;
	case 'g':_(gDay)_(gYear)_(gMonth)_(gYearMonth)_(gMonthDay)
	break;
	case 'h':_(hexBinary) 
	break;
	case 'i':_(int)_(integer)
	break; 
	case 'I':_(ID)_(IDREF)_(IDREFS)
	break;
	case 'l':_(long)_(language) 
	break;
	case 'n':_(normalizedString)_(negativeInteger)_(nonNegativeInteger)_(nonPositiveInteger)
	break;
	case 'N':_(NCName)_(Name)_(NMTOKEN)_(NMTOKENS)_(NOTATION) 
	break;
	case 'p':_(positiveInteger) 
	break;
	case 'Q':_(QName) 
	break;
	case 's':_(string)_(short) 
	break;
	case 't':_(token)_(time)
	break;
	case 'u':_(unsignedLong)_(unsignedInt)_(unsignedShort)_(unsignedByte) 
	break;
	}return nullptr;
	#undef _
}
extern daeTypewriter *daeStringRef_sys_typewrit(intptr_t st)
{
	//These are broken up in case the values are the same
	//(on exotic systems.)
	switch(st) 
	{
	case daeAtomicType::INT: return xs_best_fit<daeInt>();
	case daeAtomicType::UINT: return xs_best_fit<daeUInt>();
	case daeAtomicType::FLOAT: return xs_best_fit<daeFloat>();		
	}		
	switch(st)
	{
	case daeAtomicType::LONG: return xs_best_fit<daeLong>();
	case daeAtomicType::ULONG: return xs_best_fit<daeULong>();
	case daeAtomicType::DOUBLE: return xs_best_fit<daeDouble>();
	case daeAtomicType::BOOLEAN: return xs_best_fit<daeBoolean>();	
	case daeAtomicType::TOKEN: return xs_best_fit<daeTokenRef>();
	case daeAtomicType::STRING: return xs_best_fit<daeStringRef>();	
	}
	switch(st)
	{
	case daeAtomicType::BYTE: return xs_best_fit<daeByte>();
	case daeAtomicType::UBYTE: return xs_best_fit<daeUByte>();
	}
	switch(st)
	{
	case daeAtomicType::SHORT: return xs_best_fit<daeShort>();
	case daeAtomicType::USHORT: return xs_best_fit<daeUShort>();
	//To avoid this assert, derive a user type from @c daeBinary.
	//A BINARY must be named "xs:hexBinary" or "xs:base64Binary."
	case daeAtomicType::BINARY: assert(st!=daeAtomicType::BINARY);
	}
	assert(0); return nullptr; //this can't end well
}

//---.
}//<-'

/*C1071*/