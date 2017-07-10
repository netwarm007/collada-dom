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

//These are currently at the bottom
//of daeStringRef.cpp to ensure that
//daeStringRef_anySimpleType is handy.
//static XS::Schema domAny_xs("","");
//domAny::_Master domAny::_master(domAny_xs);
domAny::_Master::_Master(XS::Schema &xs)
:meta(xs.addElement(toc,"")),model(meta.getModel())
{
	//CHICKEN/EGG
	const_cast<daeMetaElement&>(meta)
	._jumpIntoTOC(1)._element.child = &meta;

	xs.addType<xs::anySimpleType>("xs:anySimpleType");

	daeMetaElement &el = const_cast<daeMetaElement&>(meta);

	el.addValue(toc->value,"xs:anySimpleType");

	#ifdef NDEBUG
	#error This can be removed. What is a domAny?
	#error Using a nonzero ordinal may be unwise.
	#endif
	daeCM *cm = nullptr;
	el.addCM<XS::Sequence>(cm,0,1,1);
	el.addCM<XS::Any>(cm,0,0,-1).setProcessContents("lax");
	el.addContentModel<1,1>(cm,toc);

	//This may be 2 since changing from 0 to 1 being unnamed.
	//Work on _continue_XS_Schema_addElement2 until it's not.
	assert(1==el.getTOC().size());
	
	domAny &pt = daeOpaque(el._prototype);
	
	//HACK: This assigns domAny to a special 0 process-share.
	(&pt._getClassTag())[3] = 1;
	//HACK: _isAny() now needs this to tell it's not Doc/DOM.
	(&pt._getClassTag())[1] = 0x80;
	assert(0==pt._getPShare()&&pt._isData()&&!daeUnsafe<domAny>(pt));

	memcpy(&pt._meta,&el,sizeof(el));
}

domAny::~domAny()
{
	//Bypass daeMetaElement::_self_destruct().
	//_meta is embedded and simplified compared to 
	//a shared metadata object.	
	_meta->_attribs.~daeArray(); 
	//delete/destruct the attribute-buckets.
	_attribs._self_destruct((int)_meta->_attribs.size());
}
void domAny::AttributeBucket::_self_destruct(int i)
{
	if(i>size) 
	{
		i = size; next->_self_destruct(i-size); delete next;
	}
	while(i-->0) ((daeStringRef&)refs[i]).~daeStringRef();
}

bool daeElement::_addAnyAttribute(const daePseudonym &name)const
{	
	#ifdef NDEBUG
	#error Add daeElement::_isAny_domAny() via getElementTags().
	#endif
	if(getElementType()!=domAny::elementType) 
	{		
		//This will eventually have to be implemented in this way.
		assert(!getMeta()->getAllowsAnyAttribute()); return false;
	}
	_cloneAnyAttribute(daeStringRef(name)); return true;
}
void daeElement::_cloneAnyAttribute(const daePseudonym &name)const
{
	domAny &_this = (domAny&)*this;
	typedef domAny::AttributeBucket bucket;
	size_t i = _this._meta->_attribs.size();
	//This sets up getFirstID() etc.
	XS::Attribute &back = 
	_this._meta->_anyAttribute_maybe_addID(this);
	back._attribute_name = name;
	bucket *b = &_this._attribs;
	while(i>bucket::size)
	{
		if(b->next==nullptr) b->next = new bucket;
		b = b->next; i-=bucket::size;
	}	
	back._offset = &daeOpaque(b->refs+i)-(char*)this;
	b->refs[i] = getDOM()->getEmptyURI().data();	
}

//---.
}//<-'

/*C1071*/