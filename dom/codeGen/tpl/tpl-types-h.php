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

#ifndef __$1_TYPES_H__
#define __$1_TYPES_H__

#include <$2/$2DomTypes.h>
",strtoupper($prefix),$meta_prefix);

//BASIC TYPES
foreach($typemeta as $type=>$meta)
if(empty($meta['enum'])&&!$meta['isComplex'])
if(!empty($meta['base'])) //has a base type
{ 
	//HACK? filter out new generateXMLSchemaTypes
	if(strpos($type,':')!==false) continue; 
	
	echoDoxygen(@$meta['documentation']);

	echo 'typedef ', //SCHEDULED OBSOLETE
	//special casing urifragment to be a xsURI for automatic resolution
	$meta['isString']&&isDreadedAnyURI($type)?'xsAnyURI':getFriendlyType($meta['base']),
	' ', getFriendlyType($type), ";\n";			
}
else if(!empty($meta['listType'])) //is a list type
{ 
	//HACK? filter out new generateXMLSchemaTypes
	if(strpos($type,':')!==false) continue; 
	
	echoDoxygen(@$meta['documentation']);

	$preType = getFriendlyType($type);
	if(preg_match("/xs\:/",$meta['listType']))
	echo 'typedef xs', ucfirst(substr($meta['listType'],3)), 'Array ', $preType, ";\n";	
	else echo "typedef {$meta_prefix}TArray<", getFriendlyType($meta['listType']), '> ', $preType, ";\n";
}
echo "\n";

//ENUMS
foreach($typemeta as $type=>$meta) if(!empty($meta['enum']))
{
	if(!$meta['useConstStrings'])
	{
		echoDoxygen(@$meta['documentation']);
		//NEW: convert to _enum ending if _type
		$type = asFriendlyType($type); 
		echo "enum ", $prefix.ucfirst($type), "\n{\n";
		$type = strtoupper(asFriendlyEnum($type));
		foreach($meta['enum'] as $enum=>$ea)
		{
			echo "\t", $type, "_", strtr($enum,'.-','__');
			if(!empty($ea['value'])) echo ' = ', $ea['value'];		
			echo ",";
			if(!empty($ea['documentation']))
			echo ' /**< ', getDocumentationText($ea['documentation']), ' */';
			echo "\n";
		}
		echo "\t", $type, "_COUNT = ", count($meta['enum']);
		echo "\n};\n\n";
	}
	else
	{
		$type = strtoupper(asFriendlyEnum($type));
		foreach($meta['enum'] as $enum=>$ea)
		{
			if(!empty($ea['documentation']))				
			$constStrings[] = "/**\n * ".getDocumentationText($ea['documentation'])."\n */\n";
			$constStrings[$type."_".strtr($enum,'.-','__')] = "\"".$enum."\";\n";
		}
		$constStrings[] = "\n";
	}
}

//UNIONS
//CAREFUL! & IS REQUIRED AS LONG AS: $meta['enum'][] = 
//(NOTE THIS IS OVERWRITING WHAT IS CONCEPTUALLY CONST)
foreach($typemeta as $type=>& $meta) if($meta['union_type'])
{
	if(!$meta['useConstStrings']) //NEW: assuming desirable
	{
		echoDoxygen(@$meta['documentation']);

		//unions end in _type, and it must be stripped off
		//in 1.4 some enums end in _type inexplicably, so...
		//NEW: convert to _enum ending if _type
		$type = asFriendlyType($type); 
		echo "enum ", $prefix.ucfirst($type)." //union\n{\n";
		$type = strtoupper(asFriendlyEnum($type)); 

		//look up the members
		//tokenize memberTypes string
		$types = explode(' ',$meta['union_members']);		
		$count = 1; //RESERVING 0?
		foreach($types as $ea)
		if(!empty($typemeta[$ea]))					
		foreach($typemeta[$ea]['enum'] as $enum=>$ea2)
		{
			//THIS APPEARS TO PREVENT DOUBLE ENTRY, BUT
			//IS ALSO BEING REUSED IN tpl-types.cpp.php
			$destructive_write =& $meta['enum'][$enum];
			if(!empty($destructive_write)) continue;
			else $destructive_write = $ea2; //EVIL
			echo "\t", $type, "_", strtr($enum,'.-','__');
			if(!empty($ea2['value'])) echo " = ", $ea2['value'];			
			echo ",";
			if(!empty($ea2['documentation']))
			echo ' /**< ', getDocumentationText($ea2['documentation']), ' */';			
			echo "\n"; $count++;
		}
		echo "\t", $type, "_COUNT = ", $count;
		echo "\n};\n\n";
	}
	else die("die(it seems as if something analagous to ENUMS should be done here)");
}
echo "//Element Type Enum
namespace {$namespace}_TYPE //deprecated
{
	const int
	NO_TYPE = 0,
	ANY = 1";
foreach($elementTypes as $i=>$ea) echo ",\n\t", $ea, " = ", $i+2;
echo ";
}

";
echoCode("
//Returns the total number of schema types/$1* classes
$2Int DLLSPEC $3TypeCount();

#endif
",$prefix,$meta_prefix,strtolower($namespace));
