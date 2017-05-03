/*
 * Copyright 2006 Sony Computer Entertainment Inc.
 *
 * Licensed under the MIT Open Source License, for details please see license.txt or the website
 * http://www.opensource.org/licenses/mit-license.php
 *
 */

#ifndef __COLLADA_DOM__BUILT_IN_DBASES_H__
#define __COLLADA_DOM__BUILT_IN_DBASES_H__

#include "../dae/daeElement.h"

COLLADA_(namespace)
{//-.
//<-'

/**
 * The @c daeLegacyDatabase implements a @c new/delete based @c daeDatabase.
 */
class daeLegacyDBase
{	
COLLADA_(public) //DATA MEMBERS
	
	//MEMORY LEAK DETECTORS
	int _outstanding_objects;
	int _outstanding_elements;
	int _outstanding_documents;
	int _outstanding_allocations;

	//LEGACY RESOLVER CACHE
	daeContainerObject<2> _cache;

COLLADA_(public) //CONSTRUCTOR
	/**
	 * Default Constructor
	 */
	daeLegacyDBase():
	_outstanding_objects(),
	_outstanding_elements(),
	_outstanding_documents(),
	_outstanding_allocations(){}
	/**
	 * Destructor
	 */
	~daeLegacyDBase()
	{
		assert(0==_outstanding_objects);
		assert(0==_outstanding_elements);
		assert(0==_outstanding_documents);
		assert(0==_outstanding_allocations);
	}
	
COLLADA_(public)
	/**
	 * @c DataBase has to be constructed separately.
	 */
	struct DataBase
	{
		template<class T>
		//Assuming T matches!
		DataBase(T*):userptr()
		{
			daeDBaseOK(T,object);
		}
		daeObject::userptr userptr;
	};	
	template<class T> 
	/**VARIABLE-LENGTH 
	 * @c DataBase has to be constructed separately.
	 * @a T was to be a base class so C++'s casting 
	 * could be used, but that turned out to not be
	 * feasible, because multiple-inheritance isn't
	 * concerned with the memory layout, and T must
	 * be the back of the variable length structure.
	 */
	struct UserObject : DataBase
	{
		/**VARIABLE-LENGTH
		 * UNFORTUNATELY THIS CANNOT BE A BASE CLASS BECAUSE
		 * C++ DOESN'T DEFINE THE LAYOUT OF TWO BASE CLASSES.		
		 */
		T object;
	};
	typedef UserObject<daeObject> Object;
	typedef UserObject<daeElement> Element;
	typedef UserObject<daeDocument> Document;
	/**
	 * COLLADA-DOM 2.5 Database Traits
	 *
	 * <> is just naked types without database added members & methods.
	 * - elements will need a userDB pointer.
	 * - elements need a linked-list pointer for dismantling memory leaks.
	 */
	typedef daeDBaseTraits<Document,Element,daeAlloc<>,Object> _DBaseTraits;						

COLLADA_(public) //daeDB methods

	void **_userptrptr(const Object &obj)
	{
		return (void**)&obj.DataBase::userptr;
	}
	void _ciao(daeDOM &DOM, daeDOM* &rejoinder)
	{
		rejoinder = DOM; _cache.clear();		
	}	
	daeString _ref(daeString string, size_t extent, daeDOM*&)
	{
		//Just using the system-pool.
		//This is a starter-database, likely built into the library.
		new(&string) daeStringRef(string,extent); return string;
	}	
	daeString _prototype(const daeStringRef &NOP, daeDOM*&)
	{
		//Just using the system-pool.
		//This is a starter-database, likely built into the library.
		return NOP;
	}	
	void _constructing(size_t chars, Object* &obj, daeDOM*&)
	{
		(void*&)obj = operator new(chars);		
		new(obj) DataBase(obj); _outstanding_objects++;
	}	
	void _constructing(size_t chars, Element* &elem, daeDOM*&, const daeElement &prototype)
	{
		(void*&)elem = operator new(chars); 		
		new(elem) DataBase(elem); _outstanding_elements++;
	}	
	void _constructing(size_t chars, Document* &doc, daeDOM*&)
	{
		(void*&)doc = operator new(chars);		
		new(doc) DataBase(doc); _outstanding_documents++;
	}		
	template<class T>
	void _destructing(T &obj)
	{
		//test/integrationExample.cpp required the pointers
		//to be removed prior to tearing down the documents.
		assert(nullptr==obj.DataBase::userptr);
	}
	void _destructed(Object &obj)
	{
		delete (void*)&obj; _outstanding_objects--;
	}
	void _destructed(Element &elem)
	{
		delete (void*)&elem; _outstanding_elements--;
	}
	void _destructed(Document &doc)
	{
		delete (void*)&doc; _outstanding_documents--;
	}
	void _deallocating(daeAlloc<> &AU, const Object &obj)
	{
		delete (void*)&AU; _outstanding_allocations--;
	}
	daeAlloc<> &_reallocating(size_t newT, const daeAlloc<> &AU, const daeObject &obj)
	{
		_outstanding_allocations++;
		return daeOpaque(operator new(&AU.getRaw(newT)-&daeOpaque(&AU)));
	}	
	bool _atomize_on_noting(daeContainedObject &cache_object)
	{
		_cache._getClassTag() = 1; _cache.contain(cache_object); return true;
	}
	void _noting(const Element &elem, const DAEP::Change &note, daeAttribute *attrib)
	{	
		//SEE _atomize_on_noting().
		if(0!=_cache._getClassTag()) //NOT THREAD-SAFE
		{
			_cache._getClassTag() = 0; _cache.atomizeContained();			
		}

	  ///////////////////////////////////////////////////////////////////////////////////
	  //NOTE: THIS ISN'T RESPONSIBLE FOR daeDocument::typeLookup(). THE GOAL IS TO KEEP//
	  //THAT SEPARATE, MAINLY BECAUSE IT SHOULD WORK WITH OR WITHOUT THE CHANGE-NOTICES//
	  ///////////////////////////////////////////////////////////////////////////////////

		_migrate_operation op; //C++98/03 support

		op.first = elem.object.getDocument(); //source

		if(DAEP::ATTRIBUTE==note.kind_of_change)
		{
			//Theoretically the client-side code is slightly more optimal.
			if(op.first==nullptr) 
			return;
			if(attrib->getThisID()!=nullptr)
			{
				if("id"==attrib->getName()
				||"sid"==attrib->getName())
				{
					op.first->_carry_out_change_of_ID_or_SID(note,attrib);			
					return; //_carry_out_change_of_ID_or_SID() has it now.
				}
			}
		}

		note.carry_out_change(); //Was the document changed?

		if(DAEP::ELEMENT==note.kind_of_change)
		{
			op.second = elem.object.getDocument(); //destination

			//If both documents are the same, it shouldn't effect the legacy lookups.
			//(Note, much more likely is inserting from "no" document and vice-versa.)
			if(op.first!=op.second)	op(elem.object);
		}
	}
	struct _migrate_operation //C++98/03 support
	:
	std::pair<const daeDocument*,const daeDocument*>
	{
		void operator()(const daeElement *ch)
		{
			if(ch->getOptimizationBit_has_registered_ID_attribute())
			{
				#ifdef NDEBUG
				#error What about <xs:anyAttribute>?
				#endif
				daeAttribute *ID;
				ID = ch->getMeta()->getFirstID();
				for(;ID!=nullptr;ID=ID->getNextID())
				{
					if("id"==ID->getName()
					||"sid"==ID->getName())
					first->_migrate_ID_or_SID(second,ch,ID); //source->destination
				}
			}
			if(!ch->getOptimizationBit_is_definitely_not_a_graph()) //Recurse?
			{
				ch->getContents().for_each_child(*this);
			}
		}
	};
};

//---.
}//<-'

#endif //__COLLADA_DOM__BUILT_IN_DBASES_H__
/*C1071*/