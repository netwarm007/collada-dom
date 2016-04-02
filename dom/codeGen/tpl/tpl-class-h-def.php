<?php

/*
 * Copyright 2006 Sony Computer Entertainment Inc.
 *
 * Licensed under the MIT Open Source License, for details please see license.txt or the website
 * http://www.opensource.org/licenses/mit-license.php
 *
 */

$full_element_name = //shorthand
$prefix.ucfirst($meta['element_name']);
$friendly_typedef = asFriendlyType($full_element_name);

//SCHEDULED OBSOLETE
$deep_type_constant_pop = $deep_type_constant;
$deep_type_constant.=strtoupper($meta['element_name']);
//UNUSED: domConstants.h/cpp COLLADA_TYPE/ELEMENT/ETC STRINGS
//(NOTE: these files also have COLLADA_VERSION/NAMESPACE which are, but?)
{
	/*//COLLADA_ELEMENT/TYPE_ 
	foreach($meta['elements'] as $ea)
	if(array_search($ea,$elementNames)===FALSE)
	$elementNames[] = $ea;
	if(!empty($meta['substitutionGroup'])) //one more?
	if(array_search($meta['element_name'],$elementNames)===FALSE)
	$elementNames[] = $meta['element_name'];*/
	//namespace COLLADA_TYPE; 
	//???: THESE CONDITIONS MAKE ABSOLUTELY NO SENSE
	//if(array_search($meta['element_name'],$elementTypes)===FALSE) 
	$elementTypes[] = $deep_type_constant;
}

if($meta['parent_meta']!=NULL) 
{ 
	//only echo these for the inner classes.. the main classes will have
	//them defined in a seperate file to avoid circular includes.
	echoCode("
class $full_element_name;
typedef $1SmartRef<$full_element_name> {$friendly_typedef}Ref;
typedef $1TArray<{$friendly_typedef}Ref> {$friendly_typedef}_Array;
",$meta_prefix);
}

//DOCUMENTATION
echoDoxygen(@$meta['documentation']);
//SUBSTITION GROUP/INHERITANCE
$baseClass = $meta_prefix.'Element';
if($meta['substitutionGroup']!='') 
$baseClass = $prefix.ucfirst($meta['substitutionGroup']);
if($meta['isExtension']) 
$baseClass = $prefix.ucfirst($meta['base_type']);
if(!empty($meta['baseTypeViaRestriction']))
$baseClass = $prefix.ucfirst($meta['baseTypeViaRestriction']);
echoCode("
typedef class $full_element_name : public $baseClass
{
public:

	//This function is deprecated. Use typeID instead
	virtual $1_TYPE::TypeEnum 
	getElementType()const{ return $1_TYPE::$deep_type_constant; }	
	//ID is scheduled for removal. As it's not exported
	static $2Int ID(){ return $3; } 
	virtual $2Int typeID()const{ return $3; }	
",$namespace,$meta_prefix,$typeID++); //NOTICE INCREMENTING!

//INTERNAL CLASSES
$deep_type_constant.='_'; //SCHEDULED OBSOLETE
$result = ''; $results = 0;
foreach($meta['inline_elements'] as $ea)
if(!$ea['complex_type']||$ea['isRestriction']||$ea['isExtension'])
{
	$pop = $indent; $indent.="\t"; 	
	$results++; $result.=applyTemplate('class-h-def',$ea);
	$indent = $pop;
}if($results>0)
echoCode("
public: //NESTED ELEMENT$1
",$results>1?'S':'');
echo $result;

//SCHEDULED OBSOLETE
$deep_type_constant = $deep_type_constant_pop;

//ENUMS
//NOTE: COLLADA 1.5 has no instances of this
//14.1 may? In any case, it's not been tested
if($meta['simple_type']!=NULL)
{
	$meta2 =& $meta['simple_type']->getMeta();	
	if(!empty($meta2['enum']))
	{
		//REMINDER: not properly indented either
		die("die(is this COLLADA 1.4.1? is anybody out there?)");
		
		$type = $meta2['type'];		
		if(!$meta2['useConstStrings'])
		{
			//NOTE: this was outside this block
			//HOWEVER it's clear that the constStrings path is not echoing
			echo $indent, "public: //ENUM\n";		
			//what is this comment saying?
			//Decided to name mangle the enum constants so they are more descriptive and avoid collisions			
			echoDoxygen(@$meta2['documentation'],"\t");
			//previously _type is injected. HOWEVER this is an ENUM and tpl-types-header.cpp doesn't do this, so
			echo "enum ", $prefix, ucfirst($$type), "\n{\n";		
			$type = strtoupper(asFriendlyEnum($type));
			foreach($meta2['enum'] as $enum=>$ea)
			{
				echo "\t", $type, "_", strtr($enum,'.-','__'), ",";
				if(!empty($ea['documentation']))
				echo " /**< ", getDocumentationText($ea['documentation']), " */";								
				echo "\n";
			}
			echo "\t", $type, "_COUNT";
			echo "\n};\n\n";
		}
		else
		{
			$type = strtoupper(asFriendlyEnum($type));
			foreach($meta2['enum'] as $enum=>$ea)
			{
				if(!empty($ea['documentation']))
				$constStrings[] = "/**\n * ".getDocumentationText($ea['documentation'])."\n */\n";								
				$constStrings[$type."_".strtr($enum,'.-','__')] = "\"".$enum."\";\n";
			}
			$constStrings[] = "\n";
		}
	}
}

//ATTRIBUTES
if((!empty($meta['attributes'])||$meta['useXMLNS'])
/*&&empty($meta['baseTypeViaRestriction'])*/)
{
	echoCode("
protected: //Attribute$1",count($meta['attributes'])>1?'s':'');

	if($meta['useXMLNS']) //domCOLLADA/technique
	echoCode("
	/**
	 * This element may specify its own xmlns.
	 */
	xsAnyURI _attrXmlns;");
	foreach($meta['attributes'] as $attr_name=>$ea)
	{
		$type = $ea['type']; 
		$preType = $type; $Name = $attr_name;
		getFriendlyTypeAndName($preType,$Name);		
		echoDoxygen(@$ea['documentation'],"\t");				
		echoCode("
	$preType _attr$Name;");
	}echo "\n";
}

//ELEMENTS
echoElementsCPP($meta);

//get/setX methods
echoAccessorsAndMutatorsCPP($meta);

//VALUE
//NOTE: following NOTE is never the case???
//NOTE: special casing any element with 'mixed' content model to ListOfInts type _value
if((!empty($meta['content_type'])||$meta['mixed']))
if(!$meta['abstract']&&empty($meta['baseTypeViaRestriction']))
{
	$content_type = $meta['content_type'];	
	$preType = getFriendlyType($content_type);
	if($meta['parent_meta']!=NULL)
	{
		$content = asFriendlyType($content_type); //HACK
		if(array_key_exists($content,$meta['parent_meta']['inline_elements']))
		$preType = '::'.$preType;		
	}
	if(@$classmeta[$content_type]['isComplexType']) //$valueType.="Ref";
	die('Please investigate'); //Reminder: C comment displays type without Ref suffix
	
echoCode("
protected: //Value
	/**
	 * The $preType value of the text data of this element. 
	 */
	$preType _value;
	");
}

//CONSTRUCTORS  
echoConstructorsCPP($full_element_name,$meta,$baseClass);

echoCode("
public: //STATIC METHODS
	/**
	 * Creates an instance of this class and returns a $1ElementRef referencing it.
	 * @return Returns a $1ElementRef referencing an instance of this object.
	 */
	static DLLSPEC $1ElementRef create($2 &$1);
	/**
	 * Creates a $1MetaElement object that describes this element in the meta object reflection framework.
	 * If a $1MetaElement already exists it will return that instead of creating a new one. 
	 * @return Returns a $1MetaElement describing this $3 element.
	 */
	static DLLSPEC $1MetaElement *registerElement($2 &$1);
	
}$friendly_typedef;
",$meta_prefix,strtoupper($meta_prefix),$namespace);