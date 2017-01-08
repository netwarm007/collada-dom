/*
 * Copyright 2006 Sony Computer Entertainment Inc.
 *
 * Licensed under the MIT Open Source License, for details please see license.txt or the website
 * http://www.opensource.org/licenses/mit-license.php
 *
 */

#ifndef __COLLADA_DOM__DAE_SID_RESOLVER_H__
#define __COLLADA_DOM__DAE_SID_RESOLVER_H__

#include "daeIDRef.h"

////ISSUES/////////////////////////////////////////
//daeIDREF and daeSIDREF are virtually identical,//
//and should be maintained via a common template.//
///////////////////////////////////////////////////

COLLADA_(namespace)
{//-.
//<-'

template<int size_on_stack> class daeSIDREF_size;

/**LEGACY
 * The daeSIDREF class is designed to resolve SID references within a COLLADA document.
 * The rules for SID resolution are set forth by the Addressing Syntax section in Chapter 3 of the
 * COLLADA specification which can be found at https: //www.khronos.org/collada .
 */
typedef daeSIDREF_size<260> daeSIDREF;
typedef daeSmartRef<daeSIDREF> daeSIDREFRef;
typedef daeSmartRef<const daeSIDREF> const_daeSIDREFRef;
#ifndef COLLADA_NODEPRECATED
COLLADA_DEPRECATED("daeSIDREF")
typedef daeSIDREF daeSidRef;
COLLADA_DEPRECATED("daeSIDREF")
typedef daeSIDREF daeSIDResolver; //was never a resolver
#endif

template<int size_on_stack> 
/**VARIABLE-LENGTH
 * @c daeSIDREF is a fixed size @c typedef of @c daeSIDREF_size.
 * @tparam size_on_stack can be any size; 0 is fully dynamic.
 * (UPDATE: 0 is not fully dynamic, although this might change.)
 */
class daeSIDREF_size : public daeIDREF_base<daeSIDREF>
{
	friend class daeIDREF_base<daeSIDREF_size>;

COLLADA_(protected) //daeIDREF_base DATA

	daeRefString<size_on_stack> _refString;

COLLADA_(public) //daeIDREF_base methods

	/** Statically shadows @c daeRef::getRefType(). */
	static int getRefType(){ return daeRefType::SIDREF; }

COLLADA_(public) //OPERATORS

	//using daeIDREF_base<daeSIDREF>::operator=;
	template<class T>
	COLLADA_SUPPRESS_C(4522)
	/** Pass-through Assignment Operator */
	daeSIDREF &operator=(const T &cp){ return daeIDREF_base::operator=(cp); }	
	COLLADA_SUPPRESS_C(4522)
	/**C++ Non-Default Assignment Operator */
	daeSIDREF &operator=(const daeSIDREF_size &cp){ return daeIDREF_base::operator=(cp); }	

COLLADA_(public) //FORWARDING CONSTRUCTORS
	/**
	 * Default Constructor
	 */
	daeSIDREF_size(){}	
	/**
	 * Non-Default Copy Constructor
	 * @param cp @c cp to copy into this one.
	 */
	daeSIDREF_size(const daeSIDREF_size &cp):daeIDREF_base(cp){}

	//Marking explcit, just to be safe.
	template<class S>
	/** Single Argument Forwarded Constructors */
	explicit daeSIDREF_size(const S &s):daeIDREF_base(s){}

	template<class S, class T>
	/** Two Argument Forwarded Constructors */
	daeSIDREF_size(const S &s, const T &t):daeIDREF_base(s,t){}

COLLADA_(public) //METHODS NAMED AFTER SIDREF
	
	template<class T>
	/**
	 * Copies @a SID into the  @c _refString data member.
	 * @param ID String to use to configure this @c daeSIDREF.
	 */
	inline void setSIDREF(const T &ref)
	{
		_refString.setString(*this,daeHashString(ref)); 
	}			
	template<class T> 
	/**
	 * Gets the entire SIDREF to a @a T. 
	 * @see no-argument form's Doxygentation.
	 */
	inline T &getSIDREF(T &io)const
	{
		daeString s = _refString.getString();
		return _getT(io,s,strlen(s)+1); 
	}	
	/**LEGACY-SUPPORT
	 * Gets the SIDREF string consistent with @c daeURI::getURI().
	 * @see @c data() to get a C-string.
	 * @remark @c daeIDREF and @c daeSIDREF don't store the size
	 * of their strings. That may change if there is an argument
	 * to. This API calls @c strlen(). Use @c data() to avoid it.
	 * Optimization will likely eliminate the length if not used.
	 */	
	inline daeRefView_0 getSIDREF()const
	{
		daeRefView_0 o; o.view = _refString.getString();
		o.extent = strlen(o.view); return o;
	} 		  	

COLLADA_(public) //SIDREF HELP
	/**
	 * This is the first part of a SIDREF that begins with "./".
	 * If @c true there was a ./ extracted. 
	 * @see @c getSIDREF_separated().
	 */
	enum DotSlash{};
	/**
	 * This is a helper class that gets the length of the SIDREF after 
	 * removing the member-selector business off its back. The COLLADA
	 * documentation loosely defines this. 
	 * @see @c getSIDREF_separated().
	 */
	struct Terminator : daeRefView
	{
		/**
		 * Default Constructor
		 */
		Terminator(){}
		/**
		 * Explicit Constructor
		 */
		explicit Terminator(daeString SIDREF){ operator()(SIDREF); }
		/**
		 * @param SIDREF can be anywhere inside the 0-terminated string.
		 * It's better if it's after the first / so there's no chance of
		 * confusing the initial ID for a SID.
		 */
		void operator()(daeString SIDREF)
		{
			size_t len = strlen(SIDREF);

			daeString p = SIDREF+len;

			while(p!=SIDREF)
			{
				if(p[-1]==')')
				{
					//THERE CAN BE TWO OF THESE, BUT
					//COLLADA SHOULD DEPRECATE DOUBLE
					//NOTATION! PERHAPS GRANDFATHERING
					//IN ONLY <matrix> AND <lookat>?
					for(p--;p!=SIDREF&&*p!='(';p--);
					continue;
				}
				switch(p[-1])
				{
				case 'E':

					if(len>7)
					if(p[-6]=='.')
					{
						if(p[-5]=='A')
						if(p[-4]=='N')
						if(p[-3]=='G')
						if(p[-2]=='L')
					//	if(p[-1]=='E')
						p-=6;
					}
					break; 

				case 'X': case 'R': case 'U': case 'S':					
				case 'Y': case 'G': case 'V': case 'T':					
				case 'Z': case 'B': case 'P': 
				case 'W': case 'A': case 'Q':

					if('.'==p[-2]) p-=2;					
				}; 
				break;
			}
			view = p; extent = SIDREF+len-p;
		}		

		/**WARNING
		 * Gets an array index based on the member-selection
		 * part of the SIDREF.
		 * @param NCName can be either "matrix" or "lookat"
		 * to deal with the double (#)(#) notation, which is
		 * recommended for deprecation, because determining 
		 * widths is unsound.
		 * @warning Call this only if @c empty()==false. It
		 * cannot return a meaningful value otherwise, and is
		 * setup to trigger an @c assert if the string isn't
		 * well-formed.
		 */
		size_t select(const daeName &NCName="")const
		{
			if('.'==view[0])
			{
				if(extent==2) switch(view[1])
				{
				case 'X': case 'R': case 'U': case 'S': return 0;
				case 'Y': case 'G': case 'V': case 'T': return 1;
				case 'Z': case 'B': case 'P': return 2;
				case 'W': case 'A': case 'Q': return 3;
				}
				else if(extent==6)
				{
					assert(0==strcmp(view,"ANGLE")); return 3;
				}
			}
			else if('('==view[0])
			{
				daeStringCP *e;
				size_t out = strtol(view+1,&e,10);
				if(*e==')')
				{
					if(e[1]!='(')
					{
						assert(e[3]=='\0');
						return out;
					}
					else if(e[3]==')')
					{
						//COLLADA REALLY SHOULDN'T SUPPORT
						//THIS IN THE FUTURE, AND THESE ARE
						//THGE ONLY TWO THAT MAYBE SHOULD BE
						//"GRANDFATHERED-IN."
						size_t digit = e[2]-'0';
						assert(isdigit(digit)&&e[4]=='\0');						
						switch(NCName[0])
						{
						case 'l': assert("lookat"==NCName);
							out*=3; out+=digit; return out;
						case 'm': assert("matrix"==NCName);
							out*=4; out+=digit; return out;
						}						
					}
				}
			}
			assert(0); return -1;
		}
	};
	template<class S, class T>
	/**WARNING
	 * This API returns @c true if the SIDREF begins with ./ and gathers
	 * the segments in the form of an @a S @c daeArray comprised of types
	 * that must understand an un-0-terminated @ c daeHashString.
	 *
	 * @tparam T can be a replacement for @c daeSIDREF::Terminator that expands
	 * the member selector names it is able to support.
	 */
	inline DotSlash getSIDREF_separated(daeArray<S> &NCNames, T &terminate)const
	{
		NCNames.clear();
		daeString p = data(), q = p;
		while(*q!='\0'&&*q!='/') q++;
		//Technically "." will satisfy this, but "..A" won't. If possible
		//this implementation should NOT consider denerate ID-SID SIDREFs.
		DotSlash out =  (DotSlash)(q-p==1&&*p=='.'?1:0); 
		//Don't store the "." string as S may be daeStringRef.
		if(out) p = ++q;
		//terminating after the first slash avoids the initial ID segment.
		terminate(q);
		daeString d = terminate.data();
		while(q<d) if(*q++=='/')
		{
			daeHashString s(p,q-p-1); 
			p = q; NCNames.push_back(s); 
			assert(NCNames.back().size()==s.size());
		}
		return out;
	}
};

/**LEGACY, NOT-RECOMMENDED
 * @see daeDefaultSIDREFResolver::cache.
 * "A simple class to make speed up the process of resolving the SIDREFs.
 * The result of the resolve is cached for future use.
 * This is meant for DOM internal use only."
 */
class daeSIDREFCache : public daeContainedObject
{
COLLADA_(protected) //daeContainedObject method

	/**PURE-OVERRIDE
	 * Initiates "teardown" sequence. 
	 */
	virtual void __daeContainedObject__v1__atomize(){ clear(); }

COLLADA_(public)

	//Note: the first string is the "profile".
	typedef std::pair<daeStringRef,std::string> Key;
	typedef std::pair<daeStringRef,daeString> LesserKey;	

	inline daeElementRef lookup(const LesserKey &k)
	{
		//Markus Barnes thinks relative SIDs are uncommon,
		//so this implementation isn't trying to cache them.
		//(The source element would need to be cached somehow.)
		if('.'==k.second[0]) return nullptr; //miss
		Table::iterator it = _lookupTable.find(k);
		if(it==_lookupTable.end()) return nullptr; //miss		
		return (daeElementRef&)it->second; //hit
	}

	inline void add(const LesserKey &key, const daeObjectRef &elt)
	{
		daeDatabase *db = elt->_getDBase();
		if(db!=nullptr&&db->cacheResolverResult(*this))
		_lookupTable[key] = elt; 
	}

	inline void clear(){ _lookupTable.clear(); }

COLLADA_(private) //DATA-MEMBER

	struct Less //: std::less<Key> //Not leaving up to chance:
	{	
		//Note: without this, LesserKey may convert into a Key.
		//Note: its not clear if std::string always has an < operator. Via traits?		
		inline bool operator()(const Key &a, const Key &b)const
		{
			return a.first!=b.first?a.first<b.first:strcmp(a.second.c_str(),b.second.c_str())<0;
		}

		typedef int is_transparent; //C++14: apparently this is not done by default [WG21 N3657]

		inline bool operator()(const LesserKey &a, const Key &b)const
		{
			return a.first!=b.first?a.first<b.first:strcmp(a.second,b.second.c_str())<0;
		}
		//just to be safe, define in both directions.
		inline bool operator()(const Key &a, const LesserKey &b)const
		{
			return a.first!=b.first?a.first<b.first:strcmp(a.second.c_str(),b.second)<0;
		}
	};
	
	typedef std::map<Key,daeObjectRef,Less> Table; 
	
	Table _lookupTable; 
};

/**WARNING, LEGACY-SUPPORT
 * @warning Pre-2.5 SIDREF support was either experimental or it had
 * been disabled. Nevertheless, there was a a lot of code just lying
 * around. This resolver is what remains of that code. 
 * It depends on @c daeDocument::sidLookup(), etc. It won't work if 
 * the database doesn't forward change notifications to the document.
 * This is by design. The core library isn't the ideal place for it.
 * THERE HAS ALSO been some changes. Some behavior that didn't agree
 * with the COLLADA specification had to be removed. Legacy code may 
 * no longer work. If SIDREF syntax is correct there isn't a problem.
 * @see daeDocument::_carry_out_change_of_ID_or_SID().
 *
 * The @c daeDefaultSIDREFResolver resolves a @c daeSIDREF by way of
 * the @c daeDocument class's indexing-services. If SID indexing is 
 * not yet enabled, it will be, in which case, the document will be
 * immediately scanned for all SIDs (Note: COLLADA invented SIDs; or
 * it seems to be so. Other XSD files may use their own SID concept.
 * In which case, this resolver may be unfit for those other SIDs.)
 * It is a concrete implementation of @c daeRefResolver.
 */
class daeDefaultSIDREFResolver : public daeRefResolver
{
COLLADA_(protected) //daeContainedObject method

	/**PURE-OVERRIDE*/
	virtual void __daeContainedObject__v1__atomize(){ _cache.clear(); }

COLLADA_(public) //public daeRefResolver methods

	/** Used by @c daeRefResolver::is(). */		
	static daeClientString getRefResolverNameString()
	{
		return "Default SIDREF Resolver (daeDefaultSIDREFResolver)"; 
	}
	/**PURE-OVERRIDE */
	virtual daeClientString getName()const{ return getRefResolverNameString(); }

COLLADA_(public) 
	/**
	 * Default Constructor
	 */
	daeDefaultSIDREFResolver():daeRefResolver(daeRefType::SIDREF)
	{
		_cache.__DAEP__Object__embed_subobject(this);
	} 

	/** 
	 * This is making use of the resolver, independently of 
	 * @c RefResolverList. (It uses @c _resolve() instead.)
	 */
	inline daeOK resolve(const daeSIDREF &sidref, daeRefRequest &req)const
	{
		return _resolve(sidref,req);
	}

COLLADA_(protected) //daeRefResolver::_resolve

	virtual daeOK _resolve(const daeRef &ref, daeRefRequest &req)const
	{
		const_daeDocRef doc;
		const daeSIDREF &sidref = (daeSIDREF&)ref;		

		//COLLADA_SUPPRESS_C(4144)
		switch(req.object!=nullptr?1:0)
		{
		case 1: doc = req.object->getDoc();

			if(doc!=nullptr) 
			{
				if(!sidref.getIsDocumentAgnostic())
				{
					const daeDoc *cmp = ref.getDoc();

					if(cmp!=nullptr&&doc!=cmp) return DAE_ERR_QUERY_SYNTAX;
				}
				break;
			}
			//break; //falling-through!

		case 0:	doc = ref.getDoc();

			if(doc==nullptr) return DAE_ERR_INVALID_CALL;
		}		

		daeString SIDREF = sidref.data();
		daeSIDREFCache::LesserKey key(daeStringRef(*doc,req),SIDREF);
		daeElementRef hit = _cache.lookup(key);
		if(SIDREF[0]=='.'&&hit!=&ref.getParentObject()) hit = nullptr;
		daeError out = _resolve_exported(hit,doc,sidref,req);
		if(out==DAE_OK&&hit!=req.object) _cache.add(key,req.object); return out;
	}

	COLLADA_DOM_LINKAGE
	/**
	 * The exported implementation of _resolve(). 
	 * @note It's important that the cache is not visible to @c _resolve_exported(),
	 * -because the STL makes no ABI guarantees.
	 * @see daeSIDResolver.cpp TU's implementation.
	 */
	daeOK _resolve_exported(const daeElementRef&,const const_daeDocRef&,const daeSIDREF&,daeRefRequest&)const;

COLLADA_(private) //DATA-MEMBER
	//SUB-OBJECT
	/** 
	 * Transplant from daeDOM/previously DAE. 
	 */
	mutable daeSIDREFCache _cache;
};

//---.
}//<-'

#endif //__COLLADA_DOM__DAE_SID_RESOLVER_H__
/*C1071*/