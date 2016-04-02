<?php
/*
 * Copyright 2006 Sony Computer Entertainment Inc.
 *
 * Licensed under the MIT Open Source License, for details please see license.txt or the website
 * http://www.opensource.org/licenses/mit-license.php
 *
 */

echoCode("
$copyright

#ifndef __$1_ELEMENTS_H__
#define __$1_ELEMENTS_H__

#include <$2/domAny.h>
#include <$3/$3Types.h>
",strtoupper($prefix),$meta_prefix,$prefix);

if(!empty($import_list))
{
	echo "
//BEGIN IMPORTED ELEMENTS

";	$ns = $namespace.'_IMPORT_';
	foreach($import_list as $el=>$ea)
	{
		$Type = $el; 
		$colon = strpos($el,':');
		$Name = substr($Type,$colon+1);
		getFriendlyTypeAndName($Type,$Name);		
		$macro = ucfirst(substr($Type,0,$colon));
		echoCode("
#ifndef $ns$macro
typedef domAny $Type;
#else
typedef $ns$macro($2,$3) $Type;
#endif
typedef $1SmartRef<$Type> {$Type}Ref;
typedef $1TArray<{$Type}Ref> {$Type}_Array;
",$meta_prefix,ucfirst($Name),lcfirst($Name));
	}
	
	echo "
//BEGIN LOCALLY SOURCED ELEMENTS

";
}

//global elements
foreach($classmeta as $name=>$ea)
{
	//hack: should come out okay
	$Type = $name; $Name = $name;
	getFriendlyTypeAndName($Type,$Name);
	EchoCode("
class $1$Name;
typedef $2SmartRef<$1$Name> {$Type}Ref;
typedef $2TArray<{$Type}Ref> {$Type}_Array;
",$prefix,$meta_prefix);
}

global $COLLADA_DOM_GENERATION; 
echo "
//ARBITRARILY INCLUDED TEST	
#ifdef COLLADA_DOM_GENERATION
#if COLLADA_DOM_GENERATION != $COLLADA_DOM_GENERATION
#error COLLADA_DOM_GENERATION != $COLLADA_DOM_GENERATION
#endif
#endif
#endif //__", strtoupper($prefix), "_ELEMENTS_H__";

