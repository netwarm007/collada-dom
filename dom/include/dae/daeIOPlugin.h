/*
 * Copyright 2006 Sony Computer Entertainment Inc.
 *
 * Licensed under the MIT Open Source License, for details please see license.txt or the website
 * http://www.opensource.org/licenses/mit-license.php
 *
 */

#ifndef __COLLADA_DOM__DAE_IOPLUGIN_H__
#define __COLLADA_DOM__DAE_IOPLUGIN_H__

#include "daeURI.h"
  
COLLADA_(namespace)
{//-.
//<-'
	
//THIS IS NOT COMPILER SPECIFIC. But do GCC/Clang use Win32 APIs?
#ifdef _WIN32
#ifndef _MSC_VER
#error Seen _WIN32 on non Visual Studio compiler. How to proceed?
#endif
/**
 * Define the system fopen hook for @c daeCRT. 
 */
#ifndef COLLADA_DOM_FOPEN
#define COLLADA_DOM_FOPEN(dae_fopen)\
typedef FILE*(*const dae_fopen##_f)(const wchar_t*,const wchar_t*,int);\
static dae_fopen##_f dae_fopen = _wfsopen;
#endif
#ifndef COLLADA_DOM_FILENO
#define COLLADA_DOM_FILENO _fileno
#endif
#endif //_WIN32

#ifndef COLLADA_DOM_FOPEN
/**
 * Define the system fopen hook for @c daeCRT. 
 */
#define COLLADA_DOM_FOPEN(dae_fopen)\
typedef FILE*(*const dae_fopen##_f)(const char*,const char*);\
static dae_fopen##_f dae_fopen = fopen;
#endif
COLLADA_DOM_FOPEN(dae_fopen)

/**WARNING, NOT-RECOMMENDED 
 * @warning The built-in plugins no longer use FILE handles
 * but they remain for now in case anyone wants to use them.
 */
static struct daeCRT
{
	/**LEGACY-SUPPORT, NOT-RECOMMENDED
	 * @c fread, @c fwrite & @c stats are only to make it 
	 * easy to quickly write up a test @c daeIO. 
	 * @c fopen and @c fclose are to initialize a modular
	 * file, and release it in @c daeIO::~daeIO().
	 */
	struct FILE
	{
		dae_fopen_f fopen;
		::FILE*(*const tmpfile)();
		int(*const fclose)(::FILE*);
		size_t(*const fread)(void*,size_t,size_t,::FILE*);
		size_t(*const fwrite)(const void*,size_t,size_t,::FILE*);
		struct stats
		{
			/**
			 * @c v is the size/version of @c this.
			 * @c fd is the @c fileno() file-descriptor.
			 * @c size is the size of the file via @c fseek().
			 */
			int v, fd, size; stats():v(sizeof(stats)){}
		};
		/**
		 * Calling @c stats rewinds @c FILE. @c stats() is provided
		 * only as a courtesy to make a toy @c daeIO easy to set up.
		 */
		int(*const _stat)(::FILE*,stats*);
		/**OVERLOAD Allows overload of @c stat(). */
		inline int stat(::FILE *f, stats *p){ return _stat(f,p); }
		/**OVERLOAD Simplifies calling @c stat(). */
		inline const stats &stat(::FILE *f, const stats &st=stats())const
		{
			_stat(f,&const_cast<stats&>(st)); return st;
		}

		void *_reserved;

		FILE():fopen(dae_fopen),tmpfile(::tmpfile),fclose(::fclose),fread(::fread),fwrite(::fwrite)
		,_stat(_stat_f),_reserved(){}

	private:
		/**
		 * @return Returns the modules notion of the size of @c stats.
		 */
		static int _stat_f(::FILE *f, stats *p)
		{
			#ifndef COLLADA_DOM_FILENO
			#define COLLADA_DOM_FILENO fileno
			#endif
			//If you hit an error here, try -std=gnu++11.
			p->fd = ::COLLADA_DOM_FILENO(f);
			fseek(f,0,SEEK_END); 					
			p->size = ftell(f); fseek(f,0,SEEK_SET); return sizeof(*p);
		}

	}FILE; void *const _reserved[3]; daeCRT():_reserved(){}

}daeCRT_default;

/**EXPERIMENTAL
 * The daeIO class represents the OS's file network.
 *
 * @remarks All of the different methods are provided
 * because of there being no standard way to do I/O, so 
 * every library that clients might want to use for some
 * reason has its own quirks.
 *
 * @note Normal use entails @c getLock() and @c readIn().
 * Or @c writeOut() if writing.
 *
 * @see @c daePlatform::openIO().
 */
class daeIO
{	
	const int _version;
	
COLLADA_(public)
	/**
	 * Default Constructor
	 */
	daeIO():_version(0){}
	/**
	 * Virtual Destructor
	 *
	 * The destructor should cleanup and 
	 * finalize all of the file operations.
	 */
	virtual ~daeIO(){}

	/**
	 * There should be one error only.
	 * Any error is fatal to the IO process.
	 * All methods should set the error, and all
	 * callers should check the error--
	 * --except for readIn & writeOut, which return
	 * the error state, so that loops can check it.
	 */
	virtual daeError getError() = 0;

	/**ZAE
	 * Request Partial-Content; Advanced use
	 * of @c getLock().
	 */
	struct Range
	{
		size_t first,second; 
		size_t limit_to_size(size_t s)
		{
			if(first>s) first = s;
			if(second>s) second = s; return s;
		}
		size_t size(){ return second-first; }
		bool empty(){ return first==second; }
	};
	/**
	 * Gets read & write locks on the files.
	 * 
	 * @return Returns the size of the INPUT file,
	 * -if there is any. Otherwise 0 is returned. 
	 * OUTPUT files are presumed to be 0 sized.
	 *
	 * @remarks Presently @c getLock() is required to begin
	 * reading-and/or-writing. Once called @c this @c daeIO
	 * must be closed before new operations can begin again.
	 * Some methods can be called prior to @c getLock() and
	 * themselves may implicitly call @c getLock(). In that
	 * case calling @c getLock() must return the same value.
	 *
	 * ADVANCED
	 * @param I contains start and stop points for @c readIn().
	 * @param O contains start and stop points for @c writeOut().
	 * @warning The behavior of @a O isn't well-defined.
	 * @c getLock() should adjust these to be in range. This can be
	 * used to get the size of the the output file.
	 */
	virtual size_t getLock(Range *I=nullptr, Range *O=nullptr) = 0;	
	/**ZAE
	 * Reset the @c Range provided to @c getLock() almost as if 
	 * @c this @c daeIO were closed and reopened except any locks
	 * remain in place and many steps may be avoided AND except that
	 * @a I and or @a O must be nonzero and if one is @c nullptr it is
	 * not to disrupt the operation of its respective I/O plugin.
	 *
	 * @remark In other words, setRange is "fseek" to getLock's "fopen."
	 * (Using the analogy of a very basic "FILE" based @c daeIO object.)
	 */
	virtual size_t setRange(Range *I=nullptr, Range *O=nullptr) = 0;	

	/**INPUT
	 * Read in from a file handle or memory buffer.
	 *
	 * Implement this with ReadFile or fwrite for example.
	 */
	virtual daeOK readIn(void *in, size_t chars){ return DAE_ERR_NOT_IMPLEMENTED; }

	/**OUTPUT
	 * Write out to a file handle or memory buffer.
	 *
	 * Implement this with WriteFile or fwrite for example.
	 */
	virtual daeOK writeOut(const void *out, size_t chars){ return DAE_ERR_NOT_IMPLEMENTED; }

	/**INPUT
	 * Use mmap, or CreateFileMapping for example to map the entire
	 * image for reading; Or an internal string if there is one.
	 *
	 * This is usually efficient, but only works for entire files,
	 * -and is not really a strategy for writing files. This method
	 * is exclusively for the reading IOPlugin.
	 *
	 * @remarks A @daeIO should have at most one map open at a time.	 
	 */
	virtual const void *getReadMap(){ return nullptr; }

COLLADA_(public) //LEGACY

	/**INPUT, WARNING, NOT-RECOMMENDED
	 * @warning The built-in plugins no longer use FILE handles
	 * but they remain for now in case anyone wants to use them.
	 *
	 * These are for IOPlugins that require whole FILE objects.
	 * They should call @c daeIOPlugin::getCRT() and use that to 
	 * create and close the file; or return @c nullptr.
	 *
	 * @return Returns @c nullptr if @c int is not 0. 0 is binary.
	 */
	virtual FILE *getReadFILE(int=0){ return nullptr; }
	/**OUTPUT, WARNING, NOT-RECOMMENDED
	 * @warning The built-in plugins no longer use FILE handles
	 * but they remain for now in case anyone wants to use them.
	 *
	 * @see @c getReadFILE() explanation.
	 */
	virtual FILE *getWriteFILE(int=0){ return nullptr; }
};

/**EXPERIMENTAL
 * Holds low-level request-info for @c daeIOPlugin/daeIOPlatform.
 *
 * Technically @c fulfillRequestI() can be used to workaround the
 * more limited @c daeArchive APIs. This feels more like feature
 * than oversight. Although, still, use-at-own-risk.
 */
class daeIORequest
{	
COLLADA_(public) //subject to change
	/**SKETCHY
	 * This is reserved memory. The back 8 bits are a version number denoting
	 * the size/layout of @c this record.
	 */
	long long ops;
	/**
	 * For read (input) requests this the archive that the resource is loaded
	 * into. For write (output) requests, it's simply the souce doc's archive.
	 */
	daeArchive *scope;
	/**WARNING
	 * This is a general purpose string. It can be an in memory document if a
	 * a plugin understands it to be; or it can just be arguments to a plugin.
	 *
	 * There's no 0-terminator requirement. Plugins that want @c string to be
	 * text representing a document or other resource should say if they have
	 * to be 0-terminated or if a misformatted string can read beyond the end.
	 */
	daeHashString string;	
	/**
	 * For write (output) requests, localURI should be @c daeDoc::getDocURI().
	 * The system is open-ended, for better or worse, but this is a guarantee,
	 * -and is so if @c localURI->getDoc()->getDocType()==daeDocType::ARCHIVE,
	 * then the I/O apparatus can intuit that it is to output an archive tree.
	 */
	const daeURI *localURI, *remoteURI;

	static const long long vN = 1LL<<56;

	daeIORequest(const daeArchive *a=nullptr, const daeHashString &b=nullptr, const daeURI *c=nullptr, const daeURI *d=nullptr)
	:ops(vN),scope(const_cast<daeArchive*>(a)),string(b),localURI(c),remoteURI(d)
	{
		//This is now built-in behavior if the scope is nonzero. 
		if(!isEmptyRequest()){ resolve(); narrow(); }
	}

COLLADA_(public)
	/**WARNING
	 * Helps to implement @c daeIOPlugin::addDoc() without using
	 * @c const_cast.
	 * @warning Assuming @c false==isEmptyRequest().
	 * @return Returns a pointer to be consistent with @c getDOM().
	 */
	daeDOM *getDOM()const
	{
		return (daeDOM*)((daeObject*)scope)->getDOM(); 
	}

	/**LEGACY-SUPPORT
	 * Such a request is assumed to be creating a new/blank doc. 
	 * If requests need to avoid this, set @c string equal to "".
	 * @note If @c daeIOPlugin::addDoc() returns @c nullptr, it's
	 * assumed to not be a new-doc request.
	 */
	bool isNewDocRequest()const
	{
		//Neither "openDoc" nor "openDocFromMemory."
		return remoteURI==nullptr&&string==nullptr; 
	}

	/**
	 * An I/O request that is just input, or just output, will be
	 * empty if vice versa.
	 */
	bool isEmptyRequest()const{ return scope==nullptr; }

COLLADA_(public) //daePlatform::openURI() support
	/**
	 * Resolves both URIs.
	 * @c daeArchive::openDoc<void>() does not resolve. 
	 * @note @c daeIORequest::daeIORequest() resolves.
	 */
	inline void resolve()const
	{
		if(localURI!=nullptr) localURI->resolve(scope);
		if(remoteURI!=nullptr) remoteURI->resolve(scope);
	}

	/**ZAE
	 * Narrows @c scope according to @c localURI. To narrow
	 * means to recursively assign the request to its inner
	 * most @c daeArchive. After this @c narro() calls upon
	 * @c daePlatform::narrowURI().
	 * 
	 * @see daeURI.cpp
	 */
	COLLADA_DOM_LINKAGE void narrow();

	/** Signals desire to default to @c domAny. */
	inline bool allowsAny()const
	{
		assert(localURI!=nullptr); return localURI->getAllowsAny(); 
	}

	template<class LAZY>
	/**HELPER Helps @c daePlatform::openURI(). */
	inline void unfulfillRequest(daeDocRoot<LAZY> &doc)const 
	{
		doc.error = DAE_ERR_NOT_IMPLEMENTED; doc = nullptr;
	}

	template<class ROOT>
	/**HELPER Helps @c daePlatform::openURI(). */
	inline void fulfillRequestI(daeIOPlugin *I, daeDocRoot<> &doc)const 
	{
		return fulfillRequestI(daeGetMeta<ROOT>(),I,doc);
	}
	template<class LAZY>
	/**HELPER Helps @c daePlatform::openURI(). */
	inline void fulfillRequestI(daeMeta *meta, daeIOPlugin *I, daeDocRoot<LAZY> &doc)const 
	{
		const COLLADA_INCOMPLETE(LAZY) daeArchive *a = scope; assert(!isEmptyRequest());
		doc.error = a->_read2(doc,meta,*this,I);
	}
};

/**EXPERIMENTAL
 * The @c daeIOPlugin class provides the input/output plugin interface, which is
 * the interface between the COLLADA runtime and the backend storage. A native
 * COLLADA XML plugin implementation is provided along with this interface.
 */
class daeIOPlugin
{	
	const int _version;

	friend class daeDOM;
	friend class daeArchive;
	friend class daeDocument;
	/**
	 * This is a pointer so that it can grow with each new version.
	 */
	const daeIORequest *_request; 

COLLADA_(protected) 
	/**EXPERIMENTAL
	 * The legacy LIBXML plugin needs to establish a second I/O channel
	 * in order to implement its questionable "RAW" extension feature.
	 */
	daeIOPlugin(const daeIORequest *secondary_request=nullptr)
	:_version(0),_request(secondary_request){}	

COLLADA_(public) //COLLADA::DAEP::InstantLegacyIO needs access to this.
	/**
	 * Virtual Destructor
	 */
	virtual ~daeIOPlugin(){}

COLLADA_(public) //The I/O plugin uses these methods to query the request object.
	/**
	 * This and @c getDemands() are more for @c daeIO than @c daeIOPlugin. 
	 * @see @c daePlatform::openIO(). Basically @c daeIOPlugin is its parameters. 
	 */
	const daeIORequest &getRequest(){ return *_request; }

	/**SCOPED-ENUM
	 * @see @c getDamands.
	 * These are generally tried in this order. A string is to be used if present.
	 * - unimplemented: Cannot read or write (depending on if reading or writing.)
	 * - map: Cannot stream. Requires a full view of the file in map or string form.
	 * - string: Cannot stream. The view is @c getRequest().string.extent characters.
	 * - CRT: The original LibXML and TinyXML based plugins support this via getCRT().
	 */
	struct Demands{ enum{ unimplemented=1, map=2, string=4, CRT=8 }; };
	/** 
	 * Gets the I/O @c Demands of this plugin.
	 * @a I and @a O are bitwise @c Demands that @c daeIO must fulfill.
	 * @see @c daeIOPlugin::Demands Doxygentation.
	 */
	virtual void getDemands(int &I, int &O){ I = O = 0; }

	/** 
	 * Returns a @c daeCRT for the translation-unit that requires @c daeCRT::FILE.
	 */
	virtual daeCRT *getCRT()
	{
		int I, O; getDemands(I,O); assert(0==((I|O)&Demands::CRT)); return nullptr;
	}
	/** 
	 * Guarantees @c daeCRT to make toy @c daeIO implementations straightforward.
	 */
	inline daeCRT &getCRT_default(daeCRT &def=daeCRT_default)
	{
		daeCRT *o = getCRT(); return o==nullptr?def:*o;
	}

COLLADA_(private)
	/** 
	 * @return Returns one of: @c daeDocRef(DOM),
	 * @c daeArchiveRef(DOM), or @c daeDocumentRef(DOM).
	 * @param dae_root is the the metadata passed to the open
	 * operation. It's mainly provided so a ZAE like plugin can
	 * decide if it wants to automatically open an index document.
	 */
	virtual daeOK addDoc(daeDocRef &out, daeMeta *dae_root) = 0;

	/** 
	 * Reads content into the document/element from an input.
	 *
	 * If @c addDoc() added a @c daeArchive, @a content will be
	 * the content of its compulsory root-document. For now, the
	 * way is to just read in the archive, before moving onto the
	 * root-document. The following code extracts the archive from 
	 * @a content, without risking anything:
	 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~[C++]
	 * //The only trouble is, content may not belong to a document.
	 * daeArchive *a = content.getObject()->getDoc()->getArchive(); 
	 */
	virtual daeOK readContent(daeIO &IO, daeContents &content) = 0;

	/**
	 * Writes a document/element's contents-array to an output.
	 *
	 * @see the @c readContent() instructions on how to do archives.
	 * @note Archive operations are ambiguous. The user must choose
	 * a plugin that meets their need. E.g. to write a full archive
	 * an archive writing plugin must be chosen; To write a portion
	 * of an archive only, a different plugin must be used, because
	 * these arguments are inadequate to communicate which is which.
	 */
	virtual daeOK writeContent(daeIO &IO, const daeContents &content) = 0;	

	/**
	 * @return Returns @c DAE_ERR_NOT_IMPLEMENTED if the library is
	 * to write each of the DOM's docs one-by-one as if each one is
	 * written in an unspecified order.
	 *
	 * @remark Requesting to write the DOM itself is akin to asking
	 * to write the entire-world--or that is the parts of it loaded
	 * into the DOM. @c writeContent() is inadequate as there isn't
	 * a root-document, as DOMs aren't true self-contained archives.
	 *
	 * @note There isn't a clear scheme for writing a DOM. Therefor
	 * the default behavior is to use the URI as a mask. The plugin
	 * --as always--is free to interpret any URI anyway it needs to.
	 * @see @c daeDoc::writeTo().
	 */
	virtual daeOK writeDOM(daeIO &IO, const daeDOM &DOM)
	{
		return DAE_ERR_NOT_IMPLEMENTED; 
	}
};

//Note: There's no need for two parameters since the
//simple/single-use I/O won't facilitate in & output.
template<int IO=0>
/**
 * Implements a complementary @c daeIOPlugin.
 */
class daeIOSecond : public daeIOPlugin
{
COLLADA_(public) //daeIOPlugin methods

	virtual void getDemands(int &I, int &O){ I = O = IO; }
	virtual daeCRT *getCRT(){ return IO&Demands::CRT?&daeCRT_default:nullptr; }
	virtual daeOK addDoc(daeDocRef&,daeMeta*){ return DAE_ERR_NOT_IMPLEMENTED; }
	virtual daeOK readContent(daeIO&,daeContents&){ return DAE_ERR_NOT_IMPLEMENTED; }
	virtual daeOK writeContent(daeIO&,const daeContents&){ return DAE_ERR_NOT_IMPLEMENTED; }	
	//CAUTION: THIS IS REALLY NOT A GOOD SET UP. DON'T GIVE IT A TEMPORARY daeIORequest OBJECT!!!
	//daeIOSecond(const daeIORequest &tmp=daeIORequest(nullptr,nullptr,nullptr)):daeIOPlugin(&tmp){}
	daeIOSecond(const daeIORequest &tmp=_empty_request()):daeIOPlugin(&tmp){}
	//HACK: GCC 5 or so on Linux aggressively reuses the temporary's memory. SCHEDULED FOR REMOVAL?
	static daeIORequest &_empty_request(){ static daeIORequest e(nullptr,nullptr,nullptr); return e; }
};
/**LEGACY
 * Implements an empty @c daeIOPlugin.
 */
typedef daeIOSecond<daeIOPlugin::Demands::unimplemented> daeIOEmpty;

/**ZAE
 * @c daePlatform and @c daeAtlas implement this interface.
 */
class daeIOController
{		
COLLADA_(public)
	/**
	 * Ostensibly begin a file/resource read/write operation.
	 *
	 * @param i contains arguments for reading, and the plugin
	 * selected for doing the reading, if any.
	 * @param o contains arguments for writing, and the plugin
	 * selected for doing the writing, if any.
	 *
	 * @remarks The DOM object calls @c openIO(), providing
	 * I/O request information. @I and @O also contain within
	 * them all further arguments intended for @c openIO() set
	 * up in advance by @c daeDOM. 
	 */
	virtual daeIO *openIO(daeIOPlugin &I, daeIOPlugin &O) = 0;
	/**
	 * Manage/destruct IO objects provided by @c openIO().
	 */
	virtual void closeIO(daeIO *IO) = 0;

COLLADA_(public) //UTILITIES (use if you want)

	#ifdef NDEBUG
	#error Is "Stack" misleading? Pool? Pile?
	#endif
	template<class T>
	/**COURTESY
	 * This is a simple pool for reusing memory.
	 *
	 * @TODO Register these so that unused memory 
	 * can be freed prior to their owner's demise.
	 */
	class Stack : private std::vector<T*>
	{
	COLLADA_(public)

		~Stack(){ clear(); }void clear()
		{
			while(!vector::empty())
			{
				delete vector::back(); vector::pop_back(); 
			}
		}		
		template<class S> T *pop(const S &cp)
		{
			T *IO; if(!vector::empty())
			{
				IO = vector::back(); vector::pop_back();
				new(IO) T(cp);
			}
			else IO = new T(cp); return IO;
		}
		void push(void *io)
		{
			((T*)io)->~T(); vector::push_back((T*)io);
		}		
	};	
	
	template<int BITS, class T> 
	/**COURTESY
	 * The ZAE code uses this to swap bytes. The colladadom3d
	 * source code has used it to load TGA files in the event
	 * that there's no built-in image library.
	 *
	 * @note Not knowing if the system CHAR_BIT is 8 it's not
	 * possible to do this in reverse. And it's simplistic to
	 * assume that the target field is @c char aligned at all.
	 */
	static T &BigEndian(void *inout)
	{
		daeCTC<BITS==CHAR_BIT*sizeof(T)>();
		#if __BYTE_ORDER__==__ORDER_BIG_ENDIAN__		
		char *inout_LE = (char*)inout;
		for(int k=0;k<sizeof(T);k++)
		std::swap(inout_LE[k],inout_LE[sizeof(T)-k-1]); 		
		#endif
		return *(T*)inout;
	}
	template<int BITS, class T> 
	/**COURTESY
	 * @see @c void* overload of @c BigEndian(). 
	 */
	static T &BigEndian(T &inout)
	{
		return BigEndian<BITS,T>(&inout); 
	}
};

/**
 * The daePlatform class is an abstract representation
 * of the operating-system and computer network. 
 *
 * Since 2.5 the library strives to keep its hands clean
 * and not tie clients' hands in the process.
 */
class daePlatform : public daeIOController
{	
	friend class daeDOM;

	const int _version;

COLLADA_(public)
	/**
	 * Default Constructor
	 */
	daePlatform():_version(0){}
	/**
	 * Virtual Destructor
	 */
	virtual ~daePlatform(){}

	/**
	 * The @c daeDOM constructor calls @c getDefaultBaseURI() if
	 * a URI is not among the constructor arguments.
	 * @remarks Historically this is the "current working directory"
	 * of the executing process, in the form of file:/// or file://HOST/,
	 * -but it needn't be.
	 * @return Returns the URI to use by constant reference. This 
	 * can @a URI, or any other address.
	 *
	 * @note Since @c daeURI::getIsDirectoryLike() it's unnecessary
	 * to append a slash (/) to this URI but it can't hurt to do so.
	 */
	virtual const daeURI &getDefaultBaseURI(daeURI &URI){ return URI; }

	/**
	 * Converts @a URI into an absolute/canonical URI.
	 * The system may use this opportunity to begin caching the resource.
	 *
	 * @remarks The library should pass all URIs through @c resolveURI().
	 * However it will not if @c daeURI::getIsResolved()==true, except for
	 * when the URI is being refreshed.
	 * IMPORTANT
	 * =========
	 * Doing @c daeURI::setIsResolved() is @c resolveURI()'s responsibility.
	 * There are edge cases when looking up base URIs. @c daeURI::baseLookup()
	 * exists to make it easier to get it right.
	 *
	 * WHY THIS IS IMPORTANT
	 * =====================
	 * URIs can be different while still referring to the same
	 * resources. If this happens your DOM will contain two instances
	 * of the same resource. The resources will be accessed as if
	 * they are separate entities, and modifications will not be reflected
	 * in the other instances.
	 *
	 * DUPLICATING INSTANCES
	 * =====================
	 * Two have two instances of the same resource, the application
	 * needs to have two DOM objects, and set them to use the same database.
	 * This will keep the documents separate, while leaving the door 
	 * open for moving sections of the documents back-and-forth between them.
	 */
	virtual daeOK resolveURI(daeURI &URI, const daeArchive &DOM_or_ZAE)
	{
		//LEGACY SUPPORT: This will implement RFC2396's recommendations.
		//(NOTE: resolve_RFC3986 is not-necessarily satisfactory or complete.)
		return URI.resolve_RFC3986(DOM_or_ZAE); 
	}
	/**CONST-FORM
	 * A resolved URI is considered equivalent to an unresolved URI. 
	 * @see non-const resolveURI() Doxygentation.
	 */
	inline daeOK resolveURI(const daeURI &URI, const daeArchive &DOM_or_ZAE)
	{
		return resolveURI(const_cast<daeURI&>(URI),DOM_or_ZAE);
	}

	/**ZAE
	 * This interface implements @c daeIORequest::narrow(). This is so to
	 * open an archive (e.g. ZAE or ZIP) if a URI is relative to archives
	 * and so requires those archives to exist to satisfy the I/O request.
	 *
	 * NOTE: @a is actually the request itself so that the remote URI can
	 * be had. Set its @c scope member accordingly.
	 * @param a is nonzero and must be set to 0 to disable processing. If 
	 * unset it is implied that one or more archives were opened and that 
	 * @c narrow() should proceed as normal. If set then the new value is
	 * made the scope of the I/O request (it can't be outside its scope.)
	 *
	 * @param b is the path of any open doc that is less-than-or-equal-to
	 * @a URI if their URIs have the same authority.
	 *
	 * @param c is the path of any open doc that's greather than @a URI's.
	 *
	 * @b and @c are mainly a courtesy since @c narrow() knows about them.
	 */
	virtual void narrowURI(daeRefView b, daeIORequest &a, daeRefView c)
	#ifdef COLLADA_DOM_OMIT_ZAE
	= 0;
	#else
	{ _narrowURI_open_ZAE(a); (void)b; (void)c; } //daeURI.cpp
	COLLADA_DOM_LINKAGE static void _narrowURI_open_ZAE(daeIORequest&);
	#endif

	/**WARNING
	 * This interface implements @c daeArchive::openDoc<void>().
	 * Calls @c req.fulfillRequestI() to complete the transaction.
	 *
	 * @param I If @c nullptr, choose an I/O plugin matching @a URI.
	 * @openedURI Use @c req.fulfillRequestI(), and set @a openURI to
	 * the returned doc's URI via @c daeDoc::getDocURI().
	 * @return Return the @c req.fulfillRequestI() error, or something
	 * suggestive of what happened, or even @c req.unfulfillRequest().
	 *
	 * When @a req.allowsAny()==true, the <domAny> specialization can
	 * be used, but only as a last resort.
	 *
	 * If the request is not fulfilled, call @c req.unfulfillRequest().
	 * This tells the caller that a pre-existing doc was not considered.
	 *
	 * @warning @c closeURI() does NOT follow @c openURI() or vice-versa.
	 * @warning @c newDoc<void>() is calling @c openURI() also. In which
	 * case there's no remote URI. @c newDoc() creates an empty document.
	 */
	virtual daeOK openURI(const daeIORequest &req, daeIOPlugin *I, daeURIRef &openedURI) = 0;

	/**WARNING
	 * Closes, or grants permission to close a document by its URI.
	 *
	 * @return Returns @c DAE_ERR_NOT_IMPLEMENTED to have the document
	 * linked to @a URI closed. Returns @c DAE_OK to signal @c closeURI()
	 * has closed the URI's document. 
	 * Returns @c DAE_ERR_DOCUMENT_DOES_NOT_EXIST or other code otherwise.
	 * (@c ERROR is recommended for this purpose. More codes can be added.)
	 *
	 * @warning @c openURI() does NOT precede @c closeURI() or vice-versa.
	 */
	virtual daeOK closeURI(const daeURI &URI){ return DAE_ERR_NOT_IMPLEMENTED; }

	/** 
	 * This must be implemented for "multi-thread" applications to work.
	 * The value doesn't matter. It just needs to be unique for the current
	 * thread context. On Windows for example: "return GetCurrentThreadId();"
	 */
	virtual int threadID(){ return 0; }

	/**
	 * Provide a default IO plugin when unprovided. 
	 * @param rw is @c 'r' for reading; @c 'w' for writing.
	 * return Returns @c nullptr if @a rw is neither @c 'r' nor @c 'w'.
	 *
	 * @remarks If @nullptr is returned, the library 
	 */
	virtual daeIOPlugin *pluginIO(const daeURI &context, int rw){ return nullptr; }
	/**
	 * Manage/destruct plugin provided by @c pluginIO().
	 */
	virtual void unplugIO(daeIOPlugin*){}

	/**
	 * Gets a @c daeDatabase pointer whenever a @c daeDOM is constructed
	 * with a @c nullptr @c DB argument.
	 * @return Returns @c nullptr if @c this platform desires a database 
	 * that is provided by the COLLADA-DOM library.
	 */
	virtual daeDatabase *attachDB(){ return nullptr; }
	/**
	 * Manage/destruct DB objects provided by @c attachDB().
	 */
	virtual void detachDB(daeDatabase_base *DB){}

	/**LEGACY-SUPPORT
	 * This is called when a new document is created, either as a blank
	 * or for reading.
	 * It's here to accelerate @c daeDocument::typeLookup() if desired.
	 * To do so, call @c doc.typeLookup_enable<T>(). Do this for each T
	 * for which there's code that calls @c typeLookup(). If this is not
	 * done there may be a hiccup when @c typeLookup() is called, because
	 * it must scan the entire document for that type.
	 * The database that the document's DOM is using may have other features
	 * of interest. @a doc.getMeta() can be used to decide what types of T are 
	 * pertinent.
	 * @remarks Prior to 2.5 the system aggressively indexed every element for
	 * the entire database, in one large multimap. "typeLookup" is not required
	 * for anything. It's probably best to avoid it if at all possible.
	 *
	 * @note In practice @a doc should be a completely blank document, without any
	 * content. Still, this API is not framed as a "notification" and so, it may be
	 * called by any (non-library) code with documents that may not be empty, if its
	 * purpose is to somehow repurpose the API. This is your platforms opportunity to
	 * act upon a document before it's loaded with content.
	 */
	virtual void personalizeDocument(const daeDocument &doc){ (void)doc; /*NOP*/ }

	/**BITWISE-FLAGS, ENUM
	 * This @c enum corresponds to @c getLegacyProfile() and @c setLegacyProfile().
	 * The default implementation supports them. Stray from that and support isn't
	 * guaranteed. Databases can consult the flags; It's more a back-compatability
	 * thing.
	 */
	enum Legacy
	{
		/**Use LibXML if @c pluginIO() returns @c nullptr. */
		LEGACY_LIBXML=1,
		/**@c option_to_write_COLLADA_array_values_to_RAW_file_resource(). */
		LEGACY_LIBXML_RAW=2,		
		/**Use the @c daeRawResolver. (Presumably with LEGACY_LIBXML_RAW.) */
		LEGACY_RAW_RESOLVER=4,
		/**Use the @c daeDefaultURIResolver. (Enables @c daeURI::get(), etc.) */
		LEGACY_URI_RESOLVER=8,
		/**Use the @c daeDefaultIDREFResolver. (Enables @c daeIDREF::get(), etc.) */
		LEGACY_IDREF_RESOLVER=16,
		/**WARNING
		 * Use the @c daeDefaultSIDREFResolver. (Enables @c daeSIDREF::get(), etc.) 
		 * @warning SIDREF was always experimental, and not enabled "out of the box."
		 * The legacy SIDREF resolver expects daeDocument::sidLookup() to be usable.
		 * FOR THAT TO WORK THE database must forward the SID change-notices to the
		 * documents' @c _carry_out_change_of_ID_or_SID() API. The default database
		 * does this. Database implementations can choose to facilitate these flags.
		 */
		LEGACY_SIDREF_RESOLVER=32,
		/**Use daeLibXMLPlugin::option_to_use_codec_Latin1(). (Avoid if possible.) */
		LEGACY_EXTENDED_LATIN1=64,
		
	  ////THESE ARE UNLIKELY TO BE IMPLEMENTED///////////////////////////
	  //Use daeURI::refresh() to ensure freshness. Or getURI_baseless.//
	  //The default database faciliates daeDocument::idLookup(), etc.//
	  //The daeDocument::typeLookup() facility is automatic, and can//
	  //be accelerated by personalizeDocument()./////////////////////
	  ///////////////////////////////////////////

		/**Keep @daeURI up-to-date as elements and documents are moved around. */
		//LEGACY_KEEP_URIS_FRESH=128,
		/**@c daeDocument indexes are front-loaded. (They'll work regardless.) */
		//LEGACY_INDEXING_SERVICES=256,
	};
	/**
	 * The @c daeDOM constructor calls @c getLegacyProfile()
	 * in order to ascertain to what degree it should follow
	 * through with all of the actions that up to 2016 where
	 * taken for granted. 
	 * @return Returns a combination of @c Legacy enum bits.
	 */
	virtual int getLegacyProfile() = 0;
	/**
	 * @see @c getLegacyProfile().
	 * @return Returns the new profile. It needn't match.
	 */
	virtual int setLegacyProfile(int){ return getLegacyProfile(); }
};

//---.
}//<-'

#endif //__COLLADA_DOM__DAE_IOPLUGIN_H__
/*C1071*/
