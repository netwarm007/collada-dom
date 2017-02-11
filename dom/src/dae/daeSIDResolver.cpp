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

typedef struct //daeSIDResolver_cpp
{	
	//These were arguments to the find() APIs.
	daeHashString profile; const_daeElementRef scope; const_daeDocumentRef lookup;	

	//NEW: Having this buffer as a member lets consecutive find() calls reuse it.
	daeArray<daeElementRef,30> _sidElements;
			
	//Implements a breadth-first SID search by using the database to find all elements
	//matching 'SID', then finding the element closest to 'scope'.		
	bool relocate_to(const daeStringRef &SID)
	{
		const daeName &name = scope->getNCName(); //Follow URL?
		if(name[0]=='i'&&name.extent>9&&0==memcmp(&name,"instance_",9))
		{
			#ifdef NDEBUG
			#error Try to use getTypeID() to verify it's a daeURI type?
			#endif
			const daeArray<daeStringCP> &url = scope->getAttribute("url");			
			if(!url.empty())
			{
				daeURI_parser URI(*scope,url);
				daeElementRef e = URI.getTargetedFragment();
				if(e!=nullptr)
				{
					//Found the URL: Now resolve the leftover part.
					const_daeDocumentRef e_doc = e->getDocument();
					if(_find(e,e_doc,SID))
					{
						scope = e; lookup = e_doc; return true;
					}
				}
			}
			#ifdef NDEBUG
			#error SHOULD THIS return? IS THERE ANYTHING INSIDE AN instance_ ELEMENT?
			#endif
		}
		return _find(scope,lookup,SID);
	}
	bool _find(const_daeElementRef &in_out, const daeDocument *look_in, const daeStringRef &SID)
	{
		//Get the elements with matching SIDs. 		
		look_in->sidLookup(SID,_sidElements);

		//Compute the distance from each matching element to the container element.
		unsigned minDistance = UINT_MAX;
		daeElement *closestElement = nullptr;
		for(size_t i=0;i<_sidElements.size();i++)
		{
			unsigned int distance = _computeDistance(in_out,_sidElements[i]);
			if(distance<minDistance)
			{
				minDistance = distance;	closestElement = _sidElements[i];
			}
		}

		in_out = closestElement; return in_out!=nullptr;
	}
	//Returns the distance between an element and an ancestor of the element. If 'c'
	//isn't an ancestor of 'e', or if 'e' is in a profile that doesn't match 'profile'
	//UINT_MAX is returned.
	unsigned _computeDistance(const daeElement *c, const daeElement *e)
	{
		if(c==nullptr||e==nullptr) return UINT_MAX;

		unsigned distance = 0; do
		{
			//Bail if we're looking for an element in a different profile.
			//Pre-2.5 this worked differently, but the schema is ambiguous
			//if it is not interpreted this way.
			if("technique_common"==e->getNCName()&&!profile.empty()
			 ||"technique"==e->getNCName()&&profile!=e->getAttribute("profile"))
			return UINT_MAX;			
			if(e==c) return distance; distance++;

		}while(nullptr!=(e=e->getParentElement())); return UINT_MAX;
	}		

}daeSIDResolver_cpp;

daeOK daeDefaultSIDREFResolver::_resolve_exported
(const daeElementRef &hit, const const_daeDocRef &doc, const daeSIDREF &ref, daeRefRequest &req)const
{
	daeSIDREF::Terminator t;
	if(hit!=nullptr) //In cache?
	{
		t(ref.data()); req.object = hit; goto hit;
	}
	else if(!ref.empty()) //SCOPING FOR goto hit;
	{	
		daeSIDResolver_cpp cpp;
		cpp.profile = req;
		cpp.lookup = doc->getDocument();
		const_daeDOMRef DOM = doc->getDOM();		
		daeSIDREF::DotSlash dot_slashed;
		daeArray<daeStringRef,16> NCNames(DOM);
		dot_slashed = ref.getSIDREF_separated(NCNames,t);
		daeStringRef *it = NCNames.begin();
		if(dot_slashed)
		{
			cpp.scope = ref.getElementObject();			
		}
		else if(!NCNames.empty())
		{
			//https://www.khronos.org/bugzilla/show_bug.cgi?id=1924
			//proposes letting / select the top/root fragment.
			if((it++)->empty()
			||nullptr==cpp.lookup->idLookup(it[-1],cpp.scope))
			{								
				/*THIS SEEMS AMBIGUOUS daeElement::sidLookup
				//IS A BETTER FIT FOR THESE KINDS OF LOOKUPS.
				//This does not comport with the manual, but
				//many examples in the manual, and in the CTS
				//examples omit the ID.
				//UPDATE: <texture texture> must work this way
				//because its type is NCName. But then it can't
				//be a SIDREF. The schemas are very inconsistent.
				cpp.scope = cpp.lookup->getRoot();*/
			}
		}
		if(cpp.scope==nullptr)
		return DAE_ERR_QUERY_NO_MATCH;

		//Find the element matching each SID.
		while(it<NCNames.end())
		if(!cpp.relocate_to(*it++))
		return DAE_ERR_QUERY_NO_MATCH;

		//SID resolution was successful!
		(const_daeElementRef&)req.object = cpp.scope;
	}
	else return DAE_ERR_QUERY_NO_MATCH;

hit: //fill out the request object
	const daeElement &e = (daeElement&)*req.object;
	daeCharData *cd = e.getCharDataObject();
	if(cd!=nullptr)
	{
		size_t n = 0;
		req.type = &cd->getType(); 		
		req.typeInstance = cd->getWRT(&e);
		if(cd->isArrayValue())
		{
			const daeAlloc<> *AU =
			(daeAlloc<>*const&)req.typeInstance;
			n = AU->getCount();
			req.typeInstance = AU->getRaw();
		}
		else switch(req.type->writer->getAtomicType())
		{
		case daeAtomicType::TOKEN: case daeAtomicType::STRING:
			
			n = ((daeStringRef&)req.typeInstance).size(); break;

		default: n = 1;	break;
		}

		//No type instance is returned for empty arrays
		//nor empty selections, including empty strings.
		//This resolver isn't used for write operations.
		if(t.empty()&&n!=0)
		{
			req.rangeMin = 0; req.rangeMax = n-1;
		}
		else
		{
			//Note: strings are treated as space separated
			//lists in terms of their selection. The string
			//is nullified only if the selection is past the
			//last codepoint. The caller must parse the lists.

			size_t m = t.select(e.getNCName());
			if(m<n)
			{
				req.rangeMin = req.rangeMax = m;
			}
			else req.typeInstance = nullptr;
		}						
	}
	else req.typeInstance = nullptr; return DAE_OK;
}

//---.
}//<-'

/*C1071*/