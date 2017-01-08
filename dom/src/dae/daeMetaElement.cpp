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

void daeMeta::_clearTOC(daeElement *e)const
{
	daeOffset from = _clearTOC_offset;
	daeOffset to = _content_offset-sizeof(daeChildID);
	assert(to>=from&&to<(daeOffset)_sizeof&&from>=sizeof(DAEP::Element));
	memcpy((char*)e+from,(char*)_domAny_safe_prototype+from,to-from);
	//_content_thunk.offset is not used above because it cannot get
	//pseudo-elements, as they are the prototype. So not to branch:
	{
		//SINCE cursor() must now be set, just doing it manually:
		//enum{ sz=sizeof(daeContents)-sizeof(daeContents_base) };
		//daeChildID::POD cp[1+sz/sizeof(daeChildID)] = {0x10000};
		//assert(0==sz%sizeof(daeChildID)&&daeChildID(1,0)==cp[0]);
		//memcpy((char*)e+to,cp,sizeof(cp));
		daeContents &c = getContentsWRT(e);
		(daeCounter&)c.unnamed() = 0x10000; c.cursor() = c.data();
	}
}

void daeMeta::_construct(DAEP::Object *e, DAEP::Object &parent, daeDOM &DOM)const
{
	((daeObject*)memcpy(e,_prototype,_sizeof))->_parent(parent); 	
	(daeDOM*&)((daeElement*)e)->__DAEP__Element__data.DOM = DOM; _constructor(e);	
} 

extern const daeStringCP _daePseudoElement_[];
daeElement *daeMeta::_construct_new(daePseudoElement &pseudo_parent)const
{
	#ifdef NDEBUG
	#error daePseudoElement::getDOM()?
	#endif
	daeDOM *DOM = const_cast<daeDOM*>(pseudo_parent.getDOM()); 
	//_prototype is passed to the database so it can have a
	//little bit of information with which to initialize its
	//extended data structure.
	daeElement *out = _prototype;
	DOM->_database->_new(_sizeof,out,DOM->_databaseRejoinder);	
	
	//Hack? Because the calling APIs bypass _reparent_element_to_element()
	//as an optimization; its pseudo-parent logic is being reproduced here.	
	unsigned pseudo = _daePseudoElement_==pseudo_parent->getNCName().string;
	assert(pseudo==pseudo_parent->__DAEP__Element__data.is_document_daePseudoElement);
	daeObject &parent = pseudo==0?pseudo_parent:dae(pseudo_parent.getParentObject());

	_construct(out,parent,*DOM); return out;
}

template<class T> //daeString or daeHashString
inline daeMeta *daeMeta::_findChild3(const T &pseudonym)const
{
	//This might be helpful with reoccurrences???
	//(_elem_names_cache is not designed for this.)
	XS::Element *thread_safe_cache = _elem_names_cache;
	if(*(short*)(daeString)pseudonym
	 ==*(short*)thread_safe_cache->getName().string)
	{
		if(thread_safe_cache->getName()==daeHashString(pseudonym))
		return thread_safe_cache->getChild();
	}

	daeStringMap<int>::const_iterator it;	
	it = _elem_names.find(pseudonym); if(it==_elem_names.end())
	{
		if(getAllowsAny()) 
		{
			daeMeta *me = _schema->findChild(pseudonym);
			return me!=nullptr?me:domAny::_master.meta;
		}
		else return nullptr;
	}
	//Save the name for create/placeWRT().
	_elem_names_cache = _elem0+it->second;
	return jumpIntoTOC(it->second).getChild();
}
daeMeta *daeMeta::_findChild2(const daePseudonym &pseudonym)const
{
	return _findChild3(pseudonym);
}
daeMeta *daeMeta::_findChild2(daeString pseudonym)const
{
	return _findChild3(pseudonym);
}

daeAttribute *daeMeta::_getAttribute(const daePseudonym &pseudonym)const
{
	//REMINDER: THIS IS IMPLEMENTED BY A LINEAR LOOKUP BECAUSE domAny
	//NEEDS TO BE ABLE TO DYNAMICALLY ADD ATTRIBUTES TO ITS ATTRIBUTE
	//ARRAY INSIDE ITS PSEUDO METADATA. OTHERWISE IT WOULD USE A HASH
	//LOOKUP LIKE THE CHILDREN DO. (OR HYBRID IF SOMEONE PROFILES IT)
	for(size_t i=0;i<_attribs.size();i++)
	if(pseudonym==_attribs[i]->getName()) return _attribs[i]; return nullptr;
}

extern daeDOM &daeStringRef_protoDOM;
//A content-typewriter is a big side project.
//(It'd basically be an XML document "writer.")
extern daeTypewriter &daeStringRef_counterTW;
extern daeAlloc<daeCounter,0> daeStringRef_counterLT;
void *daeMeta::_continue_XS_Schema_addElement2(void ctor(DAEP::Object*), daeFeatureID union_feature, daeOffset content_offset)
{
	//XS::Schema::addElement has initialized a lot of the model already by this stage.	
	_bring_to_life(_elems);
	_bring_to_life(_elem_names);
	_destructor_ptr = daeMetaElement_self_destruct;
	_constructor = ctor;
	size_t features = -(int)_finalFeatureID;
	assert(_attribs.size()<features);
	size_t single_elems_32 = 0;
	size_t single_elems = (int)union_feature-(int)_finalFeatureID-1;
	assert(single_elems<features&&single_elems!=1);
	size_t approx_elems = features-_attribs.size()-(_value==nullptr?0:1)-1;
	assert(approx_elems<features&&approx_elems>=1);
	//Hack: this constructs, and decreases the count.
	//The positive childID count isn't truly knowable, but this should be a tight fit.	
	_elems.grow(approx_elems);			
	for(size_t i=0;i<approx_elems;i++) 
	_bring_to_life(_elems.data()[i]);_elems.getAU()->setInternalCounter(approx_elems);	
	_elem0 = _elems.data()+single_elems-1;
	XS::Element &e1 = _elem0[1];	
	{
		//In lieu of a default constructor...				
		e1._daeCM_init(XS::ELEMENT,this,nullptr,0,0,-1);		
		e1._substitute_maxOccurs = -1;
		memset(&e1._element,0x00,sizeof(e1._element));
		e1._maxOrdinals = 1;
		e1._element.name._clear_client_string();
		e1._element.child = domAny::_master.meta;

	  //This remaining code is just to avoid having junk data inside getTOC().

		if(single_elems>0) //Zero fill the bogus e0 void?		
		{
			single_elems_32 = single_elems/32+(single_elems%32>0?1:0);

			*_elem0 = e1; 
			//HACK: The lower bound is used to orient the _deepCM entry point.
			_elems[0]._element.childID = daeChildID(-int(single_elems-1),0);
		}
		size_t iN=approx_elems-single_elems; if(iN>1) //Zero fill in e2 and up?
		{
			//_elem0 is a hole, now only because of dae_Array's legacy 0 mode.
			//The single element bitfield is located in front of e1, which is
			//always available to permit "unnamed" elements. This means e2 is
			//a hole if there are single elements, and there may be more than
			//32 single elements, in which case the bitfield goes into e3 and
			//so on. In addition, because of 64bit alignment needs, there can
			//be an additional hole on odd numbers. So the last one is filled
			//in just in case that is the case. This is all just so that if a
			//user iterates over these elements, they won't have junk in them.
			size_t zerofill = 2+single_elems_32;
			if(zerofill%2==0) zerofill++; //One more for remainder/x64 logic?
			for(size_t i=1;i<zerofill&&i<iN;i++) _elem0[i+1] = e1;
		}
	}			
	int union_size = sizeof(dae_Array<>);
	daeOffset union_offset = content_offset-union_size;
	{
		//Automatically add the unnammed dae_Array, since it's always present.
		_addChild2(&e1,1,union_offset);
	}
	//HACK: This is a starting point. _addContentModel() adds plural children.
	_clearTOC_offset = union_offset-single_elems_32*sizeof(daeChildID::POD);
	//addContentModel() may never be called.
	_CMRoot = _CMEntree = _elem_names_cache = &e1; _IDs = _IDs_id = nullptr;
	//Set up the content/cursor features so
	//that generator output doesn't have to.
	{
		//HACK: Just getting this out of the way.
		//TODO: It would be better to addFeature
		//here at the same time for all features.
		//It's just not so straightforward to do.
		daeModel &mod = getModel();				
		{
			//These are the cursor-arrays. They just appear to be counters.		
			for(daeFeatureID fit=_finalFeatureID+1;fit!=_feature_completeID;fit++)
			{
				//Maybe these could be special somehow?
				mod[fit].setAllocThunk(daeStringRef_counterLT);
				mod[fit].setTypewriter(daeStringRef_counterTW);	
			}
		}
		//This is the "unnamed" cursor array.		
		mod.addFeature(union_feature,union_offset);		
		assert(mod[union_feature]->getSize()==union_size); 				
		daeFeature &content = mod.addFeature(_finalFeatureID,content_offset);		
		//unimplemented: a contents-array typewriter
		//would effectively be an XML text<->binary job.
		content.setTypewriter(nullptr/*daeMetaElement_contentTW*/); 				
		//This is dicey. The 1 capacity is so
		//daeContent::__COLLADA__move() can not
		//branch on empty to set up the partition.
		//REMINDER: re/deAlloc won't deallocate a 1.
		new(&_content_thunk) daeAlloc<daeContent,0>(1);
		content.setAllocThunk(&_content_thunk);		
		assert(0==content_offset%sizeof(void*));
		_content_offset = content_offset; //Computable...
		_content_thunk._offset = content_offset+daeOffsetOf(daeContents,getAU()); //_au 		
	}	
	_prototype->__DAEP__Element__data.NCName = _name;
	(daeMeta*&)_prototype->__DAEP__Element__data.meta = this;
	(daeDOM*&)_prototype->__DAEP__Element__data.DOM = (daeDOM*)&daeStringRef_protoDOM;
	//Assign the prototype's thunk. This couldn't be more clumsy.
	daeContents &c = getContentsWRT(_prototype);
	c.getAU() = (daeAlloc<daeContent>*)&_content_thunk; c.cursor() = &_content_0_terminator;
	assert(c.cursor()==c.data());
	//This is the only time user/client headers see the prototype.
	return _prototype;
}
extern void daeMetaElement_self_destruct(daeMetaObject *self) 
{
	((daeMetaElement*)self)->_self_destruct();
}
void daeMetaElement::_self_destruct() 
{
	//VERY DICEY PROTOTYPE DESTRUCTION PROCEDURE
	{
		 //DEFECT REPORT
		//For this to work, the DAEP::Proto constructors
		//must not copy on an "empty" value, in order to
		//not copy the prototype into itself/leak memory.

		assert(_prototype->_isData());
		//Turn the data-bit off for the last time.
		//It's falsely indicating there is a database.
		//(So that _construct() doesn't have to flip it on.)
		(&_prototype->_getClassTag())[3]^=1; 
		assert(!_prototype->_isData());

		//It's not clear that this would work in general, but
		//it's too late at this stage to be deleting elements.
		//Since their metadata may've already been destructed.
		assert(getContentsWRT(_prototype).empty());
		daeAttribute *a = _attribs.data();
		for(size_t i=0,iN=_attribs.size();i<iN;i++)		
		{
			if(nullptr!=a[i]._destructor)
			a[i]._destructor(&a[i].getWRT(_prototype));
		}
	}
	
	//CONTENT-MODEL GRAPH DELETION PROCEDURE
	//Don't delete _CMRoot if it is one of _elems.
	if(_CMRoot<_elems.begin()||_CMRoot>=_elems.end())
	{
		//_parentCM is used to signal ownership.
		//_elems will do the deed after ~daeMetaElement().
		for(size_t i=0;i<_elems.size();i++) 
		{
			_elems[i]._parentCM = nullptr; //Protect _elems[i].
			//HACK: Must delete xs:group namefellows somewhere.
			XS::Element*q,*p=&_elems[i];
			while((q=p->_element.namefellows)!=nullptr) 
			if(this!=q->_meta)
			{
				p->_element.namefellows = q->_element.namefellows;
				delete q;
			}
			else p = q;
		}		
		delete _CMRoot; //Delete the CM graph.
	}
	
	if(_value!=nullptr)
	{
		//This likely does nothing, but you never know.
		_value->~daeValue(); 
		//This is the only real opportune time to check 
		//that generators set up daeContentModel::SIMPLE.
		assert(&_value->getSimpleType()!=nullptr);
	}

	this->~daeMetaElement();
}

XS::Attribute &daeMeta::_addAttribute(daeFeatureID fID, daeOffset os, daeAlloc<>&lt(daeAllocThunk&), daeHashString type, daeHashString name)
{
	bool value = name==nullptr;

	daeValue &out = value?*_value:_attribs[-fID-1];
	
	const XS::SimpleType &st = *_schema->findType(type);
	
	assert(nullptr!=&st&&nullptr!=&out); //These are generator check.

	daeTypewriter &tw = st.getValueTypewriter();

	daeFeature &mf = getModel().addFeature(fID,os,tw.getSize());
	{
		mf.setTypewriter(tw);		
		//These thunks assigned by _addElement().
		//But setAllocThunk() is not yet done to them.
		//(It could've been, but it would be pointless too.)
		daeAllocThunk &t = const_cast<daeAllocThunk&>(mf.getAllocThunk());
		{
			lt(t); t._prototype = &out._type;
			//Here _offset is array->object. It must be 0
			//unless the thunk is actually used by arrays.
			if(st.hasList()) t._offset = os;	
		}//HACK: set _flags.atomizing as side-effect. 
		mf.setAllocThunk(&mf.getAllocThunk()); 
		
		//If this is the last attribute, or the value
		//and the attributes are set up (allowing some
		//unnecessary flexibility to the generator) the
		//model features need to be finalized. The child
		//features are trivial, and so are not considered.		
		if(fID==_feature_completeID) _feature_complete(_feature_completeID);
	}

	out._meta = this;
	out._offset = os;
	if(name==nullptr) //value?
	{
		out._attribute_name._clear_client_string();
	}
	else out._attribute_name = name;	
	out._simpletype = &st;
	out._type.alias = st.getName();
	out._type.writer = &tw; 
	out._type.value = daeOpaque(_prototype)[os];
	if(st.hasList())
	(const daeAllocThunk*&)out._type.value = &mf.getAllocThunk();	
	if(st.getRestriction())
	{
		out._type.minLength = st.getRestriction().getMinLength();
		out._type.maxLength = st.getRestriction().getMaxLength();
	}
	else if(st.hasList())
	{
		out._type.maxLength = ~(out._type.minLength=0); 
	}
	else out._type.minLength = out._type.maxLength = 1;	

	switch(tw.getAtomicType())
	{
	case daeAtomicType::TOKEN: case daeAtomicType::STRING:

		(daeString&)out._type.value = daeStringRef_empty;

		_addAttribute_maybe_addID(out,_prototype); //ID?
	} 
	return static_cast<XS::Attribute&>(out);	
}
void daeMetaElement::_addAttribute_maybe_addID(daeAttribute &maybe_ID, const daeElement *proto_or_any)
{
	if(nullptr==_schema->_IDs.find(maybe_ID._attribute_name))
	return;	
	XS::Attribute *it = const_cast<XS::Attribute*>(_IDs); 
	XS::Attribute *ID = const_cast<XS::Attribute*>(&maybe_ID);
	if(nullptr!=it)
	{
		while(it->_next_attribute_is_ID) 
		it = it+it->_next_attribute_is_ID; 
		while(it<ID)
		{
			int diff = ID-it;
			assert(diff>0);
			it->_next_attribute_is_ID = diff; 
			if(diff!=(int)it->_next_attribute_is_ID) 
			{
				assert(0); //NOT EXPECTING
				//HACK: Determine the maximum delta
				//of a bitfield. It's not worth doing
				//this in a header.
				it->_next_attribute_is_ID = 0;
				diff = it->_next_attribute_is_ID-=1;
			}
			it+=diff;
		}; assert(it==ID);
	}else _IDs = it = ID;
	it->_this_attribute_is_ID = 1; 	
	if("id"==it->_attribute_name) _IDs_id = it; //LEGACY		
	proto_or_any->__DAEP__Element__data.getMeta_getFirstID_is_nonzero = 1;
}
void daeValue::_setDefaultString(daeHashString def)
{
	assert(_meta->_prototype->_isData());
	//Turn the data-bit off for the last time.
	//It's falsely indicating there is a database.
	//(So that _construct() doesn't have to flip it on.)
	(&_meta->_prototype->_getClassTag())[3]^=1; 
	assert(!_meta->_prototype->_isData());
	{		
		assert(_default==nullptr); _default = def;

		if(DAE_OK!=_type->stringToMemory(def,_type.value)) assert(0);
	}
	(&_meta->_prototype->_getClassTag())[3]^=1; 
}	

daeParentCM::~daeParentCM()
{
	for(size_t i=0;i<_CM.size();i++)
	{	
		//daeMetaElement::~daeMetaElement() sets these to nullptr.
		if(this==_CM[i]->getParentCM()) delete _CM[i]; 
	}
	//TODO? _deepCM could be part of a variable-length daeParentCM.
	delete[](_deepCM+_meta->getTOC().front().getChildID().getName());
}
static void daeMetaElement_newGroup(daeCM* &out, daeTOC &TOC)
{
	size_t namesN = TOC.getCapacity(); short *jump_table;
	(void*&)out = operator new(sizeof(XS::Group)+namesN*sizeof(*jump_table));	
	jump_table = (short*)((char*)out+sizeof(XS::Group));	
	new(out) XS::Group(jump_table-TOC[0].getChildID().getName());
	//__subsetof() needs to know if the names exist or not.
	for(size_t i=0;i<namesN;i++) jump_table[i] = XS::Group::_groupNames_not_included;
}
daeCM &daeMeta::_addCM(daeXS xs, daeParentCM *parent, daeCounter sub, int minO, int maxO)
{
	daeCM *out; switch(xs) //Initialize.
	{
	case XS::ALL: out = new XS::All; break;
	case XS::ANY: out = new XS::Any; break;
	case XS::CHOICE: out = new XS::Choice; break;
	case XS::ELEMENT: //The real element is resolved by setChild().
	new(out=&_schema->_temporary_working_memory)XS::Element; break;
	case XS::GROUP:	daeMetaElement_newGroup(out,getTOC()); break;
	case XS::SEQUENCE: out = new XS::Sequence; break;
	default: assert(0); return *(out=nullptr); //???
	}
	out->_daeCM_init(xs,this,parent,sub,minO,maxO);	assert(maxO!=0);

	if(daeCM::isParentCM(xs)) //Allocate the off-center child ID jump-table.
	{	
		//The daeParentCM destructor undoes this.
		daeParentCM &pout = (daeParentCM&)*out;		
		size_t avail = _elems.size();
		if(xs==XS::CHOICE) avail*=2; //OVERKILL//OVERKILL//OVERKILL//OVERKILL
		//TODO: do something like daeMetaElement_newGroup() once more solid.
		pout._deepCM = new daeParentCM::_DeepCM_string[avail];
		pout._deepCM-=_elems[0].getChildID().getName();
	}	
	else assert(parent!=nullptr||xs==XS::GROUP); //These are XML Schema rules.

	if(nullptr!=parent) //Add the CM to its parent.
	{
		//Check the generator has passed a parent type.
		assert(parent->isParentCM()); 
		
		parent->_CM.push_back(out);		

		//This is the XS::Any equivalent of setChild().
		//Establish <xs:any> path for unnamed children.
		//(Technically, depending on how <xs:any> are setup,
		//they can be sinks for named children as well. It's not
		//clear what the library's auto-placement facility should do.)
		if(XS::ANY==xs) parent->_deepCM_push_back(1,out);
	}
	else //Assign the CM graph's root pointer.
	{
		//Check the generators are not reassigning/nothing's changed.
		assert(_CMRoot==_elem0+1); 
		//_elem0 is the default for empty/any content-models.		
		_CMRoot = out;
	}
	return *out;
}
 
void XS::Group::_setGroup(daeMeta &group)
{
	_groupCM = group.getCMEntree(); //Or getCMRoot()?
}
XS::Element *XS::Group::_addChild(daeFeatureID fid, daeOffset os, daePseudonym name)
{
	daeMeta *group = getGroup();
	daeStringMap<int>::const_iterator it = group->_elem_names.find(name);	
	if(it==group->_elem_names.end()) 
	{
		//<gles2_value_group><usertype><setparam><array> requires this.
		XS::Schema::_MissingChild c = { group,_meta,this,fid,os,name };
		_meta->getSchema()._missingChildren.push_back(c); return nullptr;
	}
	int ii = it->second; XS::Element &real = 
	const_cast<daeMetaElement*>(_meta)->_addChild(group->jumpIntoTOC(ii),fid,os,name);		
	int i = real.getChildID().getName(); _groupNames[i] = ii; 
	if(_parentCM!=nullptr) _parentCM->_deepCM_push_back(i,this); return &real;
}
void daeMeta::_addContentModel(const daeCM *generator_root)
{
		//INDENTED CODE CONSTITUTES THE NORMAL FUNCTIONING OF THIS API.
		assert(generator_root==_CMRoot); 

	std::vector<XS::Schema::_MissingChild> 
	&missing = getSchema()._missingChildren;
	for(size_t i=0;i<missing.size();i++)
	{
		if(this==missing[i].meta) //There's a ciruclar transclusion situation?
		return;
	}
		//INDENTED CODE CONSTITUTES THE NORMAL FUNCTIONING OF THIS API.
		_CMEntree = const_cast<daeCM*>(_CMRoot)->_prepareContentModel();

	for(size_t j,i=0;i<missing.size();) 
	{	
		if(this==missing[i].ref) //There's a ciruclar transclusion resolution?
		{
			XS::Schema::_MissingChild &e = missing[i];
			if(nullptr==e.CM_graph_node->_addChild(e.fid,e.os,e.name)) 
			assert(0);
			daeMeta *swap = missing[i].meta;
			//Could do in bulk with erase(remove_if) but C++98 is not worth it.
			missing.erase(missing.begin()+i);
			for(j=i;j<missing.size();j++) if(swap==missing[j].meta)
			goto erased;
			const_cast<daeMetaElement*>(swap)->_addContentModel(swap->_CMRoot);
			i = 0; //just start over in case _addContentModel removed elements. 
		}
		else i++; erased:;
	}
		//HACK: it will be less if there are only non-plural children.
		_clearTOC_offset = std::min<size_t>(_clearTOC_offset,_elems.back().getOffset());
}
XS::Element &XS::Element::_setChild3(daeFeatureID fid, daeOffset os, daePseudonym name)const
{	
	assert(this==&_meta->getSchema()._temporary_working_memory);
	return const_cast<daeMetaElement*>(_meta)->_addChild(*this,fid,os,name);
}
XS::Element &daeMeta::_addChild(const XS::Element &cp, daeFeatureID fid, daeOffset os, daeHashString name)
{	
	assert(os>sizeof(daeElement));	

	//This could be easier if _addElement() had more members.
	daeOffset pivot = _elem0-_elems.begin();
	daeOffset i, toc = _content_offset-os;
	XS::Element *out;
	if(toc>sizeof(daeCursor)) 
	{
		//positive dae_Array
		i = toc/sizeof(daeCursor);
		assert(i>=2&&0==toc%sizeof(daeCursor));
	}
	else //negative dae_Array
	{
		i = -pivot-1+(fid-_finalFeatureID);		
		assert(i<0&&toc==sizeof(daeCursor));
	}
	_elem_names[name] = i;
	out = _jumpIntoTOC(i); //This includes a bounds check.
	bool first_instance_of_name = 0==out->_element.offset;

	//TODO: This can reasonably be done in bulk by addElement().
	//But it should be double-checked here if/when it's done so.
	if(first_instance_of_name) getModel().addFeature(fid,os);

	//Determine if this is the first <xs:element> with this name,
	//-and if not, link it into the namefellow singly linked list.
	if(!first_instance_of_name)
	{
		while(out->_element.namefellows!=nullptr) 
		out = out->_element.namefellows;
		out = out->_element.namefellows = new XS::Element;
	}	
	out->_element.namefellows = nullptr; //0 terminate their list.
	//Copy the addCM() information.
	memcpy(out,&cp,sizeof(daeCM));
	
	if(cp._meta!=this) //Transcluding?
	{
		//Group elements retain the parent of their content-model, and
		//if namefellows are preset, they're deleted in _self_destruct.
		out->_meta = this;
		out->_element.child = cp._element.child;
		out->_substitute_maxOccurs = cp._substitute_maxOccurs;
	}
	else //Repair the CM graph and fill out jump-tables.
	{	
		daeParentCM *p = out->_parentCM;
		assert(p!=nullptr&&p->_CM.back()==cp);		
		p->_CM.back() = out; p->_deepCM_push_back(i,out);
	}
	//Finish up.
	assert(name!=nullptr);
	out->_element.name = name; 
	out->_element.namefellows_demote = 0; return _addChild2(out,i,os);	
}
XS::Element &daeMeta::_addChild2(XS::Element *out, int i, daeOffset os)
{
	out->_element.offset = os;
	//There is ambiguity about where the only-child (i<0) kind are located.
	//This is the understood address because they share pointer-to-members.
	out->_element.content_offset = (i>0?i:1)*-daeOffset(sizeof(daeCounter));
	out->_element.childID = daeChildID(i,0);
	//Set up the dynamic dae_Array<T,0> ID. The counter would be zero either way.
	if(i>0) (daeChildID&)out->getWRT(_prototype) = out->_element.childID; return *out;
}

daeElementRef daeMeta::createWRT(daePseudoElement *parent, const daePseudonym &pseudonym)const
{	
	//Reminder: Historically this API returns nullptr.
	//(It could otherwise create an "unnamed" domAny.)	
	daeMeta *ME = _findChild3(pseudonym); 
	
	if(ME==nullptr) return nullptr;

	daeElement *out = ME->_construct_new(*parent);
	
	out->getNCName() = pseudonym; return out;	
}
//CIRCULAR-DEPENDENCY
template<> inline void daeCM_Demotion<>::maybe_demote()
{
	if(_meta.jumpIntoTOC(daeCM_Placement::name)._element.namefellows_demote!=0)
	_meta.getCMEntree()._chooseWithout(*this);
}
bool daeMeta::_typeLookup_unless(daeElement *e)
{	
	//Unfortunately, this requires deep inspection.
	//This bit is set if the contents-array's been touched.
	if(!e->getOptimizationBit_is_definitely_not_a_graph())
	{
		//This is to avoid deep inspection if users
		//aren't calling daeDocument::typeLookup() at all.
		//extern bool daeDocument_typeLookup_called;
		return daeDocument_typeLookup_called;
	}
	//In practice, this can't work in the prototype.
	//Should daeDocument_typeLookup_called be checked first?
	//daeDocument_typeLookup_called is off in some other part of memory.
	return e->getMeta()->_daeDocument_typeLookup_enabled;
}
daeError daeMeta::_placeWRT(daeElement &parent, daeElement &child)const
{
	_place_operation OK; 
	OK.prolog(*this,parent,child); if(!OK) return OK;
	daeError e = _CMEntree->_placeElement(OK); if(e<DAE_OK) return e;
	OK.default_insertion_point_to_back(); OK.epilog(child); return OK?e:OK;
}
daeError daeMeta::_placeWRT(daeElement &parent, daeContent &content)const
{
	_place_operation OK;
	OK.prolog(*this,parent,*content.getKnownChild()); if(!OK) return OK;
	daeError e = _CMEntree->_placeElement(OK); if(e<DAE_OK) return e;
	OK.default_insertion_point_to_back(); OK.epilog(content); return OK?e:OK;
}
daeError daeMeta::_placeBetweenWRT(daeElement &parent, void *a, daeElement &child, void *b)const
{
	_place_operation OK;
	OK.prolog(*this,parent,child); if(!OK) return OK;
	OK.uncursorize(a,b);
	daeError e = _CMEntree->_placeElementBetween(OK.a,OK,OK.b); if(e<DAE_OK) return e;
	OK.reach_insertion_point(); OK.epilog(child); return OK?e:OK;
}
daeError daeMeta::_placeBetweenWRT(daeElement &parent, void *a, daeContent &content, void *b)const
{
	_place_operation OK;
	OK.prolog(*this,parent,*content.getKnownChild()); if(!OK) return OK;							   
	OK.uncursorize(a,b);
	daeError e = _CMEntree->_placeElementBetween(OK.a,OK,OK.b); if(e<DAE_OK) return e;
	OK.reach_insertion_point(); OK.epilog(content); return OK?e:OK;
}
daeError daeMeta::_removeWRT(daeElement &parent, daeElement &child)const
{	
	assert(this==parent->getMeta()); //LEGACY	
	daeContents &rc = getContentsWRT(&parent);
	//TODO: check the cursor given a child-ID index.
	size_t found; if(!rc.find(&child,found))
	{
		//Historically this API has worked this way.
		return DAE_ERR_INVALID_CALL;
	}	
	return _removeWRT(parent,rc[found]);
}
daeError daeMeta::_removeWRT(daeElement &parent, daeContent &content)const
{
	assert(this==parent.getMeta()); //LEGACY
		 
	daeContents &c = getContentsWRT(&parent);
	daeCM_Demotion<> d(*this,c,content);

	//Passing placeholders through this API seems safe.
	//Still, it seems wrong to encourage this practice.
	if(nullptr!=content._child.ref)
	{
		d.count = c.__decrement_internal_counter(d.name).getIndex();
		c._remove_child(&content);
		d.maybe_demote();
	}
	else
	{
		//It must remain viable through daeNoteChange().
		daeElementRef RAII = content._child.ref;

		#ifdef NDEBUG
		#error Need a mechanism for suppressing notifications.
		#endif
		bool notify = true;
	
		daeContents::_remove_operation op(d);
		//SOMEHOW DO THIS ALL AT ONCE
		if(_typeLookup_unless(content._child.ref))	
		op.typeLookup = true;
		if(notify) daeNoteChange(op); 		
	}
	return d.maybe_demote_ORDER_IS_NOT_PRESERVED; 
}

void daeMeta::_place_operation::prolog(daeMeta &m, daeElement &p, daeElement &c)
{
	//WARNING: content IS INITIALIZED LAST THING.

	assert(m==p.getMeta()); //LEGACY
	assert(&p!=nullptr); if(&c==nullptr)
	{
		//There's not enough information to extract the parent
		//in this case, even if there is a content placeholder.
		error = DAE_ERR_INVALID_CALL; 
		return;
	}

	XS::Element *thread_safe_cache = m._elem_names_cache;
	//_elem_names_cache is set by _findChild2() via createWRT().
	//It may be counter-productive from a caching perspective, but
	//its purpose is to avoid doing a lookup for placeWRT(), which is
	//designed to follow createWRT().
	daeCTC<!c.__DAEP__Element__data._have_child_ID_onboard>();
	if(thread_safe_cache->getName().string!=c.getNCName().string)
	{	
		daeStringMap<int>::const_iterator it;	
		it = m._elem_names.find(c.getNCName());
		//IT'S NOT SAFE TO SEND A NAME TO daeCM THAT DOESN'T EXIST.
		//CHECKING getAllowsAny() MAY NOT BE THE BEST WAY TO DO IT.
		//IDEALLY 1 WILL ALWAYS HAVE AN OUTLET, BUT IT DOESN'T YET.
		//daeParentCM::_deepCM's COMMENTS HAVE SOME THOUGHTS ON IT.
		if(it==m._elem_names.end()) if(!m.getAllowsAny())
		{
			//NOTE: EVEN IF THE ABOVE RECOMMENDATIONS ARE REALIZED
			//THESE APIS HAVE HISTORICALLY ERRORED OUT IN THIS WAY.
			error = DAE_ERR_QUERY_SYNTAX; 
			//This is a good indicator that the element isn't from
			//createWRT() and requires it's name to be changed via
			//getNCName() to be accepted. This is programmer error.
			assert(it!=m._elem_names.end()); 
			return;
		}
		else name = 1; else name = it->second; 
	}
	else name = thread_safe_cache->getChildID().getName();
	//WARNING: content IS NOT YET INITIALIZED.
	new(this) daeCM_Placement<>(name,count,m.getContentsWRT(&p)); 
	count = content.__plural_or_non_size(name);
}
void daeMeta::_place_operation::uncursorize(void *after, void *before)
{
	daeCursor itt = content.end();
	daeCursor iit = content.data();	
	_a = content.__uncursorize(after); if(_a>=itt)
	{
		a = daeOrdinals; _a = iit-1;
	}
	else //Could ignore text if _a is a known to be a child.
	{	
		daeCursor rit = _a, ritext;
		ritext = (daeCursor)rit->getText(); 
		if(ritext!=nullptr)
		{
			//SHOULD THE TEXT BE SPLIT IN 2 IN THIS CASE???
			_a = ritext+ritext->getKnownText().span()-1;

			for(rit=ritext-1;rit->hasText()&&rit>iit;)
			rit-=rit->getKnownEndOfText().counterspan();
			a = rit>=iit?iit->_child.ordinal:0;
		}		
		else a = rit->_child.ordinal; 
	}
	_b = content.__uncursorize(before); if(_b>=itt) 
	{
		b = 0; _b = itt;
	}
	else //Could ignore text if _b is a known to be a child.
	{	
		daeCursor it = _b, itext;
		itext = (daeCursor)it->getText(); 
		if(itext!=nullptr)
		{	
			//SHOULD THE TEXT BE SPLIT IN 2 IN THIS CASE???
			_b = itext;

			it = itext+itext->getKnownStartOfText().span();
			while(it->hasText()) it+=it->getKnownStartOfText().span();
			assert(it<content.end()); 
		}		
		b = it->_child.ordinal; 
	}
}
void daeMeta::_place_operation::reach_insertion_point()
{
	daeCursor iit = _a+1; if(a!=ordinal)
	{
		daeCursor itt = _b; if(b!=ordinal)
		{
			daeCursor it = iit;
			while(it->hasText()) it+=it->getKnownStartOfText().span();
			if(ordinal>=it->_child.ordinal)
			{
				//THIS IS NOT NECESSARILY THE MOST EFFICIENT MOVE AT
				//THIS STAGE, BUT IT SHOULD WORK.
				while(ordinal<it->_magic_ordinal) 
				it++;
				insertion_point = it; assert(it<=itt);
			}
			else insertion_point = iit;
		}
		else insertion_point = itt;
	}
	else insertion_point = iit;
}
void daeMeta::_place_operation::epilog(daeElement &child)
{
	#ifdef NDEBUG
	#error Need a mechanism for suppressing notifications.
	#endif
	bool notify = true;

	//daeElementTags::namespaceTag
	//If 0 then the element has not been placed before.
	if(!child.isContent())
	{
		daeContents::_insert_operation op(*this,child);
		//SOMEHOW DO THIS ALL AT ONCE
		if(_typeLookup_unless(child))	
		op.typeLookup = true;
		if(notify) daeNoteChange(op); error = op;		
	}
	else //TODO: check the cursor given a child-ID index.
	{
		assert(nullptr!=child.getParentElement());
		daeElement &parent2 = (daeElement&)child.getParentObject();		
		daeContents &rc = parent2.getContents();		
		size_t found; if(!rc.find(&child,found))
		{
			//This shouldn't be. Release builds should assume success.
			assert(0); error = DAE_ERR_INVALID_CALL; return;
		}
		daeCM_Demotion<> d(parent2.getMeta(),rc,rc[found]);
		daeContents::_insert_move_operation op(d,*this);
		//SOMEHOW DO THIS ALL AT ONCE
		if(_typeLookup_unless(child))	
		op.typeLookup = parent2.getDocument()!=content.getElement().getDocument();
		if(notify) daeNoteChange(op); error = op;
	}
}
void daeMeta::_place_operation::epilog(daeContent &content)
{
	#ifdef NDEBUG
	#error Need a mechanism for suppressing notifications.
	#endif
	bool notify = true;

	daeElement &child = *content.getKnownChild();
	assert(nullptr!=child.getParentElement());
	daeElement &parent2 = (daeElement&)child.getParentObject();		
	daeCM_Demotion<> d(parent2.getMeta(),parent2.getContents(),content);
	daeContents::_insert_move_operation op(d,*this);
	//SOMEHOW DO THIS ALL AT ONCE
	if(_typeLookup_unless(child))	
	op.typeLookup = parent2.getDocument()!=content._child.ref->getDocument();
	if(notify) daeNoteChange(op); error = op;
}

//This is neither of the create/placeWRT family nor of the "WID" family.
const daeChildRef<> &daeMeta::pushBackWRT(daePseudoElement *parent, const daePseudonym &pseudonym)const
{
	daeError e;
	daeContents &c = getContentsWRT(parent);
	daeCM_Placement<> p(c); 
	daeContent* &p_insertion_point = //nonconst
	const_cast<daeContent*&>(p.insertion_point);

	//Locate the back-most child whose 
	//ordinal is ordered: or is nonzero.
	//Starting at c.end() would be bad if
	//the array is significantly unordered.
	const daeContent *iit = c.begin();
	const daeContent *rit = c.cursor();
	if(0==rit->_child.ordinal)
	{
		//This is not normal. 
		//It does occur when text-nodes are pushed-back.
		if(rit--!=iit)
		{
			//Rewind to the first non-0/ordered ordinal.
			while(0==rit->_child.ordinal&&rit>=iit)
			rit--;
			//Try to avoid this rewind next time around.
			c.cursor() = rit+1;
			if(c.cursor()->hasText())
			c.cursor()++;
		}
	}
	else //Normally elements will do this everytime.
	{
		//Normally this advances once testing twice.
		while(0!=rit[1]._child.ordinal) rit++;
	}
	const daeContent *back = rit+1;	

	XS::Element *thread_safe_cache = _elem_names_cache;
	//This might be helpful with reoccurrences???
	//(_elem_names_cache is not designed for this.)
	if(thread_safe_cache->getName()!=pseudonym)
	{	
		daeStringMap<int>::const_iterator it;	
		it = _elem_names.find(pseudonym);
		//IT'S NOT SAFE TO SEND A NAME TO daeCM THAT DOESN'T EXIST.
		//CHECKING getAllowsAny() MAY NOT BE THE BEST WAY TO DO IT.
		//IDEALLY 1 WILL ALWAYS HAVE AN OUTLET, BUT IT DOESN'T YET.
		//daeParentCM::_deepCM's COMMENTS HAVE SOME THOUGHTS ON IT.
		if(it==_elem_names.end()) 
		{
			p.name = 1; if(!getAllowsAny())
			{
				//NOTE: IF THIS ISN'T HANDLED LIKE THIS THEN 0s WILL
				//HAVE TO BE DEALT WITH AFTER _placeElementBetween().
				p.count = c.unnamed().size();
				goto unordered;
			}
		}
		else p.name = it->second;

		_elem_names_cache = _elem0+it->second;
	}
	else p.name = thread_safe_cache->getChildID().getName();

	p.count = c.__plural_or_non_size(p.name);

	//Place after the last nonzero ordinal. Text-nodes may get in the way.
	while(rit->hasText()&&rit>iit) 
	rit-=rit->getKnownEndOfText().counterspan();
	e = _CMEntree->_placeElementBetween
	(rit<iit?daeOrdinals:rit->_child.ordinal,p,daeOrdinal0); 

	//If this is hit the below code needs to be revised if 0 is legitimate.
	assert(e<DAE_OK||0!=p.ordinal);

	if(e!=DAE_OK)
	{
		if(e==DAE_ORDER_IS_NOT_PRESERVED) //<xs:all>?
		{		
			size_t grow = back-rit; if(grow>1) //There's grow-1 text nodes?
			{
				//There is text in front of "back," so it must be moved
				//to the insertion point. This is to preserve <!--comments
				//mainly. It pulls in mixed-text & processing-instructions
				//also. This is the default behavior of <xs:all>.

				//REMINDER: a DAEP::Change(DAEP::CONTENT) is not being done.
				//DAE_ORDER_IS_NOT_PRESERVED is more of a technicality. If 
				//a schema needs to preserve order, then there needs to be 
				//a way to communicate that on an individual <xs:all> basis.

				#ifdef NDEBUG
				#error The below code has not been vetted.				
				#endif
				assert(0);

				c.grow(c.size()+grow); //It's the back-reserve that's grown.
				c._insert_child<+1>(back);
				void *swapbuf = c.end();
				size_t swapmem = sizeof(daeContent)*grow;
				size_t movemem = sizeof(daeContent)*c.size()-swapmem;				
				memcpy(swapbuf,&c[rit-iit+1],swapmem);
				//The child sits behind the text-nodes.
				p.insertion_point = &c[p.insertion_point-iit];
				memmove(p_insertion_point+grow,p.insertion_point,movemem);
				memcpy(p_insertion_point,swapbuf,swapmem);								
				//CONVENTION: keep the back-memory 0 filled so regular
				//high-frequency code doesn't have to.
				memset(swapbuf,0x00,swapmem);
				//By definition there must be a child. _insert_child(back)
				//should have set the cursor on back, and the child that is
				//causing this move should slide into the position of back.
				assert(c.cursor()->hasChild());
			}
			else c._insert_child<+1>(p.insertion_point);
		}
		else unordered:
		{
			//NEW: This forces the element into the extra-schema group.
			//FYI: The other APIs don't do this due to either legacy or 
			//stability requirements.
			p.name = 1; 
			//TODO: This is needed to implement extra-schema attributes.
			daeCTC<daeElement::xs_anyAttribute_is_still_not_implemented>();

			p.ordinal = 0; goto unordered2;			
		}
	}
	else if(back==c.end()) 
	{
		c.cursor() = c.end(); unordered2:

		p.insertion_point = c._append_child<+1>();
	}
	else p.insertion_point = c._insert_child<+1>(back);

	c.__increment_internal_counter(p.name);
	new(p_insertion_point) daeContent(p.ordinal,daeChildID(p.name,p.count),nullptr);
	
	daeTOC::Atom &el = jumpIntoTOC(p.name);
	daeMeta *ME = el.getChild();
	daeElement *child = ME->_construct_new(*parent);	
	if(p.name==1) 
	{
		//This is using the system-string table.
		//It might help if callers could vouch for the source of the name's string?
		child->getNCName() = daeStringRef(pseudonym); 
	}
	else child->getNCName() = el.getName(); 
	
	#ifdef NDEBUG
	#error Need a mechanism for suppressing notifications.
	#endif
	bool notify = true;
	
	daeContents::_populate_new_operation op(*p_insertion_point,*child);
	//SOMEHOW DO THIS ALL AT ONCE
	if(_typeLookup_unless(child))
	op.typeLookup = true;
	if(notify) daeNoteChange(op);

	return p_insertion_point->_child_ref();
}

namespace
{
	//This ended up getting less use than anticipated.
	struct daeMetaElement_unpack_WID
	{
		int name; daeCounter index;
		daeElement &parent; daeMeta &meta;
		daeMetaElement_unpack_WID(daeChildID ID, daeContents &c)
		:name(ID.getName()),index(ID.getIndex()),parent(c.getElement()),meta(parent.getMeta())
		{}
	};	
	template<class T> 
	struct daeMetaElement_setWID2_a //C++98/03-SUPPORT (C2918)
	{
		T _child; daeElement *child(){ return _child; }
		daeContent &content(daeContents&){ return _child; }
	};template<>
	daeContent &daeMetaElement_setWID2_a<daeElement*>::content(daeContents &c)
	{
		size_t found; if(DAE_OK!=c.find(_child,found)) assert(0);
		return c[found];
	}
}
const daeChildRef<> &daeMeta::_addWID(daeChildID ID, daeContents &c, daeContent *it)
{
	daeMetaElement_unpack_WID up(ID,c);	
	daeTOC::Atom &el = up.meta.jumpIntoTOC(up.name);
	daeMeta *ME = el.getChild();
	daeElement *child = ME->_construct_new(up.parent);
	if(1!=up.name) child->getNCName() = el.getName(); 
	#ifdef NDEBUG
	#error Need a mechanism for suppressing notifications.
	#endif
	bool notify = true;
	if(it==nullptr)
	{
		//Under current guidelines it should be the case
		//that if it==nullptr then the index is the count.
		assert(up.index==c.__plural_or_non_size(up.name));
		daeCM_Placement<> p(up.name,up.index,c); 				
		if(DAE_OK>up.meta._CMEntree->_placeElement(p))
		{	
			//Failure is not an option.
			p.set_insertion_point_et_cetera_to_extra();
		}
		else p.default_insertion_point_to_back();
		//HACK/RAII: it is receiving p.insertion_point.
		{
			daeContents::_insert_new_operation op(p,*child);
			//SOMEHOW DO THIS ALL AT ONCE
			if(_typeLookup_unless(child))
			op.typeLookup = true;
			if(notify) daeNoteChange(op);
		}
		it = const_cast<daeContent*>(p.insertion_point);
	}				
	else
	{
		daeContents::_populate_new_operation op(*it,*child);
		//SOMEHOW DO THIS ALL AT ONCE
		if(_typeLookup_unless(child))
		op.typeLookup = true;
		if(notify) daeNoteChange(op);
	}
	assert(it->_child.ref->isContent()); return it->_child_ref();
}
template<class S> 
inline void daeMeta::_setWID3(daeChildID ID, S &source, daeContents &c, daeContent* &it)
{
	daeMetaElement_unpack_WID up(ID,c);
	daeElement *child = source.child();
	#ifdef NDEBUG
	#error Need a mechanism for suppressing notifications.
	#endif
	bool notify = true;
	if(it==nullptr)
	{
		//Under current guidelines it should be the case
		//that if it==nullptr then the index is the count.
		assert(up.index==c.__plural_or_non_size(up.name));
		daeCM_Placement<> p(up.name,up.index,c); 	
		if(DAE_OK>up.meta._CMEntree->_placeElement(p))
		{	
			//Failure is not an option.
			p.set_insertion_point_et_cetera_to_extra();
		}
		else p.default_insertion_point_to_back();

		if(child==nullptr) //Placeholder?
		{
			c.__increment_internal_counter(up.name);
			new(c._insert_child<+1>(p.insertion_point)) daeContent(p.ordinal,ID,nullptr);
		}
		else 
		{
			const daeName &rename = 
			//HACK: child->name=child->name is easier than maintaining two paths.
			up.name==1?child->getNCName():up.meta.jumpIntoTOC(up.name).getName();

			if(child->isContent())
			{		
				//THIS IS THE EXACT SAME LOGIC AS BELOW.
				assert(nullptr!=child->getParentElement());
				daeElement &parent2 = (daeElement&)child->getParentObject();		
				daeContents &rc = parent2.getContents();
				daeContent &rm = source.content(rc);
				daeCM_Demotion<> d(parent2.getMeta(),rc,rm);				
				daeContents::_insert_move_rename_operation op(d,p,rename);
				//SOMEHOW DO THIS ALL AT ONCE
				if(_typeLookup_unless(child))
				op.typeLookup = parent2.getDocument()!=up.parent.getDocument();
				if(notify) daeNoteChange(op); 
			}
			else
			{					
				daeContents::_insert_rename_operation op(p,*child,rename);
				//SOMEHOW DO THIS ALL AT ONCE
				if(_typeLookup_unless(child))
				op.typeLookup = true;
				if(notify) daeNoteChange(op);
			}
		}
		it = const_cast<daeContent*>(p.insertion_point);
	}				
	else 
	{
		if(it->_child.ref!=nullptr) //Vacate?
		{
			//_populate_move_operation() can't 
			//handle self-reassignment. vacate()
			//releases its ref. This seems for the
			//best. Note, there is no requirement to
			//do with change-notices.
			if(child==it->_child.ref) //Assigning to self?
			{
				return; //NOP
			}

			//It must remain viable through daeNoteChange().
			daeElementRef RAII = it->_child.ref;

			#ifdef NDEBUG
			#error Need a mechanism for suppressing notifications.
			#endif
			bool notify2 = true;
			daeContents::_vacate_operation op(*it);
			//SOMEHOW DO THIS ALL AT ONCE			
			if(_typeLookup_unless(it->_child.ref))
			op.typeLookup = true;
			if(notify2) daeNoteChange(op);
		}

		if(child==nullptr) //Placeholder?
		{
			//There's nothing more to do.
		}
		else 
		{
			const daeName &rename = 
			//HACK: child->name=child->name is easier than maintaining two paths.			
			up.name==1?child->getNCName():up.meta.jumpIntoTOC(up.name).getName();

			if(child->isContent())
			{
				//THIS IS THE EXACT SAME LOGIC AS ABOVE.
				assert(nullptr!=child->getParentElement());
				daeElement &parent2 = (daeElement&)child->getParentObject();		
				daeContents &rc = parent2.getContents();
				daeContent &rm = source.content(rc);
				daeCM_Demotion<> d(parent2.getMeta(),rc,rm);
				daeContents::_populate_move_rename_operation op(d,*it,up.parent,rename);
				//SOMEHOW DO THIS ALL AT ONCE				
				if(_typeLookup_unless(child))
				op.typeLookup = parent2.getDocument()!=up.parent.getDocument();
				if(notify) daeNoteChange(op); 
			}
			else 
			{
				daeContents::_populate_rename_operation op(*it,up.parent,*child,rename);
				//SOMEHOW DO THIS ALL AT ONCE
				if(_typeLookup_unless(child)) op.typeLookup = true;
				if(notify) daeNoteChange(op);
			}
		}
	}
}
const daeChildRef<> &daeMeta::_setWID2(daeChildID ID, daeElement *child, daeContents &c, daeContent *it)
{
	daeMetaElement_setWID2_a<daeElement*> //C++98/03 (C2918) 
	adapter = { child };
	_setWID3(ID,adapter,c,it); return it->_child_ref();
}
const daeChildRef<> &daeMeta::_setWID2(daeChildID ID, daeContent &child, daeContents &c, daeContent *it)
{
	daeMetaElement_setWID2_a<daeContent&> //C++98/03 (C2918)
	adapter = { child };
	_setWID3(ID,adapter,c,it); return it->_child_ref();
}
void daeMeta::_removeWID(daeContents &c, daeContent *it)
{
	if(nullptr==it) return;

	daeCM_Demotion<> d(c.getElement().getMeta(),c,*it);

	if(nullptr==it->_child.ref)
	{	
		d.count = c.__decrement_internal_counter(d.name).getIndex();
		c._remove_child(it);
		d.maybe_demote();
	}
	else
	{
		//It must remain viable through daeNoteChange().
		daeElementRef RAII = it->_child.ref;

		#ifdef NDEBUG
		#error Need a mechanism for suppressing notifications.
		#endif
		bool notify = true;

		daeContents::_remove_operation op(d); 
		//SOMEHOW DO THIS ALL AT ONCE
		if(_typeLookup_unless(it->_child.ref))
		op.typeLookup = true;
		if(notify) daeNoteChange(op); 
	}	
}
void daeMeta::_resizeWID2(daeContents &c, daeChildID::POD ID, daeChildID::POD ID_size)
{
	//_setWID2 (in a loop) especially is not the most brilliant 
	//implementation, but it can suffice for the time being. If
	//the compiler optimizes aggressively, then it's not so bad
	//as long as the "loops" aren't great or it isn't a case of
	//an element with maxOccurs describing array-like semantics.
	while(ID<ID_size) _removeWID(c,c._get<int(0)>(ID++));
	while(ID_size<ID) _setWID2(ID_size++,nullptr,c,nullptr);
}

//---.
}//<-'

/*C1071*/