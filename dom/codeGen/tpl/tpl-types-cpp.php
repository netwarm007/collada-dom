<?php
/*
 * Copyright 2006 Sony Computer Entertainment Inc.
 *
 * Licensed under the MIT Open Source License, for details please see license.txt or the website
 * http://www.opensource.org/licenses/mit-license.php
 *
 */

if(!inline_CM)
{
	echoCode("
	$copyright

	#include \"$1Types.h\"
	",$prefix);

	echoCode("
COLLADA_(namespace)
{
    COLLADA_($target_namespace,namespace)
    {//-.
//<-----'
"	);
}
else if($meta==NULL) //HACK: return on CPP file
{
	echo 
"//Generator inline_CM==true put class definitions in their h files.";
	return;
}
else if($meta!='inline') die('die(unexpected behavior in tpl-types-cpp.php)');

//DON'T KNOW WHY return; DOESN'T PREVENT echoType FROM BEING REDECLARED???
if(!inline_CM||$meta!=NULL)
{	
	function echoType($meta,$nl,$at)
	{
		$r =@@ $meta['restrictions'];
		if(empty($r)){ echo $nl, $at, ';'; return; }	
		echo $nl, 'r = ', $at, '.getRestriction();';
		$min =@@ $r['minLength']; if(!isset($min)) $min = 0;
		$max =@@ $r['maxLength']; if(!isset($max)||$max==unbounded) $max = -1;
		if($min!=0||$max!=-1) 
		{
			echo $nl, 'r->addLength(', $min;
			if($min!=$max) echo ',', $max; echo ");";		
		}	
		$min =@@ $r[$minIn='minInclusive']; if(!isset($min)) $min =@@ $r[$minIn='minExclusive'];
		$max =@@ $r[$maxIn='maxInclusive']; if(!isset($max)) $max =@@ $r[$maxIn='maxExclusive'];
		if(isset($min)) echo $nl, 'r->add', ucfirst($minIn), '("', $min, '");';
		if(isset($max)) echo $nl, 'r->add', ucfirst($maxIn), '("', $max, '");';
	}
	
	//This is a callback, called via uasort($renum,'re');
	function re($a,$b){ return $a['value']-$b['value']; }
}
 
global $COLLADA_DOM_GENERATION;
global $root_xmlns, $root_version;
echoCode("
#ifndef COLLADA_DOM_LITE
inline ::COLLADA::XS::Schema &__XS__()
{
	static struct schema : XS::Schema
	{
		schema():XS::Schema(\"$root_xmlns\",\"$root_version\")
        {//-.
	//<-----'
	assert($COLLADA_DOM_GENERATION==getGeneration());		
	setAgent(::ColladaAgent);
	");
	echo 
"	XS::Restriction *r;";

//NOTE: ADDING PREFIXES TO MATCH tpl-classes-cpp
//(AND ALSO FOR FUTURE MIXED NAMESPACE SCENARIOS)
foreach($typemeta as $type=>$meta)	
if(!empty($meta['enum'])&&!$meta['useConstStrings'])
{
	if(2===$COLLADA_DOM)
	{
		//THIS IS SO COMPLICATED TO NOT WANT TO EXPLAIN IT
		//BASICALLY AT THIS POINT UNION TYPES APPEAR TO BE
		//ENUM TYPES, AND UNION TYPES USE THE _type ENDING
		//(see the Note in the C-commented-out UNION code)
		//NEW: now normalizing _type to _enum because 1.4 has _type on some enums
		$preType = $prefix.ucfirst(asFriendlyType($type));		
	}
	else $preType = getFriendlyType($type);
		
	$renum = $meta['enum'];
	$first = reset($renum);
	$value = isset($first['value']);
	$autovalue = 0;
	if($value) uasort($renum,'re');
	$base_type = "\"{$meta['base']}\",\"$type\"";		
	$e_e = $value&&$renum!==$meta['enum']?'e,re':'e,e';
	echo "
	//ENUM: $preType
	{ 
		static const daeClientEnum e[] = 
		{"; 
	if($e_e!='e,e')
	{	
		foreach($renum as $enum=>$ea) echo "
		{ \"$enum\",{$ea['value']} },"; 
		echo "
		};
		static const daeClientEnum re[] = 
		{"; 
	}
	foreach($meta['enum'] as $enum=>$ea) echo "
		{ \"$enum\",",$value?$ea['value']:$autovalue++," },";
	echo "
		};";				
	echoType($meta,"
		","addType<$preType>($e_e,$base_type)");	
	echo "
	}";
}/*C-COMMENTED OUT IN THE ORIGINAL CODE??
//NOTE: seems to be commented out because
//tpl-types-cpp.php populates the ['enum']
//array, and so its if-statement holds true
else if($meta['union_type']) //union type
{ 
	die('die(Is this COLLADA 1.4.1??)');
	//NEW: now normalizing _type to _enum because 1.4 has _type on some enums
	$type = asFriendlyType($type); $preType = $prefix.ucfirst($type);
	echoCode("
	//ENUM: $preType
	type = new $1EnumType;
	type->_nameBindings.append(\"$preType\");
	((daeEnumType*)type)->_strings = new $1StringRefArray;
	((daeEnumType*)type)->_values = new $1EnumArray;");
	$type = strtoupper(asFriendlyEnum($type)); 
	foreach(explode(' ',$meta['union_members']) as $ea) 
	if(@!empty($typemeta[$ea]['enum']))
		foreach($typemeta[$ea]['enum'] as $enum=>$ea2) echoCode("
	((daeEnumType*)type)->_strings->append(\"$ea2\");
	((daeEnumType*)type)->_values->append({$type}_".strtr($enum,'.-','__').");");	
	echo "
	xs->addAtomicType(type);";
}*/
else if(empty($meta['useConstStrings'])) //standard typedef
{	
	if(strpos($type,':')!==false) 
	{
		//Hack. omit unused TypeMeta::generateXMLSchemaTypes
		if(!$meta['inSchema'])
		continue;
	}
		
	if($base=$meta['base']?:$meta['itemType'])
	$base = "\"$base\",";	
	$preType = getFriendlyType($type);
	echoType($meta,"
	","addType<$preType>($base\"$type\")");
}	

echo "	
	//-------.
		}//<-'		
	}singleton; return singleton;
}
#endif //COLLADA_DOM_LITE";
if(!inline_CM) echo "


}} //COLLADA_($target_namespace,namespace)
/*C1071*/
";

?>

