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

extern daeTypewriter &daeStringRef_xs_string;
extern daeTypewriter *daeStringRef_xs_(daeName);
extern daeTypewriter *daeStringRef_sys_typewrit(intptr_t);

void XS::Schema::__XS__Schema__construct
(bool invis, 
int philosophy, //COLLADA_DOM_PHILOSOPHY
int production, //COLLADA_DOM_PRODUCTION
int generation, //COLLADA_DOM_GENERATION
daeName ns, daeName version)
{	
	if(invis) //This is a pseudo constructor.
	new(static_cast<__Schema__invisible*>(this)) __Schema__invisible;
	   
	_targetNamespace = ns; _version = version; _agent = nullptr;

	_IDs.push_back("id"); _IDs.push_back("sid");

	//There could be wiggle room here, but for now, require that all 3 agree.
	assert(generation==COLLADA_DOM_GENERATION
	&&philosophy==COLLADA_DOM_PHILOSOPHY&&production==COLLADA_DOM_PRODUCTION);

	//*3/2 is to give US a heads up
	daeCTC<__size_on_client_stack*1/2>=sizeof(*this)>();
}
void XS::Schema::__XS__Schema__destruct(bool invis)
{
	daeStringMap<XS::SimpleType*>::iterator it;	
	for(it=_simpleTypes.begin();it!=_simpleTypes.end();it++)	
	delete it->second;	

	if(invis) //This is a pseudo destructor.
	static_cast<__Schema__invisible*>(this)->~__Schema__invisible();

}//CIRCULAR-DEPENDENCY
XS::SimpleType::~SimpleType(){ getRestriction().~Restriction(); };

DAEP::Model &daeProcessShare::getOpaqueModel()
{
	if(_opaqueModel==nullptr)
	_opaqueModel = &addModel<0>((daeObject*)nullptr,"");			
	return *_opaqueModel;
}static int daeMetaSchema_bogus_family_counter = 0;
daeModel &daeProcessShare_base::_addModel(int size, daeFeatureID finalID, int ext_chars)
{
	size_t features = (int)finalID*-int(sizeof(daeFeature));
	size_t fullsize = features+size+sizeof(daeMetaObject)+ext_chars;
	daeOpaque mem = memset(__DAEP__Make__v1__operator_new(fullsize),0x00,fullsize);
	daeModel &mod = (daeModel&)mem[features];	
	daeMetaObject &out = (daeMetaObject&)daeOpaque(&mod)[size];
	const_cast<daeMetaObject*&>(mod.__DAEP__Model__meta) = &out;

	//UNIMPLEMENTED
	//This is supposed to be an ID that if equal means the elements are binary equivalent.
	//This is so daeSafeCast can work with identical classes coming from different modules.
	const_cast<daeInt&>(mod.__DAEP__Model__family) = --daeMetaSchema_bogus_family_counter;
	
	out._processShare = static_cast<daeProcessShare*>(this);
	out._sizeof = size;
	out._domAny_safe_model = &mod;
	out._finalFeatureID = finalID;	
	if(0!=finalID)
	mod[daeFeatureID(-1)]._flags.feature_1 = 1;
	out._deleteList = _deleteList; _deleteList = &out; return mod;
}
daeMetaElement &XS::Schema::_addElement(long long schema, int size, daeFeatureID finalID, daeName name)
{
	assert((int)finalID<=-2);	
	//The calling method checks these formulas.
	int genus = 1+int(schema>>48);
	int attribs = 0x3F&int(schema>>32);
	//FYI: Thunks are not being assigned to the DAEP::Child features.
	//domAny has a "value" member even though it'd saying it's MIXED.
	int thunks = attribs, addvalue = 0;
	if(genus==daeObjectType::ANY||(schema&3)==daeContentModel::SIMPLE) 
	{
		thunks++; addvalue = sizeof(daeValue);
	}
	int diff = sizeof(daeMetaElement)-sizeof(daeObject);
	daeModel &mod = _addModel(size,finalID,diff+size+addvalue+thunks*sizeof(daeAllocThunk));
	mod.setObjectType(genus);

	daeMetaElement &out = const_cast<daeMetaElement&>(mod.__DAEP__Model__elements_meta());	
	{
		//_addModel() set up the daeMetaObject ones.
		out._name = name; 		
		out._domAny_safe_prototype = out._prototype;		
		out._feature_completeID = daeFeatureID(-thunks);
		out._DAEP_Schema = schema;
	}	
	//This just communicates the count to daeMetaElement.
	out._bring_to_life(out._attribs).setCountMore(attribs);
	memset(out._attribs.data(),0x00,sizeof(out._attribs[0])*attribs);

	union //HEADS UP!
	{
		char *behind_prototype; daeAllocThunk *thunkp; 
	};	
	daeFeatureID fit = out._feature_completeID;
	behind_prototype = &daeOpaque(out._prototype)+size;
	if(0!=addvalue) 
	{
		out._value = (daeValue*)behind_prototype; 
		behind_prototype+=sizeof(daeValue);
		//HACK: setAllocThunk() expects initialized memory.
		mod[fit++]._localthunk = thunkp++; //setAllocThunk();
	}
	while(fit) mod[fit++]._localthunk = thunkp++; //setAllocThunk();
		
	if(!out.getIsLocal()) _globalTypes[name] = &out; return out;
}
 
static void daeMetaSchema_deleteAllocThunk(daeMetaObject *self) 
{
	daeModel &mod = self->getModel();
	for(daeFeatureID it=self->rbeginFeatureID();it;it++)
	{
		daeFeature &f = mod[it];
		if(1==f.getFlags().new_thunk) f.deleteAllocThunk();
	}
}
void daeMetaObject::_feature_complete(daeFeatureID fID)
{
	for(const daeFeature *f=&getModel()[fID];fID;fID++,f++)
	{	
		if(!f->getFlags()) continue;

		if(1==f->_flags.subobject)
		{
			int i = -(int)fID-1; 
			assert(i<sizeof(_subobjectsMask)*CHAR_BIT);			
			_subobjectsMask|=1<<i;			
		}
		if(1==f->_flags.atomizing) _features_atomize = 1;
		if(1==f->_flags.new_thunk)
		{
			_deleteAllocThunk = 1;
			if(_destructor_ptr==nullptr)
			_destructor_ptr = daeMetaSchema_deleteAllocThunk; 
		}
	}	
}

//HACK! NOT-THREAD-SAFE _addList() sets this up in advance.
static const XS::SimpleType *daeMetaSchema_base = nullptr;
XS::SimpleType &XS::Schema::_addAtom(daeTypewriter *writ, daeName base, daeName name)
{	
	bool itemType = base==nullptr;
	assert(!itemType||daeMetaSchema_base!=nullptr);

	if(name==nullptr) std::swap(name,base);
	//There's room for some optimization here.
	XS::SimpleType* &out = _simpleTypes[name];
	if(out!=nullptr)
	{
		daeMetaSchema_base = nullptr;
		assert(out==nullptr); return *out; //unacceptable
	}
	size_t size = sizeof(XS::SimpleType);
	if(base!=nullptr) 
	size+=sizeof(XS::Restriction)-daeSizeOf(XS::SimpleType,_restriction);
	(void*&)out = operator new(size);
		
	if(base!=nullptr)	
	if(daeMetaSchema_base!=nullptr)
	{
		//HACK: NOT THREAD-SAFE
		out->_restriction = daeMetaSchema_base;
	}
	else out->_restriction = findType(base);
	else out->_restriction = nullptr;

	XS::Restriction &r = out->getRestriction();	
	if(r)	
	{
		//String-pooling must be in play.
		assert(base.string==r.getBase().getName().string); 
		//_minmax is setup by the calling/inline APIs.
		r._length[0] = 0; r._length[1] = ~r._length[0];	
		r._compare[0] = INT_MIN;  r._compare[1] = INT_MAX;
		r._enumeration = 0; 
	}
	else assert(base==nullptr);
	if(r) //hack?
	{
		//Normally the library does not inherit anything from the base.
		//That's the generators' job. This is an edge-case however. It's
		//intended to catch a length restricted list, that is implemented
		//in terms of a fixed array, deriving from an unresticted daeArray.
		out->_itemType = r.getBase().hasList()?r.getBase()._itemType:out;
	}
	else out->_itemType = out; out->_name = name; 
	
	//Here a "writ" is a partial typewriter. 
	intptr_t system_type = (intptr_t)writ;
	if(std::abs(system_type)<=64) //daeAtomicType?
	{
		daeTypewriter *xs; if(r) //Derived?
		{
			xs = r.getBase().getValueTypewriter();	
		}
		else if(itemType) //Hack: itemType?
		{
			//Prefer built-in typewriter. hexBinary requires
			//this as long as BINARY doesn't encode the base.
			xs = daeMetaSchema_base->getValueTypewriter();
		}
		else xs = daeStringRef_xs_(name);
		int t = 0; //HACK: Assuming string->token is alright.
		if((xs==nullptr||(t=xs->getAtomicType())!=system_type)
		&&(t!=daeAtomicType::TOKEN||system_type!=daeAtomicType::STRING))
		xs = daeStringRef_sys_typewrit(system_type);		
		out->_value_typewriter = xs;	
	}
	else out->_value_typewriter = writ; //should be a real typewriter.
		
	return *out;
}
XS::SimpleType &XS::Schema::_addList(daeTypewriter *writ, daeName base_or_item, daeName name)
{
	//This is a little convoluted so that it
	//can be implemented in terms of _addAtom().
	const XS::SimpleType *base = findType(base_or_item);
	assert(base!=nullptr);
	//String-pooling must be in play.
	assert(base->getName().string==base_or_item.string);
	daeMetaSchema_base = base;
	if(!base->hasList()) base_or_item = daeHashString(nullptr,0);
	XS::SimpleType &out = _addAtom(writ,base_or_item,name);
	daeMetaSchema_base = nullptr;
	//Note, _addAtom() inherits "itemType" if <xs:list> based.
	if(base_or_item==nullptr) out._itemType = base;
	assert(out.hasList()||out.getRestriction()); 
	out._value_typewriter = out._value_typewriter->per<daeArray>(); return out;
}

XS::SimpleType &XS::Schema::_addEnum
(int N, const daeClientEnum *e, const daeClientEnum *re, daeName base, daeName name, int ext)
{	
	assert(name!=nullptr&&base!=nullptr);
	//There's room for some optimization here.
	XS::SimpleType* &out = _simpleTypes[name];
	if(out!=nullptr)
	{
		assert(out==nullptr); return *out; //unacceptable
	}
	size_t size = sizeof(XS::SimpleType);
	size+=sizeof(XS::Restriction)-daeSizeOf(XS::SimpleType,_restriction);
	size+=sizeof(XS::Enumeration)-daeSizeOf(XS::Restriction,_enumeration);
	size+=ext>0?ext:sizeof(daeEnumType);
	(void*&)out = operator new(size);
	
	out->_name = name;
	out->_itemType = out; 	
	//There's room for some optimization here.				
	out->_restriction = findType(base);
	XS::Restriction &r = out->getRestriction(); assert(r);	
	{
		//String-pooling must be in play.
		assert(base.string==r.getBase().getName().string);
		r._length[0] = 0; r._length[1] = ~r._length[0];			
		r._compare[0] = INT_MIN;  r._compare[1] = INT_MAX;
		r._setMinMax<daeEnumeration>();
		r._enumeration = N; 
	}	
	XS::Enumeration &xe = r.getEnumeration(); assert(xe);
	xe._enum = e; xe._enumEnd = e+N;
	xe.string_table._begin = re; xe.string_table._end = re+N;
	out->_value_typewriter = *new(&xe+1)daeEnumType(xe); return *out;
}


//---.
}//<-'

/*C1071*/