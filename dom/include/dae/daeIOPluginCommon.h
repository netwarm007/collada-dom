/*
 * Copyright 2006 Sony Computer Entertainment Inc.
 *
 * Licensed under the MIT Open Source License, for details please see license.txt or the website
 * http://www.opensource.org/licenses/mit-license.php
 *
 */

#ifndef __COLLADA_DOM__DAE_IO_PLUGIN_COMMON_H__
#define __COLLADA_DOM__DAE_IO_PLUGIN_COMMON_H__

#include "daeURI.h"
#include "daeContent.h"
#include "daeIOPlugin.h"
#include "daeErrorHandler.h"

#ifdef BUILDING_IN_LIBXML //xmlreader.h
struct _xmlTextReader; struct _xmlTextWriter;
#endif
#ifdef BUILDING_IN_TINYXML //tinyxml.h
class TiXmlBase; class TiXmlNode; class TiXmlElement;
#endif

COLLADA_(namespace)
{//-.
//<-'
	
#if !defined(BUILDING_IN_LIBXML)\
 && !defined(BUILDING_IN_TINYXML)

#ifdef BUILDING_COLLADA_DOM
struct daeLegacyIOPlugins : daeIOEmpty //INTERNAL
{
	daeLegacyIOPlugins(int legacy, const daeURI &URI)
	{
		//There's room for improvement here.
		daeEH::Error<<
		"No platform provided daeIOPlugin.\n"
		"Nor built-in plugin.\n"
		"Could not read/write document:\n"<<
		URI.getURI();		
	}
};
#endif //BUILDING_COLLADA_DOM

#else //BUILDING_IN_LIBXML||BUILDING_IN_TINYXML

#include "../LINKAGE.HPP" //#define LINKAGE

/**LEGACY, ABSTRACT INTERFACE 
 * This class exists only to serve as a base class for the common functionality
 * between the @c daeLibXMLPlugin and @c daeTinyXMLPlugin classes.
 */
class daeIOPluginCommon : public daeIOPlugin
{
COLLADA_(protected) //daeIOPlugin methods
	/**PURE-OVERRIDE */
	virtual daeOK addDoc(daeDocRef&,daeMeta*);
	/**PURE-OVERRIDE */
	virtual daeOK readContent(daeIO&,daeContents&);

COLLADA_(protected) //Visual Studio workaround

	/**Implements constructors/destructors. */
	LINKAGE void __xstruct(int x, int legacy=0);
	
COLLADA_(public) //These status APIs are new in 2.5.

	enum
	{
	_readFlag_unattributed_is_data_loss=1,
	_readFlag_unattributed=2,
	_readFlag_unordered=4,
	_readFlag_unmixed=8,
	};
	/**
	 * These flags are reset whenever @c readContent() is called.
	 * The library isn't designed to validate documents, but these
	 * LEGACY plugins provide a modicum of feedback.
	 */
	NOALIAS_LINKAGE int _getReadFlags()
	SNIPPET( return _readFlags; )
	/**TODO
	 * @return Returns @c true if a read element could not be placed
	 * into the metadata's content-model. 2.5 retains these elements
	 * as unnamed/unordered children. Pre-2.5 they had been discared.
	 * @todo Maybe an option can dictate what action to take?
	 */
	inline bool getReadUnorderedElement()
	{
		return (_getReadFlags()&_readFlag_unordered)!=0?true:false; 
	}
	/**WARNING, TODO
	 * @return Returns @c true if a read attribute could not be made.
	 * @warning Currently this is data-loss. As soon as <xs:anyAttribute>
	 * is supported by the library, it will be used to record attributes
	 * whether or not the schema specifies <xs:anyAttribute>.
	 * @todo Maybe an option can dictate what action to take?
	 */
	inline bool getReadUnattributableAttribute()
	{
		return (_getReadFlags()&_readFlag_unattributed)!=0?true:false; 
	}
	/**WARNING, TODO
	 * @return Returns @c true if a read attribute could not be made,
	 * -and the attribute had to be discarded.
	 * @c daeElement::xs_anyAttribute_is_still_not_implemented informs
	 * this.
	 */
	inline bool getReadAndLostUnattributableAttribute()
	{
		int f = _readFlag_unattributed_is_data_loss|_readFlag_unattributed;
		return (_getReadFlags()&f)==f?true:false; 
	}
	/**WARNING
	 * @return Returns @c true if a read simple content model value
	 * could not be set because of schema mismatch.
	 * The text SHOULD appear in the contents-array as if mixed-text.
	 * UPDATE: Now that mixed-text is implemented, it turns out that
	 * there are issues (around ambiguities) that make it unsane 
	 */
	inline bool getReadUnmixedCharData()
	{
		return (_getReadFlags()&_readFlag_unmixed)!=0?true:false; 
	}

	enum{ __combined_size_on_client_stack=64*sizeof(void*) }; //ie. libXML & TinyXML

#ifndef BUILDING_COLLADA_DOM

	char __client_padding[__combined_size_on_client_stack-sizeof(daeIOPlugin)];

#else

COLLADA_(protected)
		/**
		 * Default Constructor
		 */
		daeIOPluginCommon();
		/**
		 * Virtual Destructor
		 */
		virtual ~daeIOPluginCommon(){}
			
		//STATEFUL
		int _readFlags;	
		//STATELESS		
		//ctype.h has #define _X 0100. (GCC)
		//xlocinfo.h has #define _XD _HEX. (Win32; ctype related)
		daeArray<daeStringCP> _CD,_X_CD; //_X //_XD
		typedef std::pair<daeName,daeHashString> _attrPair;
		std::vector<_attrPair> _attribs;		
		//Only the LibXML loader uses these, but the reader must pass-through.
		daeHashString (*_encoder)(const daeHashString&,daeArray<daeStringCP>&);
		daeHashString (*_decoder)(daeArray<daeStringCP>&,daeArray<daeStringCP>&);
		inline daeHashString _encode(const daeHashString &i, daeArray<daeStringCP> &CD)
		{
			return _encoder==nullptr?i:_encoder(i,CD);
		}
		inline daeHashString _encode(const unsigned char *i, daeArray<daeStringCP> &CD)
		{
			return _encoder==nullptr?(daeString)i:_encoder((daeString)i,CD).string; 
		}
		inline const unsigned char *_decode(daeArray<daeStringCP> &CD, daeArray<daeStringCP> &X)
		{
			return (const unsigned char*)(_decoder==nullptr?CD.data():_decoder(CD,X).string);
		}				
		//NEW, OPTIMIZATION
		//Returns false if nullptr==_encoder.
		//Char-set recoding is an old feature request; this avoids it for ASCII types.
		bool _maybeExtendedASCII(daeCharData&);

		/**ABSTRACT INTERFACE 
		 * The two loaders implement reading via this method. Writing is independent.
		 */
		virtual bool _read(daeIO &IO, daeContents &content) = 0;
		/**ABSTRACT INTERFACE 
		 * TinyXML's version is untested. It may never work. It had not previously.
		 */
		virtual int _errorRow() = 0;

		/** This parses the XML declaration. */
		void _xml_decl(const daeContents&,daeString&,daeString&,daeString&);
		/** This initializs the XML declaration. Both require special handling. */
		void _push_back_xml_decl(daeContents&,daeName,daeName,bool);
		
		//LEGACY
		daeElement &_beginReadElement(daePseudoElement &parent, const daeName &elementName);
		//LEGACY
		void _readElementText(daeElement &element, const daeHashString &text);

#endif //BUILDING_COLLADA_DOM
};

#ifdef BUILDING_IN_LIBXML////////////////////
/**LEGACY
 * NOT-REALLY RECOMMENDED: DOCUMENTATION FOR LIBXML IS SPARSE/NEXT TO NONEXISTENT.
 * The @c daeLibXMLPlugin class derives from @c daeIOPlugin and implements an XML
 * input/output backend using libxml2 as a parser. 
 */
class daeLibXMLPlugin : public daeIOPluginCommon
{
COLLADA_(public) 
	/**
	 * Default Constructor
	 */
	daeLibXMLPlugin(int legacy=0)
	#ifdef BUILDING_COLLADA_DOM
	;
	#else //VS workaround; similar to daeDOM's.
	{
		daeIOPluginCommon::__xstruct(daePlatform::LEGACY_LIBXML,legacy);
	}
	virtual ~daeLibXMLPlugin() //dummy
	{
		daeIOPluginCommon::__xstruct(~daePlatform::LEGACY_LIBXML);
	}
	#endif

COLLADA_(protected) //daeIOPlugin methods
	/**PURE-OVERRIDE */
	virtual void getDemands(int &I, int &O)
	{
		I = Demands::string/*|Demands::CRT*/; O = 0; 
	}
	/**PURE-OVERRIDE */
	virtual daeOK writeContent(daeIO &IO, const daeContents &content);

COLLADA_(public) //FORMERLY set/getOption()	
	/**
	 * This is to encode to a system's narrow code-page. It was added long ago to
	 * support a generic 8-bit European character-set. Extended Latin. LibXML has
	 * a default implementation for that. Users can now provide their own encoder.
	 */
	typedef daeHashString (*Encoder)(const daeHashString&,daeArray<daeStringCP>&);
	/**
	 * This is expected to deode to UTF-8. It could do otherwise with some work.
	 * The encoding would need to match the XML-declaration as written.
	 * The input strings are assumed to be compatible with the paired @c Encoder.
	 */	
	typedef daeHashString (*Decoder)(daeArray<daeStringCP>&,daeArray<daeStringCP>&);
	/**
	 * This had been limited to Latin1. You can set this up yourself now.
	 * @param e must encode strings to the system-wide character set that is can
	 * be represented by @c daeString for XML-like parsing purposes. 
	 * @param d then decodes the system strings to the file system strings, that
	 * are historically UTF-8 strings. THIS COULD BE CHANGED, BUT THIS LOADER IS
	 * NOT MEANT TO BE IMPROVED UPON, BEYOND WHAT ARE BASIC FEATURES.
	 */
	LINKAGE void option_to_use_codec(Encoder e=nullptr, Decoder d=nullptr)
	SNIPPET( _encoder = e; _decoder = d; )
	/**LEGACY-SUPPORT
	 * This is the internal implementation that leverages the LIBXML APIs.
	 * @remarks This replaces @c daeDOM::setCharEncoding(). It can be made
	 * the default FOR DEFAULT PLUGINS only by @c daePlatform::setLegacy().  
	 */
	LINKAGE void option_to_use_codec_Latin1();
	
	/**LEGACY, NOT RECOMMENDED, COLLADA EXTENSION
	 * Set @c true to save float_array & int_array data to .raw binary files. 
	 * @c daeRawResolver converts the data back into domFloat_array elements upon load.
	 *
	 * @note The Raw Resolver effectively acts like a dynamic script, that manipulates
	 * the XML document on input, pulling in the RAW files. The reverse is facilitated
	 * by this plugin. Don't ask why the resolver does this, as opposed to the plugin! 
	 *
	 * @return Returns @c true if this feature remains supported.
	 */
	LINKAGE bool option_to_write_COLLADA_array_values_to_RAW_file_resource(bool o=true)
	SNIPPET( _saveRawFile = o; return true; )
	   
#ifdef BUILDING_COLLADA_DOM

COLLADA_(public)
		/**
		 * Virtual Destructor 
		 */
		virtual ~daeLibXMLPlugin();

COLLADA_(private) //daeIOPluginCommon methods

		/**PURE-OVERRIDE */
		virtual bool _read(daeIO &IO, daeContents &content);
		/**PURE-OVERRIDE */
		virtual int _errorRow();

COLLADA_(private)		

		//STATEFUL
		bool _saveRawFile;
		//STATELESS
		daeOK _OK;
		_xmlTextReader *_reader;
		_xmlTextWriter *_writer;
		
		std::string _raw;
		daeIO *_rawIO;		
		size_t _rawByteCount;
		
		struct Reader;
		int _readElement(daePseudoElement&);
		int _readContent2(daePseudoElement&);

		void _writeElement(const daeElement &element);
		void _writeAttribute(daeAttribute &attr, const daeElement &element);
		void _writeValue(const daeElement &element);
		void _writeContent2(const daeContents&);
		void _writeRawSource(const daeElement &source, const daeElement &array, const daeElement &technique_common);

#endif //BUILDING_COLLADA_DOM
};
#if defined(BUILDING_COLLADA_DOM)\
 && !defined(BUILDING_IN_TINYXML)
struct daeLegacyIOPlugins : daeLibXMLPlugin //INTERNAL
{
	daeLegacyIOPlugins(int legacy, const daeURI&):daeLibXMLPlugin(legacy)
	{
		assert(0!=(legacy&daePlatform::LEGACY_LIBXML)); 
	}
};
#endif //BUILDING_COLLADA_DOM && !BUILDING_IN_TINYXML
#endif //BUILDING_IN_LIBXML/////////////

#ifdef BUILDING_IN_TINYXML//////////////
/**WARNING, LEGACY
 * The @c daeTinyXMLPlugin class derives from @c daeIOPlugin and implements an XML
 * input/output backend using libxml2 as a parser. 
 * @warning TinyXML COPIES ALL OF THE DATA INTO TEMPORARY STRINGS. IT'S NOT A GOOD
 * LIBRARY FOR READING/WRITING. (WHICH IS WHAT IT'S USED FOR HERE.)
 */
class daeTinyXMLPlugin : public daeIOPluginCommon
{
COLLADA_(public)
	/**
	 * Default Constructor
	 */
	daeTinyXMLPlugin()
	#ifdef BUILDING_COLLADA_DOM
	;
	#else //VS workaround; similar to daeDOM's.
	{
		daeIOPluginCommon::__xstruct(0);
	}
	virtual ~daeTinyXMLPlugin() //dummy
	{
		daeIOPluginCommon::__xstruct(~0);
	}
	#endif

COLLADA_(protected) //daeIOPlugin methods
	/**PURE-OVERRIDE */
	virtual void getDemands(int &I, int &O)
	{
		I = /*Demands::unimplemented|Demands::CRT|*/Demands::string;
		O = /*Demands::unimplemented|Demands::CRT*/0;
	}
	/**PURE-OVERRIDE */
	virtual daeOK writeContent(daeIO &IO, const daeContents &content);	
	
#ifdef BUILDING_COLLADA_DOM

COLLADA_(public)
		/**
		 * Virtual Destructor 
		 */
		virtual ~daeTinyXMLPlugin(){ /*NOP*/ }

COLLADA_(private) //daeIOPluginCommon methods

		/**PURE-OVERRIDE */
		virtual bool _read(daeIO &IO, daeContents &content);
		/**PURE-OVERRIDE */
		virtual int _errorRow();

COLLADA_(private)		

		//STATELESS
		TiXmlBase *_err;

		struct Value; 
		template<class T>
		static T *setValue(T*, const daeHashString&);
		void _readElement(TiXmlElement*,daeElement&);
		void _readContent2(TiXmlNode*,daePseudoElement&);

		void _writeElement(TiXmlNode*,const daeElement&);
		void _writeContent2(TiXmlNode*,const daeContents&);

#endif //BUILDING_COLLADA_DOM
};
#ifdef BUILDING_COLLADA_DOM
#ifdef BUILDING_IN_LIBXML
struct daeLegacyIOPlugins //INTERNAL
{
	typedef daeLibXMLPlugin A;
	typedef daeTinyXMLPlugin B;
	char IO[sizeof(A)>sizeof(B)?sizeof(A):sizeof(B)];
	daeLegacyIOPlugins(int legacy, const daeURI&)
	{
		if(0!=(legacy&daePlatform::LEGACY_LIBXML)) new(IO)A(legacy); else new(IO)B;
	}
	operator daeIOPlugin&(){ return (daeIOPlugin&)*IO; }
	~daeLegacyIOPlugins(){ ((daeIOPlugin&)*IO).~daeIOPlugin(); }
};
#else
struct daeLegacyIOPlugins : daeTinyXMLPlugin //INTERNAL
{
	daeLegacyIOPlugins(int legacy, const daeURI&)
	{
		assert(0==(legacy&daePlatform::LEGACY_LIBXML)); 
	}
};
#endif //BUILDING_IN_LIBXML
#endif //BUILDING_COLLADA_DOM
#endif //BUILDING_IN_TINYXML/////////////

#ifndef COLLADA_DOM_OMIT_ZAE
#define __COLLADA_DOM__ZAE
/**ZAE
 * ZAE files are ZIP archives including DAE files and their images.
 * @c daeZAEPlugin creates a @c daeArchive with the index document
 * already opened so it may be returned.
 */
class daeZAEPlugin : public daeIOPlugin
{
COLLADA_(public) 
	/**
	 * @param I is for reading the index document indicated by the 
	 * document named manifest.xml containing a <dae_root> element.
	 * @param O is for writing the entire archive.
	 */
	LINKAGE daeZAEPlugin(daeIOPlugin *I=nullptr, daeIOPlugin *O=nullptr)
	SNIPPET( _I = I; _O = O; )

COLLADA_(protected) //daeIOPlugin methods
	/**PURE-OVERRIDE */
	virtual daeOK addDoc(daeDocRef&,daeMeta*);
	/**PURE-OVERRIDE */
	virtual daeOK readContent(daeIO&,daeContents&);				
	/**PURE-OVERRIDE */
	virtual daeOK writeContent(daeIO&,const daeContents&);

COLLADA_(private) 

	daeIOPlugin *_I,*_O;
};
#endif //COLLADA_DOM_OMIT_ZAE

#include "../LINKAGE.HPP" //#undef LINKAGE

#endif //BUILDING_IN_LIBXML||BUILDING_IN_TINYXML

//---.
}//<-'

#endif //__COLLADA_DOM__DAE_IO_PLUGIN_COMMON_H__
/*C1071*/
