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

#include <$1.h>
#include <$1/$1Dom.h>
#include <$2/$2Types.h>
#include <$2/$2$namespace.h>
",$meta_prefix,$prefix);

foreach($typemeta as $type=>$meta) if($meta['isComplex'])
echo "#include <$prefix/$prefix", ucfirst($type), ".h>\n";

//DOES COLLADA-DOM NEED-OR-USE THIS?
echoCode("
void registerDomTypes($1& $2)
{
	$2AtomicType *type = nullptr;
	$2AtomicTypeList &atomicTypes = $2.getAtomicTypes();
",strtoupper($meta_prefix),$meta_prefix);

//NOTE: ADDING PREFIXES TO MATCH tpl-classes-cpp
//(AND ALSO FOR FUTURE MIXED NAMESPACE SCENARIOS)
foreach($typemeta as $type=>$meta)
if(!empty($meta['enum'])&&!$meta['useConstStrings'])
{
	//THIS IS SO COMPLICATED TO NOT WANT TO EXPLAIN IT
	//BASICALLY AT THIS POINT UNION TYPES APPEAR TO BE
	//ENUM TYPES, AND UNION TYPES USE THE _type ENDING
	//(see the Note in the C-commented-out UNION code)
	//NEW: now normalizing _type to _enum because 1.4 has _type on some enums
	$type = asFriendlyType($type); $preType = $prefix.ucfirst($type);
	
	echoCode("
	//ENUM: $preType
	type = new $1EnumType($1);
	type->_nameBindings.append(\"$preType\");
	(($1EnumType*)type)->_strings = new $1StringRefArray;
	(($1EnumType*)type)->_values = new $1EnumArray;"
	,$meta_prefix); 
	//SAME DEAL AS ABOVE
	$type = strtoupper(asFriendlyEnum($type)); 
	foreach($meta['enum'] as $enum=>$ea) echoCode("
	(($1EnumType*)type)->_strings->append(\"$enum\");
	(($1EnumType*)type)->_values->append({$type}_".strtr($enum,'.-','__').");"
	,$meta_prefix);	echoCode("
	atomicTypes.append(type);");
}
else if($meta['isComplex']) //UNUSED?
{
	die('die(Is this COLLADA 1.4.1??)');
	$preType = getFriendlyType($type);
	echoCode("
	//COMPLEX TYPE: $preType
	type = new $1ElementRefType($1);
	type->_nameBindings.append(\"$preType\");
	atomicTypes.append(type);",$meta_prefix);
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
	(($1EnumType*)type)->_strings = new $1StringRefArray;
	(($1EnumType*)type)->_values = new $1EnumArray;"
	,$meta_prefix);
	$type = strtoupper(asFriendlyEnum($type)); 
	foreach(explode(' ',$meta['union_members']) as $ea) 
	if(@!empty($typemeta[$ea]['enum']))
	foreach($typemeta[$ea]['enum'] as $enum=>$ea2) echoCode("
	(($1EnumType*)type)->_strings->append(\"$ea2\");
	(($1EnumType*)type)->_values->append({$type}_".strtr($enum,'.-','__').");"
	,$meta_prefix);	echo "
	atomicTypes.append(type);";
}*/
else if(!$meta['useConstStrings']) //standard typedef
{	
	//HACK? filter out new generateXMLSchemaTypes
	if(strpos($type,':')!==false) continue; 
	//LEGACY? strip _type off nameBindings.append strings
	$preType = getFriendlyType($type);	
	echoCode("
	//TYPEDEF: $preType
	type = atomicTypes.get(\"". //SCHEDULED OBSOLETE
	//special casing urifragment to be a xsURI for automatic resolution
	($meta['isString']&&isDreadedAnyURI($type)?'xsAnyURI'
	:getFriendlyType(!empty($meta['base'])?$meta['base']:$meta['listType']))."\");
	if(type==nullptr){ type = new $1RawRefType($1); atomicTypes.append(type); }
	type->_nameBindings.append(\"$preType\");",$meta_prefix);	
}
echo "}\n\n";

//LEGACY/SCHEDULED OBSOLETE
echoCode("
$1MetaElement *registerDomElements($2 &$1)
{
	$1MetaElement *meta = $prefix$namespace::registerElement($1);
	//Enable tracking of top level object by default
	meta->setIsTrackableForQueries(true);
	return meta;	
}

$1Int DLLSPEC $3TypeCount() 
{
	return $typeID+1; //+1 for xsAny
}
",$meta_prefix,strtoupper($meta_prefix),strtolower($namespace));