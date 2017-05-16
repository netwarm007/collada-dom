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
 
daeOK daeURI_base::refresh()const
{
	//NEW: URIs are typically temporary now.
	//So try to screen out very common # URIs
	//and absolute URIs are unexpected to move.
	if(getIsResolved())
	if(isAbsoluteURI()||isSameDocumentReference())
	return DAE_OK;

	const_daeDOMRef DOM = getDOM();
	if(DOM==nullptr) return DAE_ERR_INVALID_CALL;
	return DOM->getPlatform().resolveURI(*this,*DOM);
}
daeOK daeURI_base::resolve(const daeDOM *DOM)const
{
	if(getIsResolved()||DOM==nullptr&&nullptr==(DOM=getDOM()))
	return DAE_ERR_INVALID_CALL;			
	daeError out = DOM->getPlatform().resolveURI(*this,*DOM);
	assert(out!=DAE_OK||getIsResolved()); return out;
}

daeOK daeURI_base::_setURI(daeString URI, const daeURI *baseURL)
{
	if(URI==nullptr) URI = ""; bool slashed;

	//Will need this anyway, so lock the parent.
	const_daeObjectRef lock = &getParentObject();

	 //Doc-Aware URI Prologue:
	//The document must be lifted-up out of its archive,
	//or else, _docInsert will be spoiled by the new URI.
	//_movingDoc(doc) is performed below, in order to not
	//have to put it right back on error/multi-thread logic.
	daeDoc *doc = (daeDoc*&)lock;
	if(doc->_isDoc()&&this==&doc->getDocURI()) //is doc URI?
	{
		//_setURI_op is protecting against recursive calls and
		//returns DAE_NOT_NOW if it's called on another thread.
		const_daeDOMRef DOM = doc->getDOM();
		daeOK OK = doc->_doOperation<doc->_setURI_op>(DOM,URI);
		switch(OK.error)
		{
		default: return OK; //Failed.
			
		//_doOperation agrees to use DAE_ERR_NOT_IMPLEMENTED in this
		//case to signal success.
		case DAE_ERR_NOT_IMPLEMENTED: //Finishing up.

			//Force document URIs to be absolute URIs.
			if(0!=_rel_half!=0||0!=_rel_backtracks)
			{			
				//TODO: Warn if the URI is nonstandard.
				//Give daePlatform::resolveURI() access to _rel_half/backtracks.
				_rel_half = _rel_backtracks = 0;
			}
			if(doc!=&DOM->_closedDocs) //Hack? This is a special archive.
			{
				//"Front-door" into docLookup() doing re-insertion logic.
				_docHookup<0>(const_cast<daeDOM&>(*DOM),(daeDocRef&)lock);
			}
			return DAE_OK;

		case DAE_OK: break; //recursive
		}
	}
	else doc = nullptr; //working-backward

	 //Base-Suitability Prologue:
	//Bases must have both a scheme and an authority part.
	if(baseURL!=nullptr&&3>=baseURL->getURI_authorityCP())
	{
		//This is a courtesy so "" can be passed as a base.
		if(baseURL->empty())
		{
			baseURL = nullptr; goto empty_base;
		}

		 //return DAE_ERR_INVALID_CALL;
		//Instead of returning an error, see if a base is
		//required or not. If so, return error. If not, it
		//turns out that the parsed results can be retained.
		daeURI_parser parser(URI);
		if(parser.isRelativeURI()) return DAE_ERR_INVALID_CALL;		

	if(doc!=nullptr)
	doc->getArchive()._movingDoc(doc); ////POINT-OF-NO-RETURN////

		short *p = &_rel_half, *q = &parser._rel_half;
		while(p<=&_size) *p++ = *q++; goto parsed_absolute_URI;
	}	
	if(doc!=nullptr)
	doc->getArchive()._movingDoc(doc); ////POINT-OF-NO-RETURN////

empty_base: //// RESUME THE REGULAR ALGORITHM ////

	slashed = false; //goto slash;

	_00(); //Unset placemarkers are to be filled in.
	for(daeString q,pp,p=URI;;) switch(toslash(*p++))
	{		  
	case '.': //../
		//Note, ./ is not handled.
		//Resolvers must remove them
		//manually. Base concatenation
		//is defined in terms of leading
		//../ directives.
		if(p[0]=='.'&&'/'==toslash(p[1]))
		if(!slashed&&baseURL!=nullptr) 
		{
			p+=2; _rel_backtracks++;
		}
		break;

	case ':': //protocol, password or port
	
		//Note: schemes with only a :, or
		//that is, without ://, are not parsed
		//as having a protocol/authority. They are
		//permitted to have a query and/or a fragment.
		if(_authority==0)
		if('/'==toslash(p[0])&&'/'==toslash(p[1]))
		{
			p+=2; _authority = p-URI; goto nonquery;
		}
		else //Skip "hierarchical-part" to be clear.
		{
			slashed = true; //Note the path is present.

			while(*p!='?'&&*p!='#'&&*p!='\0') p++;	
		}
		else //The password or communication port number.
		{
			pp = p; while(*p!='@'&&'/'!=toslash(*p)&&*p!='\0') p++;
			(*p=='@'?_authority_password:_authority_port) = pp-URI;
		}
		break;
	
	case '@': _authority_host = pp-URI; break; //host	
	case '/':
	
		//Even though could be a double slash,
		//the modern convention is to see these as schemeless URIs.
		if('/'==toslash(p[0])&&p==URI+1) //?
		{
			p+=2; _authority = 2; 

			//This sets _rel_half and _authority.
			//(_concat recognizes _authority as nonzero.)
			if(baseURL==nullptr) 
			goto nonquery;
			goto protocol;
		}
		else if(0==_authority) //relative URL?
		{
			//Here 3 is strlen("../").
			//Roll back for rel.xml#top style URLs.
			//(As they do not have a / to indicate a path.)
slash: 		p = URI+3*_rel_backtracks; 
			if(baseURL!=nullptr) 
			{
protocol:		_setURI_concat(*baseURL,_rel_backtracks,p);
				p = URI = data();
				p+=_rel_half;
				if(_rel_half==_authority) //goto protocol;
				{
					//Assuming a \\?\ style Windows' path.
nonquery:			if(p[0]=='?'&&'/'==toslash(p[1])) p++;
					continue;
				}
			}
			else assert(_path==0&&_rel_half==0); 
		}
		else _path = p-1-URI; 		

		slashed = true; //Note the path is present.
		
		pp = p; while(*p!='?'&&*p!='#'&&*p!='\0') p++;	   
		q = p-1; if(toslash(*q)=='/') continue; //explicit directory
		else while(q>=URI) switch(toslash(*q--)) //easier to read this way
		{
		case '.': if(0==_path_extension) _path_extension = q+2-URI; break;
		//long break
		case '/': _path_filename = q+2-URI; q = URI; goto hidden_file;
		}
		hidden_file: //?
		if(_path_filename==_path_extension-1) _path_extension = p-URI;
		break;
	
	case '?': case '#': case '\0':
		
		if(p>URI+1&&!slashed) goto slash;
	
		switch(p[-1])
		{
		case '?': _query = p-URI; while(*p!='#'&&*p!='\0') p++; break;
	
		case '#': _fragment = p-URI; while(*p!='\0') p++; break;
	
		case '\0': _size = p-URI; goto long_break;		
		}
	}
	long_break: //This algorithm fills in unset placemarkers.
	{
		short nz = 1; //nonzero		
		short *z = 0==_path?&_path_extension:&_authority_password; 				
		for(short *x=&_fragment;x>=z;x--) 
		if(*x==0){ *x = x[1]-nz; nz = 0; }else nz = 1;		
		//HACK: The above algorithm is heuristical.
		if(*z<_authority) for(;z<&_path;z++) *z+=1;
	}
	assert(slashed||URI[0]=='\0');
	assert(_size>0);
	parsed_absolute_URI: setIsResolved(false); 
	//Here, URI may've already been assigned by _setURI_concat,
	//-or, something like parseURI may be in play. A string cannot
	//overwrite itself, regardless.
	if(URI!=data()) 
	{
		if(doc!=nullptr&&_fragment+1!=_size) 
		{
			_size = _fragment+1; assert('#'==URI[_size-2]);
			_this()._refString.setString(*this,URI,_size,'\0');
		}
		else _this()._refString.setString(*this,URI,_size);
	}

	return DAE_OK;
}
void daeURI_base::_setURI_concat(const daeURI &base, size_t trim, daeString rel)
{	
	//Protocol-relative is a modern // URL.
	bool protocol_relative = _authority!=0;
	size_t i, too_far = base.getURI_pathCP();
	if(protocol_relative) 
	{	
		i = base.getURI_authorityCP(); //protocol-relative
		assert(trim==0&&_authority==2); // URL?
	}
	else switch(trim==0?toslash(*rel):-1) //-1 if ../ trimming
	{
	case '/': i = base.getURI_pathCP(); break; //root local
	default: i = base.getURI_filenameCP(); break; //path local
	case '?': i = base.getURI_uptoCP<'?'>(); break; //query[fragment]
	case '\0': //RFC3986 says remove the base's #
	case '#': i = base.getURI_uptoCP<'#'>(); break; //local fragment 	
	}
	daeArray<daeStringCP,520> buf; 
	buf.assign(base.data(),i+1);	
	if(i<=base.getURI_filenameCP()) //Not ? nor #?
	{
		//Trim the ../ directives?
		while(i>too_far&&'/'!=toslash(buf[i])) 
		i--;
		for(;i>too_far&&trim>0;trim--)
		{
			if(buf[i-1]=='.') switch(toslash(buf[i-2]))
			{
			case '/': trim++; break; //This is /./
			case '.': if(toslash(buf[i-3])=='/') trim+=2; break; //This is /../
			}
			for(i--;i>too_far&&toslash(buf[i])!='/';) 
			i--;
		}
		if(!protocol_relative) //Don't end up with :///.
		{
			//Ensure / separates base and relative part.	
			if(toslash(buf[i])=='/')
			{
				i++; if(toslash(*rel)=='/') rel++; 
			}	
			else buf[i++] = '/';
		}
	}
	//Here 3 is strlen("../"), and 1 is sizeof('\0').
	size_t size = i+3*trim+strlen(rel)+1; buf.grow(size);		
	//Note: this is done prior to adjusting i by 3*trim.
	const short *cpyN = &base._path;
	const short *cpy = &base._authority_password;	
	if(!protocol_relative) while(cpy<=cpyN&&*cpy<=(short)i) cpy++;	
	//Copy over base's placemarkers. _rel_half can wait.
	memcpy(&_authority,&base._authority,(char*)cpy-&base._authority);
	//This URL, if valid, is going behind the webserver 
	//root. This is not disallowed, and so is preserved.
	for(;trim>0;trim--) 
	{
		buf[i++] = '.'; buf[i++] = '.'; buf[i++] = '/';
	}
	//Note: _rel_backtracks is not set by this subroutine.
	_rel_half = (short)i;
	memcpy(buf.data()+i,rel,(size-i)*sizeof(buf[0]));
	_this()._refString.setString(*this,buf.data(),size);
}

//THIS PRE-2.5 CODE COULD USE A REVIEW.
//THIS PRE-2.5 CODE COULD USE A REVIEW.
//THIS PRE-2.5 CODE COULD USE A REVIEW.
//This code is loosely based on the RFC 2396 normalization code from
//libXML. Specifically it does the RFC steps 6.c->6.g from section 5.2
//The path is modified in place, there is no error return.
static size_t daeURI_cpp_RFC3986_normalize(daeStringCP *const path)
{
	daeStringCP *cur; //location we are currently processing
	daeStringCP *out; //Everything from this back we are done with

	//Skip any initial / characters to get us to the start of the first segment
	for(cur=path;*cur=='/';cur++);

	//Return if we hit the end of the string
	if(*cur=='\0') return cur-path;

	//Keep everything we've seen so far.
	out = cur;

	//Analyze each segment in sequence for cases (c) and (d).
	while(*cur!='\0')
	{
		//(c) All occurrences of "./", where "." is a complete path segment, are removed from the buffer string.
		if(cur[0]=='.'&&cur[1]=='/')
		{
			cur+=2;
			//If there were multiple slashes, skip them too
			while(*cur=='/') cur++;
			continue;
		}

		//(d) If the buffer string ends with "." as a complete path segment, that "." is removed.
		if(cur[0]=='.'&&cur[1]=='\0')
		break;

		//If we passed the above tests copy the segment to the output side
		while(*cur!='/'&&*cur!='\0')
		{
			*out++ = *cur++;
		}

		if(*cur!='\0')
		{
			//Skip any occurrances of //at the end of the segment
			while(cur[0]=='/'&&cur[1]=='/') cur++;

			//Bring the last character in the segment (/ or a null terminator) into the output
			*out++ = *cur++;
		}
	}

	*out = '\0';

	//Restart at the beginning of the first segment for the next part
	for(cur=path;*cur=='/';cur++); 

	//Necessary? Assuming cur==out.
	if(*cur=='\0') return cur-path; 

	//Analyze each segment in sequence for cases (e) and (f).
	//
	//e) All occurrences of "<segment>/../", where <segment> is a
	//    complete path segment not equal to "..", are removed from the
	//    buffer string.  Removal of these path segments is performed
	//    iteratively, removing the leftmost matching pattern on each
	//    iteration, until no matching pattern remains.
	//
	//f) If the buffer string ends with "<segment>/..", where <segment>
	//    is a complete path segment not equal to "..", that
	//    "<segment>/.." is removed.
	//
	//To satisfy the "iterative" clause in (e), we need to collapse the
	//string every time we find something that needs to be removed.  Thus,
	//we don't need to keep two pointers into the string: we only need a
	//"current position" pointer.
	//
	for(;;)
	{
		daeStringCP *segp, *tmp;

		//At the beginning of each iteration of this loop, "cur" points to
		//the first character of the segment we want to examine.

		//Find the end of the current segment.
		for(segp=cur;*segp!='/'&&*segp!=0;++segp);

		//If this is the last segment, we're done (we need at least two
		//segments to meet the criteria for the (e) and (f) cases).
		if(*segp==0) break;

		//If the first segment is "..", or if the next segment _isn't_ "..",
		//keep this segment and try the next one.
		++segp;
		if(*cur=='.'&&cur[1]=='.'&&segp==cur+3
		||(*segp!='.'||segp[1]!='.'||segp[2]!='/'&&segp[2]!='\0'))
		{
			cur = segp; continue;
		}

		//If we get here, remove this segment and the next one and back up
		//to the previous segment (if there is one), to implement the
		//"iteratively" clause.  It's pretty much impossible to back up
		//while maintaining two pointers into the buffer, so just compact
		//the whole buffer now.

		//If this is the end of the buffer, we're done.
		if(segp[2]=='\0')
		{
			*cur = '\0'; break;
		}

		//Strings overlap during this copy, but not in a bad way, just avoid using strcpy
		tmp = cur;
		segp+=3;
		while((*tmp++=*segp++)!='\0');

		//If there are no previous segments, then keep going from here.
		segp = cur;
		while(segp>path&&*--segp=='/');

		if(segp==path) continue;

		//"segp" is pointing to the end of a previous segment; find it's
		//start.  We need to back up to the previous segment and start
		//over with that to handle things like "foo/bar/../..".  If we
		//don't do this, then on the first pass we'll remove the "bar/..",
		//but be pointing at the second ".." so we won't realize we can also
		//remove the "foo/..".
		for(cur=segp;cur>path&&cur[-1]!='/';cur--);
	}

	*out = '\0';

	//g) If the resulting buffer string still begins with one or more
	//    complete path segments of "..", then the reference is
	//    considered to be in error. Implementations may handle this
	//    error by retaining these components in the resolved path (i.e.,
	//    treating them as part of the final URI), by removing them from
	//    the resolved path (i.e., discarding relative levels above the
	//    root), or by avoiding traversal of the reference.
	//
	//We discard them from the final path.

	if(*path=='/')
	{
		for(cur=path;*cur=='/'
		&&(cur[1]=='.')
		&&(cur[2]=='.')
		&&(cur[3]=='/'||cur[3]=='\0');cur+=3);

		if(cur!=path)
		{
			for(out=path;*cur!='\0';*(out++)=*(cur++)); 
			
			*out = '\0';
		}
	}

	return out-path;
}
static void daeURI_cpp_RFC3986_decode(daeStringCP *const pp)
{
	daeStringCP *p = pp, *q = p; while(*p!='\0')
	{
		if(*p=='%')
		{
			//toupper is canonicalizing.
			int a = p[1] = toupper(p[1]);
			if((a-=(a>='A'?'A':'0'))>=0&&a<=15)
			{
				int b = p[2] = toupper(p[2]);
				if((b-=(b>='A'?'A':'0'))>=0&&b<=15)
				{
					int code = (a<<4)+b; switch(code)
					{
					case ':': case '/':	case '?': case '#': 
					case '[': case ']':	case '@': case '%':
						*q++ = *p++;
						*q++ = *p++;
						*q++ = *p++; continue; //reserved codes
					default: assert(code<=255);						
						*q++ = (daeStringCP&)code; p+=3; continue;
					}
				}
			}			
			//There's ample room for improvement here.
			daeEH::Warning<<"Invalid % sequence in URI:\n"<<
			pp<<"\n"
			"At:\n"<<
			p;
		}
		*q++ = *p++;
	}
	*q = '\0';
}
static void daeURI_cpp_RFC3986_lower(daeStringCP *p, const daeStringCP *const pN)
{
	for(;p<pN;p++)
	if((daeUStringCP)*p<=127) switch(*p=tolower(*p))
	{
	case '%': if(p+3>=pN) continue; 
		//Assuming legit percent-encoding.
		//Canonical percent-encodings are uppercase!
		p++; *p = toupper(*p); p++; *p = toupper(*p);
	}
}
daeOK daeURI_base::resolve_RFC3986(const daeDOM &DOM, int ops)
{	
	//This is made quite complicated to allow for 
	//document-less relative/compounded URIs that
	//will be stripped of their relative-ness now
	//that they are resolved.	
	const_daeURIRef base;	
	if(ops&RFC3986::rebase)
	baseLookup(DOM,base);	
	daeStringCP *buf_str; 
	daeArray<daeStringCP,520> buf;
	if(base!=nullptr)
	{
		buf_str = getURI_baseless(buf).data();		
	}
	else if(_this()._refString.isView()) //OPTIMIZING
	{
		buf_str = buf.assign_and_0_terminate(data(),size()).data();
	}
	else buf_str = (daeStringCP*)data(); //OPTIMIZING
	
	daeURI_parser parser(buf_str);	
	daeStringCP d,*p,*const pp = buf_str;
	{
		typedef void buf,buf_str;

		if(ops&RFC3986::toslash)
		for(p=pp;*p!='\0';p++) *p = toslash(*p);
		if(ops&RFC3986::lower)
		{
			//Assuming legit percent-encoding.
			//Canonical percent-encodings are uppercase!
			daeURI_cpp_RFC3986_lower(pp,pp+parser.getURI_authorityCP());
			daeURI_cpp_RFC3986_lower(pp+parser.getURI_hostCP(),pp+parser.getURI_portCP());
		}		
		if(ops&RFC3986::normalize)
		{
			//This is unfinished.
			#ifdef NDEBUG
			#error must prepend ../ overflow.
			#endif
			//daeURI_cpp_RFC3986_normalize could use work.
			p = pp+parser.getURI_uptoCP<'/'>();		
			//daeURI_cpp_RFC3986_normalize must start after the leading ../s.
			while(p[0]=='.'&&p[1]=='.'&&'/'==p[2]) p+=3;
			//daeURI_cpp_RFC3986_normalize is 0 terminator based.
			daeStringCP *pathN = pp+parser.getURI_uptoCP<'?'>();	
			d = *pathN; *pathN = '\0';
			p+=daeURI_cpp_RFC3986_normalize(p); 			
			*pathN = d; //repair *pathN = '\0';			
			//Move query/fragment back into place.
			//This invalidates parser from here on.
			memmove(p,pathN,(parser.size()+1-(pathN-pp))*sizeof(daeStringCP));
		}typedef void parser;				
		if(ops&RFC3986::decode) daeURI_cpp_RFC3986_decode(pp);		
	}
	//RFC3986 Section 5. Reference Resolution
	//Note: _setURI will mark itself unresolved.
	daeOK out = _setURI(pp,base); setIsResolved(); return out;
}
const_daeURIRef &daeURI_base::baseLookup(const daeDOM &DOM, const_daeURIRef &base)const
{
	//If these conditions are not met then the base will probably produce a bad URI.
	if(!is_baseLookup_friendly()) return base;

	const_daeDocRef doc = getDoc();
	base = &(doc==nullptr?DOM.getDefaultBaseURI():doc->getDocURI());
	if(this==base) base = &(doc!=nullptr?DOM.getDefaultBaseURI():DOM.getEmptyURI());
	assert(base->getIsResolved()); return base;
}	
	  
typedef struct //C++98/03 (C2918)
{ 
	bool operator()(const daeDocRef &a, daeString b)const
	{
		return strcmp(a->getDocURI().data(),b)<0; 
	}
}daeURI_docHookup_less;	
template<int doing_docLookup>
//NOTE: THIS IS DOUBLING AS THE docLookup() PROCEDURE.
void daeURI_base::_docHookup(daeArchive &a, daeDocRef &reinsert)const
{
	daeURI_docHookup_less less; //C++98/03 (C2918)	

	daeString URI = data();		
	const daeArray<daeDocRef> &docs = a.getDocs();
	const daeDocRef *b = docs.begin(), *e = docs.end();	
	const daeDocRef *lb = std::lower_bound(docs.begin(),e,URI,less);
	
	daeDoc *match = nullptr; 
	
	a._whatsupDoc = lb-b; //hint

	if(lb!=b) //looking behind by one
	{
		const daeURI &behind = lb[-1]->getDocURI();

		//First handle case where # thows off lower_bound.
		if(behind._size==_fragment&&referencesURI(behind)) 
		{
			match = lb[-1]; a._whatsupDoc-=1; //hint
		}
		else if(lb[-1]->isArchive()&&transitsURI(behind)) //Recurse?
		{
			return _docHookup<doing_docLookup>(static_cast<daeArchive&>(*lb[-1]),reinsert);
		}		
	}
	//Still unmatched?
	if(match==nullptr&&lb!=e&&referencesURI((*lb)->getDocURI()))
	{
		match = *lb;
	}
	
	if(doing_docLookup) //docLookup mode?
	{
		reinsert = match; //Back-door: return match.
	}
	else if(match!=nullptr) //Front-door: reinserting.
	{			  			   		
		//Should this happen? Ever? Let's find out.
		if(match==reinsert){ assert(0); return; }		
		//This is a nonconsensual closure, via an identical URI.
		//It might make sense to reverse course at this late stage, if this
		//is not normally expected to occur.
		//Could daeURI::getIsUnique() be the idea?
		a._closedDoc(match,reinsert);
		//There's ample room for improvement here.
		daeEH::Warning<<
		"A doc was closed, replaced by an identical URI:\n"<<
		getURI();
	}
	else a._whatsupDocInsert(reinsert);
} 
void daeURI_base::_docLookup(const daeArchive &a, const_daeDocRef &result)const
{
	_docHookup<1>(const_cast<daeArchive&>(a),(daeDocRef&)result);
}

#ifdef NDEBUG
#error Don't neglect to test this. (The RAW resolver.)
#endif
////// RAW RESOLVER //////////////////////////////////////////////////////////////////
//This is old, crusty, legacy code. It's never been entirely sound, but is maintained.
typedef struct //C++98/03 (C2918)
{
	daeAlloc<>* &valArray; size_t long_count; FILE *rawFile;

}daeURI_write_RAW_file_data_args;
template<class S, class T>
static void daeURI_write_RAW_file_data(const daeURI_write_RAW_file_data_args &in)
{	
	daeArray<T> &a = (daeArray<T>&)in.valArray; a.setCountMore(in.long_count);

	T *data = a.data();

	if(sizeof(S)!=sizeof(T))
	{
		S val; for(size_t i=0;i<in.long_count;i++)
		{
			fread(&val,sizeof(S),1,in.rawFile); data[i] = val;
		}
	}
	else fread(data,sizeof(S)*in.long_count,1,in.rawFile);
}
daeOK daeRawResolver::_resolve_exported(const daeElementRef &hit, const daeURI &uri, daeRefRequest &req)const
{	
	if(hit!=nullptr) //In cache?
	{
		req.object = hit; goto hit;
	}
	else if(uri.getURI_extensionIs("raw")) //SCOPING FOR goto hit;
	{	
		//NEW: Not taking any chances inputs wise.
		//TODO: Ensure that these belong to a COLLADA ancestor.
		daeElementRef source;
		const_daeElementRef accessor = uri.getElementObject();
		if(accessor!=nullptr&&"accessor"==accessor->getNCName())
		{
			const_daeElementRef technique_common = accessor->getParentElement();
			if(technique_common!=nullptr&&"technique_common"==source->getNCName())
			source = const_cast<daeElement*>(technique_common->getParentElement());						
		}	
		if(source==nullptr||"source"!=source->getNCName())
		{
			daeEH::Error<<
			"daeRawResolver - URI is not <source><technique_common><accessor> embedded.";
			return DAE_ERR_INVALID_CALL;
		}
		const_daeElementRef param = accessor->getChild("param");				
		bool int_type = param!=nullptr&&"int"==daeHashString(param->getAttribute("type"));
		//NEW: The original code didn't do this. But it probably never cleared its cache??
		daeElementRef array = source->getChild(int_type?"int_array":"float_array");
		if(array==nullptr) //There's no data (potentially modified) that can be clobbered?
		{
			//NEW: The FILE set up code had been located much further up.

		  ////////////////////////////////////////////////////////////////////////////////////
		  //NOTE: Using a local URI as remoteURI is not ideal, but it's how things generally//
		  //work since there's no obvious, well-formed way to derive remoteURI from the URIs//
		  ////////////////////////////////////////////////////////////////////////////////////
				
			const_daeArchiveRef a = &source->getDocument()->getArchive();		
			daeIORequest rawReq(a,nullptr,nullptr,uri);
			const_daeDOMRef DOM = a->getDOM();		
			assert(DOM!=nullptr); /*if(DOM==nullptr) 
			{
				daeEH::Error<<"daeRawResolver - URI is not DOM-embedded.";
				return DAE_ERR_INVALID_CALL;
			}*/
			  
			daeRefView fragment;
			uri.getURI_fragment(fragment); daeStringCP *end;
			long byteOffset = strtol(fragment.view,&end,10); 
			if(end-fragment.view!=fragment.extent)
			{
				daeEH::Error<<
				"daeRawResolver - URI does not have a numeric fragment.\n"
				"Cannot be a file offest.";
				return DAE_ERR_INVALID_CALL;
			}	

			daeRAII::CloseIO closeIO(DOM->getPlatform()); //RAII
			daeIOEmpty rawO;
			daeIOSecond
			<daeIOPlugin::Demands::CRT
			|daeIOPlugin::Demands::unimplemented> rawI(rawReq); 
			FILE *rawFile;
			daeIO *_rawIO = closeIO = closeIO.p.openIO(rawI,rawO);
			if(nullptr==_rawIO
			 ||nullptr==(rawFile=_rawIO->getWriteFILE()))
			{
				if(_rawIO!=nullptr)
				daeEH::Error<<"Raw FILE error: "<<uri.getURI();
				daeEH::Error<<"RAW: Couldn't open secondary I/O channel for daeRawResolve::_resolve_exported.";
				return DAE_ERR_BACKEND_IO;		
			}

			//NEW: Now, resume with the original algorithm, where the RAW data is so far unloaded.
			array = source->add(int_type?"int_array":"float_array");
			if(array==nullptr||!array->setAttribute("id",std::string(source->getID_id())+="-array"))
			{
				assert(0); return DAE_ERR_INVALID_CALL; //No damage done so far.
			}

			#ifdef NDEBUG
			#error The types are not guaranteed to be daeULong. (Or even to exist.)
			#endif
			//There probably should be better APIs for this sort of thing.
			daeAttribute *count = accessor->getAttributeObject("count");
			daeAttribute *stride = accessor->getAttributeObject("stride");
			assert(sizeof(daeULong)==count->getSize());
			daeULong &long_count = array->getAttributeObject("count")->getWRT(array);
			long_count = (const daeULong&)count->getWRT(accessor);
			long_count*=(const daeULong&)stride->getWRT(accessor);			
		
			daeCharData *arrayCD = array->getCharDataObject();
			int atomic_type = arrayCD->getType()->per<daeAtom>().getAtomicType();			
			fseek(rawFile,byteOffset,SEEK_SET); assert(long_count<COLLADA_UPTR_MAX);
			daeURI_write_RAW_file_data_args args = { arrayCD->getWRT(array),(size_t)long_count,rawFile };

			//REMINDER: This shouldn't be a switch statement
			//so that it will compile if the types are equal.
			//switch(atomic_type)
			if(atomic_type==daeAtomicType::UINT) //UINT
			{
				daeURI_write_RAW_file_data<int,daeUInt>(args); 
			}
			else if(atomic_type==daeAtomicType::ULONG) //ULONG
			{
				daeURI_write_RAW_file_data<int,daeULong>(args); 
			}
			else if(atomic_type==daeAtomicType::FLOAT) //FLOAT
			{
				daeURI_write_RAW_file_data<float,daeFloat>(args); 
			}
			else if(atomic_type==daeAtomicType::DOUBLE) //DOUBLE
			{
				daeURI_write_RAW_file_data<float,daeDouble>(args); 
			}
			else goto type_unknown;

			if(0!=ferror(rawFile)) type_unknown:
			{
				daeEH::Error<<uri.getURI()<<"\n"
				"Raw FILE error (ferror) after reading file. Possible data loss. Too late to back out.";
			}
		}
		req.object = array; //miss
	}
	else return DAE_ERR_QUERY_SYNTAX;

hit: //fill out the request object
	const daeElement &e = (daeElement&)*req.object;
	daeCharData *cd = e.getCharDataObject();
	daeAlloc<>*AU = (daeAlloc<>*const&)cd->getWRT(&e);
	size_t n = AU->getCount();
	if(n!=0)
	{
		req.type = &cd->getType();
		req.typeInstance = AU->getRaw();		
		req.rangeMin = 0;
		req.rangeMax = n-1;
	}
	else req.typeInstance = nullptr; return DAE_OK;
}

//---.
}//<-'

/*C1071*/