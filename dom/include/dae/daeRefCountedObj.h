/*
 * Copyright 2006 Sony Computer Entertainment Inc.
 *
 * Licensed under the MIT Open Source License, for details please see license.txt or the website
 * http://www.opensource.org/licenses/mit-license.php
 *
 */

#ifndef __COLLADA_DOM__DAE_OBJECT_H__
#define __COLLADA_DOM__DAE_OBJECT_H__

#include "../DAEP.h"
#include "daeDatabase.h"

COLLADA_(namespace)
{//-.
//<-'
	
/**SCOPED-ENUM 
 * @note Element types are unique if
 * the classes are generated together
 * Historically these are COLLADA_TYPE
 * _ is applied to undesignated objects
 * @see @c daeObject::getObjectType().
 * @see @c daeElement::getElementType().
 */
struct daeObjectType
{
	//REF is used by URIs and IDREFs.
	//ANY and above are element types.
	//_ is 0, and indicates undeclared.
	enum{ REF=-3,DOM,DOC,_,ANY };
};

/**EXPERIMENTAL, HOMELESS
 * COLLADA C++ Object RTTI class.
 *
 * @c daeMetaElement (based on @c daeMetaObject) is 
 * located within this class. 
 */
class daeModel : public DAEP::Model
{	
#ifdef BUILDING_COLLADA_DOM

	friend class daeFeature;
	friend class DAEP::Object;

COLLADA_(private) //INVISIBLE (protecting "obj")
	/**
	 * Does @c delete @a obj local to its process-share.
	 */
	inline void deleteObject(const DAEP::Object *obj)const
	{
		(*__DAEP__Model__make)->__DAEP__Make__v1__delete(obj);
	}	

	/**RECURSIVE
	 * @return Returns @c true if @obj has sub-object refs.
	 */
	inline bool hasEmbeddedRefsWRT(const DAEP::Object *obj)const
	{
		for(int ID=-1,mask=(*this)->_subobjectsMask;mask!=0;ID--,mask>>=1)		
		if((mask&1)!=0&&((daeFeature*)this)[ID].hasEmbeddedRefsWRT(obj))
		return true; return false;
	}

#endif //BUILDING_COLLADA_DOM

COLLADA_(public) //__DAEP__Object__v1__model mutator APIs 

	template<int Feature, class T> //Feature must count down to 1 or up to -1.
	/**MODEL SETUP API
	 * This overload is designed to highly automate the model creation process.
	 * It's similar to how element metadata record registration is streamlined.
	 * IMPORTANT
	 * @tparam Feature if 1 or -1 is the "first" feature, which is in front of
	 * the model in memory. Feature iteration normally goes from back-to-front.
	 * If there are features the first feature calls @c daeFeature::endModel().
	 */
	inline daeFeature &addFeature(const daeObject *this_, const T &nul, daeClientString name_explanation)
	{
		daeFeature &o = addFeature_variant<Feature>(this_,nul,name_explanation);
		__addTypewriter(o,(T*)nullptr);
		__addLocalthunk(o,(T*)nullptr); return o;
	}
	template<int Feature, class T> 
	/**MODEL SETUP API
	 * This version doesn't generate a typewriter nor local-thunk.
	 * @see @c daeFeature::setTypewriter() and @c setAllocThunk().
	 */
	inline daeFeature &addFeature_variant(const daeObject *this_, const T &nul, daeClientString name_explanation)
	{
		//There isn't yet a member for this, since it's not used.
		//But code should provide a string in its place.
		//o._name_explanation = name_explanation;		
		daeFeatureID fID = (daeFeatureID)(Feature<0?Feature:-Feature);
		daeFeature &o = addFeature(fID,(char*)&nul-(char*)this_,sizeof(T));
		assert(&o==&__DAEP__Model__feature(this_,&nul));		
		o._flags.subobject = __subobject_flag<T>::value; daeCTC<0!=Feature>(); return o;
	}
	/**LOW-LEVEL MODEL SETUP API
	 * This is a low-level version of this API that @c daeMetaElement is using.
	 */
	inline daeFeature &addFeature(daeFeatureID fID, daeOffset offset, size_t charsize=sizeof(short))
	{
		int f = -int(fID); assert(f>0);
		daeFeature &o = const_cast<daeFeature&>(operator[](fID));		
		//Char-sized members must come before the UCHAR_MAX'th address.			
		if(1==charsize) __mapFeature<unsigned char>(offset,UCHAR_MAX,f);
		else __mapFeature<short>(offset,SHRT_MAX,f)*=-1; //Negate.					
		assert(0==o._offset); o._offset = offset; return o;
	}
	/** @c addFeature() subroutine. */
	template<class T> T &__mapFeature(daeOffset os, int max, int f)
	{
		T &t = (T&)*((char*)this+os); 
		assert(max>=f/*&&t==0*/); if(t!=0) //OVERKILL: Union?
		{
			//Unions can't mix sizes, so try to stop if char-size?
			assert(t<0&&sizeof(T)>1); 
			operator[](daeFeatureID(t))._flags.unionized = 1;
			operator[](daeFeatureID(-f))._flags.unionized = 1;
			if(t<-f) f = -t;
		}
		return t = T(f&max);
	}
	/** Note: @c DAEP::Value uses this to forbid subobjects. */
	template<class T> struct __subobject_flag
	{ 
		typedef char Yes; typedef long No;	
		static Yes Value(DAEP::Object*); static No Value(...);
		enum{ value=sizeof(Yes)==sizeof(Value((T*)nullptr)) };
	};	
	/** Gets a default localThunk. Users can override this. */
	template<class T> static void __addLocalthunk(daeFeature &o, T*)
	{
		o.setAllocThunk(daeAlloc<T,0>::localThunk());
	}
	/** Gets a default localThunk. Users can override this. */
	template<class T> static void __addLocalthunk(daeFeature &o, daeArray<T>*)
	{
		o.setAllocThunk(new daeAlloc<T,0>); o._flags.new_thunk = 1;
		o._localthunk->_offset = o._offset;
	}
	/** Gets a default typewriter. Users can override this. */
	template<class T> static void __addTypewriter(daeFeature &o, T*)
	{
		static daeType<T> type; o._typewriter = type;
	}
	/** Gets a default typewriter. Users can override this. */
	template<class T> static void __addTypewriter(daeFeature &o, daeArray<T>*)
	{
		__addTypewriter<T>(o,nullptr); o._typewriter = o._typewriter->per<daeArray>();
	}
	using DAEP::Model::operator[];
	/** @c addFeature() is one time only, due to @c assert() checks. */
	inline daeFeature &operator[](daeFeatureID fID)
	{	
		assert((int)fID<0&&fID>=__DAEP__Model__meta->rbeginFeatureID());
		return ((daeFeature*)this)[(int)fID];
	}

	/**MODEL SETUP API
	 * Sets the @c daeHashString name of this model.
	 */
	inline daeModel &setName(daeName name)
	{
		const_cast<daeName&>(__DAEP__Model__meta->_name) = name; return *this;
	}
	/**MODEL SETUP API
	 * Sets the @c daeObjectType scoped @c enum of @c this model.
	 */
	inline daeModel &setObjectType(int type)
	{
		const_cast<daeInt&>(__DAEP__Model__genus) = type; return *this;
	}
	/**MODEL SETUP API
	 * Call this to finalize the @c daeMetaObject.
	 */
	COLLADA_DOM_LINKAGE void addFeatureComplete()
	COLLADA_DOM_SNIPPET( __DAEP__Model__meta->_feature_complete(); )

COLLADA_(public) //ACCESSORS
	/**
	 * Gets the @c daeObjectType scoped @c enum of @c this model.
	 */
	inline int getObjectType()const{ return __DAEP__Model__genus; }

COLLADA_(public) //OPERATORS	

	/**WARNING, EXPERIMENTAL
	 * This provides a shortcut into the more informational meta record.
	 *
	 * @warning THIS IS NON-TRANSITIVE. @c this!=operator->()->getModel().
	 * Usually it is. But @c domAny breaks this rule. It's unclear how to
	 * implement multiple schema versions under one C++ class. That could
	 * be an exception as well. That is an objective for 2017.
	 */
	inline const daeMetaObject *operator->()const{ return __DAEP__Model__meta; }

	/**EXPERIMENTAL
	 * Gets a reverse-iterator that is equal to 0 when it's reached its end.
	 * @note There is a gobal @c operator++ for working with @c daeFeatureID.
	 */
	inline operator daeFeatureID()const{ return (*this)->rbeginFeatureID(); }
};

/**EXPERIMENTAL, HOMELESS
 * @c daeProcessShare & XS::Schema share this base class.
 * This way @c XS::Schema's method names are able to use the XML nomenclature.
 */
class daeProcessShare_base : public DAEP::Make
{
COLLADA_(protected)
	/**
	 * This is the @c COLLADA::DOM_process_share ID.
	 */	
	size_t _ps;
	/**
	 * This points at @c COLLADA::DOM_process_share.
	 * @note The type is likely to change at some point.
	 */	
	daePShare *_tag;
	/**
	 * This is a singly-linked-list; ostensibly used
	 * by the destructor to delete the DAEP Models it has.
	 */	
	daeMetaObject *_deleteList;
	/**
	 * This is an empty model that generic objects may
	 * advertise in lieu of implementing a model, if they
	 * have no need for one.
	 * This has the side effect of making their model opaque
	 * to model-reflectors.
	 */
	DAEP::Model *_opaqueModel;
	 
	/** Implements "addModel" and "addElement". */
	COLLADA_DOM_LINKAGE daeModel &_addModel(int,daeFeatureID,int=0);

	/**
	 * Default Constructor
	 */
	daeProcessShare_base():_deleteList(),_opaqueModel()
	{
		_ps = DOM_process_share; _tag = &DOM_process_share; 
	}	

COLLADA_(public)
	/**
	 * Virtual Destructor
	 */
	virtual ~daeProcessShare_base()
	{
		//delete[] _opaqueModel; //It's among _deleteList.
		for(daeMetaObject *q,*p=_deleteList;p!=nullptr;p=q)
		{
			q = p->_deleteList; p->_destruct();			
			//The model's memory region begins at the earliest feature.
			operator delete((void*)&p->getModel()[p->_finalFeatureID]);		
		}
	}
};
/**EXPERIMENTAL, HOMELESS
 * COLLADA C++ Object originator information class.
 */
class daeProcessShare : public daeProcessShare_base
{
COLLADA_(private)

	friend class daeObject;
	/**
	 * @c daeObject::getDefaulProcessShare() Constructor
	 */
	COLLADA_DOM_LINKAGE static daeProcessShare &_default
	(size_t ps,daeProcessShare*(size_t,daePShare*)=_defaults);	
	static daeProcessShare *_defaults(size_t ps, daePShare *tag)
	{
		assert(tag!=nullptr); return new daeProcessShare(ps,tag);
	}	
	/**
	 * @c _default() using this constructor to capture the "vptr"
	 */
	daeProcessShare(size_t ps, daePShare *tag){ _ps = ps; _tag = tag; }

COLLADA_(public)
	/**
	 * Returns a shared model that presents as an empty object of
	 * @c daeObjectType::_ type.
	 * @see daeMetaSchema.cpp TU's definition.
	 */
	COLLADA_DOM_LINKAGE DAEP::Model &getOpaqueModel();

	template<int Features, class This, int N>
	/**
	 * This is for implementing "__DAEP__Object__v1__model." It works
	 * like @c XS::Schema::addElement() more or less.
	 */
	daeModel &addModel(This *toc, daeClientStringCP (&cplusplus_ident)[N])
	{
		const daeObject *upcast = toc; 
		daeFeatureID ff = (daeFeatureID)(Features*(Features<=0?1:-1));
		daeModel &o = _addModel(sizeof(This),ff); 
		return o.setName(cplusplus_ident);
	}
};
										     
/**
 * COLLADA C++ class that implements the new version 2.5 object model.
 *
 * @class COLLADA::daeObject
 *
 * There are precisely 3 data-members:
 * - _refs: ref counter. Interlocked (Atomic) functions are 32-bit only.
 * - _tags: four bytes, used to track various states; fills 32-bit hole.
 * - _containingObject: points upstream until an object contains itself.
 * @see @c DAEP::Object.
 *
 * __DAEP__Object__tags
 * ====================
 * This is a 32-bit field holding 4 @c daeTag. It corresponds to @c daeElementTags.
 * If @c daeTag is larger than 8 bits, this field must be larger to accommodate them.
 * - 1st tag is available for object-based classes to use anyway they like.
 * - 2nd tag is available for user-defined subclasses to use as they see fit.*
 * - 3rd tag is the virtual-table-layout version number. It is byte-addressable.
 * - 4th tag is the database-bit, preceding the 7-bit process-share's identifier.**
 * * Elements cannot be "user-defined." Tags 1,2&3 identify their namespace/prefix.
 * **The share cannot be separate from the tags, as the namespace is dependent on it.
 * @see @c daeElementTags.
 *
 * VIRTUAL INHERITANCE
 * ===================
 * @note Virtual inheritance is used only to
 * better ensure that the DAEP classes overlay
 * properly when they're @c reinterpret_cast to
 * their "daeX" counterparts. 
 * (This is because @c daeElement cannot inherit
 * from both @c DAEP::Element and @c daeObject.)
 */
class daeObject : public DAEP::Object
{	
	//Microsoft's Natvis can't seem to find namespaces???
	typedef DAEP::Object __super_natvis;

	template<class> friend class daeSmartRef;

	/**NOT-THREAD-SAFE
	 * Increments the reference count of this element.
	 * @note Should not be used externally if daeSmartRefs are being used, they call it
	 * automatically.
	 * TOWARD MULTI-THREAD
	 * ===================
	 * InterlockedIncrement or something else is needed here.
	 */
	inline void _ref()const{ if(this!=nullptr) __DAEP__Object__refs++; }

	/**NOT-THREAD-SAFE
	 * Decrements the reference count and deletes the object if reference count is zero.
	 * @note Should not be used externally if daeSmartRefs are being used, they call it
	 * automatically.
	 * TOWARD MULTI-THREAD
	 * ===================
	 * InterlockedDecrement or something else is needed here.
	 */
	inline void _release()const
	{
		//Notice. There are 3 types of references.
		//1: embedded. These never hit 0, and are very large numbers.
		//2: subobject. These are always less than 0. They require <=0 to be below.
		//3: regular. These are deleted when 0. Either by the database, or the DAEP Make object.

		if(this!=nullptr&&--__DAEP__Object__refs<=0) __DAEP__Object__0(); //_release2();
	}		

	/**
	 * Releases on the condition that the caller is the last remaining reference holder.
	 * This is used to "compact" @c daeDOM::_closedDocs; including any archives therein.
	 * It's exposed via @c daeSmartRef::operator~. Roughly the ~ represents destruction.
	 */
	inline bool _compact()const
	{
		//_release() checks for nullptr regardless, so do it here. (It'll be optimized.)
		//>1 is used instead of !=1 in order to ensure compaction. (Sub 1 won't delete.)
		if(this==nullptr||__DAEP__Object__refs>1) return false; _release(); return true;
	}
	
COLLADA_(public) //UNADVERTISED METHODS
	/**
	 * @return Returns @c true if @c this object doesn't self-destruct/delete.
	 * @note Objects are embedded by default. The only way to unembed them is
	 * to use @c __DAEP__Object__unembed(). Factory created objects are never
	 * embedded.
	 * EXPERIMENTAL
	 * Sub-objects, also called subordinates, are objects that are members of
	 * their parent object's data structure. Their ref-counter is always less
	 * than 0; meaning that decrementing the counter always triggers an event.
	 */
	inline bool _isEmbedded()const
	{
		daeCTC<sizeof(daeUInt)==sizeof(__DAEP__Object__refs)>();
		return (daeUInt)__DAEP__Object__refs>=__DAEP__Object__refs_embedding_threshold; 
	}

	/**
	 * This tag is free to be used by object-based classes.
	 * The class can also use @c (&_getClassTag())[1], or define a method that
	 * its user-defined subclasses may use. If such a method is not defined by
	 * the class, then it reserves the right to both tags.
	 */
	inline daeTag &_getClassTag(){ return (daeTag&)__DAEP__Object__tags; }
	/**CONST-FORM 
	 * This tag is available to sub-classes. 
	 * @see the non-const @c _getClassTag() Doxygentation. 
	 */
	inline const daeTag &_getClassTag()const{ return (daeTag&)__DAEP__Object__tags; }

	/** 
	 * @return Returns the virtual-method-tables' version.
	 * @note The version is incremented anytime any method is added to any object.
	 */
	inline daeTag _getVersion()const{ return ((daeTag*)&__DAEP__Object__tags)[2]; }

	/** ASSUMES 8-BIT TAGS
	 * @return Returns the process-share ID. This is limited to 7-bits, unless 
	 * the @c daeElementTag structure is extended to 64 bits (or greater if some
	 * systems are not byte-addressable.)
	 * The process-share corresponds to the user-executable or client-library that
	 * originates this object. If there are more than 7-bits worth, they must wait
	 * for a share to open up.
	 */
	inline daeUInt _getPShare()const
	{
		daeCTC<CHAR_BIT==8>(); return __DAEP__Object__tags>>25; 
	}
	/**
	 * This is offered for symmetry with @c getDefaultProcessShare().
	*/
	daeProcessShare &getProcessShare()const
	{
		return **__DAEP__Object__v1__model().__DAEP__Model__make;
	}
	/**WARNING
	 * Gets a @c daeProcessShare corresponding to @c _getPShare() that is stored
	 * in the library's module. The client module can host their own process-share
	 * objects, of which there may be many. There is only one default/system share.
	 * @warning THIS IS UNLIKELY THE @c daeProcessShare THIS OBJECT IS ASSIGNED TO.
	 * @see dae.cpp TU's definition.
	 */
	inline daeProcessShare &getDefaultProcessShare()const
	{
		return daeProcessShare::_default(_getPShare());
	}

	/**
	 * Used by @c daeElement::getElementTags() to do a compile-time-check. 
	 */
	enum{ _sizeof__DAEP__Object__tags=daeSizeOf(DAEP::Object,__DAEP__Object__tags) };

	/**CIRCULAR-DEPENDENCY
	 * @warning A @c daeDOM is never stored in a database.
	 * @return Returns the @c daeDatabase storing the object, or
	 * -returns @c nullptr if the object is not database stored.
	 * @see ColladaDOM.inl header's definition.
	 */
	inline daeDatabase *daeObject::_getDBase()const;
	//{
	//	return _isData()?&getDOM()->getDatabase():nullptr; 
	//}

	typedef int __isX_assume[8==CHAR_BIT];
	/**
	 * @return Returns @c true if this object belongs to a database. 
	 */
	inline bool _isData()const{ return 0x1000000==(__DAEP__Object__tags&0x1000000); }	
	/**
	 * @return Returns @c true if this object is a @c daeDOM object. 
	 * @note This is faster than accessing __DAEP__Model__genus, and is
	 * possible because DOM objects are managed by the library exclusively.
	 * @remarks This checks that the object is not data, and is using the 0th
	 * "process share", that belongs to the library, and uses 2 to identify DOMs.
	 */
	inline bool _isDOM()const{ return 0x0000200==(__DAEP__Object__tags&0xFF00FF00); }
	/**
	 * @return Returns @c true if this object is a @c daeDoc object;
	 * -note that @c daeDOM is a @c daeDoc, and that @c getObjectType()
	 * differs from @c _isDoc() in-so-far as it reports DOM, and not DOC!!
	 * 
	 * @note This is faster than accessing __DAEP__Model__genus, and is
	 * possible because Doc objects are managed by the library exclusively.
	 * @remarks Here a @c daeDocument is data, but currently other @c daeDoc
	 * are not data. All @c daeDoc belong to the 0th "process share', which is
	 * the library's share. The value 1 is used to identity Docs. Docs can be any
	 * of @c daeDocument or @c daeArchive. Hard/Symbolic links are a future feature.
	 * (that may or may never be.)
	 */
	inline bool _isDoc()const{ return 0x0000100==(__DAEP__Object__tags&0xFE00FF00); }	
	/**
	 * This got complicated when it was determined that @c domAny ill also
	 * process-share 0, and so have tags similar to @c _isDOM() & @c _isDoc().
	 * @remarks Later this will have some issues with @c daeElementTags::nameTag.
	 */
	inline bool _isAny()const{ return 0x1008000==(__DAEP__Object__tags&0xFF008000); }
	/**
	 * @return Returns @c _isDoc()||_isDOM(). Or if a non-element element container.
	 * @remarks This is the primary motivation for @c _isAny(). @c domAny is in the way.
	 */
	inline bool _isDoc_or_DOM()const{ return 0x1008000>__DAEP__Object__tags; }

	/**
	 * This makes @c a<daeElement>() easier read in keeping with @c isDOM() & @c isDoc().
	 */
	inline bool _isElement()const{ return getObjectType()>=daeObjectType::ANY; }	

COLLADA_(public) //OPERATORS

	COLLADA_DOM_OBJECT_OPERATORS(daeObject)
	
	/**
	 * Access the feature descriptor of @c this object's data members.
	 * @remarks This is chiefly to implement memory ownership semantics.
	 * It's a difficult subject.
	 * @c daeFeatureID is an @c int like enumerator, that is expected to
	 * be a negative compile-time-constant, but can be runtime if need be.
	 * The @c enum type is used so objects can overload @c operator[](int).
	 */
	inline const daeFeature &operator[](daeFeatureID ID)const
	{
		return __DAEP__Object__v1__model()[ID]; 
	}
	template<class T>
	/**
	 * Access the feature descriptor of @c this object's data members.
	 * @remarks This is chiefly to implement memory ownership semantics.
	 * It's a difficult subject.
	 */
	inline const daeFeature &operator[](const T *member)const
	{
		return __DAEP__Object__v1__model().__DAEP__Model__feature(this,member); 
	}

COLLADA_(public) //daeSafeCast() SHORTHANDS
	/**WORKAROUND
	 * This is now needed by all standard @c daeSmartRef classes since
	 * many templates use it to let smart-refs stand in for their type.	 
	 * (That allows consistent use of shorter WYSIWYG smart-ref names.)
	 */
	typedef daeObject __COLLADA__T;

	/** Follows style of daeElement::a(). */
	template<class T> T *a()
	{
		//Only the pass-through form below is required.
		//This is a start. Elements can be added later.
		return this!=nullptr&&_isDoc()?((daeDoc*)this)->a<T>():nullptr;
	}
	/** Follows style of daeElement::a(). */
	template<> daeDOM *a<daeDOM>()
	{
		return this!=nullptr&&_isDOM()?(daeDOM*)this:nullptr;
	}
	/** Follows style of daeElement::a(). */
	template<> daeDoc *a<daeDoc>()
	{
		return this!=nullptr&&_isDoc()?(daeDoc*)this:nullptr;
	}
	/** Follows style of daeElement::a(). */
	template<> daeElement *a<daeElement>()
	{
		return this!=nullptr&&_isElement()?(daeElement*)this:nullptr;
	}
	/** Pass-Through; Follows style of daeElement::a(). */
	template<> daeObject *a<daeObject>(){ return this; }	
	/**CONST-FORM Following style of daeElement::a(). */
	template<class T> const T *a()const
	{
		return const_cast<daeObject*>(this)->a<T>();
	}

COLLADA_(protected) //PROTECTED CONSTRUCTORS
	/**
	 * Non-Constructor 
	 * This is used to extract a "vptr" or prior to placement-new.
	 */
	daeObject(){ /*NOP*/ }
	/**
	 * Prototype Non-Constructor
	 * 
	 * It does install the "vptr" even though it's not necessary.
	 */
	explicit daeObject(const DAEP::Proto<> &pt):Object(pt){ /*NOP*/ }
	/**
	 * Constructor
	 *
	 * @param c can be @c this object. 
	 * Most code assumes the container is not @c nullptr.
	 */
	explicit daeObject(const DAEP::Object *c):DAEP::Object(c)
	{
		daeCTC<sizeof(daeObject)==sizeof(DAEP::Object)>();
	}

COLLADA_(private) //Reparenting APIs

	friend class daeContents_base;
	//COLLADA_DOM_LINKAGE
	/**
	 * This is a special implementation for elements only. Note, elements can
	 * only be parented to elements. In addition to being streamlined, this has
	 * to check if the parent is the "pseudo-element" and in that case, parent it
	 * to the document instead.
	 */
	daeOK _reparent_element_to_element(const daeObject &obj);

COLLADA_(protected) //Reparenting APIs

	friend class daeMetaElement;	
	/** @see dae.cpp TU's definition. 
	 * Sets obj->__DAEP__Object__parent to @c obj, 
	 * -or to @c this if @a obj is @c nullptr and @c !_isData().
	 * @return Returns @c daeError::DAE_OK if the operation is a success.
	 * @remarks @c _reparent() does not attempt to place 
	 * @a obj according to @a obj->getObjectType(). It merely does low-level
	 * generic notifications and checks against the database.
	 *
	 * @remarks Post-2.5 reparenting does double ref-counting via the parents.
	 * @c _reparent() calls @c _ref() and @c _release() somewhat like @c daeSmartRef,
	 * -except when @c isUnparentedObject().
	 * TOWARD MULTI-THREAD
	 * ===================
	 * The definition may need to use InterlockedExchange or something else.
	 */
	COLLADA_DOM_LINKAGE daeOK _reparent(const daeObject &obj);	
	/**
	 * @c daeMetaElement uses this to bootstrap its @c _prototype. 
	 * Use it if you dare.
	 * In a few places this is used to parent subobjects in constructors,
	 * -but a C++ exploit must be used, to cast the subobject to the parent's
	 * type, so that the @c protected keyword is in play.
	 */
	inline void _parent(DAEP::Object &p){ (__DAEP__Object__parent=&p)->__DAEP__Object__refs++; }

COLLADA_(public) //PUBLIC METHODS	
	/** 
	 * Gets @c __DAEP__Object__parent. 
	 * Use @c _reparent() to change the parent. Some restrictions apply.
	 */
	inline const daeObject &getParentObject()const
	{
		return *dae(__DAEP__Object__parent);
	}

	/**
	 * @return Returns the daeObjectType class's typeless enum.
	 * @remarks Synonymous with @c daeElement::getElementType().
	 */
	inline int getObjectType()const
	{
		return __DAEP__Object__v1__model().__DAEP__Model__genus; 
	}			 

	/**
	 * An object may contain itself. 
	 * A DOM type object MUST point to itself.
	 * The container cannot be nullptr. Watch out for infinite regress!
	 */
	bool isUnparentedObject()const{ return this==__DAEP__Object__parent; }	

	/**
	 * Looks for a self-contained container.
	 * @return Returns @c nullptr if a DOM type object is unavailable.
	 * TOWARD MULTI-THREAD
	 * ===================
	 * @remarks This is not returning @c daeSmartRef, as it's undefined,
	 * -but also because it's undesirable. It needs to be demonstrated that
	 * any ancestry will yield acceptable results.
	 * (This would require special care for stack born objects, and the DOM
	 * object cannot itself be stack born.)
	 */
	inline const daeDOM *getDOM()const
	{
		const DAEP::Object *p,*q = __DAEP__Object__parent;
		while(q!=(p=q->__DAEP__Object__parent)) q = p;
		return dae(p)->a<daeDOM>();
	}

	/**WARNING
	 * @warning This API returns a parent-object. It doesn't
	 * follow that the parent is a document, even though with
	 * elements, it should be, if nonzero; yet still, it isn't
	 * necessary that elements are among the documents' content.
	 * This is because it's often that reparenting gains nothing.
	 * @see @c daeElement::isContent().
	 * @see @c daeElement::getDocument().
	 *
	 * Looks for an object with a self-contained container.
	 * @return Returns @c nullptr if not a DOC type object.
	 * TOWARD MULTI-THREAD
	 * ===================
	 * @remarks This is not returning @c daeSmartRef, as it's undefined,
	 * -but also because it's undesirable. It needs to be demonstrated that
	 * any ancestry will yield acceptable results.
	 * (This would require special care for stack born objects, and the DOM
	 * object cannot itself be stack born.)
	 */
	inline const daeDoc *getDoc()const
	{	
		const DAEP::Object *o,*p,*q;
		o = q = this; p = __DAEP__Object__parent;
		while(q!=p){ o = q; q = p; p = p->__DAEP__Object__parent; }
		return ((daeObject*)o)->_isDoc()?(daeDoc*)o:nullptr;
	}
	
COLLADA_(public) //daeArray AU allocators

	#define _(f,T) \
	void(*f)(const daeObject*,daeAlloc<T>&,daeAlloc<T>&)
	template<class T> 
	/**
	 * @c reAlloc is called indirectly by @c daeArray.
	 * It could just as easily be a member of @c daeArray, 
	 * -however conceptually the authority is invested in the 
	 * object.
	 *
	 * @remarks @c this CAN be @nullptr. For purposes of 
	 * managing allocation-units, @c nullptr is a pseudo object.
	 */	
	inline void reAlloc(daeAlloc<T>* &au, size_t newT, _(fn,T))
	{
		_reAlloc<0>((daeAlloc<>*&)au,newT,(_(,))fn); //"loses qualifiers" 
	}	
	template<class T>
	/**WARNING
	 * ...In case loaders want to avoid redundant allocation overhead
	 * by tallying space-characters in parsing XML Schema style lists:
	 *
	 * @warning This is safe only if the AU is empty (a thunk) or if
	 * the the type is "raw-copyable" "plain-old-data" that can be moved.
	 * If non-empty it should not be attempted for non @c daeAtomicType types.
	 * @see @c daeAtomicType::unserialize().
	 */
	inline void reAlloc(daeDatabase &db, daeAlloc<T>* &au, size_t newT, _(fn,T)=nullptr)
	{
		return _reAlloc<1>((daeAlloc<>*&)au,newT,(_(,))fn,&db); //"loses qualifiers" 
	}
	template<int DB> //INTERNAL
	COLLADA_NOINLINE	
	/** Implements reAlloc(). */
	inline void _reAlloc(daeAlloc<>* &au, size_t newT, _(fn,), daeDatabase *db=nullptr)
	{	
		assert(newT>au->getCapacity()); //This is one-way.
		daeAlloc<> &recycling = *au;
		bool wasFree = daeAlloc<>::isFreeAU(au);
		if(!DB) db = this==nullptr?nullptr:_getDBase();	
		au = !DB&&db==nullptr?&au->newThis(newT):&db->_new(newT,*au,*this);
		assert(au!=&recycling);
		recycling.moveThunk(au,newT);
		//Copy/move the old memory over to the new memory?
		if(!DB||fn!=nullptr) fn(this,*au,recycling); 
		if(wasFree)
		if(!DB&&db==nullptr) 
		recycling.deleteThis();
		else db->_delete(recycling,*this);						
	}
	template<class T> 
	/** @see daeObject::reAlloc(). */
	inline void deAlloc(daeAlloc<T>* &au)
	{
		return _deAlloc((daeAlloc<>*&)au); //"loses qualifiers"
	}	
	/** Implements deAlloc(). */
	COLLADA_NOINLINE inline void _deAlloc(daeAlloc<>* &au)
	{
		if(!daeAlloc<>::isFreeAU(au)) return;
		daeDatabase *db = this==nullptr?nullptr:_getDBase();
		return db!=nullptr?db->_delete(*au,*this):au->deleteThis();
	}
	#undef _

COLLADA_(public) //LEGACY SUPPORT
	
 //Below are notes describing these APIs from test/integrationExample.cpp:
//The DOM used to provide an "integration library", which was a mechanism for
//converting the DOM's representation of a Collada model to the user's representation.
//The integration classes were very clumsy and not particularly useful, so they
//were removed in December 07. In their place, setUserData and getUserData methods
//were added to the daeElement class. This program shows how you might write a Collada
//importer using these new methods instead of the integration classes.

	#ifdef NDEBUG
	#error destructors should provide a user-callback.
	#error maybe daeDatabase::setUserDataDestructorCB?
	#endif
	//Pre-2.5 there was simply a _userData data member.
	//The data was not delete'd by ~daeElement. So this
	//implementation does not delete it. It goes through
	//the database now. The built-in DB does support this.
	//Still, since 2.5 it's very easy to build onto the DB.
	typedef COLLADA_DOM_USERDATA *userptr;	
	/**LEGACY
	 * Sets the user data pointer attached to this element.
	 * @param data User's custom data to store.	 
	 * @see test/integrationExample.cpp
	 */
	inline bool setUserData(userptr ud) //const
	{
		userptr *udb = getUserDB(); if(udb==nullptr) return false;
		*udb = ud; return true;
	}
	/**LEGACY
	 * Gets the user data pointer attached to this element.
	 * @return User data pointer set with @c setUserData().
	 * @see test/integrationExample.cpp
	 */
	inline userptr getUserData() //const
	{
		userptr *udb = getUserDB(); assert(udb!=nullptr);		
		return udb==nullptr?nullptr:*udb;
	}	
	/**LEGACY-SUPPORT 
	 * @c _isData()?effectively restricts this version
	 * to elements and documents allocated by a database, 
	 * and one that provides a @c userptr pointer slot.
	 * @see test/integrationExample.cpp
	 */
	inline userptr *getUserDB()const
	{
		return _isData()?_getDBase()->_userptrptr(*this):nullptr;
	}
}; 

//---.
}//<-'

#endif //__COLLADA_DOM__DAE_OBJECT_H__
/*C1071*/