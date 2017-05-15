<?php
/*
 * Copyright 2006 Sony Computer Entertainment Inc.
 *
 * Licensed under the MIT Open Source License, for details please see licensel.txt or the website
 * http://www.opensource.org/licenses/mit-license.php
 *
 */

$compare = $meta['context'];
$shortname = $meta['element_name'];
if($synth=!empty($meta['isSynth']))
{
	$longname = getIntelliSenseName($meta);
}
else $longname = getScopedClassName($meta);

global $global_parents, $global_children, $abstract; 

if($meta['parent_meta']===NULL)
{
	$childnames =@@ $global_parents[$shortname];		
	if(empty($childnames))
	$childnames = array($shortname=>array($shortname=>&$meta));
}
else $childnames = array($shortname=>&$global_children[$shortname]);

$const = $const_ = $CONST_FORM = ''; const_:
	
ob_start(); echoClassDoxygentation('',$meta,
'WYSIWYG'.(empty($const)?'':', CONST-FORM'));
$class_doxy = ob_get_contents(); ob_end_clean();

$typedef = array(); $count = 0;
if(!$synth) foreach($childnames as $k=>$ea) 
{
	if(count($ea)>1)
	{
		//__using_XSD should not require __alias.
		__aliasSuppress();
		$name = getFriendlyName($const_.$k).'__using_XSD_';
		foreach($ea as $k2=>& $ea2)
		{
			//Hack. === on array isn't like comparing objects			
			if($ea2['context']===$compare) //if($ea2===$meta) 
			$typedef[] = $name.$k2;
			$count++;
		}
		__aliasRestore();		
	}
	else foreach($ea as $k2=>& $ea2)
	{
		//Hack. === on array isn't like comparing objects					
		if($ea2['context']===$compare) //if($ea2===$meta) 
		$typedef[] = getFriendlyName($const_.$k);	
		$count++;
	}unset($ea2);
}
else $typedef[$count++] = getFriendlyName($const_.$shortname);
if($count>1)
{
	__aliasSuppress();
	$structname = '__XSD__'.$const_.getScopedClassName($meta,'__').'__';
	__aliasRestore();		
}
else if(!empty($typedef)) 
{
	$structname = $typedef[0]; 	
	if(!empty($meta['elements'][$structname]))
	$structname.='__recursive';
}
else die('die(relateClass or tpl-ColladaDOM-3.php isn\'t working out)');	

//NOTICE: THESE struct ARE SATISFYING CIRCULAR-DEPENDENCIES WITHOUT
//PREDECLARING THE typedef.

$typedef_ = $count==1&&$structname===$typedef[0]?'':'typedef ';

$names = array(); //__alias
echo $class_doxy,
$typedef_, "struct $structname
:
daeSmartRef<$const::COLLADA_target_namespace::$longname>
{
	COLLADA_DOM_3($structname,struct,daeSmartRef)
";

//this is to avoid child names with __alias.
//IOW, attributes have the lower priority __alias-wise
//(Note: elements could go first, but attributes are in the tag,
//-and so, psychologically, "we" expect them to appear first/at the top.)
ob_start(); 
	
//Elements (out of order)
//abstract elements are included as a form of documentation only
foreach($meta['elements'] as $k=>$ea) //if(empty($abstract[$k]))
{
	$local__ = empty($ea['isLocal'])?'':'local__';
	$type = guessTypeOfElement(@$ea['type'],$k);
	$name = getFriendlyName($k); $names[$name] = 1;
	$head = empty($abstract[$k])?$CONST_FORM:
"	/**DOCUMENTATION-ONLY
	 * THIS ABSTRACT TYPE INSTANCE IS DEFINED IN CASE ITS ANNOTATIONS CAN BE OF USE";	
	echoDoxygen(@$meta['element_documentation'][$k],"\t",
	getFriendlyType($local__.$type),$head);
	$match = explode(':',$type);
	if(!empty($type)&&2===count($match))
	{
		echoCode("
	typedef $1::$2 $name;",
		getFriendlyName($match[0]),getFriendlyName($const_.$match[1]));
	}
	else if(1<count($gc=$global_children[$k])
		  ||1<count(@$global_parents[reset($gc)?key($gc):0])) 
	{			
		if(!$local__)
		$meta2 =@@ $classmeta[$type];
		else $meta2 = NULL;
		if(empty($meta2))
		$meta2 =@@ $meta['classes'][$k];
		if(empty($meta2))
		{
			//SKETCHY: transclusion is involved
			global $classmeta;
			foreach($meta['transcludes'] as $ref=>$tag)
			{
				$test =@@ findLocalType($k,$classmeta[$ref]);
				if(!empty($test))
				{
					if(empty($meta2)) $meta2 = $test;
					else if($test!==$meta2) die("die($k metadata is ambiguous)"); 			
				}
			}
			if(empty($meta2))
			{
				die("die(cannot locate metadata for $k)"); 
			}
		}		
		if(!empty($meta2['isSynth'])) //merged local/simpleType?
		{
			//synthetic types should not exist if there is not a conflict, and
			//they are currently not synthesized if a global type has the name
			echoCode("
	typedef struct $1 $name;",getFriendlyName($const_.$k));
			continue; //HACK: this is a deviant patway, so not indenting below
		}			
		//__using_XSD should not require __alias.
		__aliasSuppress();
		echoCode("
	typedef struct __XSD__$const_$1__ $name;",getScopedClassName($meta2,'__'));
		__aliasRestore();
	}
	else if(empty($const)) 
	{
		$s = getFriendlyName($k);
		if(!empty($classmeta[$type]['elements'][$name]))
		$s.='__recursive';
		echoCode("
	typedef struct $s $name;");
	}
	else echoCode("
	typedef struct $1 $name;",getFriendlyName($const_.$k));
}
$out_of_order = ob_get_contents(); ob_end_clean();

//Attributes/Value
$attribs = false;
$vattribs = array();
if($type=$meta['content_type'])
$vattribs['value'] = array('type'=>$type);
//the _ should prevent the need for keyword style aliases 
attributes:
foreach($vattribs as $k=>$ea)
{
	$type = $ea['type'];
	echoDoxygen(@$ea['documentation'],"\t",getFriendlyType($type),$CONST_FORM);
	$name = strtr($k,':.-','___');
	$match = explode(':',$type);	
	$ext = 2===count($match)?1:0;	
	$match = explode('_',strtr($match[$ext],':.-','___'));	
	$match2 = explode('_',$name);
	
	if($match[0]==end($match2))
	if(count($match)>1||count($match2)>1) 
	array_shift($match);	
	//substr/find would be less extravagant. oh well
	$name = implode('_',array_merge($match2,$match));
	$alias =& $names[$name];
	if(!empty($alias))
	{
		$name.='__alias'; if($alias++>2) $name.=$alias;
	}
	
	$type = getFriendlyType($type);
	if($ext===0) echoCode("
	typedef $const::COLLADA_target_namespace::$type $name;");
	else echoCode("
	typedef $const$type $name;"); //todo? import namespaces
}
if(!$attribs)
{
	$attribs = true; $vattribs = $meta['attributes']; 
	goto attributes;
}

//Elements
echo $out_of_order;

if(!empty($typedef_))
echo $class_doxy, //$indent, 
//EVENTHOUGH THIS CAN GET VERY, VERY LONG, DON'T BREAK IT UP OVER MULTIPLE LINES,
//-BECAUSE THE IDE-TOOLTIPS WON'T DISPLAY THE COMMENT/DOCUMENTATION FOR THEM ALL.
'}', implode(', ',$typedef), ";
";
else echo "};\n";

if(empty($const))
{
	$const = 'const ';
	$const_ = 'const_';
	$CONST_FORM = "\t/**CONST-FORM"; goto const_;
}

foreach($meta['classes'] as $k=>$ea)
if(empty($ea['isSynth']))
echo applyTemplate('ColladaDOM-3',$ea);	

?>