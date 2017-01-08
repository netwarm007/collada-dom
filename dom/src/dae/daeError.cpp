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

#define _(x,y) { ##x, #x "  " y },	
static struct
{
	daeError errCode; const char *errString;

}errorsArray[] = 
{
_(DAE_OK,"Success")	
_(DAE_NOT_NOW,"Scheduling conflict")
_(DAE_ORDER_IS_NOT_PRESERVED,"The child was added; but not where you think.")
_(DAE_ERR_FATAL,"Fatal")
_(DAE_ERR_INVALID_CALL,"Invalid function call")	
_(DAE_ERROR,"Generic error")
_(DAE_ERR_CLIENT_FATAL, "Fatal: due to client-side databinding issues with generated headers etc.")
_(DAE_ERR_NAME_CLASH,"This name is in use. There can only be one.")
_(DAE_ERR_BACKEND_IO,"Backend I/O")
_(DAE_ERR_BACKEND_VALIDATION,"Backend validation")
_(DAE_ERR_BACKEND_FILE_EXISTS,"Write request failed, because file exists.")
_(DAE_ERR_QUERY_SYNTAX,"Error in the syntax of the query.")
_(DAE_ERR_QUERY_NO_MATCH,"No match to the search criteria.")
_(DAE_ERR_DOCUMENT_ALREADY_EXISTS,"A document with the same name exists already.")
_(DAE_ERR_DOCUMENT_DOES_NOT_EXIST,"No document is loaded with that name or index.")
_(DAE_ERR_NOT_IMPLEMENTED,"An interface or one or more parameters is unimplemented.")	
};
#undef _  
const char *daeErrorString(daeError errorCode)
{
	for(size_t i=0;i<sizeof(errorsArray)/sizeof(errorsArray[0]);i++)	
	if(errorsArray[i].errCode==errorCode) 
	return errorsArray[i].errString; return "Unknown Error code";
}
 
//// old daeErrorHandler.cpp definitions //////////////////

static daeErrorHandler *daeErrorHandler_instance = nullptr;

void daeErrorHandler::setErrorHandler(daeErrorHandler *eh)
{
	daeErrorHandler_instance = eh;
}

daeErrorHandler *daeErrorHandler::get()
{
	if(nullptr==daeErrorHandler_instance) return &daeStandardErrorHandler::getInstance();	
	else return daeErrorHandler_instance;
}

//---.
}//<-'

/*C1071*/