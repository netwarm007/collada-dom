<?php
/*
 * Copyright 2006 Sony Computer Entertainment Inc.
 *
 * Licensed under the MIT Open Source License, for details please see license.txt or the website
 * http://www.opensource.org/licenses/mit-license.php
 *
 */

//The new way is to combine "domTypes.h"/"domElements.h"
$combined = $meta=='combined'?'//':''; if(!empty($combined)) echo "\n";
	
echoCode("
$copyright

#ifndef __$1_ELEMENTS_H__$3
#define __$1_ELEMENTS_H__$3

$4#include \"$2Types.h\"",strtoupper($prefix),$prefix,$include_guard,$combined);

echo "COLLADA_(namespace)
{
    COLLADA_($target_namespace,namespace)
    {//-.
//<-----'
";

/*Disabling for now. Don't want to toss the code just yet
//Reminder: imported elements are not found in $classmeta
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
typedef $ns$macro($1,$2) $Type;
#endif
typedef daeSmartRef<$Type> {$Type}Ref;
typedef daeTArray<{$Type}Ref> {$Type}_Array;
",ucfirst($Name),lcfirst($Name));
	}
	
	echo "
//BEGIN LOCALLY SOURCED ELEMENTS

";
}*/

//global elements
foreach($classmeta as $k=>$ea) 
{
	if(2==$COLLADA_DOM)
	{	
		//NEW: adding annotations, so IDE's will pick them up
		echoClassDoxygentation('',$ea); 		
		//NEW: $code puts these on one line, so the comment is attached to them all
		echoCode("
class $1; typedef daeSmartRef<$1> $1Ref; typedef dae_Array<$1> $1_Array; typedef daeSmartRef<const $1> const_$1Ref; 
	",getFriendlyType($k));
	}
	else if(!empty($recursive[$k]))
	{
		echo 'typedef class ', getFriendlyName($k.'__recursive'), ' ', getFriendlyName($k), ";\n";	
	}
	else echo 'class ', getFriendlyName($k), ";\n";	
}

$synth = 0; //should now be in gen.php?
global $synthetics;
foreach($synthetics as $k =>& $ea)
{
	foreach($ea as& $ea0) break; //reset(); 
	if(!empty($ea0[0]['isSynth']))
	{
		$synth = 1; //would be better set by gen.php
		echo applyTemplate('class-h-def',$ea0[0]), "\n";
	}}
if($synth) echoCode("
//-------.
	}//<-'
#define COLLADA_target_namespace \
COLLADA::$target_namespace
#ifndef COLLADA_DOM_LITE");	
if($synth) foreach($synthetics as $k =>& $ea)
{
	foreach($ea as& $ea0) break; //reset(); 
	if(!empty($ea0[0]['isSynth']))
	echo applyTemplate('classes-cpp',$ea0[0]);
}unset($ea);
if($synth) echoCode("

#endif //!COLLADA_DOM_LITE");
if(2!==$COLLADA_DOM) 
if($synth) echoCode("
//------.
    //<-'
	namespace DAEP //WYSIWYG
	{
        COLLADA_($target_namespace,namespace)
        {//-.
//<---------'");
if(2!==$COLLADA_DOM) 
if($synth) foreach($synthetics as $k =>& $ea)
{
	foreach($ea as& $ea0) break; //reset(); 
	if(!empty($ea0[0]['isSynth']))
	echo applyTemplate('ColladaDOM-3',$ea0[0]);
}unset($ea);
if(2!==$COLLADA_DOM) 
if($synth) echoCode("
//-----------.
		}//<-'
	}");
if($synth) echoCode("
#undef COLLADA_target_namespace");
	
if(2==$COLLADA_DOM)
{
	if($synth) echo "		
	COLLADA_($target_namespace,namespace)
    {//-.
//<-----'";
echo "
//Element Type Enum
namespace {$namespace}_TYPE //deprecated
{
	const int
	NO_TYPE = 0,
	ANY = 1,";
	sort($elementTypes); //mix in the synthetics
	$end =& $elementTypes[count($elementTypes)-1];
	$end = strtr($end,',',';');
	foreach($elementTypes as $ea) echo "\n\t", $ea;
	echo "
}";
}

global $COLLADA_DOM_GENERATION;
echoCode("
$1}	
//This is at the end because some code highlighters
//(e.g. NetBeans) gray-out text that follows #error.
#if $COLLADA_DOM_GENERATION != COLLADA_DOM_GENERATION
#error Generator COLLADA_DOM_GENERATION doesn't match.
#endif
#endif //__$2_ELEMENTS_H__$include_guard",
$synth&&2!==$COLLADA_DOM?'':'}',strtoupper($prefix));

?>/*C1071*/