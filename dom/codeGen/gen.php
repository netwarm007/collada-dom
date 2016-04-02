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
require_once('src/TypeMeta.php');
global $typemeta; //static $TypeMeta::generated
//HACK? this is a built-in type. Or to generate it
//https://www.w3.org/2001/XMLSchema.xsd would need to
//be downloaded/recursively added to the generated stack
//TODO? adding scalars could render the @ operator obsolete
$typemeta =& TypeMeta::generateXMLSchemaTypes('xs:IDREFS','xs:IDREF',
'xs:dateTime xs:ID xs:NCName xs:NMTOKEN xs:Name xs:token xs:string xs:IDREF xs:anyURI');
//$typemeta is data; $classmeta (once $meta) is elements/groups
global $classmeta; $classmeta = array();

require_once('om/object-model.php');
require_once('src/SchemaParser.php');
require_once('tpl/template-engine.php');

$p = new SchemaParser(); $p->parse($XSD_file);

initGen($XSD_file); //also sets up gen.log file

$pop = $p->root_elements[0]; //last something on stack??
if($pop->getType()!='xsSchema')
die('die(fix me: first root elelement is not xs:schema)');

//TODO! REVIEW ME!!! //SCHEDULED OBSOLETE
$_globals['complex_types'] = $pop->getElementsByType('xsComplexType');
 
//prepare types/classmeta for code-gen step
//NOTE: this was done with getElementsByType
//in four separate loops, so the gen.log file
//would be broken up into logical sections. To
//do it this way may cause unexpected problems
$element_context = $global_elements = array();
foreach($pop->elements as& $ea) switch($ea->getType())
{
case 'xsSimpleType':
	
	$meta =& $ea->generate(); 
	$typemeta[$meta['type']] =& $meta; break;

case 'xsElement': //echo "ELEMENTS\n";
case 'xsComplexType': //echo "COMPLEX TYPES\n";
case 'xsGroup': //echo "GROUPS\n";
	
	$meta =& $ea->generate($element_context,$global_elements);
	$classmeta[$meta['element_name']] =& $meta; break;
}
//NOTE: implicates Collada 1.4.1
//propogate the substitutableWith lists and attributes inherited by type
foreach($classmeta as $k=>$ea)
if(!empty($ea['substitutionGroup']))
$classmeta[$ea['substitutionGroup']]['substitutableWith'][] = $k;
//NEW: add math to domElements.h
foreach($global_elements as $ea)
foreach($ea['elements'] as $ea2) if(strpos($ea2,':')!==false)
@$_globals['import_list'][$ea2]++; //assuming <xs:element ref="math:math"/> -like 

//LEGACY: COLLADA specific business
//grab the master collada version/namespace
global $COLLADA; $COLLADA =& $classmeta['COLLADA']; if(!empty($COLLADA))
{
	$_globals['constStrings']['COLLADA_VERSION'] = "\"".$pop->getAttribute('version')."\";\n";
	$_globals['constStrings']['COLLADA_NAMESPACE'] = "\"".$pop->getAttribute('xmlns')."\";\n\n";
}

//generate the dom
function genH($h, $tpl, & $meta=NULL, $flags=NULL)
{
	global $_fsystem, $_globals; file_put_contents
	($_fsystem['include'].$_globals['prefix'].$h.'.h',applyTemplate($tpl,$meta),$flags)
	||die("die(failed to write file template: $tpl )");
}
function genCPP($cpp, $tpl, & $meta=NULL, $flags=NULL)
{
	global $_fsystem, $_globals; file_put_contents
	($_fsystem['sources'].$_globals['prefix'].$cpp.'.cpp',applyTemplate($tpl,$meta),$flags)
	||die("die(failed to write file template: $tpl )");
}
genCPP('Classes','classes-cpp');
foreach($classmeta as& $ea) 
{	
	//ORDER-IS-IMPORTANT 
	//genCPP merges base/inherited fields destructively
	genH(ucfirst($ea['element_name']),'class-h',$ea); 	
	//if($ea['element_name']==='gles_sampler_states_group') //DEBUGGING
	genCPP('Classes','classes-cpp',$ea,FILE_APPEND);
}//these print gathered stuff
echo genH('Types','types-h');
echo genCPP('Types','types-cpp');
echo genH('Elements','elements-h');
echo genH('Constants','constants-h');
echo genCPP('Constants','constants-cpp');

cleanupGen();

?>
