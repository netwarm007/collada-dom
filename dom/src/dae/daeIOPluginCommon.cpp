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

void daeIOPluginCommon::__xstruct(int x, int legacy)
{			  
	switch(x) //Visual Studio workaround
	{
	#ifdef BUILDING_IN_LIBXML
	case daePlatform::LEGACY_LIBXML:
	new(this) daeLibXMLPlugin(legacy); return;
	case ~daePlatform::LEGACY_LIBXML:
	((daeLibXMLPlugin*)this)->daeLibXMLPlugin::~daeLibXMLPlugin(); return;
	#endif
	#ifdef BUILDING_IN_TINYXML
	case 0: new(this) daeTinyXMLPlugin; return;
	case ~0:
	((daeTinyXMLPlugin*)this)->daeTinyXMLPlugin::~daeTinyXMLPlugin(); return;
	#endif
	}
	assert(0); //Are libraries mismatched?
}

daeIOPluginCommon::daeIOPluginCommon()
:_encoder(),_decoder(),_readFlags()
{
	//1/3 is to give US a heads up
	daeCTC<(__combined_size_on_client_stack*1/3>=sizeof(*this))>();
}

daeOK daeIOPluginCommon::addDoc(daeDOM &DOM, daeDocRef &readDoc)
{
	readDoc = daeDocumentRef(DOM); if(readDoc!=nullptr) return DAE_OK;

	assert(readDoc!=nullptr); return DAE_ERR_BACKEND_IO; //Unexpected.
}

daeOK daeIOPluginCommon::readContent(daeIO &IO, daeContents &content)
{
	_readFlags = daeElement::xs_anyAttribute_is_still_not_implemented;

	bool OK = _read(IO,content);

	/*REFERENCE: THIS IS NO LONGER RELEVANT
	//#defined in daeZAEUncompressHandler.h
	#ifdef __COLLADA_DOM__DAE_ZAE_UNCOMPRESS_HANDLER_H__
	{
		bool zaeRoot = false;
		string extractedURI = ""; if(!OK)
		{
			daeZAEUncompressHandler zaeHandler(fileURI);
			if(zaeHandler.isZipFile())
			{
				string rootFilePath = zaeHandler.obtainRootFilePath();
				daeURI rootFileURI(*fileURI.getDOM(),cdom::nativePathToUri(rootFilePath));
				if(OK=readFromFile(rootFileURI))
				{
					zaeRoot = true;	extractedURI = rootFileURI.str();
				}
				
			}
		}
	}
	#else //__COLLADA_DOM__DAE_ZAE_UNCOMPRESS_HANDLER_H__
	{
		#if defined(BUILDING_IN_LIBXML) && defined(BUILDING_IN_MINIZIP)
		#error unexpected: #ifndef __COLLADA_DOM__DAE_ZAE_UNCOMPRESS_HANDLER_H__
		#endif
	}
	#endif //COLLADA_daeZAEUncompressHandler*/
	
	if(!OK)
	{
		const daeURI *URI = getRequest().remoteURI;
		daeEH::Error<<"Failed to load "<<(URI!=nullptr?URI->data():"XML document from memory");
		return DAE_ERR_BACKEND_IO;
	}	
	return DAE_OK;
}

daeElement &daeIOPluginCommon::_beginReadElement(daePseudoElement &parent, const daeName &elementName)
{
	const daeChildRef<> &child = parent.getMeta().pushBackWRT(&parent,elementName);
	//This is sub-optimal.
	if(0==child.ordinal())
	{
		_readFlags|=_readFlag_unordered;

		daeEH::Warning<<
		"Appended an unordered element named "<<
		elementName<<" at line "<<_errorRow()<<".\n"
		"(Could be a schema violation OR ordinal-space overflow.)";
	}

	//Process the attributes
	for(size_t i=0;i<_attribs.size();i++)
	{
		daeName &name = _attribs[i].first, value = _attribs[i].second;

		//NEW: determine if extended ASCII.
		//(Maybe attributes are so short that this is self-defeating??)
		daeAttribute *attr = child->getAttributeObject(name);
		if(attr==nullptr) goto unattributed;
		//This an old feature request.		
		//It isn't said why the document isn't just written with a Latin declaration.
		//NOTE THAT MAKING THE NAMES LATIN WOULDN'T WORK IF THE METADATA ISN'T LATIN.
		if(_maybeExtendedASCII(*attr))
		{
			value = _encoder(value,_CD);
		}
		if(!attr->stringToMemoryWRT(child,value)) unattributed:
		{
			_readFlags|=_readFlag_unattributed;
			daeCTC<daeElement::xs_anyAttribute_is_still_not_implemented>();
			daeEH::Error<<"DATA LOSS\n"
			"Could not create an attribute "<<name<<"=\""<<value<<"\""
			" at line "<<_errorRow()<<".\n"
			"(Could be a schema violation.)\n"
			"UNFORTUNATELY THERE ISN'T A SYSTEM IN PLACE TO PRESERVE THE ATTRIBUTE.";
		}
	}
	_attribs.clear(); return *child;
}

void daeIOPluginCommon::_readElementText(daeElement &element, const daeHashString &textIn)
{
	daeHashString text = textIn;
	daeCharData *CD = element.getCharDataObject();
	if(CD!=nullptr)
	{
		if(_maybeExtendedASCII(*CD))
		{
			text = _encoder(text,_CD);
		}
		#ifdef NDEBUG
		#error And comment/PI peppered text?
		#endif
		if(CD->stringToMemoryWRT(&element,text))
		return;

		//It's tempting to default to mixed text, but
		//for A) there's not parity with attributes.
		//(Maybe add an xs:anyAttribute?)
		//And B) if later set, it's impossible to say
		//if it's a default or unset or what.
	}
	else //NEW: assuming mixed-text/author knows best.
	{
		element.getContents().insert<' '>(_encode(text,_CD));
		return;
	}

	_readFlags|=_readFlag_unmixed;

	daeEH::Warning<<
	"The DOM was unable to set a value for element of type "<<
	element.getTypeName()<<" at line "<<_errorRow()<<".\n"
	"Probably a schema violation.\n"
	"(2.5 policy is to add the value as a mixed-text text-node.)";
}

bool daeIOPluginCommon::_maybeExtendedASCII(const daeValue &v)
{	
	if(nullptr!=daeIOPluginCommon::_encoder)
	{
		switch(v.getType()->per<daeAtom>().getAtomicType())
		{
		case daeAtomicType::ENUMERATION:

			//An enum is probably a short string.
			//With getSimpleType something like the following
			//could be done. But an enum is very likely to be an xs:string.
			//return _maybeExtendedASCII(st.getRestriction().getBase());

		case daeAtomicType::EXTENSION: //Could be anything?
		case daeAtomicType::STRING: case daeAtomicType::TOKEN: return true;
		}
	}
	return false;
}

void daeIOPluginCommon::_push_back_xml_decl(daeContents &content, daeName version, daeName encoding, bool standalone)
{
	//The built-in plugins only output UTF-8. 
	//They are old/unrecommended/unworth time investments.
	//This is to let users know (in a test environment) they're recoding.
	assert("UTF-8"==encoding||"utf-8"==encoding||encoding.empty());
	encoding = "UTF-8"; 

	_CD.assign(daeName("xml version=\"")).append(version).push_back('"');
	if(!encoding.empty())
	_CD.append(daeName(" encoding=\"")).append(encoding).push_back('"');
	if(standalone)
	_CD.append(daeName(" standalone=\"yes\"")); content.insert<'?'>(_CD,0);
}
void daeIOPluginCommon::_xml_decl(const daeContents &content, char* &version, char* &encoding, char* &standalone)
{
	version = "1.0"; encoding = "UTF-8"; standalone = "";	
	if(!content.data()->hasText()) 
	return;
	daeText &xml_decl = content[0].getKnownStartOfText();
	if(!xml_decl.isPI_like()
	 ||"xml "!=daeName(xml_decl.data(),4))
	return;
	xml_decl.getText(_CD);
	for(size_t i=0;i<_CD.size();i++) if(_CD[i]=='"'&&_CD[i-1]!='=')
	{
		_CD[i] = '\0';
		size_t j = i-1; while(j>0&&_CD[j]!='=') j--;
		if(j>=11&&_CD[j+1]=='"')
		{
			char *value = &_CD[j+2]; value[-2] = '\0';
			#define _(x) \
			if(0==strcmp(value-1-sizeof(#x),#x)) x = value; else assert(0); break;
			switch(value[-3])
			{
			case 'n': _(version) case 'g': _(encoding) case 'e': _(standalone) break;
			default: assert(0);
			}
			#undef _
		}
		else assert(0);
	}
}

//---.
}//<-'

/*C1071*/