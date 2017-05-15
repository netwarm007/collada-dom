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

#ifndef __$1_TYPES_H__$2
#define __$1_TYPES_H__$2

//This seems to help the likes of IntelliSense.
#include <ColladaDOM.inl>

COLLADA_H(__FILE__)",strtoupper($prefix),$include_guard);

//This is sketchy. A lot's changed since it was placed here.
echo "//EXPERIMENTAL/SKETCHY
//This should be a pointer that does not change
//It's used to identify the calling module, and should
//be about the module. The text can change, but not the pointer.
//In addition the namespace can be queried and customized if necessary.
extern ::COLLADA::daeClientString ColladaAgent(::COLLADA::XS::Schema*);
";

echoCode("
COLLADA_(namespace)
{
	//The generator will do:
	//COLLADA_($1,namespace) 
	//The client must have done:
	//#define COLLADA__$1__namespace \\
	//COLLADA_DOM_NICKNAME(TinyName,$1)
	//Or:
	//namespace $1
    COLLADA_($1,namespace)
    {//-.
//<-----'
namespace __namespace__ = $1;

union __NB__ //\"NOTEBOOK\"
{",$target_namespace);
echoNotebookCPP();
echoCode("
};");

//BASIC TYPES
foreach($typemeta as $type=>$meta)
if(!empty($meta['isArray'])) //is a list type
{ 
	//HACK? filter out new generateXMLSchemaTypes
	if(strpos($type,':')!==false) continue; 
	
	echo "\n";
	echoDoxygen($meta['documentation']);	
	$preType = getFriendlyType($type);
	$r = $meta['restrictions'];	
	if(!empty($meta['itemType']))
	{
		if(!empty($r))
		die('die(Unexpected <xs:list> has <xs:restriction>.)');
		//This was added to convert daeString to daeStringRef.
		echoCode("
typedef DAEP::Container<$1>::type $preType;",
		getFriendlyType($meta['itemType']),getAlias(strtr($type,'.-','__')));
	}
	else //NEW: set daeArray parameter for xs:length scenario only
	{	
		$baseType = getFriendlyType($meta['base']);
		echoCode("
typedef daeArray<$baseType::value_type$1> $preType;",
		empty($r)||$r['minLength']!=$r['maxLength']?'':','.$r['minLength']); 
	}
}
else if(empty($meta['enum'])&&!empty($meta['base'])) //has a base type
{ 
	//HACK? filter out new generateXMLSchemaTypes
	//if(strpos($type,':')!==false) continue; 
	
	echo "\n";
	echoDoxygen($meta['documentation']);	
	echoCode("
typedef DAEP::Cognate<$1,__NB__::$2>::type $3;",
	getFriendlyType($meta['base']),getAlias(strtr($type,'.-','__')),getFriendlyType($type));
}
echo "\n";

//ENUMS
foreach($typemeta as $type=>$meta) if(!empty($meta['enum']))
{	
	if(!$meta['useConstStrings'])
	{
		echoDoxygen($meta['documentation']);
		$union = !empty($meta['union_type']);
		if(2==$COLLADA_DOM)
		{
			if($union) $union = ' //union';
			//NEW: convert to _enum ending if _type
			$type = asFriendlyType($type); 
			echo "enum ", $prefix.ucfirst($type), "$union\n{\n";
			$type = strtoupper(asFriendlyEnum($type)).'_';
		}
		else //ColladaDOM 3
		{
			$struct_ = $union?'union ':'struct ';
			$type = getFriendlyType($type);
			echo ($union?'union ':'struct '), $type, "\n{\n\tenum __enum__\n\t{\n";
			$ctor = $type; $type = '';
		}
			
		foreach($meta['enum'] as $enum=>$ea)
		{
			$enum = identifyC($enum);
			if(2!==$COLLADA_DOM)
			if(ctype_digit($enum[0])) $enum = 'alias__'.$enum;
			else $enum = getAlias($enum);
			echo "\t", $type, $enum;
			if(isset($ea['value'])) echo ' = ', $ea['value'];		
			echo ",";
			if(!empty($ea['documentation']))
			echo ' /**< ', getDocumentationText($ea['documentation']), ' */';
			echo "\n";
		}
		//not sure why this was. it can collide with the enum though
		//echo "\t", $type, "_COUNT = ", count($meta['enum']), "\n";
		
		//NOTE: __enum__ and __value__ are used instead of type/value
		//in case "type" or "value" are one of the enumerated strings.
		if(2!=$COLLADA_DOM)
		echo "\t}__value__;
	COLLADA_DOM_3($ctor,enum)\n";	
		echo "};\n\n";
	}
	else
	{
		$type = strtoupper(asFriendlyEnum($type));
		foreach($meta['enum'] as $enum=>$ea)
		{
			if(!empty($ea['documentation']))				
			$constStrings[] = "/**\n * ".getDocumentationText($ea['documentation'])."\n */\n";
			$constStrings[$type."_".identifyC($enum)] = "\"".$enum."\";\n";
		}
		$constStrings[] = "\n";
	}
}

if(inline_CM)
{
	//$hack says this is not the CPP file/must be reference
	$hack = 'inline'; echo applyTemplate('types-cpp',$hack);
}
//__NS__ makes multiple-inherited access unambiguous
echo " 
template<int> struct __NS__:DAEP::Note<> //\"NOTESPACE\"
{
	typedef __NB__::__<> concern; 
};
//-------.
	}//<-'
	namespace DAEP
	{
		template<int M, int N> 
		/**PARTIAL-TEMPLATE-SPECIALIZATION */
		class Note<::COLLADA::$target_namespace::__NS__<M>,N>
		: 
		public ::COLLADA::$target_namespace::__NS__<M+N>
		{};
	}
}
";
echo "
#endif //__", strtoupper($prefix), '_TYPES_H__', $include_guard, "\n";

?>/*C1071*/