<?php
/*
 * Copyright 2006 Sony Computer Entertainment Inc.
 *
 * Licensed under the MIT Open Source License, for details please see license.txt or the website
 * http://www.opensource.org/licenses/mit-license.php
 *
 */

//COMMAND-LINE: bin/php.exe gen.php collada.xsd
//Note: must be run from home directory of code generator (i.e. where gen.php lives)
//CONFIGURATION: atop tpl/template-engine.php are
//global parameters. One day it should 'include' a command-line argument to set them

ini_set("memory_limit","256M");

if(file_exists($argv[1])) $XSD_file = $argv[1];
else die("die(Can't find XSD file: {$argv[1]})");

//NEW/SCHEDULED OBSOLETE
//NEW: guessTypeOfElement is depending on these
require_once('src/TypeMeta.php');
global $typemeta; //static $TypeMeta::generated
//HACK? this is a built-in type. Or to generate it
//https://www.w3.org/2001/XMLSchema.xsd would need to
//be downloaded/recursively added to the generated stack
//TODO? adding scalars could render the @ operator obsolete
$typemeta =& TypeMeta::generateXMLSchemaTypes(
'xs:IDREFS xs:NMTOKENS xs:ENTITIES', //lists
'xs:IDREF xs:NMTOKEN xs:ENTITY', //itemTypes
'xs:IDREF xs:NMTOKEN xs:ENTITY 
 xs:dateTime xs:ID xs:token xs:string xs:anyURI xml:base
 xs:normalizedString xs:Name xs:NCName xs:QName xs:NOTATION xs:language
 xs:duration xs:date xs:time xs:gYear xs:gYearMonth xs:gMonth xs:gMonthDay xs:gDay
 xs:hexBinary xs:base64Binary', //quasi-strings
'xs:unsignedByte xs:boolean xs:float xs:double xs:decimal xs:long xs:integer 
 xs:nonPositiveInteger xs:negativeInteger xs:int xs:short xs:byte xs:unsignedLong
 xs:nonNegativeInteger xs:positiveInteger xs:unsignedInt xs:unsignedShort xs:unsignedByte');
//this is a lot of work to filter out parts of included schema
//that aren't used by the schema comprising the generator output
//Right now it's really just to catch the "generateXMLSchemaTypes"
function includeTypeInSchema($type)
{
	global $typemeta;
	if(strpos($type,':')===false||empty($typemeta[$type])) return; 	
	$meta =& $typemeta[$type]; $meta['inSchema'] = true;
	includeTypeInSchema($meta['itemType']); assert(empty($meta['base']));
}
				
//$typemeta is data; $classmeta (once $meta) is elements/groups
global $classmeta; $classmeta = array();

//convert a string to a C identifier with punctuation collapsed
function identifyC($str)
{
	return trim(preg_replace('/[^a-zA-Z0-9]+/','_',iconv('UTF-8','ASCII//TRANSLIT',$str)),'_');
}

require_once('om/object-model.php');
require_once('src/SchemaParser.php');
require_once('tpl/template-engine.php');

$p = new SchemaParser(); $p->parse($XSD_file);

initGen($XSD_file); //also sets up gen.log file

$pop = $p->root_elements[0]; //last something on stack??
if($pop->getType()!='xsSchema')
die('die(fix me: first root elelement is not xs:schema)');

//HACK: generateCM_isPlural is using this to transclude their children.
$_globals['group_types'] = $pop->getElementsByTypeAccordingToName('xsGroup');

//TODO! REVIEW ME!!! //SCHEDULED OBSOLETE
$_globals['complex_types'] = $pop->getElementsByTypeAccordingToName('xsComplexType');
 
//prepare types/classmeta for code-gen step
//NOTE: this was done with getElementsByType
//in four separate loops, so the gen.log file
//would be broken up into logical sections. To
//do it this way may cause unexpected problems
$element_context = $global_elements = array();
foreach($pop->elements as $ea) switch($ea->getType())
{
case 'xsSimpleType':
	
	$meta =& $ea->generate(); 
	$typemeta[$meta['type']] =& $meta; break;

case 'xsGroup': //echo "GROUPS\n";
case 'xsElement': //echo "ELEMENTS\n";
case 'xsComplexType': //echo "COMPLEX TYPES\n";
	
	$meta =& $ea->generate($element_context,$global_elements);
	$classmeta[$meta['element_name']] =& $meta; break;
}
//these are being set by setAbstract()
global $abstract; $abstract = array();
global $subgroups; $subgroups = array();
//NOTE: implicates Collada 1.4.1
//propagate the substitutableWith lists and attributes inherited by type
foreach($classmeta as $k=>$ea)
for($sub=$ea['substitutionGroup'];!empty($sub);$sub=$sub['substitutionGroup'])
{
	$subgroups[$sub][$k] = $ea; $sub = $classmeta[$sub];
}
/*Disabling inside domElements.h
//NEW: add math to domElements.h
foreach($classmeta as $ea)
foreach($ea['elements'] as $ea2) if(strpos($ea2,':')!==false)
@$_globals['import_list'][$ea2]++;*/

//This destructively merges the derived types. It had only been done for 
//the CPP files prior to outputting the content-model metadata. It's now
//done up front because C++'s inheritance model is incompatible with XML
//Schema's model, and beside, it's best for the contents-arrays this way.
function flattenInheritanceModel(&$meta)
{	
	foreach($meta['classes'] as& $ea) 	
	flattenInheritanceModel($ea); unset($ea);
	
	$flat =& $meta['flat'];
	if(isset($flat)) return; $flat = true;
		
	global $classmeta;
	$bt = $meta['base_type']; 
	if(empty($bt)||!isset($classmeta[$bt])) 
	return;
	$base =& $classmeta[$bt]; flattenInheritanceModel($base);	

	$meta['transcludes'] = array_merge($base['transcludes'],$meta['transcludes']);
	$meta['transcludes'][$base['element_name']] = 'TRANSCLUDED';
	
	$meta['attributes'] = array_merge($base['attributes'],$meta['attributes']);		
	
	$ct =& $meta['content_type'];	
	if(''==$ct&&!$meta['mixed'])
	{
		$ct2 = $base['content_type'];
		if(''!=$ct2) $ct = $ct2;
		else if($base['mixed']) $meta['mixed'] = true;
	}
	else //NEW: CONVERT SIMPLE-CONTENT TO SIMPLE-TYPE	
	{
		global $typemeta;
		while(!isset($typemeta[$ct])) 
		$ct = $classmeta[$ct]['content_type'];				
	}
	
	$cm =& $meta['content_model']; $tempAddTo = !empty($cm); 
	if(!empty($ct))
	{
		if($tempAddTo) die('die(Both content_type and content_model.)');
		
		return; //EARLY-OUT: shouldn't be a content model
	}
	$cm2 = $base['content_model']; $tempAddTo2 = !empty($cm2);  
		
	//NEW: Adding the first test seems to eliminate past needs to merge.
	if($tempAddTo!=$tempAddTo2)
	{
		if($tempAddTo2) 
		{
			$cm = $cm2; //$meta['elements'] = $base['elements'];
			$els =& $meta['elements']; $els = $base['elements'];
			foreach($els as& $ea) 
			$ea['isLocal'] = false; unset($ea);//HACK: same as below
		}		
	}
	else if($tempAddTo) //merge
	{
		//1.5.0 doesn't enter this. This should be removed if this is needed. 
		//TODO: If the CM is already a sequence, wrapping in one may be overkill.
		die('die(COLLADA 1.4.1?)');
		
		//Reminder: isPlural is set by _elementSet2::generateCM_isPlural()
		//array_merge_recursive won't do here, as two non-singles is one plural
		$els =& $meta['elements'];
		foreach($base['elements'] as $k=>$ea) //HACKS
		{
			$el =& $els[$k]; if(empty($el)) //tweak?
			{
				$el = $ea; $el['isLocal'] = false; //not local to this class
			}
			else //this really shouldn't be merged, and can be a source of errors
			{
				$el = array_merge($ea,$el); $el['isPlural'] = true; //must be
			}
		}		
		
		//Merge $meta['content_model']. NOTE: ADDING sequenceCMopening IS NOT ALWAYS REQUIRED
		$tempArray = array();
		//adding to CM - need to add a starting sequence
		$tempArray[] = array('name'=>ElementMeta::sequenceCMopening,'minOccurs'=>1,'maxOccurs'=>1);
		$tempArray = array_merge($tempArray,$cm2);
		array_pop($tempArray); //remove the last END token
		$tempArray = array_merge($tempArray,$cm);
		//adding to CM - need to add an ending sequence
		$tempArray[] = array('name'=>ElementMeta::CMclosure,'minOccurs'=>0,'maxOccurs'=>0);	
		$cm = $tempArray;
	}	
}foreach($classmeta as& $ea) flattenInheritanceModel($ea); unset($ea);
//MORE POSTPROCESSING--THIS CONVERTS ENUM-BASED UNIONS INTO ENUMS
foreach($typemeta as $type=>& $meta) if(!empty($meta['union_type']))
{	
	//WARNING//WARNING//WARNING//WARNING//WARNING//WARNING
	//
	// No attempt should be made to assign values to union
	// enum. This means they end up with new values if not
	// specified. This should be fixed at the schema level.
	// And barring that, user/clients can specialize types.
	//
	//WARNING//WARNING//WARNING//WARNING//WARNING//WARNING	
		
	//NEW: DON'T ASSUME UNION IS ENUM BASED
	$meta_enum =& $meta['enum'];
	$types = explode(' ',$meta['union_members']);		
	//$count = 1; //Why not 0???
	foreach($types as $ea) 
	{
		$um = $typemeta[$ea];
		if(empty($um)) 
		die('die(should this ever be?)');
		
		//New: assign a base to the composite enum.
		if(!empty($meta['base']))
		{
			if($meta['base']!=$um['base']) 
			die('die(union base mismatch)');
		}
		else $meta['base'] = $um['base']; 			
		foreach($um['enum'] as $enum=>$ea2)
		{
			//THIS APPEARS TO PREVENT DOUBLE ENTRY, BUT
			//IS ALSO BEING REUSED IN tpl-types.cpp.php
			$destructive_write =& $meta_enum[$enum];
			if(!empty($destructive_write)) continue;
			else $destructive_write = $ea2; //EVIL
		}				
	}
	//New: improve ColladaDOM 3 binary-search lookup.
	ksort($meta_enum);
	unset($meta_enum);	
}

__aliasSuppress();
global $global_parents, $global_children;
$global_parents = $global_children = array();
function& trytoref($array,& $key_out) //how else to do this???
{
	return isset($array[$key_out])?$o=&$array[$key_out]:$key_out=NULL;
}
function relateClass(&$meta)
{
	$mc = $meta['classes'];	
	global $global_parents, $global_children, $classmeta;
	foreach($meta['elements'] as $k=>$ea)
	{	
		$ref =@@ $ea['ref'];
		if(!empty($ref))
		{
			$meta2 =& trytoref($classmeta,$ref);
			if(!$meta2) continue; //imported?
		}
		else //assuming if !$meta2 it is a local simple-type
		{
			$ref =@@ $ea['type'];
			if(!empty($ref)) $meta2 =& trytoref($classmeta,$ref);
		}
		$gc =& $global_children[$k]; if(!empty($meta2))
		{			
			$global_parents[$ref][$k] =& $gc; $gc[$ref] =& $meta2;
		}		
		else //assuming if !$meta2 it is a transcluded local-type
		{
			//assuming $k is not abiguous via transclusion
			$meta2 =& trytoref($mc,$k); if(!empty($meta2))
			{			
				$gc[getScopedClassName($meta2,'__')] =& $meta2; 
			}
		}
		unset($meta2);
	}	
	foreach($mc as& $ea) 	
	relateClass($ea); unset($ea);		
}foreach($classmeta as& $ea) relateClass($ea); unset($ea);
__aliasRestore();

//this is done here to include COLLADA-DOM 2.x elementType constants
//2.x was for back-compatability even though elementType is 2.5 only
global $synthetics;
//these are duplicated local/simple types
//that XML Schema encourages inline defining (by making it so easy to do)
//consolidating them can mean fewer types
//and it can open up the WYSIWYG short names
foreach($synthetics as $k =>& $ea)
{
	//this criteria can be relaxed if there is code to make them work
	//1: there's 1 representation per name (same content-type, same attributes)
	//2: there's more than one instance of the type (this is merging after all)
	//3: there's not already a global type with the same name of the local type
	//3 is so the "WYSIWYG" name doesn't require decoration, as there's not one
	//that has been decided upon. (Regular ambiguities are prefixed by __XSD__)
	if(1==count($ea)&&1<count(reset($ea))&&empty($global_parents[$k]))
	{
		$synth = 1;
		foreach($ea as& $ea0) break; //reset(); 
		foreach($ea0 as& $meta) $meta['isSynth'] = true;	
	}unset($ea); unset($meta);
}

//assuming singular root candidate
//is $pop guaranteed to be a root?
global $root_xmlns, $root_version;
$root_xmlns = $pop->getAttribute('xmlns');
$root_version = $pop->getAttribute('version');
//LEGACY: COLLADA specific business
//grab the master collada version/namespace
global $COLLADA; if(isset($classmeta['COLLADA']))
$COLLADA =& $classmeta['COLLADA']; if(!empty($COLLADA))
{
	//LEGACY: COLLADA specific business
	//Inject the master COLLADA version/namespace.
	//Note, this is overriding the schema, and being both required, and having a
	//default value, is not a valid XSD schema. This should be optional, however
	//COLLADA-DOM is not concerned with the schema.
	//Also note that this had been done in a static \"create\" method, without the
	//schema overrides. But that was a hack, and \"create\" is no longer an option.
	$xmlns =& $COLLADA['attributes']['xmlns'];
	$xmlns['use'] = 'required';$xmlns['default'] = $root_xmlns; 
	$COLLADA['attributes']['version']['default'] = $root_version;
//	$_globals['constStrings']['COLLADA_VERSION'] = "\"$root_version\";\n";
//	$_globals['constStrings']['COLLADA_NAMESPACE'] = "\"$root_xmlns\";\n\n";
}
global $COLLADA_DOM_GENERATION;
$_globals['target_namespace'] =
identifyC(empty($root_xmlns)?$XSD_file:$root_xmlns);
$_globals['include_guard'] = 
$_globals['target_namespace'].'__ColladaDOM_g'.$COLLADA_DOM_GENERATION.'__';

//generate the dom
function genH($h, $tpl, & $meta=NULL, $flags=NULL)
{
	global $_fsystem, $_globals; file_put_contents
	($_fsystem['include']/*.$_globals['prefix']*/.$h.'.h',applyTemplate($tpl,$meta),$flags)
	||die("die(failed to write file template: tpl-$tpl.php )");
}
function genCPP($cpp, $tpl, & $meta=NULL, $flags=NULL)
{
	global $_fsystem, $_globals; file_put_contents
	($_fsystem['sources'].$_globals['prefix'].$cpp.'.cpp',applyTemplate($tpl,$meta),$flags)
	||die("die(failed to write file template: tpl-$tpl.php )");
}
genCPP('Classes','classes-cpp');
foreach($classmeta as& $ea)
{	
	genH($ea['element_name'],'class-h',$ea);	
	if(!inline_CM)
	//if($ea['element_name']==='camera_type') //DEBUGGING
	genCPP('Classes','classes-cpp',$ea,FILE_APPEND);
}unset($ea); 
//HACK: merging the following headers
$combined = 'combined'; 
echo genH($_globals['target_namespace']/*'domTypes'*/,'types-h');
echo genCPP('Types','types-cpp');
echo genH($_globals['target_namespace']/*'domElements'*/,'elements-h',$combined,FILE_APPEND);
//these are effectively no longer used
echo genH('domConstants','constants-h');
echo genCPP('Constants','constants-cpp');
cleanupGen();

?>
