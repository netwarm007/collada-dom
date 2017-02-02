/*
 * Copyright 2006 Sony Computer Entertainment Inc.
 *
 * Licensed under the MIT Open Source License, for details please see license.txt or the website
 * http://www.opensource.org/licenses/mit-license.php
 *
 */

#ifndef __COLLADA_DOM__DAE_DOCUMENT_H__
#define __COLLADA_DOM__DAE_DOCUMENT_H__

#include "daeURI.h"
#include "daeElement.h"

COLLADA_(namespace)
{//-.
//<-'

/**SCOPED-ENUM
 * @c HARDLINK and @c ARCHIVE are implemented.
 */
struct daeDocType
{
	/**EXPERIMENTAL
	 * Class @c daeDoc is one of the following:
	 * - A @c daeDocument.
	 * - A hard-link to a @c daeDocument.
	 * - A symbolic-link (symlink) to a @c daeDoc.
	 * - A @c daeArchive.
	 * - A @c daeDOM, which is also a @c daeArchive.
	 * @see @c daeDoc Doxygentation's details.
	 * 
	 * @c ORPHANAGE is a proposal so free elements can
	 * reliably report a document. It could be a DOM's
	 * @c _link member; though that could be a problem.
	 * Probably @c daeDOM::_closedDocs._link is better.
	 */
	enum{ HARDLINK=0, SYMLINK, ARCHIVE, ORPHANAGE };	
};

#include "../LINKAGE.HPP" //#define LINKAGE

/**
 * Class @c daeDoc is one of the following:
 * - A @c daeDocument.
 * - A hard-link to a @c daeDocument.
 * - A symbolic-link (symlink) to a @c daeDoc.
 * - A @c daeArchive.
 * - A @c daeDOM, which is also a @c daeArchive.
 *
 * @remarks Note that a DOM is not your system's file system.
 * A hard-link means if the document is closed, the hard link keeps it open.
 * A symbolic link means, if the document is closed, it's closed, and the symlink retains
 * a link to an empty document (unless it is reopened.)
 */
class daeDoc : public daeObject
{
	friend class daeDOM;
	friend class daeArchive;
	friend class daeDocument;		

COLLADA_(public) //DATA-MEMBERS
	//SUB-OBJECT
	/**
	 * URI of the document, archive, or DOM working directory. 
	 *
	 * @note Embedding the URI in the document would conflate
	 * their reference-counters. To even consider it requires
	 * a generic way to enumerate embedded objects whenever a
	 * reference counter becomes 0.
	 *
	 * ATTENTION
	 * @c _uri has special meaning to @c daeDOM. It is always
	 * empty. This is especially important to @c daeStringRef.
	 * Its prototype-constructor is relying on the visibility
	 * of @c _uri.
	 * @see @c daeDOM::getEmptyURI() Doxygentation.
	 */
	daeURI_size<0> _uri;
	/** 
	 * If ARCHIVE, this holds the ZAE's root DAE. 
	 */
	daeDocRef _link;
	/** 
	 * If @c _archive==this, then this is the @c daeDOM.
	 */
	daeArchive *_archive; 
	
	/** 
	 * Implements @c write() and @c writeTo(). 
	 * @see @c daeDocument::_write().
	 */
	LINKAGE daeError _write(const daeURI&,daeIOPlugin*)const;
	
COLLADA_(public) //OPERATORS

	COLLADA_DOM_OBJECT_OPERATORS(daeDoc)

COLLADA_(public) //ABSTRACT INTERFACE

	/**ABSTRACT INTERFACE
	 * Initiates the "teardown" sequence.
	 * Doing this as @c virtual makes it easier to
	 * call, and looks forward to possible plugin extension.
	 * ABSTRACT?
	 * @c assert(0) is for C2259 when creating @c daeDOM on 
	 * the stack. Its constructor is exported, so it should
	 * assign the correct "vptr" and not this one.
	 */
	virtual void __daeDoc__v1__atomize(){ assert(0); /* = 0;*/ }
	/** Pro-forma. Should never be called. */
	inline void __COLLADA__atomize(){ return __daeDoc__v1__atomize(); }

COLLADA_(public)
	/**
	 * @return Returns HARDLINK, SYMLINK, or ARCHIVE. 
	 */
	inline int getDocType()const{ return 3&_getClassTag(); } 

	/**
	 * @return Returns @c true if this @c daeDoc is a @c daeDocument.
	 */
	inline bool isDocument()const
	{
		//At some point, _link==nullptr should be sufficient.
		return _link==nullptr&&getDocType()==daeDocType::HARDLINK; 
	}
	/**
	 * @return Returns @c true if this @c daeDoc is a @c daeArchive,
	 * -excluding the global pseudo-archive, that may or may not be a
	 * @c daeDOM. (It is until a DOM-as-an-archive mode is ever added.)
	 */
	inline bool isArchive()const
	{
		return getDocType()==daeDocType::ARCHIVE&&_archive!=(void*)this; 
	}

	//using daeObject::a;
	template<class T> T *a(){ return daeObject::a<T>(); }
	/**CONST-FORM Following style of daeElement::a(). */
	template<class T> const T *a()const
	{
		return const_cast<daeDoc*>(this)->a<T>();
	}
	/** Pass-Through; Follows style of daeElement::a(). */
	template<> daeDoc *a<daeDoc>(){ return this; }
	/** Follows style of daeElement::a(): @c This->a<daeDocument>(). */
	template<> daeDocument *a<daeDocument>()
	{
		return this!=nullptr&&isDocument()?(daeDocument*)this:nullptr;
	}
	/** Follows style of daeElement::a(): @c This->a<daeArchive>(). */
	template<> daeArchive *a<daeArchive>()
	{
		return this!=nullptr&&isArchive()?(daeArchive*)this:nullptr;
	}
		
	/**NOT-THREAD-SAFE
	 * Closes the doc, unloading all memory used by the document-or-archive.
	 * With the exception of @c daeDOM, @c daePlatform::closeURI() can veto
	 * the close. Details/override procedures follow:
	 *
	 * @return Returns DAE_OK if the document is closed, and FOR COMPLETENESS:
	 * If MULTI-THREAD, @c DAE_NOT_NOW is returned if the platform is deciding
	 * to close or not on a different thread from the calling one.
	 * (This doesn't mean the library is thread safe--the application must be.)
	 *
	 * @note Permission is gotten from @c daePlatform::closeURI().
	 * This can be circumvented by getDocURI().clear(), but this is discouraged,
	 * -because it's A) not nice, and B) makes the URI unusable for diagnostics.
	 */
	LINKAGE daeOK close();
	/**
	 * Tells that this doc is surely housed in @c daeDOM::_closedDocs. 
	 */
	inline bool isClosed()const
	{
		//daeArchive is based on daeDoc/not yet defined here.
		for(daeArchive *p=_archive;;p=((daeDoc*)p)->_archive) 
		if(p==((daeDoc*)p)->_archive) 
		return !((daeObject*)p)->_isDOM(); return false;
	}

	/**LEGACY-SUPPORT
	 * Writes doc to its archive/document's URI.
	 * If @c this is a @c daeDOM its docs are written. The library does this unless
	 * @a O implements @c daeIOPlugin::writeDOM() or the default I/O plugin does so.
	 * @return Returns @c false if unwritten and @c COLLADA_NOLEGACY is not defined.
	 * Othewise a daeError code is returned.
	 */
	inline daeOK write(daeIOPlugin *O=nullptr)const{ return _write(_uri,O); }
	/**WARNING, LEGACY-SUPPORT
	 * @warning If @c this is a @c daeDOM, this API behaves differently if a plugin
	 * does not takeover. In which case, @c daeURI::transitsURI(URI) is called with
	 * each document's URI, in order to filter out documents. In other words, it is
	 * a partial write operation, and the existing documents are overwritten.
	 * 
	 * Writes doc to @a URI.
	 * @param URI If @c this is a @c daeDOM then @a URI is taken to be its base-URI,
	 * -or only its base is considered. A DOM's natural base is the empty URI.
	 * @return Returns @c false if unwritten and @c COLLADA_NOLEGACY is not defined.
	 * Othewise a @c daeError code is returned.
	 */
	inline daeOK writeTo(const daeURI &URI, daeIOPlugin *O=nullptr)const{ return _write(URI,O); }								
	
	/**WARNING
	 * Gets the doc's URI that is used to locate it within the DOM's doc-tree. 
	 * @warning If a doc is created via @c daeSmartRef's @c daeDOM base constructors,
	 * -then the doc uses @c daeDOM::getEmptyURI(). @c attachDocURI() must be called.
	 * The URIs are doc-aware; meaning altering a doc's URI, relocates it in the DOM.
	 */
	inline daeURI &getDocURI(){ return _uri; }
	/**WARNING, CONST-FORM 
	 * Gets the doc's URI that is used to locate it within the DOM's doc-tree. 
	 * @warning If a doc is created via @c daeSmartRef's @c daeDOM base constructors,
	 * -then the doc uses @c daeDOM::getEmptyURI(). @c attachDocURI() must be called.
	 * The URIs are doc-aware; meaning altering a doc's URI, relocates it in the DOM.
	 */
	inline const daeURI &getDocURI()const{ return _uri; }

	/** 
	 * Gets the links' archive. 
	 * @return Should not return @c nullptr; although sometimes it is @c nullptr.
	 * (Mid-transfer.)
	 * @remarks Post-2.5 returns @c const types for upstream "getter" APIs. 
	 */
	inline const daeArchive &getArchive()const{ return *_archive; }
	
	/** 
	 * Gets the links' real document.
	 * "getDocument" is interchangeable with @c daeElement's.
	 * @c this can be @c nullptr. (It's often convenient to blindly convert doc-to-document.)
	 */
	inline daeDocumentRef getDocument(){ return _getDocument(); }
	/**CONST-PROPOGATING-FORM
	 * Gets the links' real document. 	 
	 * "getDocument" is interchangeable with @c daeElement's.
	 * @c this can be @c nullptr. (It's a common scenario, and the algorithm is there.)
	 */
	inline const_daeDocumentRef getDocument()const{ return _getDocument(); }
	/** Implements @c getDocument(). */
	inline daeDocument *_getDocument()const
	{
		const daeDoc *p = this; if(p!=nullptr) while(p->_link!=nullptr) p = p->_link;
		return const_cast<daeDoc*>(p)->a<daeDocument>();		
	}

	/**LEGACY, CIRCULAR-DEPENDENCY
	 * Accessor to get the database associated with this document.
	 * @return Returns the database associated with this document.
	 */
	inline daeDatabase &getDatabase()const;

#ifdef BUILDING_COLLADA_DOM

	/**
	 * Constructor
	 */
	explicit daeDoc(daeDOM *a, int dt=daeDocType::HARDLINK);
	/**
	 * Virtual Destructor
	 */
	virtual ~daeDoc(){}

COLLADA_(public) //DAEP::Object methods

	/**PURE-OVERRIDE */
	virtual DAEP::Model &__DAEP__Object__v1__model()const;

COLLADA_(public)
	/**
	 * These concern @c _doOperation().
	 */
	enum _op{ _no_op=0, _close_op, _setURI_op };
	/**WORKAROUND
	 * @c close() uses this to permit reentry by daePlatform::closeURI().
	 * @c daeURI_base::setURI() uses it to resolve the URIs of documents.
	 */
	int _operation_thread_id; _op _current_operation;	

	//Just a little more typesafe.
	template<_op> struct _op_arg0;
	/** The bool is set to true if closeURI returns DAE_OK. */
	template<> struct _op_arg0<_close_op>{ typedef bool type; };
	/** This is the same string that is passed to setURI. */
	template<> struct _op_arg0<_setURI_op>{ typedef const daeStringCP type; };
	template<_op op>
	/** 
	 * An "operation" here means that an API needs to be recursive on
	 * the object, and must return @c DAE_NOT_NOW when an other thread
	 * does @c this->_doOperation().
	 * (IT DOESN'T USE INTERLOCKED/ATOMIC OPERATIONS AS-IS, AND PROBABLY
	 * CANNOT ABIDE BY OVERLAPPING OPERATIONS.)
	 *
	 * @param DOM is the doc's DOM. 
	 * @return Returned code is tailored to @a op, and is very sensitive.
	 */
	daeOK _doOperation(const const_daeDOMRef &DOM, typename _op_arg0<op>::type *arg0)
	{
		return _doOperation(op,DOM,arg0);
	}

COLLADA_(private) 

	/** Implements the safer template form. */
	daeOK _doOperation(_op op, const const_daeDOMRef &DOM, const void *arg0=nullptr);

#endif //BUILDING_COLLADA_DOM
};

extern bool daeDocument_typeLookup_called;
/**
 * The @c daeDocument class implements a COLLADA document.
 */
class daeDocument : public daeDoc
{	
	friend class daeDOM;
	friend class daeDoc;
	/**
	 * Disabling @c new operator for clients.
	 * @see @c daeDOM::_addDocument().
	 */
	void *operator new(size_t n)
	{
		return ::operator new(n); //NOP
	}	

COLLADA_(public) //OPERATORS
	/** 
	 * Placement-new wants this on MSVC2015. 
	 * There's no reason for this to be @c public,
	 * -but neither is there reason for it to not be so.
	 */
	void *operator new(size_t, void *p){ return p; }

	COLLADA_DOM_OBJECT_OPERATORS(daeDocument)

COLLADA_(public) //DAEP::Object methods

	/**PURE-OVERRIDE */
	virtual void __daeDoc__v1__atomize();

COLLADA_(private) //INTERNALS	
	/** 
	 * The way this works is somewhat schizophrenic, but the
	 * I/O request framework is slightly more flexible than a
	 * straight URI would be otherwise.
	 * The "scope" and "local URI" are required to match the
	 * document unti further notice. They're double checked on
	 * the inside.
	 */
	LINKAGE daeError _write(const daeIORequest&,daeIOPlugin*)const;

	NOALIAS_LINKAGE daePseudoElement &_getPseudoElement()const
	SNIPPET( return *_pseudo.element(); )

#if defined(BUILDING_COLLADA_DOM) || defined(__INTELLISENSE__)

		struct _PseudoElement 
		:
		public daeElemental<_PseudoElement>, public DAEP::Schema<2>
		{
		COLLADA_(public) //DAEP::Object method
			/**
			 * Constructs the virtual-method-table.
			 */
			_PseudoElement(){}

		COLLADA_(public) //Parameters

			typedef struct:Elemental,Schema
			{	COLLADA_WORD_ALIGN
				COLLADA_DOM_N(0,0)
			DAEP::Value<0,dae_Array<>> _N; enum{ _No=0 };
			DAEP::Value<1,daeContents> content; typedef void notestart;
			}_;

		COLLADA_(public) //Content

			COLLADA_WORD_ALIGN
			COLLADA_DOM_N(0,0) 
			/**NO-NAMES
				* These elements are invalid according to the schema. They may be user-defined 
				* additions and substitutes.
				*/
			DAEP::Child<1,daeElement/*xsAny*/,_,(_::_)&_::_N> unnamed;
			/**
				* Children, mixed-text, comments & processing-instructions.
				*/
			DAEP::Value<2,daeContents,_,(_::_)&_::content> content;
		};	  
		struct _Pseudo
		{
			daeFeature features[2];
			char model[sizeof(daeModel)];
			char meta[sizeof(daeMeta)+sizeof(_PseudoElement)];			
			daeElement *element()const{ return operator->()->_prototype; }
			daeMetaElement *operator->()const{ return (daeMetaElement*)meta; }			
		}; 		
		/**
		 * This is a self-contained model/metadata arrangement.
		 * It has no process-share nor schema. It's assigned the
		 * schema of its children, which must share a schema.
		 */
		_Pseudo _pseudo;				

COLLADA_(protected)
		/**
		 * Constructor
		 */
		explicit daeDocument(daeDOM*);

COLLADA_(public/*C4624*/) 
		/**
		 * Virtual Destructor
		 */
		virtual ~daeDocument();	
#else 

//C4624: silence warning for the DBase types.
COLLADA_(public) ~daeDocument(){ assert(0); }

#endif //BUILDING_COLLADA_DOM

COLLADA_(public)	
	/**
	 * Gets the contents-array of this document.
	 * @note "getContents" is designed to match 2.x generated classes.
	 */
	inline daeContents &getContents()
	{
		return _getPseudoElement().getContents(); 
	}
	/**CONST-PROPOGATING-FORM
	 * Gets the contents-array of this document.
	 * @note "getContents" is designed to match 2.x generated classes.
	 */
	inline const daeContents &getContents()const
	{
		return _getPseudoElement().getContents(); 
	}

	/**
	 * Gets the "pseudo-element" metadata, describing the acceptable
	 * root elements. The CM graph is a degenerate <xs:element> root.
	 * It might be extended to support an <xs:any> like content mode.
	 * If so, this API will probably serve whatever @c setMeta() got.
	 * If @c setMeta() is never called, it will be the @c domAny one.
	 * This is here to go with @c daePlatform::personalizeDocument().
	 */
	inline daeMeta &getMeta()const
	{
		return _getPseudoElement().getMeta().jumpIntoTOC(0).getChild();
	}
	/**
	 * It seems harmless to let this be set directly if the document
	 * is empty.
	 * @return This API currently returns @c DAE_ERR_INVALID_CALL if
	 * @c false==getContents().empty().
	 */
	LINKAGE daeOK setMeta(daeMeta&);

	/**
	 * Accessor to get the XML-like ROOT node associated with this document.
	 * @return A @c daeElementRef for the the root of this document.
	 * @remarks Technically 1 means there can be more than one root.
	 */	
	inline dae_Array<daeElement,1> &getRoot()
	{
		return getContents().unnamed(); 
	}
	/**CONST-PROPOGATING-FORM
	 * Accessor to get the XML-like ROOT node associated with this document.
	 * @return A @c daeElementRef for the the root of this document.
	 * @remarks Technically 1 means there can be more than one root.
	 */	
	inline const dae_Array<daeElement,1> &getRoot()const
	{
		return getContents().unnamed(); 
	}
	
	#ifndef COLLADA_NODEPRECATED
	COLLADA_DEPRECATED("getDocURI or getBaseURI")
	/**LEGACY
	 * @return Returns a pointer to the URI of this document.	 
	 * The old pointer way is supported by overloading @c daeURI's operators.
	 */
	inline daeURI &getDocumentURI(){ return _uri; }
	#endif //COLLADA_NODEPRECATED

	/**CONST-ONLY
	 * Gets the base URI used for resolving relative URI references. 
	 * @return Returns the same @c daeURI& as @c daeDoc::getDocURI().
	 */
	inline const daeURI &getBaseURI()const{ return _uri; }
			
COLLADA_(public) //LEGACY: old "database" APIs

  /////////////////////////////////////////////////////////////////
  //TODO: Should COLLADA_NOLEGACY apply? Or some other directive?//
  /////////////////////////////////////////////////////////////////

  /////////////////////////////////////////////////////////////////
  //THESE WON'T WORK IF THE daeDatabase INTERFACE DOESN'T FORWARD// 
  //ID/SID NOTES TO daeDocument::_carry_out_change_of_ID_or_SID()//
  /////////////////////////////////////////////////////////////////

	template<class S, class T> //T is DAEP::Element based
	/**LEGACY
	 * Looks up elements by "id" attribute.
	 * @param id The ID to match on.
	 * @return Returns @a match.
	 */
	inline daeSmartRef<T> &idLookup(const S &id, daeSmartRef<T> &match, enum dae_clear clear=dae_clear)const
	{
		static_cast<const daeElement*>(dae((T*)nullptr)); //up/downcast 
		if(clear!=dae_default) match = nullptr;
		_idLookup2(id,(daeElementRef&)match,nullptr); 		
		if(nullptr==match->a<T>()) match = nullptr; return match;
	}
	#ifdef NDEBUG
	#error Is this still necessary?
	#endif
	template<class T>
	/**OVERLOAD
	 * Implements @c idLookup(), extracting @c daeStringRef from @c DAEP::Value.
	 */
	inline void _idLookup2(const T &id, daeElementRef &match, typename T::underlying_type*)const
	{
		_idLookup2((const typename T::underlying_type&)id,(daeElementRef&)match,nullptr); 

	}
	template<class T>
	/**OVERLOAD Implements @c idLookup(). */
	inline void _idLookup2(const T &id, daeElementRef &match,...)const
	{
		//C2794: MSVC2010 fails SFINEA with explict specializations.
		_idLookup2_MSVC2010(daeStringRef(*this,id),match); 
	}
	template<class T>
	/**OVERLOAD Implements @c idLookup(). */
	inline void _idLookup2_MSVC2010(const T &id, daeElementRef &match)const
	{
		_idLookup3(daeStringRef(*this,id),match); 
	}
	template<>
	/**OVERLOAD, TEMPLATE-SPECIALIZATION Implements @c idLookup(). */
	inline void _idLookup2_MSVC2010(const daeStringRef &id, daeElementRef &match)const
	{
		_idLookup3(id,match); 
	}
	
	template<class T>
	/**LEGACY
	 * @note This is not very useful on its own. SIDs are not unique. 
	 * @see daeSIDResolver.
	 * @remark Historically @a matchingElements is cleared.
	 * @param sid The COLLADA Scoped Identifier (SID) to match on.
	 * @param matchingElements The array of matching elements.
	 * @return Returns @a matchingElements.
	 */
	inline daeArray<daeElementRef> &sidLookup(const T &sid, daeArray<daeElementRef> &matchingElements, enum dae_clear clear=dae_clear)const
	{
		if(clear) matchingElements.clear(); 
		_sidLookup(daeStringRef(*this,sid),matchingElements); return matchingElements;
	}
	template<>
	/**LEGACY, TEMPLATE-SPECIALIZATION
	 * @note This is not very useful on its own. SIDs are not unique. 
	 * @see daeSIDResolver.
	 * @remark Historically @a matchingElements is cleared.
	 * @param sid The COLLADA Scoped Identifier (SID) to match on.
	 * @param matchingElements The array of matching elements.
	 * @return Returns @a matchingElements.
	 */
	inline daeArray<daeElementRef> &sidLookup(const daeStringRef& sid, daeArray<daeElementRef> &matchingElements, enum dae_clear clear/*=dae_clear*/)const
	{
		if(clear) matchingElements.clear(); _sidLookup(sid,matchingElements); return matchingElements;
	}
	
  /////////////////////////////////////////////////////////////////
  //typeLookup is a difficult part of the old library to support!//
  //It's orthogonal to the change-notice system, so that it won't//
  //become entangled with it. Users may want it disabled, because//
  //it complicates a lot of basic procedures. It seems like there//
  //could be a more general tracking facility in it, but it's not//
  //clear what it'd look like, or be, or mean to database writers//
  /////////////////////////////////////////////////////////////////

	template<typename T>
	/**LEGACY-SUPPORT
	 * This can be used to "prime the pump." 
	 * @see @c daePlatform::personalizeDocument() Doxygentation. 
	 */
	inline void typeLookup_enable()const{ daeArray<daeSmartRef<T>,1> dummy; typeLookup(dummy); }

	template<typename T> //T is DAEP::Element based, domAny will silently fail.
	/**WARNING, LEGACY-SUPPORT
	 * @warning T WON'T WORK WITH @c domAny. IT WON'T FIND ANYTHING. THERE AREN'T GOOD REASONS
	 * TO LOOK FOR @c domAny. ONLY THE MASTER META DATA WILL BE USED, SO IT'S NOT CATASTROPHIC.
	 *
	 * Looks up elements by C++ type.
	 * @remark Historically @a matchingElements is cleared.
	 * @param matchingElements The array of matching elements.
	 * @return Returns @a matchingElements.
	 */	
	inline daeArray<daeSmartRef<T>> &typeLookup(daeArray<daeSmartRef<T>> &matchingElements, enum dae_clear clear=dae_clear)const
	{
		if(clear) matchingElements.clear();
		//T won't pass daeGetMeta if not an element.
		_typeLookup(daeGetMeta<T>(),(daeArray<daeElementRef>&)matchingElements); return matchingElements;
	}
	template<class T> //daeElement or const daeElement (The template crap captures const-ness.)	
	/**LEGACY-SUPPORT */
	inline daeArray<daeSmartRef<typename daeConstOf<T,daeElement>::type>> &typeLookup
	(daeMeta &same, daeArray<daeSmartRef<T>> &matchingElements, enum dae_clear clear=dae_clear)const
	{
		if(clear) matchingElements.clear(); 
		_typeLookup(same,(daeArray<daeElementRef>&)matchingElements); return matchingElements;
	}	
		
	template<class T>
	/**LEGACY-SUPPORT
	 * Recursive version of @c daeElement::getChildrenByType().
	 * @remarks Post-2.5 this is used/added to implement @c daeDocument::typeLookup().
	 */	
	inline T &_getElementsByType(daeMeta *meta, T &matchingElements)const
	{
		daeElement::_getChildrenByType_f<1,T> f = { matchingElements,meta };		
		const_daeElementRef root = getRoot();
		//FIX-ME: CASTING BECAUSE daeElement ISN'T FULLY CONST-CORRECT.
		if(root!=nullptr) f((daeElement*&)root); return matchingElements;
	}	
	template<typename T>
	/**LEGACY-SUPPORT
	 * Advertised version of @c _getElementsByType(). Use it if you want.
	 * Similar to @c daeElement::getDescendantsByType(), but with root included. Nothing special.
	 */	
	inline daeArray<daeSmartRef<T>> &getElementsByType(daeArray<daeSmartRef<T>> &matchingElements, enum dae_clear clear=dae_clear)const
	{
		if(clear) matchingElements.clear();
		_getElementsByType(daeGetMeta<T>(),(daeArray<daeElementRef>&)matchingElements); return matchingElements;
	}	
	
COLLADA_(private) //exported indexing methods

	/** Implements @c idLookup(). */
	LINKAGE void _idLookup3(const daeStringRef&, daeElementRef&)const;	
	/** Implements @c typeLookup(). */
	LINKAGE void _typeLookup(daeMeta&, daeArray<daeElementRef>&)const;	
	/** Implements @c sidLookup(). */
	LINKAGE void _sidLookup(const daeStringRef&, daeArray<daeElementRef>&)const;

COLLADA_(public) //LEGACY-SUPPORT
	/**LEGACY
	 * This API is the mechanism by which the ID and SID maps are populated. 
	 * @c daeDatabase implementations are responsible for calling it, in response
	 * to @c daeDatabase::_v1_note(). Databases are not obliged to do so. Notification
	 * is required to go to the database. It's this way so that there aren't duplicate paths.
	 */
	LINKAGE void _carry_out_change_of_ID_or_SID(const DAEP::Change&, const XS::Attribute*)const;
	/**LEGACY
	 * Not only is it necessary to capture changes of the attributes, whenever an
	 * element having an "id" or "sid" attribute is inserted or removed from a document
	 * it's necessary to manage the document's id/sid lookup tables. It's up to the database
	 * to walk the element graph, since that's heavy lifting, and there might be other things it
	 * needs to do.
	 * @param destination can be @c nullptr. Both @c this document and it need to "compare notes." 
	 * @c this can also be @c nullptr. The API is not @c static "just because." E.g:
	 * @c source->_migrate_ID_or_SID(destination,...);
	 */
	LINKAGE void _migrate_ID_or_SID(const daeDocument *destination, const daeElement*, const XS::Attribute*)const;

#ifdef BUILDING_COLLADA_DOM

COLLADA_(private) //LEGACY: old "database" API stuff

		mutable daeStringRefMap<daeElement*> _idMap;
		typedef daeStringRefMap<daeElement*>::const_iterator _idMapIter;
		typedef std::pair<daeString,daeElement*> _idMapPair;

		mutable daeStringRefMultiMap<daeElement*> _sidMap;		
		typedef daeStringRefMultiMap<daeElement*>::const_iterator _sidMapIter;
		typedef std::pair<daeString,daeElement*> _sidMapPair;
		typedef std::pair<_sidMapIter,_sidMapIter> _sidMapRange;

		friend daeMetaElement;
		mutable daeStringRefMap<std::vector<daeElement*>> _typeMap; 
		typedef std::vector<daeElement*> _typeVec;		
		typedef daeStringRefMap<_typeVec>::iterator _typeMapIter;
		friend daeContents_base;		
		template<int op> struct _typeLookup_migrate_operation
		{
			const daeDocument *doc;	void operator()(const daeElement *ch);
		};
		friend daeElement;		
		_typeVec *_typeLookup_vector(daeMeta&)const;
		void _typeLookup_self_remove(daeMeta&,daeElement&)const;
		void _typeLookup_bulk_remove(daeMeta&/*,UNIMPLEMENTED*/)const;

#endif //BUILDING_COLLADA_DOM
};

#include "../LINKAGE.HPP" //#undef LINKAGE
	
//---.
}//<-'

#endif //__COLLADA_DOM__DAE_DOCUMENT_H__
/*C1071*/