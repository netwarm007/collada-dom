/*
 * Copyright 2006 Sony Computer Entertainment Inc.
 *
 * Licensed under the MIT Open Source License, for details please see license.txt or the website
 * http://www.opensource.org/licenses/mit-license.php
 *
 */

#ifndef __COLLADA_DOM__DAE_ERROR_H__
#define __COLLADA_DOM__DAE_ERROR_H__

#include "daePlatform.h"

COLLADA_(namespace)
{
	//Pre-2.5 these were macros.
	enum daeError 
	{
  ///////////////////////////////////
  //STOP! Before adding, update daeErors.cpp's friendly message table.
  ///////////////////////////////////
	/** Success */
	DAE_OK=0, 
	/** Not failure. Thread is blocking. The library never sleeps. */
	DAE_NOT_NOW=+1,
	/** Not failure. @c XS::All can emit this on success. */
	DAE_ORDER_IS_NOT_PRESERVED=+2,
	/** Fatal Error, should never be returned unless there is a bug in the library. */
	DAE_ERR_FATAL=-1,
	/** Call invalid, the combination of parameters given is invalid. */
	DAE_ERR_INVALID_CALL=-2,
	/** Generic error */
	DAE_ERROR=-3,
	/** 2.5: Like DAE_ERR_FATAL except the onus is on the client/databinding side. */
	DAE_ERR_CLIENT_FATAL=-4,
	/** ie. two things with the same name cannot coexist. */
	DAE_ERR_NAME_CLASH=-5,
	/** IO error, the file hasn't been found or there is a problem with the IO plugin. */
	DAE_ERR_BACKEND_IO=-100,
	/** The IOPlugin backend wasn't able to successfully validate the data. */
	DAE_ERR_BACKEND_VALIDATION=-101,
	/** The IOPlugin tried to write to a file that already exists and the "replace" parameter was set to false */
	DAE_ERR_BACKEND_FILE_EXISTS=-102,
	/** Error in the syntax of the query. */
	DAE_ERR_QUERY_SYNTAX=-200,
	/** No match to the search criteria. */
	DAE_ERR_QUERY_NO_MATCH=-201,
	/** A document with that name already exists. */
	DAE_ERR_DOCUMENT_ALREADY_EXISTS=-202,
	/** A document with that name does not exist. */
	DAE_ERR_DOCUMENT_DOES_NOT_EXIST=-203,
	/** Function is not implemented. */
	DAE_ERR_NOT_IMPLEMENTED=-1000,
  ///////////////////////////////////
  //STOP! Before adding, update daeErors.cpp's friendly message table.
  ///////////////////////////////////
	};

	/**
	 * Replacement for DAE_OK like error codes.
	 *
	 * Fix for legacy APIs that returned @c bool. 
	 *
	 * @note @c daeOK is correct for old bool-based
	 * APIs, but not for old "daeInt" based APIs, that
	 * returned DAE_OK like error codes. The older codes
	 * are now @c enum, but the new style returns @c daeOK
	 * because it's shorter, and converts to bool, if:
	 *
	 * #define COLLADA_NOLEGACY to enable bool conversions.
	 * BUT FIRST CODE MUST COMPILE WITHOUT @c daeOK ERRORS.
	 * @see the @c daeOK::operator @c bool() Doxygentation.
	 */
	struct daeOK
	{
		daeError error;

		daeOK():error(DAE_OK){}

		daeOK(daeError Error):error(Error){}
		
		daeOK(const daeOK &OK):error(OK.error){}

		daeOK(const void *e):error(e!=nullptr?DAE_OK:DAE_ERROR){}

		inline operator daeError&(){ return error; }

	//operator bool() cannot be used in headers.
	#ifndef BUILDING_COLLADA_DOM
	#ifndef COLLADA_NOLEGACY
	COLLADA_(private)
	#endif
	#endif

		//TODO: const form may be required.
		/**WARNING, LEGACY-MIGRATION-SUPPORT
		 * #define COLLADA_NOLEGACY to enable @c bool conversions. 
		 * @warning	DO NOT DO THIS IF ANY LEGACY CODE IS
		 * FAILING TO COMPILE BECAUSE OF THIS CONVERSION.
		 * ONLY AFTER OFFENDING CODE IS REMOVED CAN THIS
		 * BE USED, AS @c !DAE_OK!=daeOK().
		 */
		inline operator bool(){ return error==DAE_OK; }	
	};
					
	COLLADA_NOALIAS 
	COLLADA_DOM_LINKAGE
	/** Gets the ASCII error string.  
	 * @param errorCode Error code returned by a function of the API.
	 * @return Returns an English string describing the error.
	 * LINKAGE*/
	const char *daeErrorString(daeError errorCode);

} //COLLADA

#endif //__COLLADA_DOM__DAE_ERROR_H__
/*C1071*/