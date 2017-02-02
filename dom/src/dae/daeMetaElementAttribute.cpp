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

//_placeElementBetween //_placeElementBetween //_placeElementBetween

template<>
/**
 * Does code-elimination for the non-between placement procedures.
 */
inline bool daeCM::__between(void*,daeOrdinal,void*,daeCounter daeCM::*)const
{
	return true; 
}
template<>
/**TEMPLATE-SPECIALIZATION
 * Tells if @c this CM node intersects ordinals from @a a to @a b.
 */
inline bool daeCM::__between(daeOrdinal a, daeOrdinal o, daeOrdinal b, daeCounter daeCM::*_max)const
{
	return !(o-(this->*_max)>a||b>o);
}
	
template<class A, class B, class C>
//The <xs:any> virtual content model has many, many restrictions that
//make it very different from the rest of them. They're not explained
//in full here. The library doesn't guarantee that order is preserved.
//There will probably be three or four different implementations over
//time. But for now, it's not a big deal, COLLADA doesn't use it, and
//<xs:any> itself is a bit of a black sheep. The 4 possible varations 
//are: this one, order-preserving involving substitution-groups, with
//being 2, and without being 3, and XSD 1.1, which may be 1 or 3 more.
inline daeError XS::All::__place(A a, daeCM_Placement<C> &p, B b, daeOrdinal o)const
{	
	const XS::Element *el = (XS::Element*)_CM[_deepCM[p.name][0]];

	//This could be seen as an optimization, if it's assumed 
	//that missinsertions are worth computing rejections for.
	//if(p.count&&!el->hasSubstitutes())
	//return DAE_ERR_NAME_CLASH;
	//Instead, assume that insertion_point is always desired.
	//(Which is as a courtesy if DAE_ORDER_IS_NOT_PRESERVED.)
	o-=el->getSubtrahend();
	C::iterator it = p.content._fuzzy_lower_bound(o);	
	if(it->_ordinal_magic()>=o) 
	return DAE_ERR_NAME_CLASH;

	p.insertion_point = p.content._rewind_ordinal(it);	
	p.content.cursor() = it;
	p.ordinal = o; 

	if(__placed_between<A,B>::value)
	{
		//Here the insertion order happens to be in order.
		if((daeOrdinal)a>o&&o<(daeOrdinal)b)
		return DAE_OK;
		//Here A and B are constraining the order, but it's
		//not in fact in order.
		return DAE_ORDER_IS_NOT_PRESERVED;
	}
	return DAE_OK; //The library orders insertion as it likes.
}

template<class T, class A, class B, class C>
inline daeError daeCM::__placeT(A a, daeCM_Placement<C> &p, B b, daeOrdinal o)const
{
	daeError e = DAE_ERROR;
	daeOrdinal oM = _maxOrdinals;
	daeOrdinal oMN = o-_maxOrdinals_x_maxOccurs;
	for(;o>oMN;o-=oM) if(__between(a,o,b))
	{
		e = ((T*)this)->__placeOccurrence(a,p,b,o);
		if(e==DAE_OK) break;
	}
	else //optimizing: Close window around [a,b].
	{
		if(oMN<(daeOrdinal)b) oMN = (daeOrdinal)b;
		if((daeOrdinal)a<o) o-=(o-(daeOrdinal)a)/oM*oM;
	}
	return e;
}

template<class A, class B> 
inline daeError XS::Choice::__placeOccurrence2(A a, daeCM_Placement<> &p, B b, daeOrdinal o, daeOrdinal oCur, daeContent *it)const
{
	//INTELLIGENT INSERTION/PROMOTION ALGORITHM
	//If oCur belongs to a choice without p.name,
	//-attempt to promote that choice to one which
	//can accommodate p.name within this occorrence. 
	size_t i,j,jN,claimed; daeContent *text; 
	_DeepCM_string &named = _deepCM[p.name], *proc;	
	const _CM_index *choices = named.c_str(), *subsets;
	for(i=0;i<named.size();i++) if(_CM[choices[i]]->__claim(oCur))
	{
		claimed = choices[i];
		if(DAE_OK==_CM[claimed]->__placeXS<1>(a,p,b,o))
		return DAE_OK;
		i = 0; goto claimed1; //CASCADE: Stop calling __claim()...
	}
	for(i=0;i<named.size();i++) //ENTRY-CONDITION IS FAILING TO __claim().
	{
		#define _(X,Y) \
		proc = &_proclaimCM[choices[i]];\
		subsets = proc->c_str();\
		for(j=0,jN=proc->size();j<jN;j++) if(X subsets[j] Y)
		_(_CM[,]->__claim(oCur))
		{
			claimed = subsets[j]; goto claimed2; //CASCADE: Stop calling __claim()...
		}
	}return DAE_ERR_NAME_CLASH; //Not-required.
	//UNREACHABLE
	for(;i<named.size();i++) claimed1: //ENTRYPOINT IS claimed1: AND claimed2:
	{
		_(,==claimed) claimed2:
		{		
			text = p.content._rewind_ordinal(it,o);
			
			it = p.content._find_ordinal(text); goto rewound; //CASCADE: Rewind once...
		}
	}return DAE_ERR_NAME_CLASH; //Not-required.
	//UNREACHABLE
	for(;i<named.size();i++) //ENTRYPOINT IS rewound:
	{
		_(,==claimed) rewound:
		#undef _
		{
			if(__tryCM(text,it,choices[i],claimed,a,p,b,o)>=DAE_OK) return DAE_OK; //Promoted/placed?		
		}
	}return DAE_ERR_NAME_CLASH;	
}
template<class A, class B> 
inline daeError XS::Choice::__placeOccurrence2(A a, daeCM_Placement<_solution> &p, B b, daeOrdinal o, daeOrdinal oCur,...)const
{
	//THIS VERSION HAPPENS IN THE CONTEXT OF __tryCM().
	//EVEN IF IT FAILS, THAT'S OKAY, BECAUSE IT BUBBLES 
	//BACK UP TO THE REAL INSERTION CONTEXT AND PROCEEDS.
	_DeepCM_string &choices = _deepCM[p.name];
	for(size_t i=0;i<choices.size();i++) 
	if(_CM[choices[i]]->__claim(oCur)&&DAE_OK==_CM[choices[i]]->__placeXS<1>(a,p,b,o))
	return DAE_OK; return DAE_ERR_NAME_CLASH;	
}
template<class A, class B, class C>
inline daeError XS::Choice::__placeOccurrence(A a, daeCM_Placement<C> &p, B b, daeOrdinal o)const
{
	_DeepCM_string &named = _deepCM[p.name];	
	//FYI: _fuzzy_lower_bound() is a linear search algorithm.
	//Here _maxOrdinals is modifying it to accept anything in its range.
	C::const_iterator it = p.content._fuzzy_lower_bound(o,_maxOrdinals);
	daeOrdinal oCur; 
	if(it==p.content.end()||(oCur=o-it->_ordinal())>=_maxOrdinals)
	return _CM[named[0]]->__placeXS<1>(a,p,b,o);
											
	//Here an ordinal exists for this choice.
	p.content.cursor() = it; 

	//Indicates a trivial <xs:all> like switch.
	if(_proclaimCM==nullptr) return DAE_ERR_NAME_CLASH;

	//INTELLIGENT INSERTION/PROMOTION ALGORITHM
	return __placeOccurrence2(a,p,b,o,oCur,(daeContent*)it);
}
template<class A, class B, class C>
inline daeError XS::Group::__placeOccurrence(A a, daeCM_Placement<C> &p, B b, daeOrdinal o)const
{
	//Swapping p.name would work, but compilers might not see a swap as a static operation.
	daeCM_Placement<C> q(_groupNames[p.name],p.count,p.content);
	daeError e = _groupCM->__placeXS<0>(a,q,b,o); 
	if(e==DAE_OK){ p.ordinal = q.ordinal; p.insertion_point = q.insertion_point; }return e;
}
template<class A, class B, class C>
inline daeError XS::Sequence::__placeOccurrence(A a, daeCM_Placement<C> &p, B b, daeOrdinal o)const
{
	daeError e = DAE_ERROR;
	_DeepCM_string::const_iterator it,itt;
	for(it=_deepCM[p.name].begin(),itt=_deepCM[p.name].end();it!=itt;it++)	
	{
		const daeCM *nameclash = _CM[*it];
		daeCounter daeCM::*_unprotect_max_x_max;
		_unprotect_max_x_max = &XS::Sequence::_maxOrdinals_x_maxOccurs;
		if(nameclash->__between(a,o-nameclash->getSubtrahend(),b,_unprotect_max_x_max))
		{
			e = nameclash->__placeXS<1>(a,p,b,o); if(e==DAE_OK) break;
		}
	}return e;
}

template<class C>
inline daeError daeElementCM::__place(daeCM_Placement<C> &p, daeOrdinal o)const
{	
	daeError e = DAE_OK;
	//This first condition is designed to avoid counting "unbounded"
	//blocks in the worst case. By using p.count it can get some extra
	//cases, and the caller must produce a count to add the index anyway.
	if(p.count>=/*_maxOccurs*/_substitute_maxOccurs) 
	{
		C::const_iterator iit = p.content.begin();
		C::const_iterator it = p.content._fuzzy_lower_bound(o);
		
		if(o==it->_ordinal()) //Count is nonzero? 
		{
			if(_maxOccurs>1) //Count is potentially more than one?
			{
				C::const_iterator rit;
				daeCounter span,text = 0;
				for(rit=it-1;;) //Count backward.
				{
					while(o==rit->_ordinal()&&rit>=iit) rit--;
												   
					if(rit->hasText()&&rit>=iit)
					{
						span = rit->getKnownEndOfText().counterspan();
						text+=span;
						rit-=span;
					}
					else break;
				}
				for(it++;;) //Count forward.
				{
					while(o==it->_ordinal()) it++;

					if(it->hasText())
					{
						span = it->getKnownStartOfText().span();
						text+=span;
						it+=span; 
					}
					else break;
				}			

				if(it-rit-text<=_maxOccurs)
				{
					if(text!=0) while(it[-1].hasText()) 
					it-=it[-1].getKnownStartOfText().span();
					
					p.insertion_point = it; it = rit+1; 
					
					if(text!=0) while(it->hasText()) 
					it+=it->getKnownStartOfText().span(); 
				}
				else e = DAE_ERR_NAME_CLASH;
			}
			else e = DAE_ERR_NAME_CLASH;
		}
		else p.insertion_point = p.content._rewind_ordinal(it);

		p.content.cursor() = it; 
	}
	else //p.insertion_point = p.content._upper_bound_with_text(o);
	{
		//Here it could be undesirable to locate the upper-bound if
		//the caller doesn't want to insert after the last occurrence.
		//Whereas in the other cases, it's a courtesy to skip text nodes.
		p.insertion_point = nullptr; 
	}

	p.ordinal = o; return e;	 
}

template<int Inner, class A, class B, class C>
inline daeError daeCM::__placeXS(A a, daeCM_Placement<C> &p, B b, daeOrdinal o)const
{
	assert(o>_ordSubtrahend); 
	if(Inner==1) o-=_ordSubtrahend; 
	else assert(_ordSubtrahend==0);
	daeError e; 
	switch(_xs)
	{
	case XS::CHOICE: e = __placeT<XS::Choice>(a,p,b,o); break;
	
	case XS::GROUP: e = __placeT<XS::Group>(a,p,b,o); break;

	case XS::SEQUENCE: e = __placeT<XS::Sequence>(a,p,b,o); break;

	case XS::ELEMENT: case XS::ANY: 
		
		/*Because of _CMEntree it's possible to get here in either case.
		if(Inner==1)*/{ e = ((daeElementCM*)this)->__place(p,o); break; }
		//This was assuming <xs:any maxOccurs="unbounded">.
		//daeElementCM is meant to address that. Taking the above break.
		//e = DAE_OK; p.ordinal = o; break;
				
	case XS::ALL: //Falling through if 0==Inner.
		
		if(Inner==0){ e = ((XS::All*)this)->__place(a,p,b,o); break; }

	default: assert(0); e = DAE_ERR_FATAL;
	}
	return e;
}

daeError daeCM::_placeElement(daeCM_Placement<> &p)const
{
	return __placeXS<0>((void*)0,p,(void*)0,daeOrdinals);
}
daeError daeCM::_placeElementBetween(daeOrdinal a, daeCM_Placement<> &p, daeOrdinal b)const
{
	return __placeXS<0>(a,p,b,daeOrdinals);
}

//_prepareContentModel //_prepareContentModel //_prepareContentModel 

void daeCM::_prepareContentModel2() //RECURSIVE
{
	if(isParentCM())
	{
		const std::vector<const daeCM*>&_CM(((daeParentCM*)this)->_CM);
		for(daeCounter i=0,j=-1;i<_CM.size();j=_CM[i++]->_ordSubtrahend) 
		if(_CM[i]->_xs!=XS::ELEMENT)
		const_cast<daeCM*>(_CM[i])->_prepareContentModel2();
		else if(j==_CM[i]->_ordSubtrahend)
		{
			//maxOccurs should be equal for them all. 0 would be ambiguous.
			daeCounter max = _CM[--i]->_maxOccurs; do
			{
				assert(_CM[i]->_xs==XS::ELEMENT&&_CM[i]->_maxOccurs==max);
				((XS::Element*)_CM[i])->_substitute_maxOccurs = 0; 

			}while(++i<_CM.size()&&j==_CM[i]->_ordSubtrahend); i--; assert(max!=0); 
		}
		else ((XS::Element*)_CM[i])->_substitute_maxOccurs = _CM[i]->_maxOccurs;
	}
	switch(_xs)
	{
	case XS::ANY: ((XS::Any*)this)->_substitute_maxOccurs = _maxOccurs; break;
	case XS::CHOICE: ((XS::Choice*)this)->__preparePromotionStrategy(); break;
	}
}
template<class CM>
inline bool XS::Element::__subsetof(const CM &b,int,int)const
{
	return b.__names(getChildID().getName());
}
template<class CM>
inline bool daeParentCM::__subsetof(const CM &b, int lo, int hi)const
{
	for(int i=lo;i<hi;i++)
	if(__names(i)&&!b.__names(i)) return false; return true;
}
template<class CM>
inline bool XS::Group::__subsetof(const CM &b, int lo, int hi)const
{
	for(int i=lo;i<hi;i++)			
	if(__names(i)&&!b.__names(i)) return false; return true;
}
template<class T>
inline bool daeCM::__subsetofT(const daeCM &b, int lo, int hi)const
{
	T &_this = (T&)*this;
	if(b.isParentCM()) return _this.__subsetof((daeParentCM&)b,lo,hi);
	if(b.getXS()==XS::GROUP) return _this.__subsetof((XS::Group&)b,lo,hi);
	if(b.getXS()==XS::ELEMENT) return _this.__subsetof((XS::Element&)b,lo,hi); 
	assert(0); return false;
}
inline bool daeCM::__subsetofXS(const daeCM &b)const
{
	//Note sure if this is best, but it's logical. Or is it???
	if(b.getXS()==XS::ANY) return true;	
	if(_xs==XS::ELEMENT) return __subsetofT<XS::Element>(b,0,0);
	int lo = _meta->getTOC().front().getChildID().getName();
	int hi = lo+_meta->getTOC().size();
	if(isParentCM()) return __subsetofT<daeParentCM>(b,lo,hi); 
	if(_xs==XS::GROUP) return __subsetofT<XS::Group>(b,lo,hi); 
	assert(_xs==XS::ANY);	
	return false; //Any is lowest priority?
}

static size_t daeMetaElementAttribute__prepare_migrate
(short *buf, size_t io, const std::basic_string<short> *proclaimCM)
{	
	short cmp = buf[io]; 
	for(size_t i=io;i--!=0;)
	if(proclaimCM[buf[i]].find(cmp)!=proclaimCM->npos
	 &&proclaimCM[cmp].find(buf[i])==proclaimCM->npos)
	{
		//buf SHOULD HAVE +1 ROOM TO OVERLOW.
		memmove(buf+i+1,buf+i,io-i); buf[i] = cmp;
		return io;
	}
	return io-1;
}
void XS::Choice::__preparePromotionStrategy()
{
	_solution_cache = &_solution_cache_null;

	if(_maxOrdinals==_CM.size())
	{	
		//Indicates a trivial <xs:all> like switch.
		_proclaimCM = nullptr; 
		return; //!
	}							

	//This is for DEMOTIONS BUSINESS below.
	//Note. This could be done passively, in
	//_solve(); but it'd be schizophrenic in a
	//shared object (multiple metadata) scenario.
	daeTOC &TOC = _meta->getTOC(); 
	int lo = TOC.front().getChildID().getName();
	int hi = lo+TOC.size(); 
	//+1 is letting daeMetaElementAttribute__prepare_migrate memmove.
	daeArray<short,128> _; _.grow(std::max(TOC.size(),_CM.size())+1);
	short *buf = _.data();
	
	//Calculate subsets on a per choice basis.
	_proclaimCM = _deepCM+TOC.size()+lo;
	for(size_t i=0;i<_CM.size();i++)
	for(size_t j=0;j<_CM.size();j++)	
	if(i!=j&&_CM[i]->__subsetofXS(*_CM[j]))
	_proclaimCM[j].push_back((_CM_index)i);
	for(size_t i=0;i<_CM.size();i++) if(_proclaimCM[i].size()>1)
	{
		//ALGORITHM
		//The goal here is to move subsets to the front of the 
		//line. It doesn't seem as if there's a traditional way
		//to do the sort--based on value comparison.
		size_t j = _proclaimCM[i].size();
		COLLADA_SUPPRESS_C(4996) _proclaimCM[i].copy(buf,j);
		for(j--;0!=j;)
		j = daeMetaElementAttribute__prepare_migrate(buf,j,_proclaimCM);
		_proclaimCM[j].assign(buf,_proclaimCM[i].size());
	}

	char *demotion_mask = (char*)buf-lo;
	memset(demotion_mask+lo,0x00,hi-lo);
	for(size_t to=0;to<_CM.size();to++) if(!_proclaimCM[to].empty())
	{
		//DEMOTIONS BUSINESS							
		for(size_t i=0;i<_proclaimCM[to].size();i++)
		{
			size_t fro = _proclaimCM[to][i];
			//This is the same formula as _solve().
			for(int i=lo;i<hi;i++)
			{
				_DeepCM_string &named = _deepCM[i];
				for(size_t j=0;j<named.size();j++)			
				if(to==named[j]) demotion_mask[i]|=1;
				else if(fro==named[j]) demotion_mask[i]|=2;
			};
			for(int i=lo;i<hi;i++) switch(demotion_mask[i])
			{
			case 1: const_cast<XS::Element&>(TOC[i-lo])._element.namefellows_demote = 1;
			default: demotion_mask[i] = 0; case 0: break;
			}
		}
	}

	//Priority goes to ever smaller subsets.
	//Note: a sequential ranking algoritm was
	//written to optimize loading, but it seems
	//impossible/impractical to serve two masters.	
	for(int i=lo;i<hi;i++)	
	{
		_DeepCM_string &named = _deepCM[i]; if(named.size()>1)
		{
			//ALGORITHM
			//Same deal as above.
			size_t j = named.size(); 
			COLLADA_SUPPRESS_C(4996) named.copy(buf,j);
			for(j--;0!=j;)
			j = daeMetaElementAttribute__prepare_migrate(buf,j,_proclaimCM);
			named.assign(buf,named.size());
		}
	}	
}

//promotion/demotion //promotion/demotion //promotion/demotion 

XS::Choice::_solution XS::Choice::_solution_cache_null = {0,0,0};

const daeContent *XS::Choice::_solve(_solution &s, const daeContent *it, size_t to, size_t fro, daeOrdinal o)const
{
	//HACK: demote implements removals, which do
	//not require placements, and must signal so.
	const bool demote = 
	_proclaimCM[fro].find(to)!=_DeepCM_string::npos;

	daeContent *ins;
	const daeContent *const iit = it;

	//This shouldn't be too large, as they're now
	//being 0 filled in the constructor.
	enum{ initial_capacity=24 };
	//HACK: These try to work around _grow2<daeContent>()
	//so it doesn't have to implement a non-database path.
	size_t size = 0, capacity = initial_capacity;

	daeContents_size<initial_capacity> p_content;
	daeCM_Placement<> p(0,-1,p_content);	
	const daeCM &toCM = *_CM[to]; 
	daeOK OK; bool ordered = true;
	daeOrdinal bigO = daeOrdinals;
	//The unordered map is built first.
	//If it's ordered or insolvable, it
	//follows that the ordered map is so.
	const daeOrdinal ot = o-_maxOrdinals;
	for(daeCounter ID=0;it->_child.ordinal>ot;ID++)
	{	
		//HACK: this avoids _grow2<daeContent>().
		if(++size==capacity)
		p_content._element_less_grow(capacity=size*2);

		p.name = it->_childID().getName();		
		const daeOrdinal ito = it++->_child.ordinal;
		while(it->_magic_ordinal>=ito) it++;		
		//This is simulating insertion of content.
		//If e!=DAE_OK, the keys are still needed.
		if(OK&&(OK=toCM.__placeXS<1>((void*)0,p,(void*)0,daeOrdinals)))
		{
			if(bigO>p.ordinal)
			{
				bigO = p.ordinal; 
				ins = p_content._append_child<0>();
			}
			else
			{
				ordered = false;
				ins = p_content._insert_child<0>(p.insertion_point);
			}
		}
		else ins = p_content._append_child<0>();
		//Assuming _child.ID is not required here.
		ins->_child.ID = ID;
		ins->_child.ordinal = p.ordinal;
		daeCTC<(sizeof(daeOrdinal)<=sizeof(void*))>();
		(daeOrdinal&)ins->_child.ref = ito;
	}	
	if(!OK) ordered = true;
	//itt is a courtesy to __tryCM().
	const daeContent *const itt = it; 
	ins = p_content.data();	
	daeTOC &TOC = _meta->getTOC();
	const int names = (int)TOC.size();
	const int lo = TOC.front().getChildID().getName(), hi = lo+names;
	const size_t iN = p_content.size();	
	//+1 adds a 0-terminator for daeCM_Placement<_solution>.
	//keys ends up with one also, which place() uses a little.
	size_t new_size = iN+1;
	if(OK) new_size*=ordered?2:3;
	//These can overlap if there is only one ordinal per name.
	//But it's hard to prove, and one is used as a mask below.
	if(!demote) new_size+=names*(OK?2:1);
	s.keysN = iN;	
	s.to = _CM[to]; s.fro = _CM[fro];
	s.keys = new daeOrdinal[new_size];	
	//Zeroing this up front keeps everything simple.
	memset(s.keys,0x00,new_size*sizeof(daeOrdinal));
	const daeCounter sub = daeOrdinals-o; 
	if(OK)
	{		
		s.unordered_mapping = s.keys+iN+1;
		s.ordered_mapping = s.unordered_mapping+(ordered?0:iN+1);
		for(size_t i=0,ID;i<iN;i++)
		{
			ID = p_content[i]._child.ID;
			s.unordered_mapping[ID] = ins[i]._child.ordinal; 			
			s.keys[ID] = (daeOrdinal&)ins[i]._child.ref+sub;
		}		
	}	
	else //The solution is that there's not one.
	{
		for(size_t i=0;i<iN;i++)
		s.keys[p_content[i]._child.ID] = 
		(daeOrdinal&)ins[i]._child.ref+sub; 
		s.unordered_mapping = s.ordered_mapping = s.keys;
	}
	if(demote) goto demote;
	s.ordered_places = s.ordered_mapping+iN+1-lo;
	s.unordered_places = s.ordered_places+(OK?names:0);	
	if(OK) //Fill-out unordered_places?
	{	
	//HACK#1: These are swapped here because _solution
	//is setup to use ordered_mapping with daeContents_like.
	//They could start out swapped, but this is easier to follow.
	std::swap(s.ordered_mapping,s.unordered_mapping);
	  
		typedef void p; //SHADOWING
		//This prevents __tryCM() from promoting
		//(modifying) the content, while probing.
		daeCM_Placement<_solution> pp(0,0,s);

		//Build a mask to avoid unnecessary calls to __placeXS()
		//BUT ALSO BECAUSE __placeXS() doesn't check if a name is
		//used or not. (This is filling out all names in advance.)
		for(int i=lo;i<hi;i++)
		{
			_DeepCM_string &named = _deepCM[i];
			for(size_t j=0;j<named.size();j++)			
			if(to==named[j]) s.ordered_places[i]|=1;
			else if(fro==named[j]) s.ordered_places[i]|=2;
		}assert(s.unordered_places!=s.ordered_places);
		//If this holds then to and fro are subsets of one another.
		bool all_0_or_3 = true; 
		//NOTE: ordered_places is used here and above, so the mask
		//can be reused in the ordered calculation below/less code.
		for(int i=lo;i<hi;i++) if(1==s.ordered_places[i])
		{	
			all_0_or_3 = false; pp.name = i;
			if(DAE_OK==toCM.__placeXS<1>((void*)0,pp,(void*)0,daeOrdinals))			
			{
				s.unordered_places[i] = pp.ordinal;
			}
		}else assert(s.ordered_places[i]!=2);
		//addPartialCM() can't address cases where two choices have
		//exactly the same names involved: There's no way to divide&
		//conquer, so they'll have to fight this out among themselves.
		if(all_0_or_3) for(int i=lo;i<hi;i++) if(3==s.ordered_places[i])
		{
			s.ordered_places[i] = 1; pp.name = i;
			if(DAE_OK==toCM.__placeXS<1>((void*)0,pp,(void*)0,daeOrdinals))			
			{
				s.unordered_places[i] = pp.ordinal;
			}
		}

	//HACK#2: Unswap. See HACK#1 explanation above.
	std::swap(s.ordered_mapping,s.unordered_mapping);
	}
	demote:	
	//This is a readability aid.
	bool ordered_OK = OK; 
	if(!ordered) //Fill-out ordered_mapping?
	{	
		it = iit; p_content._0_fill();
		for(size_t i=0;it->_child.ordinal>ot;i++)
		{	
			p.name = it->_childID().getName();		
			const daeOrdinal ito = it++->_child.ordinal;
			while(it->_magic_ordinal>=ito) it++;		
			//This is simulating insertion of content.
			//If e!=DAE_OK, the keys are still needed.
			if(!toCM.__placeXS<1>(p.ordinal,p,daeOrdinal0,daeOrdinals))
			{
				ordered_OK = false; break;
			}
			ins = p_content._append_child<0>();
			ins->_child.ordinal = p.ordinal;		
			s.ordered_mapping[i] = p.ordinal+sub;
		}
	}
	if(!demote)
	if(ordered_OK) //Fill-out ordered_places?
	{
		//TRICKY: Save the last inserted ordinal.
		const daeOrdinal a = p.ordinal;

		typedef void p; //SHADOWING
		//This prevents __tryCM() from promoting
		//(modifying) the content, while probing.
		daeCM_Placement<_solution> pp(0,0,s);

		//NOTE, HERE ordered_places already holds the mask
		//that is calculated for unordered_places up above.							
		for(int i=lo;i<hi;i++) switch(s.ordered_places[i])
		{
		case 1: pp.name = i; 
		//NOTE: This is calculating a default place in the
		//back. XS::Choice::_solution::place() replaces it.
		if(DAE_OK==toCM.__placeXS<1>(a,pp,daeOrdinal0,daeOrdinals))			
		{
			s.ordered_places[i] = pp.ordinal; break;
		}
		default: s.ordered_places[i] = 0; case 0: break;
		}
	}
	else if(OK) //Indicates ordered_mapping is in error.
	{
		memset(s.ordered_places+lo,0x00,names*sizeof(daeOrdinal));
	}

	//NEW: KEEPING APART
	//_solution::place() can't call __placeXS() then.
	if(!OK||!ordered_OK) s.ordered_mapping = nullptr;

	//Preventing daeContent::~daeContent().
	p_content.getAU()->setInternalCounter(0); return itt;
} 

template<class A, class B>
inline daeOrdinal XS::Choice::_solution::place(A,int,B,daeOrdinal)const
{
	daeCTC<0>(); //TODO: not _placeElement() nor _placeElementBetween().
}
template<>
inline daeOrdinal XS::Choice::_solution::place(void*, int name, void*, daeOrdinal)const
{
	return unordered_places[name];
} 
template<>
inline daeOrdinal XS::Choice::_solution::place(daeOrdinal a, int name, daeOrdinal b, daeOrdinal o)const
{
	//Note, the below code assumes k starts here.
	size_t k = keysN-1;

	//This is the optimize default for back insertion.
	if(keys[k]>=a) return ordered_places[name];

	//This is indicating that there is no ordered solution. 
	if(ordered_mapping==nullptr) return 0;

  ////THIS IS A PROPOSED ROUTE, THAT IS POSSIBLE IN SOME CASES////

	#ifdef _DEBUG 
	//It must be proved there's a 1:1 name:ordinal relationship.
	if(ordered_places==unordered_places)
	{
		assert(0); //THIS IS NOT IMPLEMENTED
		daeOrdinal p = ordered_places[name]; 	
		//Return if no range tests are required.
		if(p==0||a>keys[0]&&b<keys[k]) 
		return p;
		//Could be a binary-search, but will be small typically.
		for(k=0;ordered_mapping[k]>p;k++);
		//Past end?
		if(k==keysN) return b>=keys[keysN-1]?0:p; 
		//else
		return keys[k]>=a||b>keys[k]?0:p;
	}
	#endif

  ////THIS SIMULATES INSERTION INTO A MINIATURE CONTENTS-ARRAY////

	//HERE b IS UNCHARACTERISTICALLY DONE FIRST, BECAUSE k==keysN-1.

	if(b>=keys[0])
	{
		//This is mainly to prevent underflow on the loop below.
		//There's not a meaningful value for b if this doesn't hold.
		k = 0; assert(b==keys[0]);
	}
	else while(b>keys[k]) k--; //Could be a binary-search, but will be small typically.
	
	if(b!=ordered_mapping[k])
	{
		/*A 0-terminator exists at [keysN-1+1]*/
		//This is inexact, but works on the assumption that b belongs to existing content.
		b = /*k==keysN-1?0:*/ordered_mapping[k+1];
	}
	else b = ordered_mapping[k];	

	k = 0; while(keys[k]>a) k++; //Could be a binary-search, but will be small typically.	
	
	if(a!=ordered_mapping[k])
	{
		//This is inexact, but works on the assumption that a belongs to existing content.
		a = k==0?daeOrdinals:ordered_mapping[k-1];
	}
	else a = ordered_mapping[k];	
			
	daeCM_Placement<_solution> p
	(name,0,const_cast<_solution&>(*this));
	if(DAE_OK==to->__placeXS<1>(a,p,b,o)) return p.ordinal; return 0;
} 

template<class A, class B>
inline bool XS::Choice::_solution::requires_rearrange(A,B)const
{
	daeCTC<0>(); //TODO: not _placeElement() nor _placeElementBetween().
}
template<>
inline bool XS::Choice::_solution::requires_rearrange(void*,void*)const
{
	return ordered_mapping!=unordered_mapping; 
}
template<> 
inline bool XS::Choice::_solution::requires_rearrange(daeOrdinal,daeOrdinal)const
{
	assert(ordered_mapping!=nullptr); return false;
}

namespace //C++98/03 (C2918)
{
	struct daeMetaElementAttribute__tryCM_pred //COMPARATOR
	{
		typedef XS::Choice::_solution _solution;
		int cmp(const daeContent *q, const _solution &b)const
		{
			daeOrdinal *p = b.keys;
			for(size_t i=b.keysN;i!=0;i--)
			{
				daeOrdinal qO = q++->_ordinal(), pO = *p++-sub; 

				if(qO!=pO) //Assume straight ordinals until a hiccup.
				{
					//May be a reoccurrence or text nodes.
					//TODO? q+=q->getKnownStartOfText().span();
					if(qO>=q[-2]._ordinal()&&i!=b.keysN)
					{
						qO = (q-=2)->_ordinal();
						while(q->_ordinal_magic()>=qO) q++;

						//Repeat the original logic after adjustment.
						qO = q++->_ordinal();
						if(qO==pO) continue;
					}
					return qO<pO;
				}
			}
			//result saves 0 so to avoid doing extra tests.
			while(q->hasText()) q+=q->getKnownStartOfText().span();
			//itt is used to tell if there is text/repeats.
			itt = q; 
			return result = q->_ordinal()>sub-max?0:+1; 
		}
		bool operator()(const daeContent *a, const _solution &b)const
		{				
			return b.to==to?b.fro==fro?cmp(a,b)<0:fro<b.fro:to<b.to;
		}
		bool operator()(const _solution &a, const daeContent *b)const
		{
			return a.to==to?a.fro==fro?cmp(b,a)>0:a.fro<fro:a.to<to;
		}
		const daeCM *to,*fro; daeCounter sub,max;
		//These two gather datapoints for optization only.
		mutable int result; mutable const daeContent *itt;
	};
}
template<class A, class B>
daeError XS::Choice::__tryCM(daeContent *text, daeContent *it, size_t to, size_t fro, A a, daeCM_Placement<> &p, B b, daeOrdinal o)const
{
	const daeCM *toCM = _CM[to], *froCM = _CM[fro];

	daeCounter sub = daeOrdinals-o; daeCTC<sizeof(sub)==sizeof(o)>();
	
	daeMetaElementAttribute__tryCM_pred //C++98/03 (C2918)
	pred = { toCM,froCM,sub,_maxOrdinals,1 };
	if(_solution_cache->to!=toCM
	 ||_solution_cache->fro!=froCM
	 ||0!=pred.cmp(it,*_solution_cache))
	{
		std::vector<_solution>::iterator lb;
		lb = std::lower_bound(_solutions.begin(),_solutions.end(),it,pred);
		if(pred.result!=0) //Inserting (NOT-THREAD-SAFE)
		{
			lb = _solutions.insert(lb,_solution()); 
			
			pred.itt = _solve(*lb,it,to,fro,o); 
		}
		_solution_cache = &*lb;
	}
	const _solution &s = *_solution_cache;	
	
	if(s.unordered_places==nullptr) //Demoting?
	{	
		//Should hold if __tryWithout() is calling.
		assert(_proclaimCM[fro].find(to)!=_DeepCM_string::npos);
		//HACK? FROM THE LOOKS OF THINGS THIS IS HOW TO SEE
		//THAT ONE OF THE NAMES DIDN'T BELONG IN THE SUBSET.
		if(s.keys==s.unordered_mapping) return DAE_ERROR;
	}
	else //Promoting
	{
		//Should hold if __placeOccurrence2() is calling.
		assert(_proclaimCM[to].find(fro)!=_DeepCM_string::npos);

		daeOrdinal s_place = s.place(a,p.name,b,o+sub);

		if(0==s_place) return DAE_ERROR;

		p.ordinal = s_place-sub;			
	}

	daeError e; if(s.requires_rearrange(a,b))
	{
		e = DAE_ORDER_IS_NOT_PRESERVED;
		s.rearrange(text,pred.itt,p.content,sub);
	}
	else //The mapping is in the key order.
	{
		size_t i = s.keysN; e = DAE_OK; 

		daeOrdinal ito, p_sub, *p = s.ordered_mapping;

		if(i==pred.itt-it) //Trivial?
		{
			do it++->_child.ordinal = *p++-sub; 
			while(--i>0); 
		}
		else do //There are either reoccurrences or text involved.
		{
			ito = it->_child.ordinal;
		
			it++->_child.ordinal = p_sub = *p-sub; p++;
		
			while(ito<=it->_child.ordinal)
			{
				while(ito==it->_child.ordinal) it++->_child.ordinal = p_sub;

				while(it->hasText()) it+=it->getKnownStartOfText().span();
			}

		}while(--i>0);		
	}
	if(s.unordered_places!=nullptr) //Promoting?
	//Whatever is done, insertion_point needs to be set/not nullptr.
	p.insertion_point = p.content._upper_bound_with_text(p.ordinal); return e;
}

void XS::Choice::_solution::rearrange(daeContent *text, const daeContent *itt, daeContents &c, daeOrdinal sub)const
{	
	#ifdef NDEBUG
	#error SORTING IS NOT IMPLEMENTED. 
	#error IMPLEMENT IT OR ADD A RELEASE-MODE ALERT.
	#endif
	assert(0);
	//THIS WASN'T WORKED ON EITHER FOR TIME OR TO TAKE A BREAK.
	//COLLADA'S SCHEMA LIKELY DO NOT REQUIRE IT.
	//THE ALGORITHM IS AS FOLLOWS:
	//1) Iterate over the ordinals, setting them equal to the spans.
	//That is, if there's an N repeating ordinal, it's span is N. If
	//there are text-units, they are 1 span each.
	//(With itt-text steps 1&3 can be merged.)
	//2) Copy the affected content to memory lying beyond end().
	//grow() the array if necessary.
	//3) Build an index in the back of the space that was copied from.
	//The index is the sum of the spans.
	//(With itt-text steps 1&3 can be merged.)
	//4) _solution needs a new member that uses the index to copy without
	//actually sorting anything.
	//5) Use that member and the index to move the copy, piece by piece,
	//-back to the original/final resting place, in the new order, and set the 
	//ordinals there according to unordered_mapping. 
	//6) Double-check that c.cursor() is on a child node, and not a text-unit.
	//7) NOW MEMORY IN THE BACK OF THE CONTENTS-ARRAY IS EXPECTED TO BE 0 FILLED.
}

void daeCM::_chooseWithout(daeCM_Demotion<> &d)const
{
	if(d.count>0) //This should help large arrays.
	{	
		daeCursor it = d.insertion_point+1;
		while(it->hasText()&&it<d.content.end())
		it+=it->getKnownStartOfText().span();
		if(d.ordinal==it->_ordinal_magic())
		return; //Reoccurs.
		it = d.insertion_point-1;
		while(it->hasText()&&it>=d.content.begin())
		it-=it->getKnownEndOfText().counterspan();
		if(d.ordinal==it->_ordinal_magic()&&it>=d.content.begin())
		return; //Reoccurs.
	}
	_chooseWithout2(d,daeOrdinals);
}
daeContent *daeCM::_chooseWithout2(daeCM_Demotion<> &d, daeOrdinal o)const
{	
	assert(o>_ordSubtrahend&&o>=d.ordinal); 
	o-=_ordSubtrahend; 

	switch(_xs)
	{		
	case XS::SEQUENCE: case XS::CHOICE: break;
	default: return const_cast<daeContent*>(d.insertion_point);
	case XS::GROUP: 
	{
		//This is analagous to XS::Group::__placeOccurrence().
		XS::Group &_this = (XS::Group&)*this;
		int swap = d.name; d.name = _this._groupNames[d.name];
		daeContent *floor = _this._groupCM->_chooseWithout2(d,o);
		d.name = swap; return floor;
	}}	

	//This is analagous to daeCM::__placeT().
	daeOrdinal oM = _maxOrdinals; o-=(o-d.ordinal)/oM*oM;

	//This is analagous to XS::Sequence::__placeOccurrence().
	daeParentCM &_this = (daeParentCM&)*this;
	daeParentCM::_DeepCM_string::const_iterator it,itt;
	for(it=_this._deepCM[d.name].begin(),itt=_this._deepCM[d.name].end();it!=itt;it++)	
	{
		const daeCM *nameclash = _this._CM[*it];
		if(nameclash->__between((void*)0,o-nameclash->_ordSubtrahend,(void*)0,&daeCM::_maxOrdinals_x_maxOccurs))
		{	
			//floor/ceiling are the namefellow scan window.
			daeContent *floor = nameclash->_chooseWithout2(d,o);

			//This return is to delay scanning if unneeded.
			XS::Choice &_this = (XS::Choice&)*this;	
			if(_this._xs!=XS::CHOICE||_this._proclaimCM[*it].empty()) 
			return floor;

			if(d.count>0) //Any namefellow prevents processing.
			{
				//nullptr signals the presence of a namefellow.
				if(floor==nullptr) return nullptr;

				daeChildID aligned_name(d.name,0);
				daeCursor ceiling = d.insertion_point+1;
				for(oM=o-oM;oM<ceiling->_ordinal_magic();)
				{
					if(ceiling->hasText())
					ceiling+=ceiling->getKnownStartOfText().span();					
					else if(aligned_name.getName()==ceiling->getKnownChild().name())
					return nullptr; //Namefellow.
					else ceiling++;
				}
				while(--floor>=d.content.begin())
				{
					if(floor->hasText())
					floor-=floor->getKnownEndOfText().counterspan()-1;					
					else if(o<floor->_ordinal_magic())
					break;
					else if(aligned_name.getName()==floor->getKnownChild().name())
					return nullptr; //Namefellow.
				}
				floor++; 

				//HACK: this is how ceiling is passed backward.
				d.insertion_point = ceiling;
			}
			else floor = d.content._rewind_ordinal(d.content._find_ordinal(floor),o);

			_this.__tryWithout(floor,*it,d,o); return floor;
		}
	}	
	assert(0); return nullptr; //Programmer error?
}
void XS::Choice::__tryWithout(daeContent *text, size_t fro, daeCM_Demotion<> &d, daeOrdinal o)const
{
	daeContent *it = d.content._find_ordinal(text);

	if(it->_ordinal()<=o-_maxOrdinals) return; //Should imply the choice is empty.

	_DeepCM_string &to = _proclaimCM[fro];

	size_t i = to.size(); assert(i!=0); //_chooseWithout2() is ensuring this.

	//TODO? IS A CONSTRAINED REMOVAL OPTION APPROPRIATE?
	//Best? This just works down the list from the largest subset to the smallest.
	while(i-->0) switch(__tryCM(text,it,to[i],fro,(void*)0,d,(void*)0,o)) 
	{
	case DAE_ORDER_IS_NOT_PRESERVED: 
		
		d.maybe_demote_ORDER_IS_NOT_PRESERVED = DAE_ORDER_IS_NOT_PRESERVED;

	case DAE_OK: break; //falling through
	}	
}

//---.
}//<-'

/*C1071*/