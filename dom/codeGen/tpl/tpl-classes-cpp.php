<?php
/*
 * Copyright 2006 Sony Computer Entertainment Inc.
 *
 * Licensed under the MIT Open Source License, for details please see license.txt or the website
 * http://www.opensource.org/licenses/mit-license.php
 *
 */

if(empty($meta)) //creating the file?
{
	echoCode("
$copyright
	
#include <$1.h>
#include <$1/$1Dom.h>
#include <$1/$1MetaCMPolicy.h>
#include <$1/$1MetaSequence.h>
#include <$1/$1MetaChoice.h>
#include <$1/$1MetaGroup.h>
#include <$1/$1MetaAny.h>
#include <$1/$1MetaElementAttribute.h>

extern $1String $2_VERSION;
extern $1String $2_NAMESPACE;",$meta_prefix,$namespace);
	
	return; //otherwise appending to file...
}
else if($meta['parent_meta']==NULL) 
{
	echoCode("
		
#include <$1/$1$2.h>",
$prefix,ucfirst($meta['element_name']));
}

$longname = $meta['context'];
foreach($longname as& $ea) 
$ea = $prefix.ucfirst(strtr($ea,'.-','__')); 
$longname = implode('::',$longname);
$shortname = getFriendlyType($meta['element_name']);

//LEGACY: COLLADA specific business
//REMINDER: this workflow is funny???
global $COLLADA;
if($meta===$COLLADA) echoCode("
$1ElementRef $longname::create($2 &$1)
{
	{$shortname}Ref ref = new $shortname($1);
	ref->_meta = $1.getMeta(domCOLLADA::ID());
	ref->setAttribute(\"version\",COLLADA_VERSION);
	ref->setAttribute(\"xmlns\",COLLADA_NAMESPACE);
	ref->_meta = nullptr; return ref;
}",$meta_prefix,strtoupper($meta_prefix));
else echoCode("
$1ElementRef $longname::create($2 &$1)
{
	return new $shortname($1);
}",$meta_prefix,strtoupper($meta_prefix));

//THIS IS DESTRUCTIVE?! //SCHEDULED OBSOLETE
if($meta['complex_type']&&!$meta['isRestriction']||!empty($meta['baseTypeViaRestriction']))
{
	//echo "element {$meta['element_name']} is of base {$meta['base_type']}\n";
	//import content model from type
	$meta2 =& $classmeta[$meta['base_type']];
	$meta['elements'] = array_merge($meta2['elements'],$meta['elements']);
	$meta['element_attrs'] = array_merge($meta2['element_attrs'],$meta['element_attrs']);
	$meta['content_type'] = $meta2['content_type'];
	$meta['attributes'] = array_merge($meta2['attributes'],$meta['attributes']);
	$tempArray = array();
	$tempAddTo = !empty($meta['content_model']); 
	if($tempAddTo) //adding to CM - need to add a starting sequence
	$tempArray[] = array('name'=>ElementMeta::sequenceCMopening,'minOccurs'=>1,'maxOccurs'=>1);
	$tempArray = array_merge($tempArray,$meta2['content_model']);
	array_pop($tempArray); //remove the last END token
	$tempArray = array_merge($tempArray,$meta['content_model']);
	if($tempAddTo) //adding to CM - need to add an ending sequence
	$tempArray[] = array('name'=>ElementMeta::CMclosure,'minOccurs'=>0,'maxOccurs'=>0);	
	$meta['content_model'] = $tempArray;
}

//implicates COLLADA 1.4.1
//apparently these are not included in the headers
foreach($meta['elements'] as $ea) 
if(!empty($classmeta[$ea]['substitutableWith']))
foreach($classmeta[$ea]['substitutableWith'] as $ea2)
echo "#include <$prefix/$prefix", ucfirst($ea2), ".h>\n";

//NOTE: setName takes this name in case clients are
//relying on it as a default name for many elements
//(this is keeping with the old cleanSchema script)
//NOTE: setName is pretty useless/should be removed
$name = asFriendlyType($meta['element_name']);
echoCode("
$1MetaElement *$longname::registerElement($2 &$1)
{
	$1MetaElement *meta = $1.getMeta(ID());
	if(meta!=nullptr) return meta;

	meta = new $1MetaElement($1); $1.setMeta(ID(),*meta);
	meta->setName(\"$name\");	
	meta->registerClass($longname::create);",
$meta_prefix,strtoupper($meta_prefix));
if($meta['isGroup']) echoCode("
	meta->setIsTransparent(true);");
if($meta['abstract']) echoCode("
	meta->setIsAbstract(true);");
if($meta['parent_meta']!=NULL) echoCode("
	meta->setIsInnerClass(true);");	
echoCode("
	meta->setElementSize(sizeof($shortname));");

if(!empty($meta['elements'])||$meta['has_any'])
{
	echo "
	{$meta_prefix}MetaCMPolicy *cm = nullptr;";
	if(!$meta['has_any']) echo "
	{$meta_prefix}MetaElementAttribute *mea = nullptr;\n";		

	$needsContents = false;	
	$currentCM = array('cm'=>NULL,'ord'=>0);
	$currentOrd = $choiceNum = 0; $contGroup = false;
	$cmStack = array(); //the top of the stack is not the currentCM/Ord
	$push_cm = function($cm) use(& $currentCM,& $currentOrd,& $cmStack)
	{
		$currentCM['ord'] = $currentOrd; array_push($cmStack,$currentCM);
		$currentCM['cm'] = $cm;	$currentOrd = 0;
	};

	//NOTE: this is tightly coupled to generateContentModel
	foreach($meta['content_model'] as $ea)
	{
		//copying-on-write ($ea is being pushed onto $cmStack)
		if($ea['maxOccurs']===unbounded) $ea['maxOccurs'] = -1;		
		extract($ea); //extracting $name, $minOccurs & $maxOccurs		
		switch(is_int($name)?$name:-1) //cases must agree with type
		{
		case ElementMeta::sequenceCMopening: //xs:sequence
			
			echoCode("
	cm = new {$meta_prefix}MetaSequence(meta,cm,$currentOrd,$minOccurs,$maxOccurs);");			
			$push_cm($ea); break;
			
		case ElementMeta::choiceCMopening: //xs:choice			
			
			echoCode("
	cm = new {$meta_prefix}MetaChoice(meta,cm,$choiceNum,$currentOrd,$minOccurs,$maxOccurs);");
			$choiceNum++; $needsContents = true; 			
			$push_cm($ea); break;
			
		case ElementMeta::groupCMcontinuing: //xs:group			

			//groups actually add two parts to the content model. The first is the group, the second an element
			$contGroup = true; goto group; 							
			
		case ElementMeta::allCMopening: //xs:all			
			
			die('die(xs:all content model appears unimplemented)');
			$needsContents = true;			
			//echo "
//	cm = new {$meta_prefix}MetaAll(meta,cm,$minOccurs,$maxOccurs);";						
			$push_cm($ea); break;
		
		case ElementMeta::anyCM: //xs:any 
			
			echoCode("
	cm = new {$meta_prefix}MetaAny(meta,cm,$currentOrd,$minOccurs,$maxOccurs);");
			//assuming not END terminated?
			if(@ElementMeta::sequenceCMopening===$currentCM['cm']['name'])
			$currentOrd++;
			break;
			
		case -1: //named element string
		
			$attrs = $meta['element_attrs'][$name];			
			if(strpos($name,':')!==false) $preType = getFriendlyType($name); //import
			else $preType = $prefix.ucfirst(guessTypeOfElement(@$attrs['type'],$name));									
			$arrayOrNot = $attrs['maxOccurs']>1; //not necessarily same as $maxOccurs
			echoCode("
	mea = new $1MetaElement".($arrayOrNot?'Array':'')."Attribute(meta,cm,$currentOrd,$minOccurs,$maxOccurs);
	mea->setName(\"$name\");
	mea->setOffset($1OffsetOf($shortname,_elem".strtr(ucfirst($name),'.-:','___').($arrayOrNot?'_array':'')."));
	mea->setElementType($preType::registerElement($1));",$meta_prefix);			
			
			$inner = 'mea';	if($contGroup) //continuing group?
			{
				$contGroup = false; //should substitutableWith apply to groups?
				$inner = "new {$meta_prefix}MetaGroup(mea,meta,cm,$currentOrd,$minOccurs,$maxOccurs)";		
			}//NEW: fix for documented workaround for groups not contained in sequences			
			echo "\t", !empty($cmStack)?"cm->appendChild($inner)":"cm = $inner", ";\n"; 
	
			//implicates COLLADA 1.4.1
			if(!empty($classmeta[$name]))
			foreach($classmeta[$name]['substitutableWith'] as $ea2)
			echoCode("
	mea = new $1MetaElement".($arrayOrNot?'Array':'')."Attribute(meta,cm,$currentOrd,$minOccurs,$maxOccurs);
	mea->setName(\"$ea2\");
	mea->setOffset($1OffsetOf($shortname,_elem".strtr(ucfirst($name),'.-:','___').($arrayOrNot?'_array':'')."));
	mea->setElementType(" //it seems like according to substitutionGroup rules, this should suffice, long term
	.$prefix.ucfirst($ea2)."::registerElement($1));
	cm->appendChild(mea);",$meta_prefix);			

			if(@ElementMeta::sequenceCMopening===$currentCM['cm']['name'])
			$currentOrd++;
			break;
			
		case ElementMeta::CMclosure: //pop the stack
						
			$level = count($cmStack);					
			//SCHEDULED OBSOLETE?
			//NEW: for whatever reason some sequences are double-terminated
			//before the for-loops did $i<count()-1; but foreach must break 
			if($level===0) break;			
			
			if($level>1) echoCode("
	cm->setMaxOrdinal(".($currentOrd-1>=0?$currentOrd-1:0).");
	cm->getParent()->appendChild(cm);
	cm = cm->getParent();");
				
			//PREVIOUS $name/maxOccurs			
			extract($currentCM['cm']); 			
			$currentCM = array_pop($cmStack); 			
			if(ElementMeta::choiceCMopening!==$currentCM['cm']['name']) 
			{								
				//SCHEDULED OBSOLETE
				if(-1===$maxOccurs) $maxOccurs = big_number; //3000
			
				if($name===ElementMeta::sequenceCMopening) //CM previously
				{
					if($maxOccurs>=big_number)
					$currentOrd = $currentCM['ord']+big_number; //3000
					else //multiply by the sequence's total number of slots
					$currentOrd = $currentCM['ord']+$maxOccurs*$currentOrd;							
				}
				else $currentOrd = $currentCM['ord']+$maxOccurs;	
			}
			else $currentOrd = $currentCM['ord']; //NEW: reset choice
			break;						
		default: die("die(unrecognized CM code: $name)"); 
		}if($contGroup) break; group: continue; //paranoia
	}if($contGroup) die('die($contGroup was not matched)');
	echoCode("
	cm->setMaxOrdinal(".($currentOrd-1>=0?$currentOrd-1:0).");
	meta->setCMRoot(cm);");	
	
	if($meta['has_any'])
	{
		$needsContents = true;
		echoCode("
	meta->setAllowsAny(true);");
	}
	//if there is more than one kind of sub-element
	//_contents keeps an order for those sub-elements
	if($meta['hasChoice']||$needsContents)
	{
		//Ordered list of sub-elements
		echoCode("
	meta->addContents({$meta_prefix}OffsetOf($shortname,_contents));
	meta->addContentsOrder({$meta_prefix}OffsetOf($shortname,_contentsOrder));");
		if($choiceNum>0) 
		echoCode("
	meta->addCMDataArray({$meta_prefix}OffsetOf($shortname,_CMData),$choiceNum);");
	}
	else if($choiceNum>0) die("die($choiceNum does not match 'hasChoice')");
}

$pretty = 0; //add space for readability

//TAKE CARE OF THE ENUM IF IT HAS ONE!! WHY ARE WE YELLING!?!
if($meta['simple_type']!=NULL)
{	
	$meta2 =& $meta['simple_type']->getMeta();

	//FIX ME:
	//NOT SURE IF THE _type SUFFIX IS USED OR NOT. BUT!!! SUFFIXES ARE UNRELIABLED
	if(!empty($meta2['enum'])&&!$meta2['useConstStrings'])
	{
		die('die(Is this COLLADA 1.4.1??)');
		
		if(!$pretty++) echo "\n";
		$type = ucfirst($meta2['type']);
		echoCode("

	//ENUM: {$type}_type
	$1AtomicType *type;
	type = new $1EnumType;
	type->_nameBindings.append(\"{$type}_type\");
	(($1EnumType*)type)->_strings = new $1StringRefArray;
	(($1EnumType*)type)->_values = new $1EnumArray;",$meta_prefix);
		$type = strtoupper(asFriendlyEnum($type));
		foreach($meta2['enum'] as $enum=>$ea) 
		echoCode("
	(($1EnumType*)type)->_strings->append(\"$enum\");
	(($1EnumType*)type)->_values->append({$type}_".strtr($enum,'.-','__').");",$meta_prefix);	
		echo "
	{$meta_prefix}AtomicType::append(type);";
	}
}

//NOTE: following NOTE is never the case???
//NOTE: special casing any element with 'mixed' content model to ListOfInts type _value
if((!empty($meta['content_type'])||$meta['mixed'])&&!$meta['abstract'])
{
	if(!$pretty++) echo "\n";
	if($meta['mixed']) die('die(Is this COLLADA 1.4.1??)');
	$content_type = $meta['mixed']?'ListOfInts':$meta['content_type'];	
	$preType = getFriendlyType($content_type);
	$Array = @$typemeta[$content_type]['isArray']?'Array':'';
	echoCode("
	//Add attribute: _value
	{
		$1MetaAttribute *ma = new $1Meta{$Array}Attribute;
		ma->setName(\"_value\");
		ma->setType($1.getAtomicTypes().get(\"$preType\"));	
		ma->setOffset($1OffsetOf($shortname,_value));
		ma->setContainer(meta);
		meta->appendAttribute(ma);
	}",$meta_prefix);
}
if($meta['useXMLNS'])
{
	if(!$pretty++) echo "\n";
	echoCode("
	//Add attribute: xmlns
	{
		$1MetaAttribute *ma = new $1MetaAttribute;
		ma->setName(\"xmlns\");
		ma->setType($1.getAtomicTypes().get(\"xsAnyURI\"));
		ma->setOffset($1OffsetOf($shortname,_attrXmlns));
		ma->setContainer(meta);
		//ma->setIsRequired(true);
		meta->appendAttribute(ma);
	}",$meta_prefix);
}
foreach($meta['attributes'] as $attr_name=>$ea)
{
	if(!$pretty++) echo "\n";
	$type = $ea['type'];
	$preType = $type; $Name = $attr_name;
	getFriendlyTypeAndName($preType,$Name);
	$Array = @$typemeta[$type]['isArray']?'Array':'';
	echoCode("
	//Add attribute: $attr_name
	{
		$1MetaAttribute *ma = new $1Meta{$Array}Attribute;
		ma->setName(\"$attr_name\");
		ma->setType($1.getAtomicTypes().get(\"$preType\"));	
		ma->setOffset($1OffsetOf($shortname,_attr$Name));
		ma->setContainer(meta);",$meta_prefix);		
	if(isset($ea['default'])) 
	echoCode("
		ma->setDefaultString(\"{$ea['default']}\");");
	if(!empty($ea['use'])) 
	echoCode("
		ma->setIsRequired(".($ea['use']=='required'?'true':'false').');'); 
	echoCode("
		meta->appendAttribute(ma);
	}");	
}
echo "		
	return meta;
}
";
foreach($meta['inline_elements'] as $ea)
if(!$ea['complex_type']||$ea['isRestriction']||$ea['isExtension'])
echo applyTemplate('classes-cpp',$ea);

?>