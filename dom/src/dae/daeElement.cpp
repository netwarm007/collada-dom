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

void DAEP::Element::__COLLADA__atomize()
{	
	daeElement &_this = dae(*this);
	const daeDocument *doc = _this.getDocument(); 
	daeMeta &meta = _this.getMeta();			
	_this.__clear2(meta.getContentsWRT(&_this),meta,doc);
	_this.__atomize2(meta);	
}
inline void daeElement::__atomize2(daeMeta &meta)
{
	//If this is called, it should follow the element has been removed.
	assert(!isContent());		
	assert(!meta.hasObjectsEmbedded()); //__DAEP__Object__0 assumes so.
	if(!meta.hasFeaturesWithAtomize())
	return;
	DAEP::Model &model = meta.getModel();
	for(daeFeatureID f=meta._feature_completeID;f!=0;f++) model[f].atomizeWRT(this);	
}	
void daeElement::__clear2(daeContents &content, daeMeta &meta, const daeDocument *doc)
{	
	if(content.empty()) return;

	//NOTE: This kind of feels like a "clearContentsWRT" API, but it's 
	//implemented as a daeElement method because the contents array is
	//used to call it, or can be, and so it isn't quite the same thing.

	//This is implementing daeArray<daeContent>::clear().
	for(size_t i=0;i<content.size();i++) 
	{
		daeElement *ch = content[i].getChild();		
		if(nullptr==ch) 
		{
			if(content[i].hasText())
			i+=content[i].getKnownStartOfText().span()-1;
		}
		else //ORDER IS IMPORTANT HERE.
		{
			//ALGORITHM
			//FIRST __clear2() recurses through the descendants,
			//-so that they're removed at the leaves. This keeps
			//the database from having to traverse the graph for
			//every removal.
			//Then, when there are no more grandchildren, replace
			//the child with a nullptr placeholder so that the
			//cursor-stop indices don't have to be managed as the
			//contents-array is being taken down.
			//Lastly, child removed, __atomize2() tears down any
			//types among the attributes that might include refs.
			//That must be done, so that the destructor can ever
			//be called. (Because of the ref-counting, destruction
			//is always a two-pass process. That's what "atomize"
			//means: the first pass of destruction.

			daeMeta &meta = ch->getMeta(); //SHADOWING
			ch->__clear2(meta.getContentsWRT(ch),meta,doc);

			//The element must survive its vacation.
			daeElementRef compact = ch;
			{
				//_vacate_operation does this first.
				//It's done here because the document
				//is on hand, so no need to look it up.
				if(meta._daeDocument_typeLookup_enabled) //LEGACY
				{
					#ifdef NDEBUG
					#error Maybe try to bulk erase reoccurrences?
					#endif
					if(doc!=nullptr) doc->_typeLookup_self_remove(meta,*ch);
				}	

				#ifdef NDEBUG
				#error Need a mechanism for suppressing notifications.
				#endif
				bool notify = true;
				daeContents_base::_vacate_operation op(content[i]);
				if(notify) daeNoteChange(op);				
			}
			//OPTIMIZATION
			//The cryptic/hard to search for compacting operator~()
			//returns true if the object is deleted. If it is being
			//deleted then __atomize2() will have already been done.
			if(!~compact) ch->__atomize2(meta);
		}
	}
	//This resets all of the internal counters to 0.
	meta._clearTOC(this); assert(content._==content.data());

	//NEW: Pursuing a strategy where the back is all 0s.
	//(So that increasing the size needn't 0 terminate.)
	size_t zero_fill = content.size()*sizeof(daeContent);	
	//Zero/set the special end-pointer to the beginning.
	memset(content.begin(),0x00,zero_fill);
	content.getAU()->setInternalCounter(0);
}	

void daeElement::__grow(daeArray<daeContent> &contents_of_this, size_t minCapacity)
{
	daeContents &c = static_cast<daeContents&>(contents_of_this);			
	//NEW: This is the real reason this is implemented apart from
	//daeArray.h & daeContent.h. Although getDatabase() is a plus.
	daeOffset _ = c.cursor()-c.data();
	//This should be the same logic as daeArray::_grow3(). +1 for 0-terminator.
	size_t newT = minCapacity<4?4:std::max(c.getCapacity()*2,minCapacity+1);
	//NEW: This is repurposing bits and pieces of code from all over to
	//make this simpler to write than it has any right to be.
	{
		//This sets a hint to traverse the element's graph when it's migrated.
		//It's for keeping things like "id" and "sid" lookup tables up-to-date.
		__DAEP__Element__data.daeContents_capacity_is_nonzero = 1;	
		//This reAlloc() was added for loaders that tally the number of spaces
		//in a list, and so can reserve memory in order to avoid reallocations.
		reAlloc(getDatabase(),c.getAU(),newT,c._element_less_mover);
		//This makes it so insertion code doesn't generate 0-termination codes.
		memset(c.end(),0x00,sizeof(daeContent)*(c.getCapacity()-c.size()));		
	}
	//Finally repair the pointer-based cursor. (Previously daeCursor
	//was an offset, and so repair had not been necessary.)
	c.cursor() = c.data()+_;
}

daeElement *daeElement::add(daeString list_of_names)
{
	daeMeta *meta = getMeta();
	daeElement *root = nullptr, *out = nullptr;
	for(daeString p=list_of_names,q=p;*q!='\0';p=q+1)
	{
		while(*q!='\0'&&*q!=' ') q++;
		out = add(meta->createWRT(*this,daeHashString(p,q-p)));		
		if(out==nullptr) break;
		if(root==nullptr) root = out;
	}
	if(out==nullptr&&root!=nullptr) removeChildElement(root); return out;
}

int daeElement::_getAttributeIndex(const daePseudonym &name)const
{
	//REMINDER: THIS IS IMPLEMENTED BY A LINEAR LOOKUP BECAUSE domAny
	//NEEDS TO BE ABLE TO DYNAMICALLY ADD ATTRIBUTES TO ITS ATTRIBUTE
	//ARRAY INSIDE ITS PSEUDO METADATA. OTHERWISE IT WOULD USE A HASH
	//LOOKUP LIKE THE CHILDREN DO. (OR HYBRID IF SOMEONE PROFILES IT)
	const daeArray<daeAttribute> &attrs = getMeta()->getAttributes();
	size_t i;
	for(i=0;i<attrs.size();i++)
	if(name==attrs[i]->getName()) return i;		
	if(_addAnyAttribute(name)) return (int&)i; return -1;
}

daeAttribute *daeElement::_getAttributeObject(size_t i)const
{
	const daeArray<daeAttribute> &attrs = getMeta()->getAttributes();
	if(i<attrs.size()) return attrs[i]; return nullptr;
}
	 
daeArray<daeStringCP> &daeElement::_getAttribute(daeArray<daeStringCP> &out, size_t i)const
{
	daeAttribute *attr = nullptr;
	attr = getAttributeObject(i); 	
	if(attr!=nullptr) 
	attr->memoryToStringWRT(this,out);
	else out.clear_and_0_terminate(); return out;
}
		
daeOK daeElement::_setAttribute(size_t i, daeString value)
{
	daeAttribute *attr = getAttributeObject(i); 		
	if(attr!=nullptr) return attr->stringToMemoryWRT(this,value,value+strlen(value)); 
	return DAE_ERR_QUERY_NO_MATCH;
}
daeOK daeElement::_setAttribute(size_t i, const daeHashString &value)
{
	daeAttribute *attr = getAttributeObject(i); 		
	if(attr!=nullptr) return attr->stringToMemoryWRT(this,value); 
	return DAE_ERR_QUERY_NO_MATCH;
}
   
daeArray<daeStringCP> &daeElement::getCharData(daeArray<daeStringCP> &out)const
{
	daeCharData *CD = getCharDataObject();
	if(CD!=nullptr)
	CD->memoryToStringWRT(this,out);
	else out.clear_and_0_terminate(); return out;
}

daeOK daeElement::_setCharData(const daeHashString &value)
{
	daeCharData *CD = getCharDataObject();
	if(CD!=nullptr) return CD->stringToMemoryWRT(this,value); 
	return DAE_ERR_QUERY_NO_MATCH;
}

daeElementRef daeElement::clone(daeDOM &DOM, clone_Suffix *suffix)const
{		
	daeElementRef ret = DOM._addElement(getMeta()); ret->getNCName() = getNCName();

	daeMeta *meta = getMeta();
	const daeArray<daeAttribute> &attrs = meta->getAttributes();
	//This is an optimized approach, versus a pure-string based one.
	if(daeObjectType::ANY==getElementType())
	{
		#ifdef NDEBUG
		#error maybe just give domAny a private copy-constructor?
		#endif
		for(size_t i=0;i<attrs.size();i++)
		{
			//attrs[i]->copyWRT(ret,this); could be direct, but no big deal for now.
			ret->_cloneAnyAttribute(attrs[i].getName()); attrs[i]->copyWRT(ret,this);
		}
	}
	else for(size_t i=0;i<attrs.size();i++) attrs[i]->copyWRT(ret,this);
	daeCharData *CD = getCharDataObject();
	if(CD!=nullptr) CD->copyWRT(ret,this);

	#ifdef NDEBUG //NOT-THREADSAFE
	#error what about the hidden-partition?
	#endif
	daeContents &dst = meta->getContentsWRT(ret);
	const daeContents &src = meta->getContentsWRT(this);
	dst.grow(src.getCapacity()); 
	dst.getAU()->setInternalCounter(src.size());
	memcpy(dst.data(),src.data(),src.getCapacity());
	const daeElement *cp; //Copy the elements one-by-one. NOT-THREADSAFE
	for(size_t i=0;i<dst.size();i++) if((cp=dst[i].getChild())!=nullptr)
	new(&dst[i]._child.ref) daeSmartRef<>(cp->clone(DOM,suffix));
	
	#ifdef NDEBUG
	#error would be better if the new id/name were used above to create the new element.
	#endif
	if(suffix!=nullptr) suffix->_suffix(ret); return ret;
}
void daeElement::clone_Suffix::_suffix(daeElement *clone)
{		
	#define _(x) if(x!=nullptr){\
	int i = clone->getAttributeIndex(#x);\
	if(i>=0&&!clone->getAttribute(*this,i).empty())\
	clone->setAttribute(i,append_and_0_terminate(x,x.extent).data()); }
	_(id)_(name)
	#undef _
}

static std::ostream &formatColumns(std::ostream &os, daeString c1, daeString c2)
{
	if(c1==nullptr) c1 = ""; 
	if(c2==nullptr) c2 = "";
	int w = (int)os.width();
c2:	int n = strnlen(c1,w);
	if(n>w) //append ellipses
	{
		os.width(0);
		w-=3;
		for(int i=0;i<w;i++) os.put(*c1++);		
		os<<"...";
	}
	else os << c1; 
	if(c2!=nullptr){ c1 = c2; c2 = nullptr; goto c2; }	
	return os;
}
daeArray<daeStringCP> &daeElement::compare_Result::format(daeArray<daeStringCP> &out)
{
	out.clear_and_0_terminate();
	if(elt1==nullptr||elt2==nullptr) return out;
	daeArray<daeStringCP,256> tmp1,tmp2;
	daeOStringBuf<1024> buf(out);
	std::ostream msg(&buf); msg
	<<std::setw(13)<<std::left<<""
	<<std::setw(50)<<std::left<<"Element 1"<<"Element 2\n"
	<<std::setw(13)<<std::left<<""
	<<std::setw(50)<<std::left<<"---------"<<"---------\n"
	<<std::setw(13)<<std::left<<"Name"
	<<std::setw(50)<<std::left,formatColumns(msg,elt1->getNCName(),elt2->getNCName())<<std::endl
	<<std::setw(13)<<std::left<<"Type"
	<<std::setw(50)<<std::left,formatColumns(msg,elt1->getTypeName(),elt2->getTypeName())<<std::endl
	<<std::setw(13)<<std::left<<"id"
	<<std::setw(50)<<std::left,formatColumns(msg,elt1->getID_id(),elt2->getID_id())<<std::endl
	<<std::setw(13)<<std::left<<"Attr name"
	<<std::setw(50)<<std::left,formatColumns(msg,attrMismatch,attrMismatch)<<std::endl
	<<std::setw(13)<<std::left<<"Attr value"
	<<std::setw(50)<<std::left,formatColumns
	(msg,elt1->getAttribute(tmp1,attrMismatch).data()
	,elt2->getAttribute(tmp2,attrMismatch).data())<<std::endl
	<<std::setw(13)<<std::left<<"Char data"
	<<std::setw(50)<<std::left,formatColumns
	(msg,elt1->getCharData(tmp1).data(),elt2->getCharData(tmp2).data())<<std::endl
	<<std::setw(13)<<std::left<<"Child count"
	<<std::setw(50)<<std::left<<elt1->getChildrenCount()<<elt2->getChildrenCount();
	return out;
}

namespace //static
{
	daeElement::compare_Result compareMatch()
	{
		daeElement::compare_Result result;
		result.compareValue = 0; return result;
	}

	daeElement::compare_Result nameMismatch(const daeElement &elt1, const daeElement &elt2)
	{
		daeElement::compare_Result result;
		result.elt1 = &elt1; result.elt2 = &elt2;
		result.compareValue = strcmp(elt1.getNCName(),elt2.getNCName());
		result.nameMismatch = true; return result;
	}

	daeElement::compare_Result attrMismatch(const daeElement &elt1, const daeElement &elt2, const daePseudonym &name)
	{
		daeElement::compare_Result result;
		result.elt1 = &elt1; result.elt2 = &elt2;
		result.compareValue = 
		strcmp(elt1.getAttribute(name).data(),elt2.getAttribute(name).data());
		result.attrMismatch = name; return result;
	}

	daeElement::compare_Result charDataMismatch(const daeElement &elt1, const daeElement &elt2)
	{
		daeElement::compare_Result result;
		result.elt1 = &elt1; result.elt2 = &elt2;
		result.compareValue = strcmp(elt1.getCharData().data(),elt2.getCharData().data());
		result.charDataMismatch = true; return result;
	}

	daeElement::compare_Result childCountMismatch(int a, int b, const daeElement &elt1, const daeElement &elt2)
	{
		daeElement::compare_Result result;
		result.elt1 = &elt1; result.elt2 = &elt2;		
		result.compareValue = a-b;
		result.childCountMismatch = true; return result;
	}

	daeElement::compare_Result compareElementsChildren(const daeElement &elt1, const daeElement &elt2)
	{
		//Compare children
		#ifdef NDEBUG
		#error should compare the contents arrays
		#endif
		daeArray<const_daeElementRef,64> children1, children2;
		if(elt1.getChildren(children1).size()
		 !=elt2.getChildren(children2).size())
		return childCountMismatch(children1.size(),children2.size(),elt1,elt2);
		for(size_t i=0;i<children1.size();i++)
		{
			daeElement::compare_Result result = 
			daeElement::compareWithFullResult(*children1[i],*children2[i]);
			if(0!=result.compareValue)
			return result;
		}

		return compareMatch();
	}

	daeElement::compare_Result compareElementsSameType(const daeElement &elt1, const daeElement &elt2)
	{
		//Compare attributes 
		daeMeta &meta = elt1.getMeta();
		const daeArray<daeAttribute> &attribs = meta.getAttributes();
		for(size_t i=0;i<attribs.size();i++)
		if(attribs[i].compareWRT(&elt1,&elt2)!=0)
		return attrMismatch(elt1,elt2,attribs[i].getName());

		//Compare character data
		if(meta.getValue()!=nullptr)
		if(meta.getValue()->compareWRT(&elt1,&elt2)!=0)
		return charDataMismatch(elt1,elt2);

		return compareElementsChildren(elt1,elt2);
	}

	daeElement::compare_Result compareElementsDifferentTypes(const daeElement &elt1, const daeElement &elt2)
	{
		daeArray<daeStringCP,256> value1, value2;

		//Compare attributes. Be careful because each element could have a
		//different number of attributes.
		if(elt1.getAttributeCount()>elt2.getAttributeCount())
		return attrMismatch(elt1,elt2,elt1.getAttributeName(elt2.getAttributeCount()));
		if(elt2.getAttributeCount()>elt1.getAttributeCount())
		return attrMismatch(elt1,elt2,elt2.getAttributeName(elt1.getAttributeCount()));
		for(size_t i=0;i<elt1.getAttributeCount();i++)
		{
			elt1.getAttribute(value1,i);
			elt2.getAttribute(value2,elt1.getAttributeName(i));
			if(value1!=value2)
			return attrMismatch(elt1,elt2,elt1.getAttributeName(i));
		}

		//Compare character data
		if(elt1.getCharData(value1)!=elt2.getCharData(value2))
		return charDataMismatch(elt1,elt2);

		return compareElementsChildren(elt1,elt2);
	}

} //namespace

daeElement::compare_Result daeElement::compareWithFullResult(const daeElement &elt1, const daeElement &elt2)
{
	//Check the element name
	if(elt1.getNCName()!=elt2.getNCName())
	return nameMismatch(elt1,elt2);

	//Dispatch to a specific function based on whether or not the types are the same	
	if(elt1.getMeta()!=elt2.getMeta())
	return compareElementsDifferentTypes(elt1,elt2);
	else
	return compareElementsSameType(elt1,elt2);
}

#ifndef COLLADA_NODEPRECATED
const_daeURIRef daeElement::getDocumentURI()const
{
	const_daeDocumentRef d = getDocument(); return d==nullptr?nullptr:&d->getDocURI();
}
#endif
 
namespace //C++98/03-SUPPORT (C2918)
{
	struct daeElement_getChild_f
	{	const daeElement::matchElement &matcher;
		void operator()(daeElement *ch){ if(ch!=nullptr&&matcher(ch)) throw(ch); }
	};
}
daeElement *daeElement::getChild(const matchElement &matcher)
{
	daeElement_getChild_f f = { matcher };
	try{ getContents().for_each_child(f); }
	catch(daeElement *out){ return out; }return nullptr;
}

daeElementRef daeElement::getDescendant(const matchElement &matcher)
{
	//daeContents::for_each_child() isn't up to this.
	//And even if it could, the refs/locks are multi-level.
	daeArray<daeElementRef,64> elts; getChildren(elts);
	for(size_t i=0;i<elts.size();i++)
	{
		//Check the current element for a match.
		if(matcher(elts[i])) return elts[i];

		//Append the element's children to the queue.
		elts[i]->getChildren(elts);
	}		  
	return nullptr;
}

const_daeElementRef daeElement::getAncestor(const matchElement &matcher)const
{
	const_daeElementRef elt = getParent(); while(elt!=nullptr)
	{
		if(matcher(elt)) return elt; elt = elt->getParent();
	}
	return nullptr;
}

void daeElement::_sidLookup(daeString ref, daeElementRef &match, const daeDoc *docIn)const
{
	const daeStringRef &known_ref = *(daeStringRef*)&ref;

	//This is copy/pasted from daeSIDResolver.cpp.
	//This is designed to get constructions of the type
	//<rotate sid="rotateX"> where there may be several
	//SIDs to eliminate by the regular bottom-up search.
	if(30<known_ref.getSID_frequency())
	{
		daeElement::matchSID matcher(known_ref);
		const daeElement *e = getChild(matcher);
		if(e!=nullptr)
		{
			match = e; return; 
		}
	}

	const daeDocument *doc = (daeDocument*)docIn;
	if(doc==nullptr) return; assert(doc==docIn->getDocument());

	//This is a simplified form of daeSIDResolver.cpp's _find() and
	//_computeDistance() procedures.
	unsigned minDistance = UINT_MAX;
	const daeElement *closestElement = nullptr;
	daeDocument::_sidMapRange range = doc->_sidMap.equal_range(ref);
	for(daeDocument::_sidMapIter it=range.first;it!=range.second;it++)	
	{
		unsigned distance = 0; 
		const daeElement *p = it->second; do
		{
			if(this==p) 
			{
				if(distance<minDistance)
				{
					minDistance = distance;	closestElement = it->second;					
				}
				break;
			}
			else distance++;

		}while(nullptr!=(p=p->getParentElement()));		
	}
	if(closestElement!=nullptr)
	match = const_cast<daeElement*>(closestElement);
}

//---.
}//<-'

/*C1071*/