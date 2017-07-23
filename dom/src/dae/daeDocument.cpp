/*
 * Copyright 2006 Sony Computer Entertainment Inc.
 *
 * Licensed under the MIT Open Source License, for details please see license.txt or the website
 * http://www.opensource.org/licenses/mit-license.php
 *
 */
#include <ColladaDOM.inl> //PCH

#include "../../include/dae/daeRAII.hpp"

COLLADA_(namespace)
{//-.
//<-'

daeOK daeDocument::setMeta(daeMeta &rm)
{
	if(getContents().empty()) 
	{			
		#ifdef NDEBUG
		#error There ought to be a lock on the process-share
		#endif		
		XS::Element &cm = 
		const_cast<XS::Element&>(_pseudo->jumpIntoTOC(1));
		assert(&cm==&_pseudo->getCMEntree());
		cm._element.name = rm.getName();
		cm._element.child = &rm; 
	}
	else return DAE_ERR_INVALID_CALL; return DAE_OK;
}
namespace DAEP //GCC refuses to disable this (erroneous) warning
{
	template<> inline DAEP::Model &DAEP::Elemental
	<daeDocument::_PseudoElement>::__DAEP__Object__v1__model()const
	{
		return dae(this)->getMeta()->getModel();
	}
}
extern const daeStringCP _daePseudoElement_[] = ":daePseudoElement:";
daeDocument::daeDocument(daeDOM *DOM):daeDoc(DOM,daeDocType::HARDLINK)
{	
	memset(&_pseudo,0x00,sizeof(_pseudo));
	daeModel &model = (daeModel&)_pseudo.model;
	daeMetaElement &meta = (daeMetaElement&)_pseudo.meta;
	
	//Do like XS::Scema::addElement().
	meta._name.string = _daePseudoElement_;
	(daeMeta*&)model.__DAEP__Model__meta = &meta;
	meta._processShare = domAny::_master.meta._processShare;	
	meta._sizeof = sizeof(_PseudoElement);
	meta._domAny_safe_model = &model;	
	meta._domAny_safe_prototype = meta._prototype;
	meta._finalFeatureID = daeFeatureID(-2);
	meta._DAEP_Schema = 0; //NOT SURE???
	meta._bring_to_life(meta._attribs);
	_PseudoElement::Essentials ee;
	model.setObjectType(1); //HACK: get around assert.
	meta._continue_XS_Schema_addElement<_PseudoElement>(ee);
	model.setObjectType(0); //HACK: got around assert.
	//Prevent construction of this element, both because it's unique
	//and users would have to be tampering for shits and giggles to
	//ever do it, and because the pseudo-element IS the prototype.
	//And so it couldn't be copied. The constructor is needed to
	//destruct a metadata record, but this one isn't destructed.
	meta._constructor = nullptr;	

	daePseudoElement &e = _getPseudoElement();	
	e.__DAEP__Object__parent = this; //HACK
	//HACK: it's too late to call __DAEP__Object__embed_subobject().
	e.__DAEP__Object__refs = _uri.__DAEP__Object__refs;
	(daeDOM*&)e.__DAEP__Element__data.DOM = DOM;

	daeTOC &TOC = _pseudo->getTOC();
	for(size_t i=0;i<TOC.size();i++)
	{
		//Just for the hell of it, mark root node as required.
		const_cast<XS::Element&>(TOC[i])._minOccurs = 1;
	}

	assert(!e._isElement());
	assert(e.getNCName().string==_daePseudoElement_);
	e.__DAEP__Element__data.is_document_daePseudoElement = 1; 	
	//This is to be sure !_isAny() and that it's not a system object.
	assert(0!=e._getPShare()&&!e._isAny());

	#ifdef _DEBUG
	//This is the document's top-level content for NatvisFile.natvis.
	__Natvis_content = ((_PseudoElement*)&e)->content.operator->();
	#endif
}
daeDocument::~daeDocument() //VERY SKETCHY
{
	//Trying to avoid _constructor, but let go of the CM graph.
	_pseudo->~daeMetaElement(); deAlloc(getContents().getAU());
}

daeOK daeDoc::close()
{
	//Indicates daePlatform::closeURI() closed it itself.
	bool closed = false;
	daeOK OK = _doOperation<_close_op>(getDOM(),&closed);
	if(!closed&&OK) _archive->_closedDoc(this);
	return OK;
}
void daeDocument::__daeDoc__v1__atomize()
{					
	//Clearing these ahead avoids individual erasure of their entries.
	_typeMap.clear(); _idMap.clear(); _sidMap.clear();
	//This should do it for the root, etc.
	getContents().clear();
}
daeOK daeDoc::_doOperation(int op, const const_daeDOMRef &DOM, const void *arg0)
{
	daePlatform &pf = DOM->getPlatform();

	daeOK OK;
	int id = pf.threadID();
	if(_current_operation==_no_op::value)
	{	
		_current_operation = op; _operation_thread_id = id;

		switch(op)
		{
		case _close_op::value: //Called by daeDoc::close().
		{
			//closeURI() isn't given empty URIs. They will be closed
			//unconditionally. Users can clear the URI to force this.
			if(_uri.empty()) break;			
			//Just-in-case, don't bother closeURI() with closed docs.
			if(isClosed()){ assert(0); break; }

			daeURIRef URI = (daeURI*)_uri;				
			//DAE_OK is ambiguous. (_doOperation also returns DAE_OK.)
			*(_close_op::type*)arg0 = OK = pf.closeURI(*URI);
			assert(!OK||isClosed());
			//Indicates the daePlatform does not implement closeURI(). 
			if(OK==DAE_ERR_NOT_IMPLEMENTED) 
			OK = DAE_OK;
			break;
		}
		case _setURI_op::value: //Called by daeURI_base::_setURI().
		{	
			if(_uri==&DOM->getEmptyURI()) 
			{
				//assert in case it SHOULD be non-empty.
				assert(0); return DAE_ERR_INVALID_CALL;
			}

			//setURI() is recursive.
			OK = _uri.setURI((_setURI_op::type*)arg0);
			if(OK)
			{	
				//resolve() is recursive.
				_uri.resolve(*DOM); 

				//Signals to _setURI to finish up on its end.
				OK = DAE_ERR_NOT_IMPLEMENTED;
			}
			else assert(OK!=DAE_ERR_NOT_IMPLEMENTED);
			break;
		}}

		_current_operation = _no_op::value;
	}
	else if(id!=_operation_thread_id)
	{
		//Multi-thread applications may take advantage of this--although
		//notice that this API is not doing any special synchronizations.
		return DAE_NOT_NOW;
	}
	else //Overlapping operations will likely produce unexpected results.
	{
		//Presently this might mean, an attempt to set the URI mid-close.
		assert(op==_current_operation);
	}
	return OK;
}

daeError daeDoc::_write(const daeURI &URI, daeIOPlugin *O)const
{
	const_daeDocumentRef document = getDocument();
	if(nullptr==document)
	{
		const daeDOM *DOM = this->a<daeDOM>();

		if(nullptr==DOM)
		{
			assert(0); return DAE_ERR_FATAL;
		}

		//Note: it should be illegal to symlink a DOM.
		return DOM->_write_this_DOM(URI,O);
	}
	//HACK: _uri is used instead of document->_uri to hint to
	//the plugin if it's a mass/archive write or single write.
	daeIORequest reqO(document->_archive,nullptr,_uri,URI);
	return document->_write(reqO,O);
}
daeError daeDocument::_write(const daeIORequest &reqO, daeIOPlugin *O)const
{
	//Kind of schizophrenic isn't it?
	if(_archive!=reqO.scope||reqO.localURI->getDoc()->_getDocument()!=this) 
	{
		assert(0); return DAE_ERR_INVALID_CALL;
	}	
	daeDOMRef DOM = const_cast<daeDOM*>(getDOM());	
	if(reqO.remoteURI!=nullptr)
	{
		if(reqO.remoteURI!=reqO.localURI)
		reqO.remoteURI->resolve(DOM);
	}
	else if(O==nullptr)	
	return DAE_ERR_INVALID_CALL;
	daeRAII::UnplugIO RAII(DOM->getPlatform());
	if(O==nullptr) RAII = O = RAII.p.pluginIO(*reqO.remoteURI,'w'); 
	daeRAII::InstantLegacyIO legacyIO; //emits error
	if(O==nullptr) O = &legacyIO(RAII.p.getLegacyProfile(),*reqO.localURI);						
	daeRAII::Reset<const daeIORequest*> _O___request(O->_request);
	O->_request = &reqO;	
	daeIOEmpty I;
	daeRAII::CloseIO RAII2(RAII.p); //emits error	
	daeIO *IO = RAII2 = RAII.p.openIO(I,*O);
	if(IO==nullptr)
	return DAE_ERR_CLIENT_FATAL;
	return O->writeContent(*IO,getContents());	
}
  
//// sid/idLookup //// sid/idLookup //// sid/idLookup //// sid/idLookup //// sid/idLookup 

static daeString daeDocument_ID_id(daeString ref)
{
	return (daeString)(((daeOffset&)ref)&~daeOffset(1));
}
void daeDocument::_idLookup(daeString ref, daeElementRef &match)const
{
	ref = daeDocument_ID_id(ref); //Permit # string-refs for <source> and just convenience.

	_idMapIter cit = _idMap.find(ref); if(cit!=_idMap.end()) match = cit->second;
} 
void daeDocument::_sidLookup(daeString ref, daeArray<daeElementRef> &matchingElements)const
{
	//REMINDER: daeElement::_sidLookup() accesses these directly.
	_sidMapRange range = _sidMap.equal_range(ref);
	for(_sidMapIter cit=range.first;cit!=range.second;cit++)	
	matchingElements.push_back(cit->second);
}
void daeDocument::_carry_out_change_of_ID_or_SID(const DAEP::Change &c, const XS::Attribute *a)const
{	
	//Should this be allowed? For symmetry?
	assert(this!=nullptr); //if(this==nullptr) return;

	const daeElement *e = dae(c.element_of_change);

	//REMINDER: THIS IS NOT DESIGNED TO HANDLE ELEMENT MOVEMENT.
	switch(a->getType()->getAtomicType())
	{
	case daeAtomicType::TOKEN: break;
	case daeAtomicType::STRING: assert(e->_isAny()); break;
	default: assert(0); return;
	}
	bool id = a->getName().string[0]=='i';
	assert(daeName(id?"id":"sid")==a->getName());

	daeString got = (const daeString&)a->getWRT(e); if(got[0]!='\0')
	{
		if(!id) //sid?
		{
			_sidMapRange range = _sidMap.equal_range(got);
			for(_sidMapIter cit=range.first;cit!=range.second;cit++)	
			{
				if(cit->second==e)
				{
					_sidMap.erase(cit); break;
				}
			}		
		}
		else _idMap.erase(daeDocument_ID_id(got)); 
	}
	c.carry_out_change(); got = (const daeString&)a->getWRT(e); if(got[0]!='\0') 
	{
		if(id)
		{
			daeElement* &uniqueID = _idMap[daeDocument_ID_id(got)];
			#ifdef NDEBUG
			#error This is probably worth outputting something to the error logs.
			#error What if this is a proposed "orphans" document?
			#endif
			if(nullptr!=uniqueID)			
			daeEH::Error<<"Non-unique id=\""<<got<<"\" added to daeDocument map.";
			uniqueID = const_cast<daeElement*>(e);
		}
		else _sidMap.insert(std::make_pair(got,const_cast<daeElement*>(e)));
	}
}
void daeDocument::_migrate_ID_or_SID(const daeDocument *destination, const daeElement *e, const XS::Attribute *a)const
{
	//"this" is the source, just to make things simpler.
	const daeDocument *source = this;
	assert(source!=destination); //Callers should avoid this.

	switch(a->getType()->getAtomicType())
	{
	case daeAtomicType::TOKEN: break;
	case daeAtomicType::STRING: assert(e->_isAny()); break;
	default: assert(0); return;
	}
	daeString got = (const daeString&)a->getWRT(e);
	if(got[0]=='\0') return; 

	bool id = a->getName().string[0]=='i';
	assert(daeName(id?"id":"sid")==a->getName());		

	//These are both similar to _carry_out_change_of_ID_or_SID().
	if(nullptr!=source) 
	{
		if(!id) //sid?
		{
			_sidMapRange range = source->_sidMap.equal_range(got);
			for(_sidMapIter cit=range.first;cit!=range.second;cit++)	
			{
				if(cit->second==e)
				{
					_sidMap.erase(cit); break;
				}
			}		
		}
		else source->_idMap.erase(daeDocument_ID_id(got)); 
	}
	if(nullptr!=destination) 
	{
		if(id)
		{
			daeElement* &uniqueID = destination->_idMap[daeDocument_ID_id(got)];
			#ifdef NDEBUG
			#error This is probably worth outputting something to the error logs.
			#error What if this is a proposed "orphans" document?
			#endif
			if(nullptr!=uniqueID)			
			daeEH::Error<<"Non-unique id=\""<<got<<"\" added to daeDocument map.";
			uniqueID = const_cast<daeElement*>(e);
		}
		else destination->_sidMap.insert(std::make_pair(got,const_cast<daeElement*>(e)));
	}	
}

//// typeLookup //// typeLookup //// typeLookup //// typeLookup //// typeLookup //// typeLookup 

	//Too much goes into fulfilling this legacy feature.

//This is used to avoid graph-traversals 
//if typeLookup has never once been called. 
COLLADA_(extern) bool daeDocument_typeLookup_called = false;
void daeDocument::_typeLookup(daeMeta &meta, daeArray<daeElementRef> &matchingElements)const
{
	//USERS MAY WANT TO DISABLE TYPELOOKUP BY MACRO.
	daeDocument_typeLookup_called = true;

	//The inline APIs are doing this.
	assert(matchingElements.empty());	
	std::pair<_typeMapIter,bool> ins = 
	_typeMap.insert(std::make_pair((daeString)&meta,_typeVec()));
	_typeVec &vec = ins.first->second; if(ins.second)
	{
		meta._daeDocument_typeLookup_enabled = true;
		_getElementsByType(meta,vec);
	}//vec.data() is C++11.
	matchingElements.append(vec.empty()?nullptr:&vec[0],vec.size()); 
}		  
template<int op> 
void daeDocument::_typeLookup_migrate_operation<op>::operator()(const daeElement *ch)
{
	daeMeta &meta = ch->getMeta();
				   	
	_typeMapIter cit = doc->_typeMap.find((daeString)&meta); 		
		
	if(cit!=doc->_typeMap.end()) 
	{
		std::vector<daeElement*> &v = cit->second;
		//ALGORITHM
		//These lists aren't expected to be large, and this should only
		//be entered if the document is not being closed, yet still the
		//element is being removed (i.e. an editing/scripting context.)
		//It can't do much better, except to try to erase them en masse.
		if(op==1) v.erase(std::find(v.begin(),v.end(),ch));	
		if(op==0) v.push_back(const_cast<daeElement*>(ch));	
	}
	if(!ch->getOptimizationBit_is_definitely_not_a_graph()) //Recurse?
	{
		meta.getContentsWRT(ch).for_each_child(*this);
	}
}//CIRCULAR-DEPENDENCY
void daeContents_base::_operation::typeLookup_insert(daeElement *e)
{
	daeDocument::_typeLookup_migrate_operation<0> op = { e->getDocument() };
	if(!op.doc->_typeMap.empty()) op(e);
}//CIRCULAR-DEPENDENCY
void daeContents_base::_operation::typeLookup_remove(daeElement *e)
{
	daeDocument::_typeLookup_migrate_operation<1> op = { e->getDocument() };
	if(!op.doc->_typeMap.empty()) op(e);
}
daeDocument::_typeVec *daeDocument::_typeLookup_vector(daeMeta &m)const
{
	if(!_typeMap.empty())
	{	
		_typeMapIter cit = _typeMap.find((daeString)&m); 		
		if(cit!=_typeMap.end()) 
		return &cit->second;
	}
	return nullptr;
}
#ifdef NDEBUG
#error Implement _typeLookup_bulk_remove()?
#endif
void daeDocument::_typeLookup_self_remove(daeMeta &meta, daeElement &e)const
{
	std::vector<daeElement*> *v = _typeLookup_vector(meta);
	if(v!=nullptr) v->erase(std::find(v->begin(),v->end(),&e));	
}
	
inline daeCursor daeContents_base::__insert(int KoT, daeString s, size_t e, const void *before)
{
	#if _DEBUG
	for(size_t i=0;i<e;i++) assert(s[i]>='\t');
	#endif

	daeContents &c = _this();

	const int text_off = daeOffsetOf(daeText,_text);
	const int span_max = sizeof(daeContent)*255-text_off-1;

	//This just seems more readible with a loop.
	int nodes,i;
	for(nodes=0,i=(int)e;i>span_max;i-=span_max)
	nodes+=255;
	nodes+=(text_off+i+1)/sizeof(*_);
	nodes+=(text_off+i+1)%sizeof(*_)!=0?1:0;
	__uncursorize(before);
	cursor() = __uncursorize(before);
	c.grow(c.size()+nodes); //ROOM FOR OPTIMIZATION
	{
		typedef void before;

		union{ daeContent *p; daeText *t; }; p = _;

		//This is not the best, but it's consistent.
		while(p->hasText())
		{
			t = p->getText(); if(!t->isMoreText()) break;
		}

		memmove(p+nodes,p,(c.end()-p)*sizeof(*p));
		c.getAU()->setInternalCounter(c.size()+nodes);
		for(i=(int)e;i>0;i-=span_max)
		{	
			int extent = std::min(i,span_max);
			int span = i>=span_max?255:nodes%255;
			assert(span!=0);
			daeEOText *eot = (daeEOText*)(p+span-1);
			memset(eot,'\n',sizeof(*p));
			eot->_textview[eot->_cspan] = span;	
			t->_.hole = KoT;
			t->_.extent = extent;
			t->_.span = span;
			t->_.continued = i!=(int)e;
			t->_.continues = i>span_max;
			t->_.reserved = 0;
			memcpy(t->_text,s,extent*sizeof(daeStringCP)); s+=extent; p+=span;
			assert(span==eot->_textview[eot->_cspan]);	
		}

		//Note: _/cursor() isn't permitted to remain on text. 
		//It's more safe to do this than to require the user to clear it after.
		_ = p; while(_->hasText()) _+=_->getKnownText().span(); return p-nodes;
	}
}
daeCursor daeContents_base::_comment(daeString s, size_t e, const void *_)
{
	if(s[0]=='<'&&e>=7&&s[1]=='!'&&s[2]=='-'&&s[3]=='-')
	{
		s+=4; e-=3; assert(s[e]=='-'&&s[e+1]=='-'&&s[e+1]=='>');
	}
	return __insert(daeKindOfText::COMMENT,s,e,_); 
}
daeCursor daeContents_base::_mix(daeString s, size_t e, const void *_)
{
	return __insert(daeKindOfText::MIXED,s,e,_);
}
daeCursor daeContents_base::_instruct(daeString s, size_t e, const void *_)
{
	if(s[0]=='<'&&e>=4&&s[1]=='?')
	{
		s+=2; e-=4; assert(s[e]=='?'&&s[e+1]=='>');
	}
	return __insert(daeKindOfText::PI_LIKE,s,e,_);
}

//---.
}//<-'

/*C1071*/
