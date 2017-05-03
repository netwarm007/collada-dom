<?php
/*
 * Copyright 2006 Sony Computer Entertainment Inc.
 *
 * Licensed under the MIT Open Source License, for details please see licensel.txt or the website
 * http://www.opensource.org/licenses/mit-license.php
 *
 */

if(empty($meta)) //creating the file?
{
	if(!inline_CM)
	echoCode("
$copyright
	

using namespace COLLADA;");
	else echo 
"//Generator inline_CM==true put class definitions in their h files.";
	
	return; //otherwise appending to file...
}
else if($meta['parent_meta']===NULL)
{
	if(!inline_CM) 
	echoCode("
		
		
#include \"$1$2.h\"",
	ucfirst($meta['element_name']));
	else 
	echo 
"#define COLLADA_target_namespace \
COLLADA::$target_namespace
COLLADA_(namespace)
{//-.
//<-'
#ifndef COLLADA_DOM_LITE
";
}
else if(empty($meta['isSynth']))
echo "\n";

if(empty($meta['isSynth']))
$longname = getScopedClassName($meta);
else //__name__
$longname = getIntelliSenseName($meta);
$shortname = getFriendlyType($meta['element_name']);
//NOTE: setName takes this name in case clients are
//relying on it as a default name for many elements
//(this is keeping with the old cleanSchema script)
//1.5 FIX: The actual, global name must be used, or
//the <technique> element won't work as <xs:any> is
//supposed to. The COLLADA PDF is incorrect on this.
$globalName = /*asFriendlyType(*/$meta['element_name']/*)*/;
echoCode("
template<>
COLLADA_(inline) DAEP::Model& DAEP::Elemental
<::COLLADA_target_namespace:: $longname
>::__DAEP__Object__v1__model()const
{
	static DAEP::Model *om = nullptr; if(om!=nullptr) return *om;

	Elemental::TOC toc; daeMetaElement &el = 
	::COLLADA_target_namespace::__XS__().addElement(toc,\"$globalName\");");

//ATTRIBUTES
if(2!==$COLLADA_DOM) 
$nc = $meta['elements']; else $nc = array();
if(!empty($meta['attributes']))
{
	echo "
	XS::Attribute *a;\n";
	if($_attr=2===$COLLADA_DOM?'attr':'')
	$nc2 = array(); else $nc2 = $meta['attributes'];
	foreach($meta['attributes'] as $k=>$ea)
	{
		$type = $ea['type'];
		$clash = getNameClash($nc,$k,'__ATTRIBUTE');
		$name =	$_attr.getFriendlyName($k.$clash);		
		echoCode("
	a = el.addAttribute(toc->$name,\"$type\",\"$k\");");		
		if(isset($ea['default'])) 
		echoCode("
	a->setDefaultString(\"{$ea['default']}\");");
		if(!empty($ea['use']))
		if($ea['use']=='required') //TODO: 'fixed' & 'prohibited'
		echoCode("
	a->setIsRequired();"); 	
	}
}
else $nc2 = array();

//VALUE
if(!empty($meta['content_type']))
{
	$type = $meta['content_type'];	
	$clash = getNameClash($nc,'value','__CONTENT',$nc2);
	echo "
	el.addValue(toc->value$clash,\"$type\")";
	if(!empty($meta['default']))
	echo '.setDefaultString("', $meta['default'], '");';
	else echo ';';
	echo "\n";	
}

if(!empty($meta['content_model']))
{	
	//om needs to be set so the routine is reentrant
	//in cases where there are circular dependencies
	echo "		
	om = &el.getModel();";
	echoContentModelCPP($indent,$meta," return *om;");
	echo 
"}";
}
else echo "		
	om = &el.getModel(); return *om;
}";

foreach($meta['classes'] as $ea)
if(empty($ea['isSynth']))
echo applyTemplate('classes-cpp',$ea);

if(inline_CM&&$meta['parent_meta']===NULL) 
{
	echo "\n", '#endif //!COLLADA_DOM_LITE';
	
	if(2!==$COLLADA_DOM)
	{
		echo "
//------.
    //<-'
	namespace DAEP //WYSIWYG
	{
        COLLADA_($target_namespace,namespace)
        {//-.
//<---------'
";						
		echo applyTemplate('ColladaDOM-3',$meta);	
		echo
"//-----------.
        }//<-'
    }
}";
	}
	else echo "
//---.
}//<-'";
echo "
#undef COLLADA_target_namespace
";
}

?>