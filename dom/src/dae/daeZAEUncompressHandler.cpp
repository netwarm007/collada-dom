/*
 * Copyright 2006 Sony Computer Entertainment Inc.
 *
 * Licensed under the MIT Open Source License, for details please see license.txt or the website
 * http://www.opensource.org/licenses/mit-license.php
 *
 */
#include <ColladaDOM.inl> //PCH

#ifdef __COLLADA_DOM__ZAE

COLLADA_(namespace)
{//-.
//<-'

/**HELPER
 * daeGZ is a candidate for exposing to clients in order
 * help download GZ compressed files. E.g. when the HTTP
 * header says the content is GZ-compressed. It's common.
 */
class daeGZ
{		
	char _32k_dictionary[32*1024]; 
	char _private_implementation[4096*4]; 	
	size_t _restart,_final_input;
	size_t _size;
	#ifdef NDEBUG
	#error Use daeArray<char> to expose daeGZ.
	#endif
	std::vector<char> _data;
	daeError _error;

COLLADA_(public)

	daeGZ(size_t fin){ reset(fin); }	
	
	inline void reset(size_t fin)
	{
		_data.clear(); _size = 0;

		_restart = 0; _final_input = fin; _error = DAE_OK;
	}

	//SCHEDULED FOR REMOVAL? Maybe data()?
	template<class T> inline operator T*()
	{
		assert(!empty()); return &_data[0]; 
	}

	inline size_t size()const{ return _size; }

	inline bool empty()const{ return _size==0; }

	inline void clear()
	{
		_data.erase(_data.begin(),_data.end()-getUnderflow());
		_size = 0;
	}	
	
	inline size_t getUnderflow()const
	{
		return _data.size()-_size; 
	}
	inline size_t getRemaining()const
	{
		return _final_input-_restart-getUnderflow(); 
	}

	/**
	 * @return Returns @c size() or 0.
	 */
	COLLADA_DOM_LINKAGE size_t inflate(const void *in, size_t in_chars);
		
	inline daeError getError()const{ return _error; }

	inline bool isOK()const{ return _error==DAE_OK; }
};

/**@todo Expose @c daeZAE?
 *
 * TO-DO LIST
 * Enable encryption features?
 * Convert non-ASCII IBM437 paths to UTF8.
 * 64-bit structures.

 4.4.5 compression method: (2 bytes)

        0 - The file is stored (no compression)
        1 - The file is Shrunk
        2 - The file is Reduced with compression factor 1
        3 - The file is Reduced with compression factor 2
        4 - The file is Reduced with compression factor 3
        5 - The file is Reduced with compression factor 4
        6 - The file is Imploded
        7 - Reserved for Tokenizing compression algorithm
        8 - The file is Deflated
        9 - Enhanced Deflating using Deflate64(tm)
       10 - PKWARE Data Compression Library Imploding (old IBM TERSE)
       11 - Reserved by PKWARE
       12 - File is compressed using BZIP2 algorithm
       13 - Reserved by PKWARE
       14 - LZMA (EFS)
       15 - Reserved by PKWARE
       16 - Reserved by PKWARE
       17 - Reserved by PKWARE
       18 - File is compressed using IBM TERSE (new)
       19 - IBM LZ77 z Architecture (PFS)
       97 - WavPack compressed data
       98 - PPMd version I, Rev 1
 */
class daeZAE : public daeAtlas
{
COLLADA_(public) //CENTRAL-DIRECTORY

	daeZAE():_initialized(){}
	bool _initialized; 
	enum{ _8192=8192 };
	daeError maybe_init(daeIO &IO);
	daeError _init_central_directory(daeIO*,char*,int,int,int);
	struct cde //VARIABLE-LENGTH
	{				
		//LOSSY! dates, comments, etc. won't be written.
		unsigned isize,dsize,lhpos; daeStringCP path[1];

		static bool less(cde *a, cde *b)
		{
			return strcmp(a->path,b->path)<0; 
		}
	};
	std::vector<cde*> CD; daeStringAlligator<cde,16> A;

	typedef std::vector<cde*>::const_iterator iterator;

	inline cde *find(daeString path)
	{
		cde *v = (cde*)(path-daeOffsetOf(cde,path));
		iterator it = std::lower_bound(CD.begin(),CD.end(),v,cde::less);
		if(it==CD.end()) return nullptr;
		int len = 0; while(path[len]!='\0'&&path[len]!='#') len++;
		if(0==memcmp(path,(*it)->path,len)&&'\0'==(*it)->path[len])
		return *it; return nullptr;
	}
	
COLLADA_(public) //HELPER

	//EXPERIMENTAL
	//This logic is kind of a mess. I'm not smart enough to come up with a 
	//better arrangment. Maybe daeAtlas can facilitate this sort of thing?
	struct trio
	{
		daeIOPlugin *first;
		const daeArchive *second;		
		cde *third;
		trio(daeIOPlugin *p, daeZAE *a)
		:first(p),second(p->getRequest().scope),third()
		{
			if(nullptr!=second) 
			for(;second->isArchive();second=&second->getArchive()) 			
			if(a==&second->getAtlas())
			{
				size_t zae = second->getDocURI().getURI_uptoCP<'?'>()+1;
				const daeURI *URI = p->getRequest().localURI;				
				if(URI->size()>zae)
				third = a->find(URI->data()+zae);
				else assert(0);
				return;
			}			
			//HACK: This I/O framework is not ideal in this hypothetical case.
			//Here either the I or O side is outside of the scope of this ZAE.
			second = p->getRequest().scope; 
		}		
	};

COLLADA_(public) //I/O
		
	struct io : daeIO 
	{				 
		struct rw : daeIOSecond<>
		{
			cde *e; 						
			int e_overflowed;
			daeIO *IO;
			daeIORequest req;			
			rw(daeIO *IO=nullptr)
			:e(),e_overflowed(-1),IO(IO) //piggybackI
			{}
			rw(const trio &p):daeIOSecond(req)
			,e(p.third),e_overflowed(-1),IO()
			,req(e==nullptr?nullptr:&p.second->getArchive())
			{
				#ifdef NDEBUG
				#error ASSUMING remoteURI is localURI?
				#endif				
				if(!req.isEmptyRequest())
				req.remoteURI = &p.second->getDocURI();					
			}
			~rw()
			{ 
				if(!req.isEmptyRequest()) 
				req.scope->getIOController().closeIO(IO);
			}
			void maybe_openIO(daeIOPlugin &I, daeIOPlugin &O)
			{
				if(!req.isEmptyRequest())
				IO = req.scope->getIOController().openIO(I,O);
			}
		}r,w; daeIOEmpty _;
		daeOK OK; size_t lock;
		io(const std::pair<trio,trio> &IO)
		:r(IO.first),w(IO.second),lock(-1)
		{
			r.maybe_openIO(r,_); w.maybe_openIO(_,w);		
		} 		
		io(std::pair<daeIO*,cde*> I):r(I.first),lock(-1)
		{
			r.e = I.second; //piggybackI
		} 	

		//daeIO methods//////////////////////////

		virtual daeError getError()
		{
			return OK;  
		}
		virtual size_t getLock(Range *I, Range *O)
		{
			if(OK==DAE_OK&&lock==size_t(-1))
			{	
				lock = 0;
				if(r.e!=nullptr)
				{	
					enum{extraN=64};
					int est = r.e->dsize+30+strlen(r.e->path)+extraN;
					Range dI = {r.e->lhpos,r.e->lhpos+est};
					r.IO->setRange(&dI,nullptr);
					char buf[4096]; 
					if(!r.IO->readIn(buf,30))
					{
						OK = DAE_ERR_BACKEND_IO;
						return 0;
					}
									  					
					//Read to end of ZIP's headers.
					int gpbits = BigEndian<16,unsigned short>(buf+6);
					int method = BigEndian<16,unsigned short>(buf+8);
					int rem = BigEndian<16,unsigned short>(buf+26);
					int extra = BigEndian<16,unsigned short>(buf+28);
					rem+=extra; assert(extra<=extraN);
					if((unsigned)est<30+rem+r.e->dsize) //HACK?
					{
						//The estimate was inadequate.
						//HACK: Assuming not interested in Extra field.
						dI.first+=30+rem;
						dI.second = dI.first+r.e->dsize;
						r.IO->setRange(&dI,nullptr);						
						daeEH::Warning<<"daeZAEPlugin's Extra data allotment ("<<+extraN<<") is inadequate. Resetting download range. Extra was "<<extra;
					}
					else for(size_t rd;rem!=0;r.IO->readIn(buf,rd),rem-=rd)
					rd = std::min<int>(rem,sizeof(buf));
					
					if(OK=r.IO->getError()) switch(method)
					{
					default:
							
						daeEH::Error<<"ZIP decompression method ("<<method<<") is unsupported.";
							
						OK = DAE_ERR_NOT_IMPLEMENTED; break;

					case 8: r.e_overflowed = 0;

						gz = zae->gz_stack.pop(r.e->dsize); //break;

					case 0: lock = r.e->isize; break;
					}						

					//This can be optimized if not compressed/encrypted.
					if(I!=nullptr)
					{
						I->limit_to_size(lock);	
						//HACK? Must read ahead.
						rem = I->first;
						for(size_t rd;rem!=0;r.IO->readIn(buf,rd),rem-=rd)
						rd = std::min<int>(rem,sizeof(buf));
					}
				}
				else if(r.IO!=nullptr) //Pass-thru?
				{
					lock = r.IO->getLock(I,nullptr);
				}
				if(w.e!=nullptr)
				{
					assert(O==nullptr); //incomplete

					#ifdef NDEBUG
					#error incomplete
					#endif
				}
				else if(w.IO!=nullptr) //Pass-thru?
				{
					w.IO->getLock(nullptr,O);
				}
			}
			return lock;
		}
		virtual size_t setRange(Range *rngI, Range *rngO)
		{
			if(lock==size_t(-1)) return getLock(rngI,rngO);

			//Reminder: Must advance the GZ buffer.
			#ifdef NDEBUG
			#error incomplete
			#endif
			assert(0);
			OK = DAE_ERR_NOT_IMPLEMENTED; return 0;
		}
		virtual daeOK readIn(void *in, size_t chars)
		{
			bool z = r.e_overflowed>-1;
			if(z&&!gz->empty()) inflated:
			{
				size_t cp = gz->size()-r.e_overflowed;
				if(cp!=0)
				{
					cp = std::min<size_t>(chars,cp);
					memcpy(in,(char*)*gz+r.e_overflowed,cp); 
					r.e_overflowed+=(int)cp;								
					(char*&)in+=cp;
					chars-=cp; 					
				}
				if(r.e_overflowed==(int)gz->size())
				{
					r.e_overflowed = 0;
					gz->clear();
				}

				if(0==chars) return OK;
			}

			char *buf = (char*)in, underflow[512];
			size_t charz = chars; if(z)
			{
				//This is just to keep the output buffer
				//from being very large when reading in 
				//an entire file. Maybe it should depend
				//on the ratio of dsize and isize.
				charz = std::min<size_t>(charz,4*4096);

				charz = std::max(charz,sizeof(underflow));				
				charz = std::min(charz,gz->getRemaining());				
				if(charz>chars) buf = underflow;
			}

			if(OK) 
			if(OK=r.IO->readIn(buf,charz))
			if(z)
			{
				if(0==gz->inflate(buf,charz))
				{
					OK = gz->getError();				
				}
				else goto inflated;
			}

			return OK;			
		}
		virtual daeOK writeOut(const void *out, size_t chars)
		{
			#ifdef NDEBUG
			#error incomplete
			#endif
			assert(0);
			OK = DAE_ERR_NOT_IMPLEMENTED; return 0;
		}

		//daeIOController methods ////////
		
		daeSmartRef<daeZAE> zae; daeGZ *gz;

		io *set_zae(daeZAE *ioc)
		{
			zae = ioc; gz = nullptr; return this;
		}

		~io(){ if(gz!=nullptr) zae->gz_stack.push(gz); }
	};

COLLADA_(public) //daeIOController methods
	
	daeIOController::Stack<io> io_stack;
	daeIOController::Stack<daeGZ> gz_stack;

	/**PURE-OVERRIDE */
	daeIO *openIO(daeIOPlugin &I, daeIOPlugin &O)
	{
		return io_stack.pop(std::make_pair(trio(&I,this),trio(&O,this)))->set_zae(this);
	}
	/**PURE-OVERRIDE */
	void closeIO(daeIO *IO){ io_stack.push(IO); }

COLLADA_(public) //INTERNAL

	inline daeIO *piggybackI(daeIO &IO, daeString path)
	{
		cde *e = find(path);
		return e==nullptr?nullptr:io_stack.pop(std::make_pair(&IO,e))->set_zae(this);
	}
};

daeError daeZAE::maybe_init(daeIO &IO)
{
	if(this==nullptr)
	{
		assert(this!=nullptr);
		return DAE_ERR_BACKEND_IO;
	}
	if(!_initialized) _initialized = true;
	else return DAE_OK;

	//ZIP files (which ZAE is) are organized in reverse.
	//Conceptually eof is requesting the Content-Length.
	daeIO::Range r = {-1,-1};
	size_t eof = IO.getLock(&r); if(eof!=0) 
	{
		r.first = std::max<size_t>(eof,_8192)-_8192; 
		r.second = eof;
		if(eof==IO.setRange(&r))
		{
			size_t bufN = r.second-r.first;
			char buf[_8192];
			daeOK OK = IO.readIn(buf,bufN);
			if(!OK) return OK;

			//HACK: Just throwing this together.
			int sig = 0x06054b50; BigEndian<32>(sig);
			char *eocd = buf+bufN-sizeof(sig);
			while(eocd>=buf&&*(int*)eocd!=sig) 
			eocd--; 
			size_t eocdN = bufN-((char*)eocd-buf);
			if(eocd>buf&&eocdN>=22)
			{
				#ifdef NDEBUG
				#error 64bit ZIP?
				#endif
				daeCTC<CHAR_BIT==8>();
				int n = BigEndian<16,unsigned short>(eocd+8);
				int cd = BigEndian<32,unsigned int>(eocd+12);
				if(eocd-cd<buf)
				{
					r.first = r.second = eof-eocdN;
					if((size_t)cd>=r.first)
					{
						r.first-=cd;
						if(eof==IO.setRange(&r))
						{
							return _init_central_directory(&IO,buf,0,cd,n);
						}
						else assert(0);
					}
					else daeEH::Error<<"Found ZIP central-directory is out-of-bounds.";
				}	
				else return _init_central_directory(nullptr,eocd-cd,cd,0,n);
			}
			else daeEH::Error<<"Did not find ZIP signature (06054b50) in last "<<_8192<<"Bs.";
		}
		else assert(0);
	}
	return DAE_ERR_BACKEND_IO;
}
daeError daeZAE::_init_central_directory(daeIO *IO, char *buf, int bufN, int rem, int reserve)
{
	CD.reserve(reserve);

	char *cdp,*eob; int rd;
	if(IO==nullptr) goto short_cd; do
	{
		bufN = buf+bufN-cdp;
		rd = std::min(rem,_8192-bufN);
		memmove(buf,cdp,bufN);
		IO->readIn(buf+bufN,rd); bufN+=rd; rem-=rd;
		
		short_cd: cdp = buf; eob = buf+bufN; for(;cdp+46<eob;)
		{
			assert((0x02014b50==BigEndian<32,unsigned int>(cdp)));

			int pathlen = BigEndian<16,unsigned short>(cdp+28);
			if(cdp+46+pathlen>eob) break;

			int eN = sizeof(cde)+pathlen;
			if(A._nexT->path+pathlen>=A._end)
			A._reserve(std::max(eN,A._reserveN()));
			cde &e = *A._nexT; A._nexT+=eN;

			//dsize will include the local-header's size.
			e.dsize = BigEndian<32,unsigned int>(cdp+20);
			e.isize = BigEndian<32,unsigned int>(cdp+24);
			e.lhpos = BigEndian<32,unsigned int>(cdp+42); 
			//TODO? IBM437 to UTF8.
			memcpy(e.path,cdp+46,pathlen);
			e.path[pathlen] = '\0';

			int extra = BigEndian<16,unsigned short>(cdp+30);
			int comment = BigEndian<16,unsigned short>(cdp+32);
									   
			CD.push_back(&e); cdp+=46+pathlen+extra+comment;
		}
	
	}while(rem>0); std::sort(CD.begin(),CD.end(),cde::less);

	return IO!=nullptr?IO->getError():DAE_OK;
}

daeOK daeZAEPlugin::addDoc(daeDocRef &add, daeMeta *rootmeta)
{	
	daeZAE *zae = new daeZAE();
	zae->__DAEP__Object__unembed();
	daeArchiveRef out(*getRequest().getDOM());		
	out->_getAtlas() = zae;

	//This is not pretty. Basically, assuming the caller means
	//to open the index document as daeArchive::_read2 does it.
	if(rootmeta!=nullptr
	&&getRequest().localURI!=nullptr
	&&getRequest().localURI->getURI_extensionIs("zae"))
	{
		//This branch continues to daeZAEPlugin::readContent().
		out->getDocURI() = *getRequest().localURI;
		out->setDocument(out->newDoc("dae_root"));
	}
	else //HACK: Open any ZIP without dae_root or manifest.xml?
	{			
		#ifdef NDEBUG
		#error Can (or should) this wait until it's called for?
		#endif

		//This is what _read2 would do, but there isn't an API
		//that is analogous to daeIOPlugin::readContent.
		daeIOEmpty O;
		daeRAII::CloseIO RAII(getRequest().scope->getIOController()); //emits error	
		daeIO *IO = RAII = RAII.p.openIO(*this,O);
		if(IO==nullptr)
		return DAE_ERR_BACKEND_IO;
		daeOK OK = zae->maybe_init(*IO);
		if(!OK) return OK;
	}
	add = out; return DAE_OK;
}
daeOK daeZAEPlugin::readContent(daeIO &IO, daeContents &content)
{
	//Assuming this means the archive is being opened, since it is
	//hard to imagine doing a partial read into the index document.

	daeDocument *d = (daeDocument*)content.getElement().getDoc();
	daeZAE *zae = dynamic_cast<daeZAE*>(&d->getArchive().getAtlas());
	daeOK OK = zae->maybe_init(IO); if(!OK) return OK;
	
	daeIO *zae_IO = zae->piggybackI(IO,"manifest.xml");
	if(zae_IO!=nullptr) //dae_root?
	{
		daeURI localURI; //dae_root

		size_t eof = zae_IO->getLock();			
		char *manifest_xml = new char[eof]; 
		OK = zae_IO->readIn(manifest_xml,eof); 					
		if(OK&&eof!=0)
		{
			//Here daeZAEPlugin could generate a full daeDocument
			//just to pluck out the dae_root element. It doesn't
			//have to be the final word on ZAE/archive plugins.
			//users are better to open manifest.xml themself.
			manifest_xml[eof-1] = '\0'; 
			char *e,*p = strstr(manifest_xml,"<dae_root"); 
			if(p!=nullptr) p = strchr(p,'>');
			if(p!=nullptr) e = strchr(++p,'<'); 
			if(p!=nullptr&&e!=nullptr)					
			{
				while(COLLADA_isspace(*p)) p++;
				while(COLLADA_isspace(e[-1])) e--; *e = '\0'; 

				if(e>p) localURI.setURI(p); //dae_root?
			}
		}	
		delete[] manifest_xml;		
		
		//NOTE: piggybackI would work here, except replicating _read2's
		//plugin selection logic should be avoided.
		//TODO? encapsulate the plugin selection logic into some struct.
		//(Especially because IO is not yet closed.)
		if(!localURI.empty())
		{				
			//HACK: _read2 is recursive here so that the document can be
			//returned by the outer _read2.
			daeDocRef d2(d);
			if(getRequest().remoteURI!=getRequest().localURI) //unlikely
			{
				//Should daeAtlas hold onto the original remote URI?
				daeURI_parser remoteURI_base(getRequest().remoteURI->data());
				remoteURI_base.setIsDirectoryLike();
				remoteURI_base.setIsResolved();
				daeURI remoteURI(remoteURI_base,localURI.data());
				OK = daeArchive::_read2(d2,d->getMeta(),
			daeIORequest(&d->getArchive(),nullptr,&localURI,&remoteURI),_I);
			}else OK = daeArchive::_read2(d2,d->getMeta(),
			daeIORequest(&d->getArchive(),nullptr,&localURI,&localURI),_I);				
		}
		else daeEH::Warning<<"ZAE is without <dae_root> via manifest.xml.";
	}
	else daeEH::Warning<<"ZAE is without manifest.xml.";

	if(!d->getDocURI().empty())
	{			
		if(getRequest().localURI!=nullptr) //Overrules dae_root fragment?
		{
			daeRefView fragment = getRequest().localURI->getURI_fragment();
			if(!fragment.empty()) d->getFragment() = fragment;
		}
	}
	else //d->close(); //Should it be empty or missing?
	{		
		//This should close it. But what about the ZAE archive itself?
		OK = DAE_ERR_BACKEND_IO;
	}

	return OK;
}

daeOK daeZAEPlugin::writeContent(daeIO &IO, const daeContents &content)
{
	#ifdef NDEBUG
	#error incomplete
	#endif
	assert(0);
	return DAE_ERR_BACKEND_IO; 
}



//// GZ //// GZ //// GZ //// GZ //// GZ //// GZ //// GZ ////



//https://github.com/uroni/miniz/blob/master/miniz_tinfl.c
//(changes made for readability/maintainability)
//REMINDER: MAY NEED TO SUPPORT CRC-32 CHECKSUMS
/**************************************************************************
 *
 * Copyright 2013-2014 RAD Game Tools and Valve Software
 * Copyright 2010-2014 Rich Geldreich and Tenacious Software LLC
 * All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 **************************************************************************/
namespace //SCHEDULED FOR REMOVAL?
{
//Reminder: This is how Miniz defines these types ca. 2017.
typedef unsigned char /*mz_*/uint8_t;
typedef signed short /*mz_*/int16_t;
typedef unsigned short /*mz_*/uint16_t;
typedef unsigned int /*mz_*/uint32_t;
typedef long long /*mz_*/int64_t;
typedef unsigned long long /*mz_*/uint64_t;
static daeCTC<CHAR_BIT==8> _uint8_check;
static daeCTC<sizeof(uint16_t)*CHAR_BIT==16> _uint16_check;
static daeCTC<sizeof(uint32_t)*CHAR_BIT==32> _uint32_check;
#define TINFL_BITBUF_SIZE 32
typedef uint32_t tinfl_bit_buf_t; //uint64_t
}
enum // Internal/private bits
{
  TINFL_MAX_HUFF_TABLES = 3, 
  TINFL_MAX_HUFF_SYMBOLS_0 = 288,
  TINFL_MAX_HUFF_SYMBOLS_1 = 32,
  TINFL_MAX_HUFF_SYMBOLS_2 = 19,
  TINFL_FAST_LOOKUP_BITS = 10, 
  TINFL_FAST_LOOKUP_SIZE = 1 << TINFL_FAST_LOOKUP_BITS,
};
typedef struct _tinfl_huff_table //compiler
{
	uint8_t m_code_size[TINFL_MAX_HUFF_SYMBOLS_0];
	int16_t m_look_up[TINFL_FAST_LOOKUP_SIZE], m_tree[TINFL_MAX_HUFF_SYMBOLS_0*2];

}tinfl_huff_table;
typedef struct _tinfl_decompressor //compiler
{
	//daeGZ
	//Miniz houses this and the dictionary separate from the decompressor, but I can't see
	//how the structures are separate when using TINFL_FLAG_USING_NON_WRAPPING_OUTPUT_BUF
	//mode. If all input is available and output fills the dictionary flush, then it can
	//be independent. But COLLADA-DOM doesn't have that luxury.
	struct
	{
		unsigned short wrapping_output_buf;

	}ext;

	uint32_t m_state, m_num_bits, m_zhdr0, m_zhdr1, m_z_adler32, m_final, m_type,
	m_check_adler32, m_dist, m_counter, m_num_extra, m_table_sizes[TINFL_MAX_HUFF_TABLES];

	tinfl_bit_buf_t m_bit_buf;
	size_t m_dist_from_out_buf_start;
	tinfl_huff_table m_tables[TINFL_MAX_HUFF_TABLES];
	uint8_t m_raw_header[4], m_len_codes[TINFL_MAX_HUFF_SYMBOLS_0+TINFL_MAX_HUFF_SYMBOLS_1+137];

}tinfl_decompressor;
enum 
{	
	//If set, the input has a valid zlib header and ends with an adler32 checksum (it's a valid zlib stream). Otherwise, the input is a raw deflate stream.	
	//TINFL_FLAG_PARSE_ZLIB_HEADER = 1,
	//If set, there are more input bytes available beyond the end of the supplied input buffer. If clear, the input buffer contains all remaining input.
	TINFL_FLAG_HAS_MORE_INPUT = 2,
	//If set, the output buffer is large enough to hold the entire decompressed stream. If clear, the output buffer is at least the size of the dictionary (typically 32KB).
	TINFL_FLAG_USING_NON_WRAPPING_OUTPUT_BUF = 4,	
	//TINFL_FLAG_COMPUTE_ADLER32 = 8,	
	//Force adler-32 checksum computation of the decompressed bytes.
};
typedef enum _tinfl_status //compiler
{
	TINFL_STATUS_FAILED_CANNOT_MAKE_PROGRESS = -4,
	TINFL_STATUS_BAD_PARAM = -3,
	TINFL_STATUS_ADLER32_MISMATCH = -2,
	TINFL_STATUS_FAILED = -1,
	TINFL_STATUS_DONE = 0,
	TINFL_STATUS_NEEDS_MORE_INPUT = 1,
	TINFL_STATUS_HAS_MORE_OUTPUT = 2

}tinfl_status;
static tinfl_status tinfl_decompress(tinfl_decompressor *r, 
const uint8_t*pIn_buf_next,size_t*pIn_buf_size,uint8_t*pOut_buf_start,uint8_t*pOut_buf_next,size_t*pOut_buf_size, 
const uint32_t decomp_flags)
{
	tinfl_status status = TINFL_STATUS_FAILED; //out

	static const int s_length_base[31] = 
	{ 3,4,5,6,7,8,9,10,11,13, 15,17,19,23,27,31,35,43,51,59, 67,83,99,115,131,163,195,227,258,0,0 };
	static const int s_length_extra[31] = 
	{ 0,0,0,0,0,0,0,0,1,1,1,1,2,2,2,2,3,3,3,3,4,4,4,4,5,5,5,5,0,0,0 };
	static const int s_dist_base[32] = 
	{ 1,2,3,4,5,7,9,13,17,25,33,49,65,97,129,193, 257,385,513,769,1025,1537,2049,3073,4097,6145,8193,12289,16385,24577,0,0};
	static const int s_dist_extra[32] = 
	{ 0,0,0,0,1,1,2,2,3,3,4,4,5,5,6,6,7,7,8,8,9,9,10,10,11,11,12,12,13,13};
	static const uint8_t s_length_dezigzag[19] = 
	{ 16,17,18,0,8,7,9,6,10,5,11,4,12,3,13,2,14,1,15 };
	static const int s_min_table_sizes[3] = { 257,1,4 };
		
	uint32_t num_bits, dist, counter, num_extra; tinfl_bit_buf_t bit_buf;	
	const uint8_t *pIn_buf_cur = pIn_buf_next, *const pIn_buf_end = pIn_buf_next+*pIn_buf_size;
	uint8_t *pOut_buf_cur = pOut_buf_next, *const pOut_buf_end = pOut_buf_next+*pOut_buf_size; 
	size_t out_buf_size_mask = decomp_flags&TINFL_FLAG_USING_NON_WRAPPING_OUTPUT_BUF?size_t(-1):((pOut_buf_next-pOut_buf_start)+*pOut_buf_size)-1;
	size_t dist_from_out_buf_start;

	//Ensure the output buffer's size is a power of 2, unless the output buffer is large enough to hold the entire output file (in which case it doesn't matter).
	if((out_buf_size_mask+1)&out_buf_size_mask||pOut_buf_next<pOut_buf_start){ *pIn_buf_size = *pOut_buf_size = 0; return TINFL_STATUS_BAD_PARAM; }

	num_bits = r->m_num_bits; bit_buf = r->m_bit_buf; dist = r->m_dist; counter = r->m_counter; num_extra = r->m_num_extra; 
	dist_from_out_buf_start = r->m_dist_from_out_buf_start;
	
	#define TINFL_CR_BEGIN switch(r->m_state){ default: assert(0); case 0:
	#define TINFL_CR_RETURN(state_index,result) \
	{ status = result; r->m_state = state_index; goto common_exit; case state_index:; }
	#define TINFL_CR_RETURN_FOREVER(state_index,result) \
	for(;;){ TINFL_CR_RETURN(state_index,result); }
	#define TINFL_CR_FINISH } 	
	#define TINFL_GET_BYTE(state_index,c)\
	{\
		while(pIn_buf_cur>=pIn_buf_end)\
		TINFL_CR_RETURN(state_index,decomp_flags&TINFL_FLAG_HAS_MORE_INPUT?TINFL_STATUS_NEEDS_MORE_INPUT:TINFL_STATUS_FAILED_CANNOT_MAKE_PROGRESS)\
		c = *pIn_buf_cur++;\
	}
	#define TINFL_NEED_BITS(state_index,n) \
	do{ unsigned c; TINFL_GET_BYTE(state_index,c); bit_buf|=tinfl_bit_buf_t(c)<<num_bits; num_bits+=8;\
	}while(num_bits<unsigned(n));
	#define TINFL_SKIP_BITS(state_index,n) \
	{ if(num_bits<unsigned(n)) TINFL_NEED_BITS(state_index,n) bit_buf>>=(n); num_bits-=(n); }
	#define TINFL_GET_BITS(state_index,b,n) \
	{ if(num_bits<unsigned(n)) TINFL_NEED_BITS(state_index,n) b = bit_buf&((1<<(n))-1); bit_buf>>=(n); num_bits-=(n); } 
	//TINFL_HUFF_BITBUF_FILL() is only used rarely, when the number of bytes remaining in the input buffer falls below 2.
	//It reads just enough bytes from the input stream that are needed to decode the next Huffman code (and absolutely no more). It works by trying to fully decode a
	//Huffman code by using whatever bits are currently present in the bit buffer. If this fails, it reads another byte, and tries again until it succeeds or until the
	//bit buffer contains >=15 bits (deflate's max. Huffman code size).
	#define TINFL_HUFF_BITBUF_FILL(state_index,pHuff) \
	do{\
		temp = (pHuff)->m_look_up[bit_buf&(TINFL_FAST_LOOKUP_SIZE-1)]; \
		if(temp>=0)\
		{\
			code_len = temp>>9;\
			if(code_len&&num_bits>=code_len) break; \
		}\
		else if(num_bits>TINFL_FAST_LOOKUP_BITS)\
		{\
			code_len = TINFL_FAST_LOOKUP_BITS;\
			do{ temp = (pHuff)->m_tree[~temp+((bit_buf>>code_len++)&1)];\
			}while(temp<0&&num_bits>=code_len+1);\
			if(temp>=0) break;\
		}\
		TINFL_GET_BYTE(state_index,c)\
		bit_buf|=tinfl_bit_buf_t(c)<<num_bits;\
		num_bits+=8;\
	}while(num_bits<15);
	//TINFL_HUFF_DECODE() decodes the next Huffman coded symbol. It's more complex than you would initially expect because the zlib API expects the decompressor to never read
	//beyond the final byte of the deflate stream. (In other words, when this macro wants to read another byte from the input, it REALLY needs another byte in order to fully
	//decode the next Huffman code.) Handling this properly is particularly important on raw deflate (non-zlib) streams, which aren't followed by a byte aligned adler-32.
	//The slow path is only executed at the very end of the input buffer.
	#define TINFL_HUFF_DECODE(state_index,sym,pHuff) \
	{\
		int temp; unsigned code_len, c;\
		if(num_bits<15)\
		if(pIn_buf_end-pIn_buf_cur>=2)\
		{\
			bit_buf|=((tinfl_bit_buf_t)pIn_buf_cur[0]<<num_bits)|((tinfl_bit_buf_t)pIn_buf_cur[1]<<num_bits+8);\
			pIn_buf_cur+=2; num_bits+=16;\
		}\
		else TINFL_HUFF_BITBUF_FILL(state_index,pHuff)\
		temp = (pHuff)->m_look_up[bit_buf&(TINFL_FAST_LOOKUP_SIZE-1)];\
		if(temp<0)\
		{\
			code_len = TINFL_FAST_LOOKUP_BITS;\
			do{ temp = (pHuff)->m_tree[~temp+((bit_buf>>code_len++)&1)];\
			}while(temp<0);\
		}\
		else{ code_len = temp>>9; temp&=511; }\
		sym = temp;\
		bit_buf>>=code_len;	num_bits-=code_len;\
	}
	#define TINFL_ZERO_MEM(obj) memset(&(obj),0,sizeof(obj));
	#if 1 //MINIZ_USE_UNALIGNED_LOADS_AND_STORES && MINIZ_LITTLE_ENDIAN
	#define TINFL_READ_LE16(p) *((const uint16_t*)(p))
	#define TINFL_READ_LE32(p) *((const uint32_t*)(p))
	#else
	#define TINFL_READ_LE16(p) ((uint32_t)(((const uint8_t*)(p))[0])|((uint32_t)(((const uint8_t*)(p))[1])<<8U))
	#define TINFL_READ_LE32(p) ((uint32_t)(((const uint8_t*)(p))[0])|((uint32_t)(((const uint8_t*)(p))[1])<<8U)|\
					           ((uint32_t)(((const uint8_t*)(p))[2])<<16U)|\
							   ((uint32_t)(((const uint8_t*)(p))[3])<<24U))
	#endif
	TINFL_CR_BEGIN //switch(r->m_state){ case 0:
	//
	bit_buf = num_bits = dist = counter = 0;
	num_extra = r->m_zhdr0 = r->m_zhdr1 = 0; 
	r->m_z_adler32 = r->m_check_adler32 = 1;
	//
	/*if(0!=(decomp_flags&TINFL_FLAG_PARSE_ZLIB_HEADER))
	{
		TINFL_GET_BYTE(1,r->m_zhdr0); TINFL_GET_BYTE(2,r->m_zhdr1)
		counter = (r->m_zhdr0*256+r->m_zhdr1)%31!=0||(r->m_zhdr1&32)!=0||(r->m_zhdr0&15)!=8?1:0;
		if(0==(decomp_flags&TINFL_FLAG_USING_NON_WRAPPING_OUTPUT_BUF)) 
		counter|=1U<<8U+(r->m_zhdr0>>4)>32768U||out_buf_size_mask+1<size_t(1U<<8U+(r->m_zhdr0>>4));
		if(counter) TINFL_CR_RETURN_FOREVER(36,TINFL_STATUS_FAILED)
	}*/
	do
	{	TINFL_GET_BITS(3,r->m_final,3); 
		r->m_type = r->m_final>>1;
		if(r->m_type==0)
		{
			TINFL_SKIP_BITS(5,num_bits&7)
			for(counter=0;counter<4;++counter) 
			{
				if(num_bits) 
					TINFL_GET_BITS(6,r->m_raw_header[counter],8) 
				else TINFL_GET_BYTE(7,r->m_raw_header[counter]) 
			}

			counter = r->m_raw_header[0]|(r->m_raw_header[1]<<8);
			if(counter!=(unsigned)(0xFFFF^(r->m_raw_header[2]|(r->m_raw_header[3]<<8)))) 
			TINFL_CR_RETURN_FOREVER(39,TINFL_STATUS_FAILED)

			while(counter&&num_bits)
			{
				TINFL_GET_BITS(51,dist,8)
				while(pOut_buf_cur>=pOut_buf_end)
				TINFL_CR_RETURN(52,TINFL_STATUS_HAS_MORE_OUTPUT)
				*pOut_buf_cur++ = (uint8_t)dist;
				counter--;
			}

			while(counter)
			{
				while(pOut_buf_cur>=pOut_buf_end)
				TINFL_CR_RETURN(9,TINFL_STATUS_HAS_MORE_OUTPUT)

				while(pIn_buf_cur>=pIn_buf_end)
				TINFL_CR_RETURN(38,decomp_flags&TINFL_FLAG_HAS_MORE_INPUT?TINFL_STATUS_NEEDS_MORE_INPUT:TINFL_STATUS_FAILED_CANNOT_MAKE_PROGRESS)				

				size_t n = std::min(counter,std::min<size_t>(pOut_buf_end-pOut_buf_cur,pIn_buf_end-pIn_buf_cur));
				memcpy(pOut_buf_cur,pIn_buf_cur,n);				
				pIn_buf_cur+=n; pOut_buf_cur+=n; 
				counter-=(unsigned)n;
			}
		}
		else if(r->m_type==3)
		{
			TINFL_CR_RETURN_FOREVER(10,TINFL_STATUS_FAILED)
		}
		else
		{
			if(r->m_type==1)
			{
				uint8_t *p = r->m_tables[0].m_code_size; 
				r->m_table_sizes[0] = 288; r->m_table_sizes[1] = 32; 
				memset(r->m_tables[1].m_code_size,5,32); 
				unsigned i=0;
				for(;i<=143;++i) *p++ = 8; 
				for(;i<=255;++i) *p++ = 9;
				for(;i<=279;++i) *p++ = 7; 
				for(;i<=287;++i) *p++ = 8;
			}
			else
			{
				for(counter=0;counter<3;counter++)
				{ 
					TINFL_GET_BITS(11,r->m_table_sizes[counter],"\05\05\04"[counter])
					r->m_table_sizes[counter]+=s_min_table_sizes[counter]; 
				}
				TINFL_ZERO_MEM(r->m_tables[2].m_code_size)
				for(counter=0;counter<r->m_table_sizes[2];counter++) 
				{
					unsigned s; TINFL_GET_BITS(14,s,3)
					r->m_tables[2].m_code_size[s_length_dezigzag[counter]] = (uint8_t)s; 
				}
				r->m_table_sizes[2] = 19;
			}

			for(;(int)r->m_type>=0;r->m_type--)
			{					
				tinfl_huff_table *pTable;
				pTable = &r->m_tables[r->m_type]; //C2360
				TINFL_ZERO_MEM(pTable->m_look_up) TINFL_ZERO_MEM(pTable->m_tree)
				unsigned i,used_syms,total,sym_index,next_code[17],total_syms[16];
				TINFL_ZERO_MEM(total_syms)
				for(i=0;i<r->m_table_sizes[r->m_type];i++) 
				total_syms[pTable->m_code_size[i]]++;
				used_syms = total = next_code[0] = next_code[1] = 0;
				for(i=1;i<=15;used_syms+=total_syms[i++]) 
				next_code[i+1] = total = total+total_syms[i]<<1; 
				if(65536!=total&&used_syms>1)
				TINFL_CR_RETURN_FOREVER(35,TINFL_STATUS_FAILED)
				
				int tree_next, tree_cur; 
				for(tree_next=-1,sym_index=0;sym_index<r->m_table_sizes[r->m_type];sym_index++)
				{
					unsigned rev_code = 0, l, cur_code;
					unsigned code_size = pTable->m_code_size[sym_index]; 
					if(!code_size) continue;
					cur_code = next_code[code_size]++; 
					for(l=code_size;l>0;l--,cur_code>>=1) 
					rev_code = (rev_code<<1)|(cur_code&1);
					if(code_size <= TINFL_FAST_LOOKUP_BITS)
					{
						int16_t k = (int16_t)((code_size<<9)|sym_index); 
						while(rev_code<TINFL_FAST_LOOKUP_SIZE)
						{
							pTable->m_look_up[rev_code] = k; rev_code+=(1<<code_size); 
						} 
						continue; 
					}
					tree_cur = pTable->m_look_up[rev_code&(TINFL_FAST_LOOKUP_SIZE-1)];
					if(tree_cur==0)
					{
						pTable->m_look_up[rev_code&(TINFL_FAST_LOOKUP_SIZE-1)] = (int16_t)tree_next;
						tree_cur = tree_next; tree_next-=2; 
					}
					rev_code>>=(TINFL_FAST_LOOKUP_BITS-1);
					for(unsigned j=code_size;j>(TINFL_FAST_LOOKUP_BITS+1);j--)
					{
						tree_cur-=((rev_code>>=1)&1);
						if(!pTable->m_tree[-tree_cur-1])
						{ 
							pTable->m_tree[-tree_cur-1] = (int16_t)tree_next;
							tree_cur = tree_next; tree_next-=2;
						}
						else tree_cur = pTable->m_tree[-tree_cur-1];
					}
					tree_cur-=(rev_code>>=1)&1;
					pTable->m_tree[-tree_cur-1] = (int16_t)sym_index;
				}
				if(r->m_type==2)
				{
					for(counter=0;counter<r->m_table_sizes[0]+r->m_table_sizes[1];)
					{
						TINFL_HUFF_DECODE(16,dist,&r->m_tables[2])
						if(dist<16)
						{
							r->m_len_codes[counter++] = (uint8_t)dist; 
							continue; 
						}
						if(dist==16&&0==counter)
						TINFL_CR_RETURN_FOREVER(17,TINFL_STATUS_FAILED)
						unsigned s; 
						num_extra = "\02\03\07"[dist-16]; 
						TINFL_GET_BITS(18,s,num_extra) s+="\03\03\013"[dist-16];
						memset(r->m_len_codes+counter,(dist==16)?r->m_len_codes[counter-1]:0,s); 
						counter+=s;
					}
					if((r->m_table_sizes[0]+r->m_table_sizes[1])!=counter)
					{
						TINFL_CR_RETURN_FOREVER(21,TINFL_STATUS_FAILED)
					}
					memcpy(r->m_tables[0].m_code_size,r->m_len_codes,r->m_table_sizes[0]);
					memcpy(r->m_tables[1].m_code_size,r->m_len_codes+r->m_table_sizes[0],r->m_table_sizes[1]);
				}
			}

			for(uint8_t *pSrc;;)
			{
				for(;;)
				{
					if(pIn_buf_end-pIn_buf_cur<4||pOut_buf_end-pOut_buf_cur<2)
					{
						TINFL_HUFF_DECODE(23,counter,&r->m_tables[0])
						if(counter>=256) break;
						while(pOut_buf_cur>=pOut_buf_end)
						TINFL_CR_RETURN(24,TINFL_STATUS_HAS_MORE_OUTPUT)
						*pOut_buf_cur++ = (uint8_t)counter;
					}
					else
					{							
						#if 64==TINFL_BITBUF_SIZE //TINFL_USE_64BIT_BITBUF
						if(num_bits<30){ bit_buf|=(((tinfl_bit_buf_t)TINFL_READ_LE32(pIn_buf_cur))<<num_bits); pIn_buf_cur+=4; num_bits+=32; }
						#else
						if(num_bits<15){ bit_buf|=(((tinfl_bit_buf_t)TINFL_READ_LE16(pIn_buf_cur))<<num_bits); pIn_buf_cur+=2; num_bits+=16; }
						#endif

						unsigned code_len;
						int sym2 = r->m_tables[0].m_look_up[bit_buf&(TINFL_FAST_LOOKUP_SIZE-1)];
						if(sym2<0) 
						{
							code_len = TINFL_FAST_LOOKUP_BITS; 							
							do{ sym2 = r->m_tables[0].m_tree[~sym2+((bit_buf>>code_len++)&1)]; 							
							}while(sym2<0);
						}
						else code_len = sym2>>9;
						counter = sym2; bit_buf>>=code_len; num_bits-=code_len;
						if(counter&256) break;

						#if 64!=TINFL_BITBUF_SIZE //!TINFL_USE_64BIT_BITBUF
						if(num_bits<15){ bit_buf|=(((tinfl_bit_buf_t)TINFL_READ_LE16(pIn_buf_cur))<<num_bits); pIn_buf_cur+=2; num_bits+=16; }
						#endif

						sym2 = 
						r->m_tables[0].m_look_up[bit_buf&(TINFL_FAST_LOOKUP_SIZE-1)];
						if(sym2<0)
						{
							code_len = TINFL_FAST_LOOKUP_BITS; 
							do{ sym2 = r->m_tables[0].m_tree[~sym2+((bit_buf>>code_len++)&1)];
							}while(sym2<0);
						}
						else code_len = sym2>>9;
						bit_buf>>=code_len; num_bits-=code_len;
						pOut_buf_cur[0] = (uint8_t)counter;
						if(sym2&256)
						{
							pOut_buf_cur++;	counter = sym2;
							break;
						}
						pOut_buf_cur[1] = (uint8_t)sym2;
						pOut_buf_cur+=2;
					}
				}
				counter&=511;
				if(counter==256) break;
				num_extra = s_length_extra[counter-257];
				counter = s_length_base[counter-257];
				if(num_extra)
				{
					unsigned extra_bits; 
					TINFL_GET_BITS(25,extra_bits,num_extra)
					counter+=extra_bits; 
				}
				TINFL_HUFF_DECODE(26,dist,&r->m_tables[1])
				num_extra = s_dist_extra[dist]; 
				dist = s_dist_base[dist];
				if(num_extra)
				{
					unsigned extra_bits;
					TINFL_GET_BITS(27,extra_bits,num_extra)
					dist+=extra_bits; 
				}
				dist_from_out_buf_start = pOut_buf_cur-pOut_buf_start;
				if(dist>dist_from_out_buf_start&&decomp_flags&TINFL_FLAG_USING_NON_WRAPPING_OUTPUT_BUF)				
				TINFL_CR_RETURN_FOREVER(37,TINFL_STATUS_FAILED)

				pSrc = pOut_buf_start+((dist_from_out_buf_start-dist)&out_buf_size_mask);

				if(std::max(pOut_buf_cur,pSrc)+counter>pOut_buf_end)
				{
					while(counter--)
					{
						while(pOut_buf_cur>=pOut_buf_end)
						TINFL_CR_RETURN(53,TINFL_STATUS_HAS_MORE_OUTPUT)						
						*pOut_buf_cur++ = pOut_buf_start[(dist_from_out_buf_start-dist)&out_buf_size_mask];
						dist_from_out_buf_start++;
					}
					continue;
				}
				#if 1 //MINIZ_USE_UNALIGNED_LOADS_AND_STORES
				else if(counter>=9&&counter<=dist)
				{
					const uint8_t *pSrc_end = pSrc+(counter&~7);

					do
					{
						((uint32_t*)pOut_buf_cur)[0] = ((const uint32_t*)pSrc)[0];
						((uint32_t*)pOut_buf_cur)[1] = ((const uint32_t*)pSrc)[1];
						pSrc+=8; pOut_buf_cur+=8;

					}while(pSrc<pSrc_end);

					counter&=7;
					if(counter<3)
					{
						if(counter!=0)
						{
							pOut_buf_cur[0] = pSrc[0];
							if(counter>1)
							pOut_buf_cur[1] = pSrc[1];
							pOut_buf_cur+=counter;
						}
						continue;
					}
				}
				#endif
				do
				{
					pOut_buf_cur[0] = pSrc[0];
					pOut_buf_cur[1] = pSrc[1];
					pOut_buf_cur[2] = pSrc[2];
					pOut_buf_cur+=3; pSrc+=3;
					counter-=3;

				}while((int)counter>2);

				if((int)counter>0)
				{
					pOut_buf_cur[0] = pSrc[0];
					if((int)counter>1)
					pOut_buf_cur[1] = pSrc[1];
					pOut_buf_cur+=counter;
				}
			}
		}
	}while(0==(r->m_final&1));

	/* Ensure byte alignment and put back any bytes from the bitbuf if we've looked ahead too far on gzip, or other Deflate streams followed by arbitrary data. */
	/* I'm being super conservative here. A number of simplifications can be made to the byte alignment part, and the Adler32 check shouldn't ever need to worry about reading from the bitbuf now. */
    TINFL_SKIP_BITS(32,num_bits&7);
    while(pIn_buf_cur>pIn_buf_next&&num_bits>=8)
    {
        pIn_buf_cur--; num_bits-=8;
    }
    bit_buf&=tinfl_bit_buf_t((uint64_t(1)<<num_bits)-uint64_t(1));
    assert(0==num_bits); /* if this assert fires then we've read beyond the end of non-deflate/zlib streams with following data (such as gzip streams). */

	/*if(0!=(decomp_flags&TINFL_FLAG_PARSE_ZLIB_HEADER))
	{
		for(counter=0;counter<4;counter++) 
		{
			unsigned s; 
			if(0!=num_bits) TINFL_GET_BITS(41,s,8) else TINFL_GET_BYTE(42,s)
			r->m_z_adler32 = (r->m_z_adler32<<8)|s; 
		}
	}*/
	TINFL_CR_RETURN_FOREVER(34,TINFL_STATUS_DONE)
	TINFL_CR_FINISH //}

common_exit: /////////////////////////////////////////////////////////////////

	/* As long as we aren't telling the caller that we NEED more input to make forward progress: */
    /* Put back any bytes from the bitbuf in case we've looked ahead too far on gzip, or other Deflate streams followed by arbitrary data. */
    /* We need to be very careful here to NOT push back any bytes we definitely know we need to make forward progress, though, or we'll lock the caller up into an inf loop. */
    if(status!=TINFL_STATUS_NEEDS_MORE_INPUT
	 &&status!=TINFL_STATUS_FAILED_CANNOT_MAKE_PROGRESS)
    {
        while(pIn_buf_cur>pIn_buf_next&&num_bits>=8)
        {
            pIn_buf_cur--; num_bits-=8;
        }
    }

	r->m_num_bits = num_bits; 
	r->m_bit_buf = bit_buf&tinfl_bit_buf_t((uint64_t(1)<<num_bits)-uint64_t(1));
	r->m_dist = dist; 
	r->m_counter = counter; 
	r->m_num_extra = num_extra;
	r->m_dist_from_out_buf_start = dist_from_out_buf_start;

	*pIn_buf_size = pIn_buf_cur-pIn_buf_next; 
	*pOut_buf_size = pOut_buf_cur-pOut_buf_next;

	/*if(status>=0&&0!=(decomp_flags&(TINFL_FLAG_PARSE_ZLIB_HEADER|TINFL_FLAG_COMPUTE_ADLER32)))
	{			
		const uint8_t *ptr = pOut_buf_next; 
		size_t buf_len = *pOut_buf_size, block_len = buf_len%5552;
		uint32_t s1 = r->m_check_adler32&0xffff, s2 = r->m_check_adler32>>16; 
		while(0!=buf_len)
		{
			uint32_t i = 0;
			for(;i+7<block_len;i+=8,ptr+=8)
			{
				s2+=s1+=ptr[0]; s2+=s1+=ptr[1]; s2+=s1+=ptr[2]; s2+=s1+=ptr[3];
				s2+=s1+=ptr[4]; s2+=s1+=ptr[5]; s2+=s1+=ptr[6]; s2+=s1+=ptr[7];
			}
			for(;i<block_len;i++)
			{
				s2+=s1+=*ptr++;
			}

			s1%=65521U; s2%=65521U; buf_len-=block_len; block_len = 5552;
		}
		r->m_check_adler32 = (s2<<16)+s1; 
		if(status==TINFL_STATUS_DONE
		&&decomp_flags&TINFL_FLAG_PARSE_ZLIB_HEADER&&r->m_check_adler32!=r->m_z_adler32)
		status = TINFL_STATUS_ADLER32_MISMATCH;
	}*/
	return status;
}

size_t daeGZ::inflate(const void *in, size_t in_chars)
{	
	if(_error!=DAE_OK) return 0;

	//http://code.google.com/p/miniz/source/browse/trunk/tinfl.c 			
	daeCTC<(sizeof(_private_implementation)>=sizeof(tinfl_decompressor))>();
	tinfl_decompressor &decomp = (tinfl_decompressor&)_private_implementation;
	if(_restart==0) 
	{
		decomp.m_state = 0; //tinfl_init(&decomp);

		//These are extensions to the original Miniz data structure.
		//Really the 32k dictionary should be housed inside it also.
		decomp.ext.wrapping_output_buf = 0;
	}
	
	//THIS IS really all one big loop. 
	//tinfl_decompress expects _32k_dictionary to be a persistent
	//circular buffer, which really complicates things when input
	//is partially buffered.
	//I guess there is good reason for this. Like the buffer must
	//store partial work that is gradually resolved into an ouput.
	uint32_t f = TINFL_FLAG_HAS_MORE_INPUT;
	char buf[4096];	
	size_t underflow = getUnderflow();
	assert(underflow<sizeof(buf));
	if(0!=underflow)
	memcpy(buf,&_data[_size],underflow);
	_data.resize(_size); 	
	for(bool looping=true;looping;)
	{
		//TODO? In theory buf can be discarded as soon as carried
		//overflow is consumed. Whether it can perform any better 
		//or not, it seems easier to conceptualize this algorithm.
		const size_t cp = std::min(sizeof(buf)-underflow,in_chars);
		const size_t bufN = underflow+cp;		
		memcpy(buf+underflow,in,cp);
		(char*&)in+=cp; in_chars-=cp;
			 	
		size_t rem = _final_input-_restart;
		if(bufN>=rem) if(bufN!=rem)
		{
			_error = DAE_ERR_INVALID_CALL; return 0;
		}
		else f = 0;
		looping = in_chars!=0||f==0&&underflow!=0;

		underflow = bufN; //Misleading...
		const char *offset = _32k_dictionary+decomp.ext.wrapping_output_buf;
		size_t out = sizeof(_32k_dictionary)-decomp.ext.wrapping_output_buf;
		tinfl_status status =
		tinfl_decompress(&decomp,(uint8_t*)buf,&underflow,
		(uint8_t*)_32k_dictionary,(uint8_t*)offset,&out,f);	
		underflow = bufN-underflow;
		const size_t bufN_underflow = bufN-underflow;
		
		decomp.ext.wrapping_output_buf+=out;
		if(decomp.ext.wrapping_output_buf>=sizeof(_32k_dictionary))
		{
			decomp.ext.wrapping_output_buf-=sizeof(_32k_dictionary);
			assert(0==decomp.ext.wrapping_output_buf);
		}

		_restart+=bufN_underflow;
	
		switch(status)
		{
		case TINFL_STATUS_DONE: looping = false;
		case TINFL_STATUS_HAS_MORE_OUTPUT:
		case TINFL_STATUS_NEEDS_MORE_INPUT: break;		

		default: _error = DAE_ERR_BACKEND_IO; return 0; 
		}
				
		_data.insert(_data.end(),offset,offset+out);	
		_size = _data.size();
		if(underflow!=0) 
		if(looping) memmove(buf,buf+bufN_underflow,underflow);
		else _data.insert(_data.end(),buf+bufN_underflow,buf+bufN);
	}															   			
	return _size;
}

//---.
}//<-'

#endif //__COLLADA_DOM__ZAE

/*C1071*/