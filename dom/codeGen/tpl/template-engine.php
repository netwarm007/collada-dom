<?php
/*
 * Copyright 2006 Sony Computer Entertainment Inc.
 *
 * Licensed under the MIT Open Source License, for details please see license.txt or the website
 * http://www.opensource.org/licenses/mit-license.php
 *
 */

$COLLADA_DOM = 3; //2 or 3
$COLLADA_DOM_GENERATION = 1;

define('indent_CM',true); //debugging aid. Probably best for output
define('inline_CM',true); //put classes-cpp template inside class.h

$synthetics = array(); //experimental global

////BEGIN CONFIG////
$_globals = array(); 
//use '> >' here if strict C++98/03 is desired
$_globals['>> '] = '>> '; 
//2016: _globals are extract'ed into templates
//if templates do not require _fsystem is used
//IMPORTANT: 'copyright' appears as $copyright
//AND SO ON. SO DON'T ASSUME THESE ARE UNUSED!
$_globals['copyright'] = "
/*THIS CODE IS PROCEDURALLY-GENERATED. PLEASE EDIT THE GENERATOR; NOT THIS CODE.
 *
 * Copyright 2006 Sony Computer Entertainment Inc.
 *
 * Licensed under the SCEA Shared Source License, Version 1.0 (the \"License\"); you may not use this
 * file except in compliance with the License. You may obtain a copy of the License at:
 * http://research.scea.com/scea_shared_source_license.html
 *
 * Unless required by applicable law or agreed to in writing, software distributed under the License
 * is distributed on an \"AS IS\" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or
 * implied. See the License for the specific language governing permissions and limitations under the
 * License.
 */";
if(2===$COLLADA_DOM)
{
	$_globals['prefix']	= 'dom'; //$prefix
	$_globals['namespace'] = 'COLLADA'; //$namespace
}
else $_globals['prefix'] = 'dom'; //just preventing PHP errors
$_fsystem['rootdir'] = getcwd()."/../generated-$COLLADA_DOM/"; 
$_fsystem['include'] = $_fsystem['rootdir'].'COLLADA/'; 
$_fsystem['sources'] = $_fsystem['rootdir'].'src/'; 
$_fsystem['logfile'] = $_fsystem['rootdir'].'gen.log';

//maybe these should be objects so that __aliasPop() etc.
//can compare their object instance?
global $keywords; $keywords = array( //1 is getAlias 2 was getGlobalType
'int'=>1,'long'=>1,'short'=>1,'bool'=>1,'enum'=>1,'float'=>1,'double'=>1,
'inline'=>1,'typename'=>1,'operator'=>1,
/*use of "local__" makes this impossible
'Object'=>2,'Element'=>2,'Elemental'=>2,'Schema'=>2*/);
//there are numerous utilities below, for manipulating these
global $__alias; $__alias = array(&$keywords);
function getAlias($word)
{
	global $__alias; foreach($__alias as $ea)
	if(1==@$ea[$word]) return $word.'__alias'; return $word;
}

////END CONFIGURATION /////////////////

$_globals['include_guard'] = ''; //$include_guard
$_globals['include_list'] = array(); //$include_list
$_globals['indent'] = ''; //$indent
$_globals['genus'] = 1; //$genus
$_globals['notes'] = 0; //$notes
if(2===$COLLADA_DOM)
{
	$_globals['constStrings'] = array(); //$constStrings	
	$_globals['elementTypes'] = array(); //$elementTypes
}

if(defined('big_number')) die('die(big_number is defined?)');
//SCHEDULED FOR REMOVAL?
//big_number is an arbitrary ordinal multiplier for unbounded sequences.
//
//3000 can't really be raised higher. The theoretical limit is 4096, but 
//then any bounded elements on the side will be squeezed out. This limit
//is because of a 24-bit limit on ordinals. Text-nodes are identified by
//the high 8 bits. 
//
//This will work because 3000*3000 is under 16,777,215.
//<xs:sequence maxOccurs="unbounded">
//<xs:sequence maxOccurs="unbounded" />
//</xs:sequence>
//This won't because (3000+3000)*3000 exceeds 16,777,215.
//<xs:sequence maxOccurs="unbounded">
//<xs:sequence maxOccurs="unbounded" />
//<xs:sequence maxOccurs="unbounded" />
//</xs:sequence>
//NOTE The outer-most "compositor" is now allowed to ignore big_number if
//it is not an <xs:group> element. This relaxes the requirements somewhat.
//
//Possible solutions include: using 64-bit ordinals and/or hand-weighting
//problem content-models. Content nodes are 128-bit on 64-bit systems. To
//use 64-bit ordinals yields 172-bit content nodes. 64-bits would be more
//operations on 32-bit systems.
if(!defined('big_number')) define('big_number',3000);

//these work in accordance with getAlias()
//they should allow for finegrained control of 
//__alias when it comes time to be fully airtight
function __aliasAdd(& $add)
{
	global $__alias; $__alias[] =& $add;
}
function __aliasPop(& $added)
{
	//array !== isn't the same as object !==
	global $__alias; $end =/**&*/ array_pop($__alias);
	if($end!==$added) die('die(__aliasPop())');	
}
function __aliasRemove(& $added)
{
	global $__alias; while(!empty($__alias))
	{
		//array !== isn't the same as object !==
		$end =/**&*/ array_pop($__alias);
		if($end!==$added) $save[] = $end; else break;
	}
	while(!empty($save)) $__alias[] = array_pop($save);
	if($end!==$added) die('die(__aliasRemove())');
}
function __aliasSuppress()
{
	global $__aliasOff, $__alias;
	if(empty($__alias)) die('die(__aliasSuppress())');
	$__aliasOff = $__alias; $__alias = array();
}
function __aliasRestore()
{
	global $__aliasOff, $__alias;
	if(empty($__aliasOff)) die('die(__aliasRestore())');
	$__alias = $__aliasOff; $__aliasOff = NULL;
}

//arguments and variables are injected
function applyTemplate($template, & $meta=NULL)
{
	global $_globals, $COLLADA_DOM; 
	//2016: tpl- files can forgo $_globals
	extract($_globals,EXTR_REFS|EXTR_SKIP);
	//2 more	
	//Should $globa_elements be preferred over $classmeta?
	global $classmeta, $typemeta;
	ob_start();
	include('tpl/tpl-'.$template.'.php');
	$_result = ob_get_contents(); 
	ob_end_clean();
	return $_result;
}

function initGen($file)
{
	global $_fsystem;
	$_fsystem['file_name'] = $file;
	$_fsystem['gen_start_time'] = date("M d Y H:i:s");	

	//Verify target dirs exist, create if not
	makeGenDir($_fsystem['rootdir']);
	makeGenDir($_fsystem['include']);
	makeGenDir($_fsystem['sources']);
	
	//Start buffering output
	ob_start();
}

function makeGenDir($dir)
{
	//NOTICE: recursive form of mkdir is too fraught with danger to be of use here
	if(!is_dir($dir)&&!mkdir($dir)) die("die(Could not create directory $dir\n)");
}

function cleanupGen()
{
	global $_fsystem;
	//Get output buffer
	$_result = ob_get_contents();
	ob_end_clean();
	//Assemble report
	ob_start();
	global $COLLADA_DOM_GENERATION;
	$banner = "
========================================
      COLLADA_DOM_GENERATION = $COLLADA_DOM_GENERATION
----------------------------------------
XSD file:     {$_fsystem['file_name']}
Start time:   {$_fsystem['gen_start_time']}
End time:     ".date("M d Y H:i:s")."
----------------------------------------";
	echo "$banner		

$_result

end
";
	file_put_contents($_fsystem['logfile'],ob_get_contents());
	ob_end_clean();
	$realdir = realpath($_fsystem['rootdir']);
	echo "
Generation complete...$banner	
Configured root directory:
$realdir";
}

//NEW: this is to bring sanity to the C++ output 
//(originally it was messily cobbled together in no particular way)
function echoCode($code)
{
	global $_globals; 
	$tabs = $_globals['indent'];
	$argc = func_num_args();
	$rep = array(); $sub = array();	
	for($i=1;$i<$argc;$i++)
	{ $rep[] = "$$i"; $sub[] = func_get_arg($i); }	
	$lines = explode("\n",str_replace($rep,$sub,$code));
	if(!empty($lines[0])) die("die(for consistenty begin on the next line/rtrim)");
	$linec = count($lines);	
	for($i=1;$i<$linec;$i++) echo $tabs, $lines[$i], "\n";
}
function getDocumentationText($doc) //may need to do more
{
	//<xs:documentation> are full of double spaces/tabs etc.
	//$doc = str_replace(array("\n","\t"),array(' ',''),$doc);
	$doc = trim(preg_replace('/\s+/',' ',$doc)); //collapse whitespace	
	return iconv('UTF-8','ASCII//TRANSLIT',$doc); //1.4 has pretty quotes
}
function echoDoxygen($doc, $tab='', $see='', $tags='')
{
	global $_globals; 
	$tabs = $_globals['indent']; if('*'!=$tab) $tabs.=$tab; 
	
	$undoc = empty($doc)?'UNDOCUMENTED':'';
	if($undoc) $doc = 
	"The XSD schema does not provide documentation in this case.";	
	if($tags) //TRANSCLUDED
	{
		//The caller has to handle $indent becuase $tab=='*' doesn't
		//put a newline afterward.
		echo /*$tabs,*/ $tags; 
		if($undoc&&preg_match("/[A-Z]+/",$tags))
		echo ', '; 
	} 
	else echo $tabs, '/**'; if($undoc) echo $undoc; 
	echo "\n";
	$doc = getDocumentationText($doc); //collapse whitespace
	while(!empty($doc))
	if(preg_match("/(.{0,70}[^\s]*)(\s*)/",$doc,$matches))
	{
		//Print blocks of 70 chars thru the next word
		echo $tabs, ' * ', $matches[1], "\n";
		$doc = substr($doc,strlen($matches[0]));
	}
	if(!empty($see)) 
	echo $tabs, ' * @see XSD @c ', $see, "\n";	
	echo $tabs, ' *';
	if('*'!=$tab) echo "/\n";
}
function echoClassDoxygentation_synth($meta,$tags)
{
	global $synthetics;
	$alike = reset($synthetics[$meta['element_name']]);
	foreach($alike as $k0=>$meta)	
	if(!empty($meta['documentation'])) 
	break;	
	if($tags) $tags.=', '; $tags.='SYNTHESIZED';
	//DOCUMENTATION //'*' asks to leave open
	echoDoxygen($d=$meta['documentation'],'*'
	,getScopedClassName($meta),'/**'.$tags); 	
	foreach($alike as $k=>$meta) if($k!==$k0)	
	{
		$doc = $meta['documentation'];
		$see = getScopedClassName($meta);
		if($d!=$doc)
		{
			echoDoxygen($d=$doc,'*'
			,$see,"\n * BINARY-COMPATIBLE LOCAL-SIMPLE-TYPE");					
		}
		else echo ' @see XSD @c ', $see, "\n *";
	}echo "/\n";
}
function echoClassDoxygentation($indent,$meta,$tags='')
{
	//NEW: sow the local typedef name since getIntelliSenseName()
	if(NULL!==$meta['parent_meta']||'WYSIWYG'==substr($tags,0,7))
	{
		//$indent narrows documentation of the typdef that appear
		//inside of the classes that originally defined the synth
		if(!empty($meta['isSynth'])&&empty($indent))
		return echoClassDoxygentation_synth($meta,$tags);
		else
		$see = getScopedClassName($meta);		
	}
	else $see = '';
	
	//DOCUMENTATION //'*' asks to leave open
	echoDoxygen($meta['documentation'],'*',$see,$indent.'/**'.$tags); 
	global $classmeta;
	foreach($meta['transcludes'] as $ref=>$tag)
	echoDoxygen($classmeta[$ref]['documentation'],'*'
	,getFriendlyType($ref),"\n$indent * $tag");
	echo "/\n";
}
function echoInclude(&$list,$h)
{
	//Check for : is excluding imported types.
	//They must be included or typdef'ined prior.
	//Also, since going from the domCOLLADA.h style, 
	//: is the only character forbidden by filesystems.
	if(!in_array($h,$list)&&strpos($h,':')===false)
	{
		$list[] = $h; 
		echo '#include "', $h, ".h\"\n";
	}
}
function echoInclude_dependents($meta,$top)
{
	foreach($meta['transcludes'] as $k=>$ea)
	if($k!==$top) echo "
#include \"$k.h\"";
	foreach($meta['classes'] as $ea) echoInclude_dependents($ea,$top);
}

function isLocalType($type,$meta)
{
	while($meta!=NULL)
	{
		if(!empty($meta['classes'][$type]))
		{
			return true;
		}		
		$meta = $meta['parent_meta'];
	}
	return false;		
}
function findLocalType($type,$meta)
{
	$cl = $meta['classes'];
	if(!empty($cl[$type])) return $cl[$type];
	foreach($cl as $ea)
	{
		$test = findLocalType($type,$ea);
		if($test!=NULL) return test;
	}
	return NULL;
}

if(2==$COLLADA_DOM)
{	
	//deal with difference in 1.5.0 spec
	//THIS IS very specific to the Collada 1.5 schema, and needs to be customizable
	//TODO: include a commandline supplied PHP file, or else default to the Collada/C++ file
	//THIS included file should also setup TypeMeta::generateXMLSchemaTypes' arguments (see gen.php)
	function asFriendlyType($name)
	{
		//NEW: converting to enum ending
		if("_type"!==substr($name,strlen($name)-5)) return $name; 
		global $typemeta;
		return substr($name,0,strlen($name)-5).(@empty($typemeta[$name]['enum'])?'':'_enum');
	}
	function asFriendlyEnum($name) //scheduled obsolete: COLLADA 1.5 backward compatibility
	{
		return "_enum"===substr($name,strlen($name)-5)?substr($name,0,strlen($name)-5):$name; 		
	}

	function getFriendlyType($type)
	{
		global $_globals; 
		//NEW: supporting import: eg. MathML
		//generalizing from preg_match("/xs\:/")
		$match = explode(':',$type);	
		if(2===count($match)) $type = $match[0].ucfirst($match[1]); 	
		else //hack: local namespace
		$type = $_globals['prefix'].ucfirst(asFriendlyType($type)); 
		return strtr($type,':.-','___'); 
	}
	function getGlobalType($meta,$type)
	{
		//'isSynth' types will apply this unnecessarily,
		//-but it's not worth bothering to rule them out 
		$preType = getFriendlyType($type);	
		if(isLocalType(asFriendlyType($type),$meta))
		$preType = '__namespace__::'.$preType;		
		return $preType;
	}
	function getFriendlyName($name) //C identifier mangling
	{
		return ucfirst(strtr($name,':.-','___'));
	}	
	function getNameClash()
	{
		//JUST PASSHTROUGH, 2.X NAME-CLASHES ARE INEVITABLE
		//The 1.5.0 schema has one nameclash, in skin_type.h
		//but it's avoided since "_array" is used as a suffix
		//1.4.1 may as well, similarly narrowly escaped, if so
		return '';
	}
	function getScopedClassName($meta, $sep='::')
	{
		$longname = $meta['context'];
		foreach($longname as& $ea) $ea = getFriendlyType($ea);
		return implode($sep,$longname);
	}	
}
else //ColladaDOM 3
{
	function asFriendlyType($name)
	{
		return $name;
	}
	function asFriendlyEnum($name)
	{
		return $name;
	}	
	function getFriendlyType($type)
	{
		global $_globals; 
		//NEW: supporting import: eg. MathML
		//generalizing from preg_match("/xs\:/")
		$match = explode(':',$type);	
		if(2===count($match)) 
		return getFriendlyName($match[0]).'::'.getFriendlyName($match[1]);
		else return getFriendlyName($type);
	}
	function getGlobalType($meta,$type)
	{
		//use of "local__" makes this obsolete in this mode
		//global $keywords; 
		$type = getFriendlyType($type);	
		//if(isLocalType($type,$meta)||2==@$keywords[$type])
		//$type = '__namespace__::'.$type;		
		return $type;
	}
	function getFriendlyName($name) //C identifier mangling
	{
		return getAlias(strtr($name,':.-','___'));
	}		
	function getNameClash($a,$name,$o,$a2=array())
	{
		//"value" is always a name-clash, even if there's no value
		return !empty($a[$name])||!empty($a2[$name])
		||($name=='value'||$name=='content')&&$o!='__CONTENT'?$o:'';
	}
	function getScopedClassName($meta, $sep='::')
	{
		$longname = $meta['context']; $l = false;
		foreach($longname as& $ea) if(!$l)
		{
			$ea = getFriendlyType($ea); $l = $sep==='::';
		}
		else $ea = 'local__'.$ea; return implode($sep,$longname);
	}
}
function getFriendlyTypeAndName(& $type,& $name) //C identifier mangling
{
	$type = getFriendlyType($type);	$name = getFriendlyName($name);
}
function guessTypeOfElement($type,$name) 
{
	//trying to preserve imported imported types without
	//relying on the names being the same
	//global $classmeta;
	//HACK: screens out elements of basic type:
	//<xs:element name="subject" type="xs:string">
	//return @!empty($type)&&!empty($classmeta[$type])?$type:$name;		
	global $typemeta;
	return empty($type)||isset($typemeta[$type])?$name:$type;
}
//Note, this is because Microsoft's IntelliSense won't process
//large classes, and as soon as it errors, it's gone TU-wide!!
function getIntelliSenseName($meta)
{
	if(empty($meta['isSynth']))
	{
		$longname = $meta['context'];
		if(1===count($longname)) return $longname[0];
	}//THESE MIGHT REQUIRE __name__alias ON SOME COMPILERS
	else $longname[] = $meta['element_name'];	
	foreach($longname as& $ea) $ea = strtr($ea,'.-','__');
	return '__'.implode('__',$longname).'__';
}	

////THE REMAINING SUBROUTINES RIGHTLY BELONG IN class-h-def////
////perhaps it's easier on PHP to have them defined here???////
////ACTUALLY, THEY SEEM TO BE HERE BECAUSE TEMPLATES CANNOT////
////DEFINE A function WHEN THEY ARE INCLUDED MULTIPLE TIMES////

class TypeOfChild
{
	var $type, $child;
	
	//$inc,$meta is added to support transclusion
	function TypeOfChild($name,$attrs,$inc,$meta)
	{
		global $COLLADA_DOM, $classmeta;
		$this->child = $name; 
		$attr = $attrs[$name];	
		if(2!=$COLLADA_DOM&&!empty($attr['isLocal']))
		{
			$this->type = 'local__'.$name; return;
		}
		$type = guessTypeOfElement(@$attr['type'],$name);
		$this->type = getFriendlyType($type);		 
		//TRANSCLUDED?		
		foreach($inc as $ref=>$tag)
		{
			$test =@@ findLocalType($type,$classmeta[$ref]);
			if(!empty($test))
			{
				$this->type = getScopedClassName($test);
				return;
			}
		}
	}
};
function layoutTOC(& $meta)
{
	global $abstract;
	$lo =& $meta['laid out']; if(!empty($lo)) return $lo;		
	$els = $meta['elements'];	
	$inc = $meta['transcludes'];
	$plural = 1; $single = 0;
	foreach($els as $k=>$ea) 
	if(empty($abstract[$k]))
	if($ea['isPlural']) $plural++; else $single++;		
	$plural+=ceil($single/32); //bitfield for single count bits	
	if($plural%2!==0) $plural++; //even number for alignment	
	$lo = array(); 
	foreach($els as $k=>$ea) 
	if($ea['isPlural']&&empty($abstract[$k]))
	$lo[$plural--] = new TypeOfChild($k,$els,$inc,$meta);				
	$single = -1; 
	foreach($els as $k=>$ea) 
	if(!$ea['isPlural']&&empty($abstract[$k])) 
	$lo[$single--] = new TypeOfChild($k,$els,$inc,$meta);
	return $lo;
}

function echoNoNameCPP($name='extra_schema',$doc='/')
{
	global $COLLADA_DOM;
	if(2==$COLLADA_DOM) $name = 'elem'.ucfirst($name);
	echoCode("
	/**NO-NAMES
	 * These elements are invalid according to the schema. They may be user-defined 
	 * additions and substitutes.
	 *$doc
	DAEP::Child<1,xs::any,_,(_::_)&_::_N> {$name}__unnamed;");
}
function echoAnyEtcCPP()
{
	echoNoNameCPP('any_et_cetera',"	
	 * ANY, DOCUMENTATION-NOT-SUPPORTED
	 * This member is a placeholder for an <xs:any> element in the schema. Feature 
	 * support for <xs:any> is limited. 
	 */");
}
function echoElementsCPP(&$meta,$elementsPtoMs,$N,$any) 
{		
	global $COLLADA_DOM;
	$lo = layoutTOC($meta);
	if(!empty($lo)||$any) echoCode("
public: //Elements");		
		
	//TODO: EVENTUALLY THIS SHOULD BE REPLACED
	//BY SOME ATTRIBUTE DATA-MEMBERS THAT WILL 
	//GET THE ALIGNMENT ON TRACK. IF THERE ARE
	//NO ATTRIBUTES, ALIGNMENT ISN'T NECESSARY
	//aligning contents-arrays on 64bit builds	
	echoCode("
		
	COLLADA_WORD_ALIGN"); //COLLADA_ALIGN(sizeof(_*))
	
	$i = $elementsPtoMs;
	if($_elem=2==$COLLADA_DOM?'elem':'')
	$nc = array(); else $nc = $meta['attributes'];
	foreach($lo as $k=>$ea)
	{
		if($k>1)
		{
			$_ = '_'.$i++;
		}
		else if($k==-1) 
		{
			$_ = '_N'; 
			
			$i-=$elementsPtoMs;
			$singles = count($lo)-$i; $odd = $i&1;
		
			if($i) echo "\n";
			echoCode("
	COLLADA_DOM_N($N) union //NO-NAMES & ONLY-CHILDS
	{",ceil($singles/32)-$odd);
			if($any) echoAnyEtcCPP(); else echoNoNameCPP();		
		}
		$clash = getNameClash($nc,$ea->child,'__ELEMENT');
		$name = $_elem.getFriendlyName($ea->child.$clash);
		echoDoxygen(@$meta['element_documentation'][$ea->child],"\t",$ea->type);		
		echoCode("
	DAEP::Child<$k,$ea->type,_,(_::_)&_::$_> $name;");
	}
	if(empty($singles))
	{ 
		$i-=$elementsPtoMs;
		if($i) echo "\n";
		echoCode("
	COLLADA_DOM_N($N) $1",$i?'//NO-NAMES':'');
		if($any) echoAnyEtcCPP(); else echoNoNameCPP();		
	}
	else echoCode("
	};");		 
	echo "\n";
}

//this is $COLLADA_DOM==2 only
function echoAccessorsAndMutatorsCPP_attribs($meta)
{
	global $_globals, $typemeta;
	foreach($meta['attributes'] as $name=>$a_list)
	{
		$type = $a_list['type']; 		
		$preType = $type; $Name = $name;
		getFriendlyTypeAndName($preType,$Name);
		//@: missing simple scalar types
		$typemeta_type = @$typemeta[$type]; 
		if(@$typemeta_type['isArray'])
		echoCode("
	/**
	 * Gets the $name array attribute.
	 * @return Returns a $preType reference of the $name array attribute.
	 */
	$preType &get$Name(){ return attr$Name; }
	/**
	 * Gets the $name array attribute.
	 * @return Returns a constant $preType reference of the $name array attribute.
	 */
	const $preType &get$Name()const{ return attr$Name; }
	/**
	 * Sets the $name array attribute.
	 * @param at$Name The new value for the $name array attribute.
	 */
	void set$Name(const $preType &at$Name){ attr$Name = at$Name; }
	");	else
		{
			//This so setting a daeURI won't convert to daeURI
			//and then to daeStringRef, which is ambiguous, in
			//addition to multiple string conversion backflips.
			if(@$typemeta_type['isString'])
			$setType = 'daeHashString2';
			else $setType = $preType;
			echoCode("
	/**
	 * Gets the $name attribute.
	 * @return Returns a $preType of the $name attribute.
	 */
	$preType get$Name()const{ return attr$Name; }
	/**
	 * Sets the $name attribute.
	 * @param at$Name The new value for the $name attribute.
	 */
	void set$Name(const $setType &at$Name){ attr$Name = at$Name; }	
	");
		}
	}
}

//this is $COLLADA_DOM==2 only
function echoAccessorsAndMutatorsCPP(& $meta)
{
	echo echoCode("
public: //COLLADA-DOM 2"); //Accessors and Mutators	
		
	global $_globals, $classmeta;	
	global $typemeta; //this line was missing until 2016!!
	
	echoAccessorsAndMutatorsCPP_attribs($meta);

	$lo = layoutTOC($meta);
	foreach($lo as $k=>$ea)
	{	
		$c = $ea->child; 
		$preType = 'dae_Array<'.$ea->type.','.$k.'>';		
		if($k>0) //array?
		echoCode("
	/**
	 * Gets the $c element array.
	 * @return Returns a reference to the array of $c elements.
	 */
	$preType &get$1_array(){ return elem$1; }
	/**CONST-FORM
	 * Gets the $c element array.
	 * @return Returns a constant reference to the array of $c elements.
	 */
	const $preType &get$1_array()const{ return elem$1; }
	",getFriendlyName($c)); 
		else //not an array?
		echoCode("
	/**	
	 * Gets the $c element.
	 * @return Returns a reference to the $c element.
	 */
	$preType &get$1(){ return elem$1; }
	/**CONST-FORM
	 * Gets the $c element.
	 * @return Returns a constant reference to the $c element.
	 */
	const $preType &get$1()const{ return elem$1; }
	",getFriendlyName($c));	 	
	}
	
	$content_type = $meta['content_type'];
	if(!empty($content_type))
	{
		$preType = getGlobalType($meta,$content_type);	
		//@: missing simple scalar types
		$content_typemeta = @$typemeta[$content_type];
		if(@$content_typemeta['isArray'])	
		echoCode("
	/**
	 * Gets the value array.
	 * @return Returns a @c $preType reference of the value array.
	 */
	$preType &getValue(){ return value; }
	/**CONST-FORM
	 * Gets the value array.
	 * @return Returns a constant @c $preType reference of the value array.
	 */
	const $preType &getValue()const{ return value; }
	/**
	 * Sets the value array.
	 * @param val The new value for the value array.
	 */
	void setValue(const $preType &val){ value = val; }
	"); else 
		{
			//This so setting a daeURI won't convert to daeURI
			//and then to daeStringRef, which is ambiguous, in
			//addition to multiple string conversion backflips.
			if(@$content_typemeta['isString'])
			$setType = 'daeHashString2';
			else $setType = $preType;
			echoCode("
	/**
	 * Gets the value of this element.
	 * @return Returns a @c $preType of the value.
	 */
	$preType getValue()const{ return value; }
	/**
	 * Sets the value of this element.
	 * @param val The new value for this element.
	 */
	void setValue(const $setType val){ value = val; }
	"); 
		}
	}
		
	//Reminder: all kinds hold comments and/or processing-instructions.	
	echoCode("
	/**
	 * Gets the contents-array.
	 * @return Returns a reference to the contents-array.
	 */
	daeContents &getContents(){ return content; }
	/**CONST-FORM
	 * Gets the contents-array.
	 * @return Returns a constant reference to the contents-array.
	 */
	const daeContents &getContents()const{ return content; }");		
}

//CONTENT-MODEL OUTPUT
//These are pulled out of tpl-classes-cpp.php so that
//they don't have to be conditionally defined. 
function addPartialCM($out)
{
	$depth = 0; $runs = array();
	foreach($out as $i=>$cm) switch($cm['name'])
	{
	case ElementMeta::allCMopening: 
	case ElementMeta::sequenceCMopening: 

		if($depth>0) $depth++; break;

	case ElementMeta::choiceCMopening: 

		if(0===$depth++) $k = $i+1; break;

	case ElementMeta::CMclosure:

		if(0!==$depth&&0===--$depth) $runs[$k] = $i; 
	} 

	$in = $out; $i = 0; foreach($runs as $k=>$ea)
	{
		$d = genPartials($in,$k,$ea); if(!empty($d))
		{
			if($i===0) $out = array();
			array_merge($out,array_slice($in,$i,$ea-$i),$d);
			$i = $ea;
		}
	}		
	if($i!==0) array_merge($out,array_slice($in,$i,count($in)-$i));

	return $out;
}	
function genPartials($in, $i, $hi)
{
	//EXPERIMENTAL
	//The idea here is to generate synthetic choices that are
	//a "vin diagram" of two or more choices. This is done by
	//copying/merging the choices, and pulling out names that
	//are excluded.
	//This will allow the intelligent insertion APIs to defer
	//the decision making process until there's not ambiguity.
	
	$depth = 0; $cur = 0;
	for($ns=array();$i<$hi;$i++) switch($name=$in[$i]['name'])
	{
	case ElementMeta::allCMopening: 		
	case ElementMeta::choiceCMopening: 
	case ElementMeta::sequenceCMopening: 
		
		$depth++; break;
	
	case ElementMeta::CMclosure:
		
		if(--$depth===0) $cur++; break;
		
	case ElementMeta::groupCM:
		
		global $classmeta;
		$group = $classmeta[$in[$i]['ref']];
		foreach($group['elements'] as $k=>$ea) $ns[$cur][] = $k; 
		
		goto inc;
	
	case ElementMeta::anyCM: 
		
		$ns[$cur][] = 'any_et_cetera__unnamed'; 
		
		goto inc;
			
	default: $ns[$cur][] = $name; 
		
	inc: if($depth===0) $cur++; break;
	}	
	//REMINDER: THIS IS DOUBLE SORTING
	foreach($ns as& $ea) sort($ea=array_unique($ea));
	unset($ea);
		
	$d = array(); //build list of partials
	foreach($ns as $k=>$ea) if(count($ea)>1)	
	{
		$ns2 = $ns; //duplicating "internal pointer" 
		foreach($ns2 as $k2=>$ea2) if(count($ea2)>1)
		{
			if($k!==$k2) 
			{
				$i = implode(' ',array_intersect($ea,$ea2));
				if(!empty($i)) $d[$i][] = $k;
			}
		}	
	}
	if(empty($d)) return $d;
	
	foreach($ns as $k=>$ea) //remove non-partial combinations 
	{
		$i = implode(' ',$ea); if(isset($d[$i])) $d[$i] = NULL;
	}
	
	foreach($d as $ea) if(!empty($ea)) //THIS MUST BE IMPLEMENTED
	{
		//If there are partials, a content model must be built for them
		//using the addPartialCM C++ API. The rest is done. 'd' is used 
		//to house the names that should be excluded by C++ // comments.
		die('die(UNIMPLEMENTED -- COLLADA\'s schemas don\'t have partials.)');		
	}	
	
	return array();
}
function echoGroupCM($indent, $pt,& $group, $nc)
{	
	//*there might be reason to leave the abstract types
	//in, but right now there's no member to be bound to
	global $COLLADA_DOM, $classmeta, $abstract, $subgroups;		
	$_elem=2==$COLLADA_DOM?'elem':'';	
	foreach($group['content_model'] as $ea)
	{
		$name = $ea['name']; if(is_int($name)) 
		{	
			if($name==ElementMeta::groupCM)
			echoGroupCM($indent,$pt,$classmeta[$ea['ref']],$nc);								
		}
		else //map the children to the group's CM
		{
			$el = $group['elements'][$name];						
			if(empty($abstract[$name])) //*
			{
				if(isset($pt[$name])) echo '\\'; //partial?
				$clash = getNameClash($nc,$name,'__ELEMENT');
				echo  "\n", $indent, 
'	.addChild(toc->', $_elem, getFriendlyName($name.$clash), ',"', $name, '")';
			}			
			if(!empty($el['ref'])&&isset($subgroups[$name]))			
			foreach($subgroups[$name] as $k2=>$ea2) if(empty($abstract[$k2])) //*
			{
				if(isset($pt[$k2])) echo '\\'; //partial?
				$clash = getNameClash($nc,$k2,'__ELEMENT');
				echo "\n", $indent,
'	.addChild(toc->', $_elem, getFriendlyName($k2.$clash), ',"', $k2, '")';
			}			
		}
	}		
	//If the groups are out of order in the schema,
	//Then disable echo, and reapply this template.
	$ordMax =& $group['ordMax']; 
	if(!isset($ordMax)) applyTemplate('classes-cpp',$group);
	return $ordMax;
}
function echoContentModelCPP(& $indent,& $meta, $closure_text)
{	
	//*there might be reason to leave the abstract types
	//in, but right now there's no member to be bound to
	global $COLLADA_DOM, $classmeta, $abstract, $subgroups;
	if($_elem=2==$COLLADA_DOM?'elem':'')
	$nc = array(); else $nc = $meta['attributes'];
	
	echo "
	daeCM *cm = nullptr;\n";
	$curCM = array('cm'=>NULL,'ord'=>0);		
	$curOrd = 0;
	$cmStack = array(); //the top of the stack is not the currentCM/Ord
	$push_cm = function($cm) use(&$curCM,&$curOrd,&$cmStack,&$indent)
	{
		//Not doing first level as it's usually a compulsory.
		if(indent_CM&&!empty($cmStack)) $indent.="\t";
		$curCM['ord'] = $curOrd; array_push($cmStack,$curCM);
		$curCM['cm'] = $cm;	$curOrd = 0; 
	};

	$pt = NULL; //addPartialCM	
	$content_model = addPartialCM($meta['content_model']);	
	//NOTE: this is tightly coupled to generateCM_isPlural
	foreach($content_model as $ea)
	{
		$addCM = $pt=@@$ea['d']?'addPartialCM':'addCM';
			
		extract($ea); //extracting $name, $minOccurs & $maxOccurs	
		if($maxOccurs===unbounded) $maxOccurs = -1;
		switch(is_int($name)?$name:-1) //cases must agree with type
		{
		case ElementMeta::sequenceCMopening: //xs:sequence			
			echoCode("
	el.$addCM<XS::Sequence>(cm,$curOrd,$minOccurs,$maxOccurs);");
			$push_cm($ea); break;
			
		case ElementMeta::choiceCMopening: //xs:choice				
			echoCode("
	el.$addCM<XS::Choice>(cm,$curOrd,$minOccurs,$maxOccurs);");			
			$push_cm($ea); break;
			
		case ElementMeta::allCMopening: //xs:all		
			echoCode("
	el.$addCM<XS::All>(cm,$curOrd,$minOccurs,$maxOccurs);");
			$push_cm($ea); break;
		
		case ElementMeta::anyCM: //xs:any 			
			//Hack: 'any' is setup in base-types.php. it shouldn't be.
			echoCode("
	el.$addCM<XS::Any>(cm,$curOrd,$minOccurs,$maxOccurs){$ea['any']};");
			$curOrd++; break;
			
		case -1: //named element string
					
			$el = $meta['elements'][$name];
			//addCM is repeated only because any one can be abstract
			if(empty($abstract[$name])) //*
			{
				if(isset($pt[$name])) echo '//'; //partial?
				$clash = getNameClash($nc,$name,'__ELEMENT');
				echoCode("
	el.$addCM<XS::Element>(cm,$curOrd,$minOccurs,$maxOccurs).setChild(toc->$1,\"$name\");"
				,$_elem.getFriendlyName($name.$clash));
			}
			if(!empty($el['ref'])&&isset($subgroups[$name]))			
			foreach($subgroups[$name] as $k2=>$ea2) if(empty($abstract[$k2])) //*
			{
				if(isset($pt[$k2])) echo '//'; //partial?
				$clash = getNameClash($nc,$k2,'__ELEMENT');
				echoCode("
	el.$addCM<XS::Element>(cm,$curOrd,$minOccurs,$maxOccurs).setChild(toc->$1,\"$k2\");"
				,$_elem.getFriendlyName($k2.$clash));				
			}			
			$curOrd++;
			break;

		case ElementMeta::groupCM: //xs:group

			//There's no way to have C++'s templates inferred this type.
			$preType = getFriendlyType($ref); 
			echo $indent, //not pretty
"	el.$addCM<XS::Group>(cm,$curOrd,$minOccurs,$maxOccurs).setGroup
",			$indent,
"		<::COLLADA_target_namespace:: $preType>()";
			$push_cm($ea); 
			$curOrd = echoGroupCM($indent,$pt,$classmeta[$ref],$nc);
			echo ";\n"; //not pretty
			
			//break; //groupMClosure: //falling-through...
			
		case ElementMeta::CMclosure: //pop the stack
						
			//In this past some CMs were double terminated. Maybe no more?
			if(empty($cmStack)) die('die(extra CMclosure node detected)');
						
			//PREVIOUS $name/maxOccurs			
			extract($cm=$curCM['cm']);
			$curCM = array_pop($cmStack); 
			if(isset($cm['d'])) $pt = NULL;
			
			//FYI: Elsewhere >big_number calls die().
			if(-1===$maxOccurs) $maxOccurs = big_number; //3000			
			else if($maxOccurs>big_number) $maxOccurs = big_number;
						
			if(!empty($cmStack)) 
			{
				echoCode("
	el.popCM<$curOrd,$maxOccurs>(cm);");
				//multiply by the sequence's total number of slots
				$curOrd = $curCM['ord']+$maxOccurs*($curOrd);	
			}
			else //This form relaxes non-group tails. 
			{	echoCode("
	el.addContentModel<$curOrd,$maxOccurs>(cm,toc);$closure_text");
				//Save count for echoGroupCM
				$meta['ordMax'] = $curOrd; 
			}						
			if(indent_CM) $indent = substr($indent,0,-1);
			break;						
		default: die("die(unrecognized CM code: $name)"); 
		}
	}
}

//EXPERIMENTAL
//This outputs the body of the COLLADA-DOM 2.5 __NB__
//struct. They are used by DAEP Concern & DAEP Cognate
//Note, they are not exact, and cannot be. Children and
//attributes are blended together, and the context names
//only go one level deep. It's parent-child relationships
//according to their names and nothing else.
function echoNotebookCPP_class($type,$meta,$inline,$i)
{
	$e = $meta['elements'];
	$a = $meta['attributes'];	
	$ii = isset($inline[$type])?$inline[$type]:array();
	if(empty($e)&&empty($a)&&empty($ii))
	return;
	
	//Note. typedef is used to cover recursive names, and
	//not have to tack __alias onto the inside class name
	//__alias there would be particularly vexing, because
	//they represent members, which would not use __alias
	$name = getAlias(strtr($type,'.-','__'));		
	echoCode("
	typedef union _$i
	{");	
	$alias = array($name=>1);	
	foreach($ii as $k=>$ea) 
	{		
		echoCode("
		struct $1:__<_$i>{};",getAlias(strtr($k,'.:-','___')));
	}
	foreach($a as $k=>$ea) if(empty($ii[$k]))
	{		
		echoCode("
		struct $1:__<_$i>{};",getAlias(strtr($k,'.:-','___')));
	}	
	foreach($e as $k=>$ea) if(empty($a[$k])&&empty($ii[$k]))
	{		
		echoCode("
		struct $1:__<_$i>{};",getAlias(strtr($k,'.:-','___')));
	}	
	echoCode("
	}$name;");
}
function echoNotebookCPP_locate($meta,&$inline)
{
	foreach($meta['classes'] as $k=>$ea)
	{
		$i =& $inline[$k];
		foreach($ea['attributes'] as $k=>$ea2)
		$i[$k] = 1;
		foreach($ea['elements'] as $k=>$ea2)
		$i[$k] = 1;
		echoNotebookCPP_locate($ea,$inline);
	}
}
function echoNotebookCPP()
{	
	global $_fsystem;
	$signed_X = 'signed_'.
	identifyC(substr($_fsystem['include'],strlen($_fsystem['rootdir'])));	
	echoCode("
	template<class C=__NB__> struct __
	{
		typedef C context; 
		typedef __NB__ schema; 
		typedef signed $signed_X;		
	};");
	
	global $classmeta, $typemeta;
	$inline = array();	
	foreach($classmeta as $ea)
	echoNotebookCPP_locate($ea,$inline); //recurse
	$i = 0;
	foreach($classmeta as $type=>$meta)
	echoNotebookCPP_class($type,$meta,$inline,$i++);		
	foreach($inline as $k=>$ea) 
	if(!empty($ea)&&empty($classmeta[$k]))
	{		
		$name = getAlias(strtr($k,'.-','__'));
		//Note. typedef is used to cover recursive names, and
		//not have to tack __alias onto the inside class name
		//__alias there would be particularly vexing, because
		//they represent members, which would not use __alias		
		if(!empty($typemeta[$k]))
		echoCode("
	typedef struct _$i:__<> //Both global simpleType and local complexType?
	{",assert(!isset($ea['schema'])&&!isset($ea['context'])));
		else
		echoCode("
	typedef union _$i
	{");		
		foreach($ea as $k2=>$ea2) 
		echoCode("
		struct $1:__<_$i>{};",getAlias(strtr($k2,'.:-','___')));		
		echoCode("
	}$name;"); $i++;
	}	
	//These are so "DAEP::Cognate" has something to hold onto
	foreach($typemeta as $type=>$meta)
	if(strpos($type,':')==false)
	if(!$meta['isArray']&&empty($meta['enum'])&&!empty($meta['base']))
	{
		if(empty($inline[$type]))
		echoCode("
		struct $1:__<>{};",getAlias(strtr($type,'.-','__')));	
	}
}

//UNUSED "_long" doesn't use the COLLADA_DOM_NOTE macro
//It's a little over twice as long and has the effect of
//a brick wall
function echoNotesCPP_enum($comma,$meta,$space)
{
	$default =@@ $meta['default']; $fixed =@@ $meta['fixed'];	
	$text = $default?:$fixed; if(!empty($text))
	{		
		//this is counting the number of spaces, assuming that
		//if any, they are list separators. 
		$value = 1+substr_count($text,' ');
		$text = $default?'has_default':'is_fixed';
		echo $comma, 'enum{ ', $text, '=', $value, ' };', $space;
	}
}
function echoNotesCPP_long($n,$context,$concern,$meta)
{
	$concern = getAlias(strtr($concern,'.:-','___'));
	echo "
template<> struct __NS__<$n>:DAEP::Note<>{ typedef __NB__::$context::$concern concern; ";			
	echoNotesCPP_enum('',$meta,' '); echo '};';
}
function echoNotesCPP_short($n,$context,$concern,$meta)
{
	$concern = getAlias(strtr($concern,'.:-','___'));
	echo "COLLADA_DOM_NOTE($n,$context::$concern";
	echoNotesCPP_enum(',',$meta,''); echo ")\n";
}
function echoNotesCPP($meta,$notestart,$_No)
{	
	$a = $meta['attributes'];
	$e = $meta['elements'];
	if(empty($e)&&empty($a)) return;	
	
	$context = getAlias(strtr($meta['element_name'],'.-','__'));		
	
	$i = $notestart; 
	foreach($a as $k=>$ea) echoNotesCPP_short($i++,$context,$k,$ea);
	
	//this is where the "value" note should go if one is needed
	//if(!empty($meta['content_type'])) 
	//$i++;
	
	$i = $notestart+$_No+1;
	$lo = layoutTOC($meta); if(!empty($lo))
	{
		foreach($lo as $k=>$ea)
		echoNotesCPP_short($i-$k,$context,$ea->child,$e[$ea->child]);		
	}
}

?>
