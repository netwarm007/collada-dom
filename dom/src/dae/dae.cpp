/*
 * Copyright 2006 Sony Computer Entertainment Inc.
 *
 * Licensed under the MIT Open Source License, for details please see license.txt or the website
 * http://www.opensource.org/licenses/mit-license.php
 *
 */
#include <ColladaDOM.inl> //PCH

#include "../../include/dae/daeRAII.hpp"
#include "../../include/modules/daeSTLDatabase.h"
 
COLLADA_(namespace)
{//-.
//<-'

static size_t daeDOM_outstanding = 0;
static struct daeDOM_outstanding_check
{
	//daeDOM have unique reference counting requirements, because
	//documents and other objects must refer to a DOM to locate a
	//database. And so a daeDOM must be released after any others.
	~daeDOM_outstanding_check(){ assert(0==daeDOM_outstanding); }

}daeDOM_outstanding_check;

static void dae_disable_deleter(const DAEP::Object *obj)
{
	//This identical to delete DOM; But just to be sure.
	DAEP::Make make; make.__DAEP__Make__v1__delete(obj);	
}
void daeArchive::_setDeleter(daeArchive::_deleter_f f)
{
	if(_deleter!=nullptr)
	{
		assert(_deleter==nullptr); //TODO: Emit a warning.
	}
	else if(f==nullptr)
	{
		_deleter = dae_disable_deleter;
	}
	else _deleter = f;
}

void DAEP::Object::__DAEP__Object__0()const
{	
	if(__DAEP__Object__refs<0) //sub-object?
	{
		const daeObject &op = *dae(__DAEP__Object__parent);
		assert(this!=op&&op[this].isEmbeddedObjectWRT(&op));
		if(__DAEP__Object__refs<=-__DAEP__Object__refs_embedding_threshold)
		op.__DAEP__Object__0();				
		return; //UNCONDITIONAL
	}

	#ifdef NDEBUG //GCC and apostrophes.
	#error TODO? Elements are the bulk of objects. If they have
	#error their own __DAEP__Object__0 they could use getMeta().
	#error "This one should still handle them, in case it's used."
	#endif
	const daeModel &model = __DAEP__Object__v1__model();

	int type = model.getObjectType();	

	if(type>=daeObjectType::ANY) //Element?
	{
		daeElement &el = (daeElement&)*this;
		el.__atomize2(el.getMeta());		
		return el.getDatabase()._delete(el); 	
	}
	else if(model->hasObjectsEmbedded())
	{
		//Return if the object class/complex has outstanding refs.
		if(0<__DAEP__Object__refs||model.hasEmbeddedRefsWRT(this))
		return;
	}
	else assert(0==__DAEP__Object__refs);
						   
	daeObject &_this = dae(*this);	
	daeDatabase *db = _this._getDBase();
	if(db==nullptr) switch(type) //Not data?	
	{
	case daeObjectType::DOM: 
			
		assert(_this._isDOM());	
		assert(dynamic_cast<const daeDOM*>(this)!=nullptr);
		//break;

	case daeObjectType::DOC: //FALLING-THROUGH
		
		if(daeDocType::ARCHIVE==((daeDoc*)this)->getDocType())
		{
			assert(dynamic_cast<const daeArchive*>(this)!=nullptr);				
			daeArchive::_deleter_f f(((daeArchive*)this)->_deleter);
			if(f!=nullptr)
			return f(this); assert(0==_this._getPShare());
		}

	default: return model.deleteObject(this);
	}
	else if(type==daeObjectType::DOC)
	{
		//Assuming other docs are "not data."
		assert(dynamic_cast<const daeDocument*>(this)!=nullptr);
		return db->_delete((daeDocument&)*this);		
	}
	return db->_delete(_this); //generic object
}

daeOK daeObject::_reparent(const daeObject &c)	
{	
	#ifdef NDEBUG
	if(&c==nullptr) return DAE_ERR_INVALID_CALL; //paranoia
	#endif
	assert(&c!=nullptr);
	if(_isData())
	{
		const daeDOM *DOM = c.getDOM(); assert(DOM!=nullptr);
		if(&c==this||DOM!=getDOM()) return DAE_ERR_INVALID_CALL;
	}	
	if(&c!=this) c._ref();
	const daeObject *swap = dae(__DAEP__Object__parent);
	__DAEP__Object__parent = &c;
	if(swap!=this) swap->_release(); return DAE_OK;
}
extern const daeStringCP _daePseudoElement_[];
daeOK daeObject::_reparent_element_to_element(const daeObject &cIn)	
{	
	assert(this->_isElement());
	daeElement *c = (daeElement*)&cIn;
	assert(c!=nullptr&&c->_isData());
	{
		//daeElement::getDOM() looks in __DAEP__Element_data.
		const daeDOM *DOM = c->getDOM(); assert(DOM!=nullptr);
		if(c==this||DOM!=getDOM()) return DAE_ERR_INVALID_CALL;

		//Using the string address here because it's much more 
		//bits, and is unlikely to be accidentally set somehow.
		//The further check is because the name can be set via
		//getNCName(); in which case, the user shoots themself.
		unsigned pseudo = _daePseudoElement_==c->getNCName().string;
		assert(pseudo==c->__DAEP__Element__data.is_document_daePseudoElement);
		if(1==pseudo)
		{			
			c = (daeElement*)&c->getParentObject();
			assert((void*)c==c->getDoc()&&(void*)c==((daeDoc*)c)->_getDocument());
		}
		else assert(c->_isElement());
	}	
	/*if(&c!=this)*/ c->_ref();
	const daeObject *swap = dae(__DAEP__Object__parent);
	__DAEP__Object__parent = (daeObject*)c;
	/*if(swap!=this)*/ swap->_release(); return DAE_OK;
}

//DOM_process_share is initialized by daeStringRef.cpp.
//The 0 share is special. The 1 share is assumed to be
//the same, and overwritten in case it's 0 initialized.
static struct
{
	daePShare *PS; daeProcessShare *library_side_share;

}dae_PShares[128] = {{&DOM_process_share},{&DOM_process_share}};
daeOK daePShare::grant(size_t size)
{
	//TODO? Keep a record of the heighest granted share number?
	enum{ extent = sizeof(dae_PShares)/sizeof(dae_PShares[0]) };	

	if(size!=sizeof(daePShare))
	{
		//The tag includes the data-bit.
		size_t i = tag>>1; assert(tag%2==0);
		if(0==size&&i<extent&&dae_PShares[i].PS==this)
		{
			if(tag>1) //This is the removal procedure.
			{
				delete dae_PShares[i].library_side_share;
				dae_PShares[i].library_side_share = nullptr;
				dae_PShares[i].PS = nullptr;				
			}
			tag = 0; return DAE_OK;
		}			
		assert(size==sizeof(daePShare)); 
		return DAE_ERR_NOT_IMPLEMENTED;
	}
	
	size_t i; 
	//i begins at 1 in order to reserve 0 for internal classes.
	for(i=1;i<extent;i++) if(this==dae_PShares[i].PS)
	{
		goto OK;
	}
	for(i=1;i<extent;i++) if(nullptr==dae_PShares[i].PS)
	{
		dae_PShares[i].PS = this; goto OK;
	}
	//Must wait for an opening?
	//TODO: Check to see if any of the modules have been unloaded?
	//(NOTE: it could be hard to say if so/not in definite terms.)
	return DAE_ERROR;		
	//<<1 is making way for the _isData() bit/flag.
OK:	tag = 0xFF&i<<1; daeCTC<(extent-1<=0xFF>>1)>();
	return DAE_OK; 
}
daeProcessShare &daeProcessShare::_default(size_t ps,
daeProcessShare *_default_constructor(size_t,daePShare*))
{
	if(ps<sizeof(dae_PShares)/sizeof(dae_PShares[0]))
	{
		daeProcessShare* &out = dae_PShares[ps].library_side_share;
		if(out==nullptr) 
		out = _default_constructor(ps,dae_PShares[ps].PS); return *out;
	}
	assert(0); return *(daeProcessShare*)nullptr; //Brace for crash?
}

daePlatform *daeDOM::_globalPlatform = nullptr;
daePlatform &daeArchive::_getPlatform(const daeDOM &DOM)
{
	return *DOM._platform; //This is here because of a cicular-dependency.
}

extern daeAtlas &daeStringRef_dummyAtlas;
static struct dae_dummyPlatform : daePlatform
{
virtual daeOK resolveURI(daeURI&, const daeDOM&){ return DAE_ERR_CLIENT_FATAL; }
virtual daeOK openURI(const daeIORequest&,daeIOPlugin*,daeURIRef&){ return DAE_ERR_CLIENT_FATAL; }
virtual daeOK closeURI(const daeURI&){ return DAE_ERR_CLIENT_FATAL; }
virtual daeIO *openIO(daeIOPlugin&, daeIOPlugin&){ return nullptr; }
virtual void closeIO(daeIO*){}
virtual int getLegacyProfile(){ return 0; }
}dae_dummyPlatform;

//ORDER-OF-INITIALIZATION MATTERS 
daeDoc::daeDoc(daeDOM *DOM, int dt):
daeObject(DOM),_link()
,_archive(DOM->_closedDocs),_current_operation(_no_op::value)
,_operation_thread_id() //STFU
{ 	
	_uri.setIsAttached();
	_uri.setIsResolved();
	_uri.__DAEP__Object__embed_subobject(this);
	//daeURI is not a system object.
	//This is to ensure that only system objects are 0.
	assert(0!=_uri._getPShare());

	_getClassTag() = dt; 	
	(&_getClassTag())[1] = 1;
	//HACK: Assign daeDoc to the special 0 process-share.
	(&_getClassTag())[3] = 0; assert(_isDoc()); 
	//_archive(DOM->_closedDocs) assumes it is a pointer.
	if(this==DOM) _archive = DOM;
	if(_archive!=this) //Eg. DOM or daeDOM::_closedDocs
	{
		//Archive APIs assume the doc is in an archive.
		_archive->_whatsupDoc = _archive->_docs.size();
		_archive->_docs.push_back(this);	
	}	
}
daeArchive::daeArchive(daeDOM &DOM) 
:daeDoc(DOM,daeDocType::ARCHIVE),_deleter()
,_whatsupDoc() //STFU
{
	_uri.setIsDirectoryLike();

	//MUST ASSUME DOM IS UNINITIALIZED HERE//
}
daeDOM::daeDOM(daeDatabase_base *DB, daePlatform *OS)
COLLADA_SUPPRESS_C(4355)
:daeArchive(*this),_closedDocs(*this)
{
	_atlas = &daeStringRef_dummyAtlas;

	_closedDocs.__DAEP__Object__embed_subobject(this);
	_refResolvers.__DAEP__Object__embed_subobject(this);

	//Indicates a DOM. Overrides _isDoc().
	(&_getClassTag())[1] = 2; assert(_isDOM()); 
	//daeDoc::daeDoc is responsible for these arrangements
	assert(_archive==this&&_closedDocs._archive==_closedDocs);

  ////WARNING: NOT EVERYTHING IS INITIALIZED AT THIS POINT////

	if(OS==nullptr) OS = _globalPlatform;
	if(OS==nullptr)
	{
		OS = &dae_dummyPlatform;
		//If a daeDOM is a global static object not initialized at startup it's
		//likely to trigger this as the first console output.
		//There's not an official way to change it but a savvy programmer might
		//manually call the object's destructor before its constructor to do so.
		//daeEH::Error<<"No daePlatform provided. Using dummy to avoid a crash.";
	}
	_platform = OS;
	
	_detachDatabase = nullptr==DB;
	if(_detachDatabase)
	_detachDatabase = nullptr!=(DB=OS->attachDB());
	_deleteDatabase = nullptr==DB;
	_database = _deleteDatabase?new daeDB<daeLegacyDBase>:DB;	
	//
	// Say hello to the database, and set its empty string-ref
	// to be the empty URI's string, so daeStringRef can find it
	// without resorting to cold calling an it-can-do-anything API.
	//
	_databaseRejoinder = nullptr;
	_database->_ciao(*this,_databaseRejoinder);	
	_uri._refString.setView(_database->_ref("",0,_databaseRejoinder));
	assert(_uri==&getEmptyURI()&&_uri.empty());

	int legacy = OS->getLegacyProfile();

	//BE CAREFUL: The order of the URI resolvers is significant.
	//HACK: __DAEP__Object__unembed() shouldn't be used this way.
	if(legacy&OS->LEGACY_RAW_RESOLVER)
	_refResolvers.list_append(new daeRawResolver)->__DAEP__Object__unembed();
	if(legacy&OS->LEGACY_URI_RESOLVER)
	_refResolvers.list_append(new daeDefaultURIResolver)->__DAEP__Object__unembed();
	if(legacy&OS->LEGACY_IDREF_RESOLVER)
	_refResolvers.list_append(new daeDefaultIDREFResolver)->__DAEP__Object__unembed();
	if(legacy&OS->LEGACY_SIDREF_RESOLVER)
	_refResolvers.list_append(new daeDefaultSIDREFResolver)->__DAEP__Object__unembed();
	
	//Lock-in the "current-working-directory."
	daeURI *defBaseURI = &getDefaultBaseURI();
	const daeURI *absURI = &OS->getDefaultBaseURI(*defBaseURI);
	if(absURI!=defBaseURI) defBaseURI->setURI(absURI->data());
	defBaseURI->resolve(*this);

	//*3/2 is to give US a heads up
	daeCTC<__size_on_client_stack*1/2>=sizeof(*this)>();

	daeDOM_outstanding++; _destructed = false;
}
daeDOM::~daeDOM() 
{  
	if(_destructed) //HACK: Let users manage a global DOM object.
	{
		//HACL: daeArray doesn't set its pointer back to a thunk.
		//So do that.
		new(this) daeArchive(*this); 
		new(&_closedDocs) daeArchive(*this);
		new(&_refResolvers) daeRefResolverList;

		return;		
	}	
	_destructed = true;

	daeDOM_outstanding--;

	if(!isClosed()) _closedDoc(this);
									
	if(!_closedDocs._docs.empty())	
	if(!_isEmbedded()||_closedDocs._docs[0]!=this
	||1!=_closedDocs._docs.size()||!_docs.empty())
	{
		daeEH::Warning<<
		"Class daeDOM's \"destructor\" has unreconciled references.\n"
		"This suggests a \"memory leak\" and can precipitate a \"crash.\"";
	}

	//The database deletes _databaseRejoinder where appropriate.
	//The database can use this opportunity to survey the aftermath.
	_database->_ciao(*this,_databaseRejoinder);

	//It can't be both.
	if(_detachDatabase) _platform->detachDB(_database);	
	if(_deleteDatabase) delete _database;	
}

daeDOM *daeDOM::__daeDOM__new(daeDatabase_base *DB, daePlatform *OS)
{					
	daeDOM *obj = new daeDOM(DB,OS);	
	obj->__DAEP__Object__unembed(0);
	obj->_deleter = dae_disable_deleter; return obj;
}
daeArchive *daeDOM::_addArchive()
{							   
	daeArchive *obj = new daeArchive(*this);	
	obj->__DAEP__Object__unembed(0);	
	obj->_deleter = dae_disable_deleter; return obj;
}
daeDocument *daeDOM::_addDocument()
{
	daeDocument *obj; _database->_new(sizeof(*obj),obj,_databaseRejoinder);
	new(obj) daeDocument(this);
	obj->__DAEP__Object__unembed(1); return obj;
}
daeObject *daeDOM::_addObject(size_t size)
{
	//The caller (_add2) placement-new constructs obj, etc.
	daeObject *obj; _database->_new(size,obj,_databaseRejoinder); return obj;
}
daeElement *daeDOM::_addElement(daeMeta &meta)
{
	//_prototype is passed to the database so it can have a
	//little bit of information with which to initialize its
	//extended data structure.
	daeElement *obj = meta._prototype;
	_database->_new(meta.getSize(),obj,_databaseRejoinder);	
	meta._construct(obj,*this,*this); return obj;
}

daeError daeArchive::_read2(daeDocRef &out, daeMeta *rootmeta, const daeIORequest &reqI, daeIOPlugin *I)
{
  //This method is "static" because a "this" pointer is 
  //normally redundant, as reqI.scope should contain it.

	if(!reqI.isEmptyRequest()) reqI.resolve();
	else return DAE_ERR_INVALID_CALL; //meaningful?

	daeDOMRef DOM = reqI.getDOM();	
		
	if(reqI.scope!=DOM
	 &&reqI.localURI!=nullptr
	&&!reqI.localURI->transitsDoc(reqI.scope))
	{
		return DAE_ERR_QUERY_NO_MATCH; //or DAE_ERR_INVALID_CALL?
	}
	 
	daeRAII::UnplugIO RAII(*DOM->_platform);	
	#ifdef NDEBUG
	#error Is localURI sufficient below?
	#endif
	if(I==nullptr&&reqI.localURI!=nullptr)
	{
		I = RAII = RAII.p.pluginIO(*reqI.localURI,'r'); 		
		
		#ifdef __COLLADA_DOM__ZAE		
		//Reminder: For some reason the 1.5.0 manual says
		//ZAE can include "zip, rar, kmz, zae", etc. more
		//archives internally.
		if(I==nullptr)
		if(reqI.localURI->getURI_extensionIs("zae")
		 ||reqI.localURI->getURI_extensionIs("zip"))
		{
			static daeZAEPlugin zae; I = &zae;
		}
		#endif
	}
	daeRAII::InstantLegacyIO legacyIO; //emits error
	if(I==nullptr&&reqI.localURI!=nullptr)
	I = &legacyIO(RAII.p.getLegacyProfile(),*reqI.localURI);	
	daeRAII::Reset<const daeIORequest*> _I___request(I->_request);
	I->_request = &reqI;

	//daeDocRef out; 
	daeOK OK; //daeOK OK = I->addDoc(*DOM,out)
	if(out==nullptr)
	{
		OK = I->addDoc(out,rootmeta); 
		if(!OK) return OK;
	}
	if(out==nullptr) return OK;	 	
	daeDocumentRef document = out->getDocument();
	if(document==nullptr)
	{
		if(rootmeta!=nullptr) 
		return DAE_ERR_INVALID_CALL;
	}
	else if(!document->getContents().empty())
	{
		//This is to let the I/O plugin fill out the document
		//itself if it wants to. It's too late to do anything.
		//ALSO: IN THE FUTURE I->addDoc() MAY BE SYMLINK LIKE.
		assert(reqI.isNewDocRequest());
	}
	else //STANDARD OPERATING PROCEDURE
	{
		if(rootmeta!=nullptr)
		document->setMeta(*rootmeta);
		//This exists to prime typeLookup().
		RAII.p.personalizeDocument(*document);					
		if(reqI.isNewDocRequest())
		{
			if(rootmeta!=nullptr)
			document->getRoot() = DOM->_addElement(*rootmeta);		
		}
		else
		{
			daeIOEmpty O;
			daeRAII::CloseIO RAII2(reqI.scope->getIOController()); //emits error	
			daeIO *IO = RAII2 = RAII2.p.openIO(*I,O);
			if(IO==nullptr) 
			return DAE_ERR_CLIENT_FATAL;	
			OK = I->readContent(*IO,document->getContents());
		}
	}
		
	if(out!=nullptr&&reqI.localURI!=nullptr)
	if(out->_uri.empty()||out->_uri!=*reqI.localURI)
	out->_uri.setURI(reqI.localURI->data());

	//HMMM? If !OK should out==nullptr also? This wasn't a problem 
	//when they were fused with daeDocRoot.
	//Reminder: This is a low-level API. 
	//Close the document to curtail unexpected accidents for users?
	if(!OK&&document!=nullptr) document->close(); return OK;
}	 

daeOK daeDOM::_write_this_DOM(const daeURI &URI, daeIOPlugin *O)const
{
	daeRAII::UnplugIO RAII(*_platform);
	if(O==nullptr) RAII = O = RAII.p.pluginIO(getEmptyURI(),'w'); 
	if(O!=nullptr)
	{
		daeOK OK;
		daeIORequest reqO(this,nullptr,getEmptyURI(),URI);
		daeRAII::Reset<const daeIORequest*> _O___request(O->_request);
		O->_request = &reqO;	
		daeIOEmpty I;
		daeRAII::CloseIO RAII2(RAII.p);
		daeIO *IO = RAII2 = RAII.p.openIO(I,*O);
		OK = O->writeDOM(*IO,*this);
		if(OK!=DAE_ERR_NOT_IMPLEMENTED)
		return OK;
	}
	bool unfiltered = URI.empty();
	if(!unfiltered) URI.resolve(this);
	for(size_t i=0;i<_docs.size();i++)
	{
		daeDocument *doc = _docs[i]->_getDocument();		
		if(unfiltered||doc->_uri.transitsURI(URI))
		{
			daeOK OK = doc->write(); 
			if(!OK) 
			return OK;
		}
	}
	return DAE_OK;
}

void daeArchive::_uprootDoc(daeDoc *doc, daeDoc *uprooted_replacement)
{	
	size_t hint = _whatsupDoc;
	size_t docsN = _docs.getCount();
	if(hint>=docsN||doc!=_docs[hint])
	{
		for(hint=0;hint<docsN&&doc!=_docs[hint];)
		hint++;
		if(hint>=docsN) //docsN may be 0
		{
			//Special exception?
			//FYI: This should be the DOM being closed.
			//In which case it can't be used, but it's refs are outstanding.
			if(doc->_isDOM()&&doc==this) return;

			//Possible explanation?
			assert(_archive!=this);
			//This indicates a serious problem.
			assert(hint<docsN); return;
		}
	}

	daeDocRef *p = _docs.data();

	p[hint] = uprooted_replacement; //overwrite doc's ref

	if(uprooted_replacement==nullptr) 
	{			
		//hack: Avoid shuffling ref counts
		memmove(p+hint,p+hint+1,(docsN-hint-1)*sizeof(void*));
		(void*&)p[docsN-1] = nullptr;
		_docs.getAU()->setInternalCounter(docsN-1);
		if(hint<_whatsupDoc) _whatsupDoc--;
	}
	else uprooted_replacement->_archive = this;
}
daeDOM *daeArchive::_removeDoc(daeDoc *doc, daeDoc *uprooted_replacement=nullptr)
{
	daeDOM *DOM = (daeDOM*)getDOM();
	daeArchive *closedDocs = &DOM->_closedDocs;
	if(closedDocs==doc->_archive) 
	{
		if(uprooted_replacement==nullptr)
		return DOM;
		assert(uprooted_replacement==nullptr);
	}	
	_uprootDoc(doc,uprooted_replacement);	
	//_closedDocs is unsorted.
	doc->_archive = closedDocs;	
	closedDocs->_whatsupDoc = closedDocs->_docs.size();
	closedDocs->_docs.push_back(doc); return DOM; 
}
void daeArchive::_closedDoc(daeDoc *doc, daeDoc *replacement)
{
	//HACK? ZAE archives come out of _removeDoc deleted...
	daeDocRef safe; 
	
	if(!doc->isClosed())
	{
		safe = doc; //NEW

		if(replacement!=nullptr) 
		_uprootDoc(replacement,nullptr);
		daeDOM *DOM = _removeDoc(doc,/*uprooted*/replacement);
		(void)DOM;
	}

	//NEW: These are in addition to atomize so not to have to do
	//this inside of every doc-type (maybe including user types.)
	doc->_link = nullptr; 
	if(doc==doc->_archive->_link) doc->_archive->_link = nullptr;

	doc->__daeDoc__v1__atomize();	
}

void daeArchive::_compactDocs(size_t *before, size_t *after)
{
	//SCHEDULED FOR REMOVAL
	//TODO: FACILITATE POST-CLOSE FORENSICS.
	//This is aggressively severing the downstream edges of the graph.
	bool all_closed = getDOM()->isClosed();		

	daeDocRef *p = _docs.data();
	size_t i,j;	
	for(i=j=0;i<_docs.size();i++)
	{	
		if(daeDocType::ARCHIVE==p[i]->getDocType())		
		static_cast<daeArchive&>(*p[i])._compactDocs(before,after);		
		if(all_closed) //HACK
		{
			//SCHEDULED FOR REMOVAL
			//Refer to all_closed comment.
			p[i].~daeSmartRef(); continue;
		}
		//Cryptic/hard to search for compacting operator~().
		if(~p[i]) continue; //destructed?
		//hack: avoid shuffling the references.
		(daeDoc*&)p[j++] = (daeDoc*&)p[i];		
	}
	if(before!=nullptr) *before+=_docs.size();
	if(after!=nullptr) *after+=j;
	_docs.getAU()->setInternalCounter(j);
}
	
void daeArchive::_whatsupDocInsert(daeDoc *doc)
{	
	//This may adjust _whatsupDoc by -1.
	doc->_archive->_uprootDoc(doc,nullptr);

	size_t hint = _whatsupDoc;
	size_t docsN = _docs.getCount();
	if(hint>docsN) hint = docsN; 

	//hack: Avoid shuffling ref counts.
	_docs.grow(docsN+1);
	daeDocRef *p = _docs.data();
	memmove(p+hint+1,p+hint,(docsN-hint)*sizeof(void*));
	new(p+hint) daeDocRef(doc); doc->_archive = this;
	_docs.getAU()->setInternalCounter(docsN+1);

	//The caller is responsible for keeping this list sorted.
	assert((hint==0||doc->_uri>p[hint-1]->_uri)
	&&(hint+1>=docsN||doc->_uri<p[hint+1]->_uri));
}

void daeArchive::__daeDoc__v1__atomize()
{
	for(size_t i=0,iN=_docs.size();i<iN;i++)
	_docs[i]->__daeDoc__v1__atomize(); _compactDocs();
}

void daeDOM::__daeDoc__v1__atomize()
{						
	daeArchive::__daeDoc__v1__atomize(); clear_closedDocs();
	
	_refResolvers.clear(); //Assuming close() called and can't reopen.
}

//JUST DOING THE MINIMUM FOR THE TIME BEING.
DAEP::Model &daeDoc::__DAEP__Object__v1__model()const
{
	static DAEP::Model *om = nullptr; if(om!=nullptr) return *om;

	daeModel &m = getDefaultProcessShare().addModel<1>(this,"COLLADA::daeDoc");	
	//This is required to conflate the object/subobject reference-counters.
	m.addFeature_variant<1>(this,_uri,"URI //SUBOBJECT");
	m.addFeatureComplete();

	m.setObjectType(daeObjectType::DOC); om = &m; return *om;
}
DAEP::Model &daeDOM::__DAEP__Object__v1__model()const
{
	static DAEP::Model *om = nullptr; if(om!=nullptr) return *om;

	////COPIED FROM daeDoc::__DAEP__Object__v1__model().
	////MAYBE THIS SHOULD CALL INTO A COMMON SUBROUTINE?

	daeModel &m = getDefaultProcessShare().addModel<3>(this,"COLLADA::daeDOM");	
	//This is required to conflate the object/subobject reference-counters.
	m.addFeature_variant<1>(this,_uri,"URI //SUBOBJECT");
	m.addFeature_variant<2>(this,_closedDocs,"closedDocs //SUBOBJECT");
	m.addFeature_variant<3>(this,_refResolvers,"refResolvers //SUBOBJECT");
	m.addFeatureComplete();

	m.setObjectType(daeObjectType::DOM); om = &m; return *om; 	
}

//---.
}//<-'

/*C1071*/
