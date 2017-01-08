/*
 * Copyright 2006 Sony Computer Entertainment Inc.
 *
 * Licensed under the MIT Open Source License, for details please see license.txt or the website
 * http://www.opensource.org/licenses/mit-license.php
 *
 */

#ifndef __COLLADA_DOM__DAE_ERROR_HANDLER_H__
#define __COLLADA_DOM__DAE_ERROR_HANDLER_H__

#include "daeTypes.h"

COLLADA_(namespace)
{//-.
//<-'

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
	virtual void handleError(daeBoundaryStringIn msg) = 0;
	/**ABSTRACT INTERFACE
	 * This function is called when there is a warning and a string needs to be sent to the user.
	 * You must overwrite this function in your plugin.
	 * @param msg Warning message.
	 */
	virtual void handleWarning(daeBoundaryStringIn msg) = 0;
	
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
};

/**
 * The @c daeStandardErrorHandler class is the default implementation of daeErrorHandler. It routes the Error
 * and Warning messaged to stdout.
 */
class daeStandardErrorHandler : public daeErrorHandler
{
COLLADA_(public)

	daeStandardErrorHandler(){} 
	
	virtual ~daeStandardErrorHandler(){}

COLLADA_(public)

	/**PURE-OVERRIDE */
	virtual void handleError(daeBoundaryStringIn msg)
	{		
		//fprintf( stderr, "Error: %s\n", msg );
		//fflush( stderr );
		printf("Error: %s\n",msg.c_str);
	}
	
	/**PURE-OVERRIDE */
	virtual void handleWarning(daeBoundaryStringIn msg)
	{
		//fprintf( stderr, "Warning: %s\n", msg );
		//fflush( stderr );
		printf("Warning: %s\n",msg.c_str);
	}

	static daeStandardErrorHandler &getInstance()
	{
		static daeStandardErrorHandler perTranslationUnitInstance;
		return perTranslationUnitInstance; //theInstance; 
	}
};

/**
 * The @c daeQuietErrorHandler class is an alternative implementation of daeErrorHandler. It suppresses
 * error and warning messages. The easiest way to use it is like this:
 *   daeErrorHandler::setErrorHandler(&daeQuietErrorHandler::getInstance());
 */
class daeQuietErrorHandler : public daeErrorHandler
{
COLLADA_(public)

	daeQuietErrorHandler(){}

	/**PURE-OVERRIDE */
	virtual void handleError(daeBoundaryStringIn msg){}

	/**PURE-OVERRIDE */
	virtual void handleWarning(daeBoundaryStringIn msg){}

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