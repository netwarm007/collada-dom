/*
 * Copyright 2006 Sony Computer Entertainment Inc.
 *
 * Licensed under the MIT Open Source License, for details please see license.txt or the website
 * http://www.opensource.org/licenses/mit-license.php
 *
 */

#ifndef __COLLADA_DOM__DAE_ERROR_HANDLER_H__
#define __COLLADA_DOM__DAE_ERROR_HANDLER_H__

#include "daeStringRef.h"

COLLADA_(namespace)
{//-.
//<-'

/**SHORTHAND
 * @c daeEH is a shorthand for @c daeErrorHandler added to make the established
 * workflow of getting the global singleton less of a mouthful.
 * @see @c daeErrorHandler::Error and friends and associated global << operator.
 */
typedef class daeErrorHandler daeEH;
/**
 * The @c daeErrorHandler class is a plugin that allows the use to overwrite how error and warning
 * messages get handled in the client application. An example of this would be a class that reports
 * the message to a gui front end instead of just printing on stdout.
 */
class daeErrorHandler
{
COLLADA_(public)
	/**
	 * Default Constructor & Virtual Destructor
	 */
	daeErrorHandler(){} virtual ~daeErrorHandler(){}

	/**ABSTRACT INTERFACE
	 * This function is called when there is an error and a string needs to be sent to the user.
	 * You must overwrite this function in your plugin.
	 * @param msg Error message.
	 */
	virtual void handleError(const daeHashString &msg, enum dae_clear=dae_clear) = 0;
	/**ABSTRACT INTERFACE
	 * This function is called when there is a warning and a string needs to be sent to the user.
	 * You must overwrite this function in your plugin.
	 * @param msg Warning message.
	 */
	virtual void handleWarning(const daeHashString &msg, enum dae_clear=dae_clear) = 0;
	
	COLLADA_DOM_LINKAGE
	/**
	 * Sets the daeErrorHandler to the one specified.
	 * @param eh The new daeErrorHandler to use. Passing in nullptr results in the default plugin being used.
	 * @see daeError.cpp
	 */
	static void setErrorHandler(daeErrorHandler *eh);

	COLLADA_NOALIAS
	COLLADA_DOM_LINKAGE
	/**
	 * Returns the current daeErrorHandlerPlugin. A program has one globally-accessible
	 * daeErrorHandler active at a time.
	 * @return The current daeErrorHandler.
	 * @see daeError.cpp
	 */
	static daeErrorHandler *get();

COLLADA_(public) //This is 2017 extension ahead of work on old COLLADA Viewer component.
	/**
	 * Post 2.5 these can be combined with global @c operator<<() to initiate an
	 * "error" stream. @c Verbose is used to simplify the old RT & FX components.
	 */
	enum Error{ Error=1 }; enum Warning{ Warning=2 }; enum Verbose{ Verbose=4 };

	template<void(daeEH::*H)(const daeHashString&,enum dae_clear)>
	/**HELPER
	 * This class is taken from a rewrite of the Assimp library that was in
	 * work prior to rewriting COLLADA-DOM. It's hastily adapted here so to
	 * simplify use of @c daeErrorHandler ahead of replacing the @c Print()
	 * function of the RT and FX components of the Viewer package with this
	 * class in order to eliminate the platform specific parts of those two
	 * static libraries.
	 * @tparam H is just an expedient way to pass either @c handleWarning()
	 * @c handleError() through the @c operator<<() chain.
	 */
	struct LogStream 
	{	
		daeErrorHandler *eh;		
		inline LogStream(daeErrorHandler *eh):eh(eh){}
		template<typename const_ptr>
		inline LogStream operator<<(const_ptr *p)
		{
			//HACK: _pointer exists only because the FX component has an
			//old log message or two that want %p to format CGparamter?!
			_pointer<daeConstOf<int,const_ptr>::type>::format(p,eh); return *this; 
		}
		template<class NON_PORTABLE_p_SPECIFIER> struct _pointer
		{ 
			template<class T> static void format(const T *t, daeErrorHandler *eh)
			{
				char msg[96]; //ASSUMING WON'T OVERUN.				
				COLLADA_SUPPRESS_C(4996)sprintf(msg,"%p",t); 
				(eh->*H)(msg,dae_append);
			}
			template<> static void format(const daeUStringCP *msg, daeErrorHandler *eh)
			{
				(eh->*H)(p,dae_append); //xmlChar*???
			}
		};
		template<> struct _pointer<daeStringCP>
		{
			static void format(daeString msg, daeErrorHandler *eh)
			{
				(eh->*H)(msg,dae_append);
			}
		};
		template<class T> 
		inline LogStream operator<<(const T &t)
		{ 
			//HACK: _formatter filters out classes. Everything else is a
			//number. C-array strings are converted to pointers. It'd be
			//nice to convert them to daeHashString, but there's no rush.
			_formatter<daeArrayAware<T>::is_class>::format(t,eh); return *this;
		}		
		template<bool> struct _formatter
		{ 
			template<class T> static void format(const T &t, daeErrorHandler *eh)
			{
				(eh->*H)(t,dae_append);
			}
		};
		template<> struct _formatter<false>
		{ 
			template<class T> static void format(const T &t, daeErrorHandler *eh)
			{
				char msg[64]; //ASSUMING WON'T OVERUN.
				if(t==(int)t) //long long can be used with the correct printf flags?
				COLLADA_SUPPRESS_C(4996)sprintf(msg,"%d",(int)t); else 
				COLLADA_SUPPRESS_C(4996)sprintf(msg,"%f",(double)t); 
				(eh->*H)(msg,dae_append);
			}
		};
	};
	template<void(daeEH::*H)(const daeHashString&,enum dae_clear)>
	/**
	 * @c LogStream2 is designed to wait until all of the << insertion operators
	 * have left scope. It being the last, it automatically clears and completes
	 * the line of output.
	 */
	struct LogStream2 : LogStream<H>
	{
		LogStream2(const LogStream<H> &cp):LogStream<H>(cp){}
		~LogStream2(){ (LogStream<H>::eh->*H)("\n",dae_clear); }
	};					
}; 
template<class T> 
/**OPERATOR
 * This is used like so: @c daeErrorHandler::Error<<"message"<<1234<<"etc.";
 * The inputs must be a string or string like class or a non-class number or a 
 * class-wrapped number prefixed with the unary + operator.
 */
inline daeEH::LogStream2<&daeEH::handleError> operator<<(enum daeEH::Error, const T &t)
{
	return daeEH::LogStream<&daeEH::handleError>(daeEH::get()) << t; 
}	
template<class T> 
/**OPERATOR
 * This is used like so: @c daeErrorHandler::Warning<<"message"<<1234<<"etc.";
 * The inputs must be a string or string like class or a non-class number or a 
 * class-wrapped number prefixed with the unary + operator.
 */
inline daeEH::LogStream2<&daeEH::handleWarning> operator<<(enum daeEH::Warning, const T &t)
{
	return daeEH::LogStream<&daeEH::handleWarning>(daeEH::get()) << t; 
}
template<class T> 
/**OPERATOR
 * This is a HACK to implement the old RT and FX component's logging function
 * without adding more modes of ouptut to @c daeErrorHandler. The whole thing
 * is going to eventually need to be redesigned. 
 */
inline daeEH::LogStream2<&daeEH::handleWarning> operator<<(enum daeEH::Verbose, const T &t)
{
	//HACK: This suppresses the Warning: label of daeStandardErrorHandler.
	//It puts it in append mode, and it will avoid labeling an empty line. 
	daeEH *eh = daeEH::get(); eh->handleWarning(nullptr,dae_append); 
	return daeEH::LogStream<&daeEH::handleWarning>(eh) << t; 
}

/**WARNING
 * @warning Pre-2.5 this was writing to @c stdout (via @c printf) unflushed.
 * There was no explanation as to why, so @c handleError() has been made to
 * write to @c stderr as the name of the class suggests. (Originally it was
 * called @c stdErrPlugin; 2.5 deprecates the name as it's not a plugin and
 * its construction is somewhat awkward and not clear where it comes from.)
 *
 * The @c daeStandardErrorHandler class implements @c daeErrorHandler. 
 * Historically it routes the error and warning messages to @c stdout; but
 * as the warning says, 2.5 changes errors to @c stderr. This decision is
 * not irreversible. Applications shouldn't use the default regardless, as
 * it's impossible to know what they'll get if they do.
 */
class daeStandardErrorHandler : public daeErrorHandler
{
COLLADA_(protected)

	bool _clear;
	template<int errN>
	inline void _print(FILE *f, 
	const daeStringCP (&err)[errN], const daeHashString &msg, bool clear)
	{
		//Note: this is made complicated by trying to support the
		//older style of output while also implementing LogStream.
		if(!msg.empty())
		{
			if(_clear) fwrite(err,errN-1,1,f);
			fwrite(msg.string,msg.extent*sizeof(daeStringCP),1,f);
		}
		if(_clear=clear)
		{
			if(msg.empty()||msg.string[msg.extent-1]!='\n') 
			fputc('\n',f);
			fflush(f);
		}
	}

COLLADA_(public)

	daeStandardErrorHandler():_clear(true){} 
	
	virtual ~daeStandardErrorHandler(){}

COLLADA_(public)

	/**PURE-OVERRIDE */
	virtual void handleError(const daeHashString &msg, enum dae_clear clear)
	{	
		_print(stderr,"Error: ",msg,clear==dae_clear); //C4800
	}
	
	/**PURE-OVERRIDE */
	virtual void handleWarning(const daeHashString &msg, enum dae_clear clear)
	{
		_print(stdout,"Warning: ",msg,clear==dae_clear); //C4800
	}

	static daeStandardErrorHandler &getInstance()
	{
		static daeStandardErrorHandler perTranslationUnitInstance;
		return perTranslationUnitInstance; //theInstance; 
	}
};

/**
 * The @c daeQuietErrorHandler class is an alternative implementation of 
 * @c daeErrorHandler. It suppresses error and warning messages. For eample:
 * @c daeErrorHandler::setErrorHandler(&daeQuietErrorHandler::getInstance());
 */
class daeQuietErrorHandler : public daeErrorHandler
{
COLLADA_(public)

	daeQuietErrorHandler(){}

	/**PURE-OVERRIDE */
	virtual void handleError(const daeHashString&,enum dae_clear){}

	/**PURE-OVERRIDE */
	virtual void handleWarning(const daeHashString&,enum dae_clear){}

	static daeQuietErrorHandler &getInstance()
	{
		static daeQuietErrorHandler perTranslationUnitInstance;
		return perTranslationUnitInstance; //theInstance; 
	}

COLLADA_(private) //not worth having its on CPP file

	//static daeQuietErrorHandler theInstance;
};

//---.
}//<-'

#endif //__COLLADA_DOM__DAE_ERROR_HANDLER_H__
/*C1071*/