<?php

/*
 * Copyright 2006 Sony Computer Entertainment Inc.
 *
 * Licensed under the MIT Open Source License, for details please see license.txt or the website
 * http://www.opensource.org/licenses/mit-license.php
 *
 */

$COLLADA_DOM_GENERATION = 1; //global

////BEGIN CONFIG////
$_globals = array(); 
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
//TODO: these should be arguments
$_globals['meta_prefix'] = 'dae'; //$meta_prefix
$_globals['prefix']		 = 'dom'; //$prefix
//note, this is not meta_namespace
$_globals['namespace'] = 'COLLADA'; //$namespace
$_fsystem['rootdir'] = getcwd().'/../generated/'; 
$_fsystem['include'] = $_fsystem['rootdir'].'dom/'; 
$_fsystem['sources'] = $_fsystem['rootdir'].'src/'; 
$_fsystem['logfile'] = $_fsystem['rootdir'].'gen.log';
if(defined('big_number')) die('die(big_number is defined?)');
//deal with difference in 1.5.0 spec
//THIS IS very specific to the Collada 1.5 schema, and needs to be customizable
//TODO: include a commandline supplied PHP file, or else default to the Collada/C++ file
//THIS included file should also setup TypeMeta::generateXMLSchemaTypes' arguments (see gen.php)
function asFriendlyType($name)
{ return "_type"===substr($name,strlen($name)-5)?substr($name,0,strlen($name)-5):$name; }
function asFriendlyEnum($name) //scheduled obsolete: COLLADA 1.5 backward compatibility
{ return "_enum"===substr($name,strlen($name)-5)?substr($name,0,strlen($name)-5):$name; }
//SCHEDULED OBSOLETE
//arbitrarily large number for unbounded sequences
if(!defined('big_number')) define('big_number',3000);

////END CONFIGURATION /////////////////

$_globals['include_list']	 = array(); //$include_list
//NEW: COLLADA 1.5.0 only has math:math
$_globals['import_list']	 = array(); //$import_list
//SCHEDULED FOR REVIEW--APPEARS DUBIOUS
$_globals['complex_types']	 = array(); //$complex_types
//outputting COLLADA_VERSION/NAMESPACE only
$_globals['constStrings']	 = array(); //$constStrings
//SCHEDULED FOR REMOVAL: outputting typeIDs only
$_globals['elementTypes']	 = array(); //$elementTypes
//UNUSED: not outputting COLLADA_ELEMENT_ string constants
//$_globals['elementNames']	 = array(); //$elementNames
//HACK: needed as no elements include <COLLADA> among their children
//$_globals['elementNames'][] = "COLLADA"; 
$_globals['indent'] = ''; //$indent
$_globals['typeID']	= 0; //$typeID
//NEW/SOON TO BE OBSOLETE
$_globals['deep_type_constant'] = ''; //$deep_type_constant

//arguments and variables are injected
function applyTemplate($template, & $meta=NULL)
{
	global $_globals; 
	//2016: tpl- files can forgo $_globals
	extract($_globals,EXTR_REFS|EXTR_SKIP); 
	global $classmeta, $typemeta; //2 more	
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

/*UNUSED: worth having around?
function printAllSubChildren(& $meta, $prefix, $suffix)
{
	global $classmeta;
	foreach($meta['elements'] as $ea) if(!empty($classmeta[$ea]))
	{
		if($classmeta[$ea]['isGroup'])
		printAllSubChildren($classmeta[$ea],$prefix,$suffix);
		else if(!$classmeta[$ea]['abstract'])
		echo $prefix, "_Meta->children.append(\"", $ea."\");", $suffix, ");\n";		
		foreach($classmeta[$ea]['substitutableWith'] as $ea2)
		echo $prefix, "_Meta->children.append(\"", $ea2, "\");", $suffix, ");\n";
	}
	else
	{
		echo $prefix, $ea, $suffix;
		$type = $meta['element_attrs'][$ea]['type'];
		if(!empty($classmeta[$type])&&$classmeta[$type]['isComplexType'])
		echo ", \"", $type, "\"", ");\n";
	}
}*/

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
	return preg_replace('/\s+/', ' ',$doc); //collapse whitespace
}
function echoDoxygen($doc, $tab='', $see='')
{
	global $_globals; 
	$tabs = $_globals['indent'].$tab;
	$undoc = empty($doc)?'UNDOCUMENTED':0;
	if($undoc) $doc = 
	"The XML schema does not provide documentation in this case.";
	if(!empty($see)) $doc.=' @see '.$see; 	
	echo $tabs, '/**'; if($undoc) echo $undoc; 
	echo "\n";
	$doc = getDocumentationText($doc); //collapse whitespace
	while(!empty($doc))
	if(preg_match("/(.{0,70}[^\s]*)(\s*)/",$doc,$matches))
	{
		//Print blocks of 70 chars thru the next word
		echo $tabs, ' * ', $matches[1], "\n";
		$doc = substr($doc,strlen($matches[0]));
	}echo $tabs, " */\n";
}

////END UTILITY FUNCTIONS////
//tpl-class-def.php has subroutines defined below
//not sure why. getFriendlyType is enjoyed by all

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
function getFriendlyTypeAndName(& $type, & $name) //C identifier mangling
{
	$type = getFriendlyType($type);	$name = ucfirst(strtr($name,':.-','___'));
}
function isDreadedAnyURI($type) //SCHEDULED OBSOLETE
{
	//THIS IS A "FEATURE" HARDCODED INTO THE STATIC PART OF COLLADA-DOM
	//That is A) not worth the trouble here, and B) kind of bewildering
	
	return $type==='urifragment_type' //COLLADA 1.5.0
	||$type==='xs:anyURI'||$type==='URIFragmentType'; //1.4.1
}
function guessTypeOfElement($type,$name) 
{
	global $classmeta;
	//HACK: screens out elements of basic type:
	//<xs:element name="subject" type="xs:string">
	return @!empty($type)&&!empty($classmeta[$type])?$type:$name;	
}

////THE REMAINING SUBROUTINES RIGHTLY BELONG IN class-h-def////
////perhaps it's easier on PHP to have them defined here???////

//USED ONCE: tpl-class-h-def.php
function echoConstructorsCPP($elemName, & $meta, $baseClass)
{
	global $_globals;	
	$meta_prefix = $_globals['meta_prefix'];	
	$db = $meta_prefix; //"homonym"
	$initializers = $baseClass.'('.$db.')';
	if($meta['useXMLNS']) //domCOLLADA/technique
	$initializers.=', _attrXmlns('.$meta_prefix.', *this)';		
	//initialization of attributes
	$closure = function($type) use(& $db) //HACKS?
	{
		if(isDreadedAnyURI($type)) return  $db.',*this)';
		else if($type==='xs:IDREF') return '*this)';
		else if($type==='xs:IDREFS') return 'new xsIDREF(*this))'; return ')';
	};
	if(!empty($meta['attributes']))
	foreach($meta['attributes'] as $attr_name=>$ea)
	$initializers.=', _attr'.ucfirst($attr_name).'('.$closure($ea['type']);	
	//initialization of elements
	$attrs = $meta['element_attrs'];
	foreach($meta['elements'] as $ea)
	{			
		$initializers.=", _elem".ucfirst(strtr($ea,':.-','___'));				
		if($attrs[$ea]['maxOccurs']>1)
		$initializers.='_array';
		$initializers.='()';
	}	
	if(empty($meta['baseTypeViaRestriction']))	
	if(($meta['content_type']!=''||$meta['mixed'])&&!$meta['abstract'])
	$initializers.=', _value('.$closure($meta['content_type']);	
	$destructorBody = '';
	if($meta['hasChoice'])
	$destructorBody = ' '.$meta_prefix.'Element::deleteCMDataArray(_CMData); ';	
	$DB = strtoupper($db);
echoCode("
protected:
	/**
	 * Constructor
	 */
	$elemName($DB &$db) : $initializers{}
	/**
	 * Destructor
	 */
	virtual ~$elemName(){{$destructorBody}}
	/**
	 * Overloaded assignment operator
	 */
	virtual $elemName &operator=(const $elemName &cpy){ (void)cpy; return *this; }
	");
}

//USED ONCE: tpl-class-h-def.php
function echoElementsCPP(& $meta)
{
	if((!empty($meta['elements'])&&!$meta['isRestriction'])||$meta['has_any'])
	echoCode("
protected: //Element$1",count($meta['elements'])>1?'s':'');	
	else return;
	
	global $_globals, $classmeta;	
	
	$needsContents = false;
	foreach($meta['elements'] as $ea)
	{
		$Name = ucfirst($ea); 
		$attrs = $meta['element_attrs'][$ea];	
		$preType = guessTypeOfElement(@$attrs['type'],$ea);
		$Name = $ea; getFriendlyTypeAndName($preType,$Name); 
		echoDoxygen(@$meta['element_documentation'][$ea],"\t",$preType);				
		$arrayOrNot = $attrs['maxOccurs']>1;		
		if($arrayOrNot) $Name.='_array';
		$preType.=($arrayOrNot?"_Array":"Ref");		
		echoCode("
	$preType _elem$Name;");
		if(!empty($classmeta[$ea]))			
		if(!empty($classmeta[$ea]['substitutableWith']))
		$needsContents = true;
	}//$needsContents?
	if($meta['hasChoice']||$needsContents||$meta['has_any'])
	echoCode("
	/**
	 * Used to preserve order in elements that do not specify strict sequencing of sub-elements.
	 */
	$1ElementRefArray _contents;
	/**
	 * Used to preserve order in elements that have a complex content model.
	 */
	$1UIntArray _contentsOrder;",$_globals['meta_prefix']);
	if($meta['hasChoice'])
	echoCode("
	/**
	 * Used to store information needed for some content model objects.
	 */
	$1TArray<$1CharArray*> _CMData;",$_globals['meta_prefix']);	
	echo "\n";
}

//USED ONCE: tpl-class-h-def.php
function echoAccessorsAndMutatorsCPP_attribs(& $meta)
{
	global $_globals, $typemeta;
	
	if($meta['useXMLNS']) //domCOLLADA/technique	
	echoCode("
	/**
	 * Gets the xmlns attribute.
	 * @return Returns a xsAnyURI reference of the xmlns attribute.
	 */
	xsAnyURI &getXmlns(){ return _attrXmlns; }
	/**
	 * Gets the xmlns attribute.
	 * @return Returns a constant xsAnyURI reference of the xmlns attribute.
	 */
	const xsAnyURI &getXmlns()const{ return _attrXmlns; }
	/**
	 * Sets the xmlns attribute.
	 * @param xmlns The new value for the xmlns attribute.
	 */
	void setXmlns(const xsAnyURI &xmlns){ _attrXmlns = xmlns; }
	");
	
	$potential_id_change = function($Name)
	{
		return $Name=="Id"?"
	{
		*($1StringRef*)&_attr$Name = at$Name;		
		if(_document!=nullptr) _document->changeElementID(this,_attrId);
	}":"{ *($1StringRef*)&_attr$Name = at$Name; }";
	};			
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
	$preType &get$Name(){ return _attr$Name; }
	/**
	 * Gets the $name array attribute.
	 * @return Returns a constant $preType reference of the $name array attribute.
	 */
	const $preType &get$Name()const{ return _attr$Name; }
	/**
	 * Sets the $name array attribute.
	 * @param at$Name The new value for the $name array attribute.
	 */
	void set$Name(const $preType &at$Name){ _attr$Name = at$Name; }
	");	//^one example of this is <profile_GLES2 platforms>
		else if(@!$typemeta_type['isString'])
		echoCode("
	/**
	 * Gets the $name attribute.
	 * @return Returns a $preType of the $name attribute.
	 */
	$preType get$Name()const{ return _attr$Name; }
	/**
	 * Sets the $name attribute.
	 * @param at$Name The new value for the $name attribute.
	 */
	void set$Name(const $preType &at$Name){ _attr$Name = at$Name; }	
	"); //add xsString to help with back compatibility?
		else 
		//2016: there was a bug: ucfirst($type)=='urifragment'		
		if(isDreadedAnyURI($type))
		echoCode("
	/**
	 * Gets the $name attribute.
	 * @return Returns a $preType reference of the $name attribute.
	 */
	$preType &get$Name(){ return _attr$Name; }
	/**
	 * Gets the $name attribute.
	 * @return Returns a constant $preType reference of the $name attribute.
	 */
	const $preType &get$Name()const{ return _attr$Name; }
	/**
	 * Sets the $name attribute.
	 * @param at$Name The new value for the $name attribute.
	 */
	void set$Name(const $preType &at$Name){ _attr$Name = at$Name; }	
	/**
	 * codeGen: adding for backward compatibility
	 */
	void set$Name(xsString at$Name){ _attr$Name = at$Name; }
	"); //C-style string	
		else
		echoCode("
	/**
	 * Gets the $name attribute.
	 * @return Returns a $preType of the $name attribute.
	 */
	$preType get$Name()const{ return _attr$Name; }	
	/**
	 * Sets the $name attribute.
	 * @param at$Name The new value for the $name attribute.
	 */
	void set$Name($preType at$Name){$potential_id_change($Name)}
	",$_globals['meta_prefix']); //$potential_id_change outputs $1
	}
}

//USED ONCE: tpl-class-h-def.php
function echoAccessorsAndMutatorsCPP(& $meta)
{
	global $_globals, $classmeta;
	//probably only xs:restriction would fail these
	if(!empty($meta['attributes'])||$meta['useXMLNS']
	||!empty($meta['elements'])||(($meta['content_type']!=''||$meta['mixed'])&&!$meta['abstract']))	
	echo echoCode("
public: //Accessors and Mutators");
	else return;
	
	global $typemeta; //this line was missing until 2016!!
	
	if($meta['isExtension']) //SUBROUTINE
	echoAccessorsAndMutatorsCPP_attribs($classmeta[$meta['base_type']]);	
	echoAccessorsAndMutatorsCPP_attribs($meta);

	$needsContents = false;
	foreach($meta['elements'] as $ea)
	{	
		$attrs = $meta['element_attrs'][$ea];
		$preType = guessTypeOfElement(@$attrs['type'],$ea);
		$Name = $ea; getFriendlyTypeAndName($preType,$Name);		
		$arrayOrNot = $attrs['maxOccurs']>1;		
		if($arrayOrNot) $Name.='_array';
		$preType.=($arrayOrNot?"_Array":"Ref");			
		if($arrayOrNot) //array?
		echoCode("
	/**
	 * Gets the $ea element array.
	 * @return Returns a reference to the array of $ea elements.
	 */
	$preType &get$Name(){ return _elem$Name; }
	/**
	 * Gets the $ea element array.
	 * @return Returns a constant reference to the array of $ea elements.
	 */
	const $preType &get$Name()const{ return _elem$Name; }"); 
		if(!$arrayOrNot) //not an array?
		echoCode("
	/**
	 * Gets the $ea element.
	 * @return Returns a $1SmartRef to the $ea element.
	 */
	const $preType get$Name()const{ return _elem$Name; }",$_globals['meta_prefix']);	 
		//array or not, it doesn't matter
		echoCode("
	/**	
	 * this does not work. Use $1Element::add instead
	 */
	//void set$Name(const $preType &e$Name){ _elem$Name = e$Name; }
	",$_globals['meta_prefix']);	

		if(!empty($classmeta[$ea]))		
		if(!empty($classmeta[$ea]['substitutableWith']))
		$needsContents = true;
	}//$needsContents?
	if($meta['hasChoice']||$needsContents||$meta['has_any'])
	echoCode("
	/**
	 * Gets the _contents array.
	 * @return Returns a reference to the _contents element array.
	 */
	$1ElementRefArray &getContents(){ return _contents; }
	/**
	 * Gets the _contents array.
	 * @return Returns a constant reference to the _contents element array.
	 */
	const $1ElementRefArray &getContents()const{ return _contents; }
	",$_globals['meta_prefix']);	
	
	//THE REMAINING CONCERNS get/setValue
	$content_type = $meta['content_type'];
	if(!empty($meta['baseTypeViaRestriction'])
	||$meta['abstract']||empty($content_type)&&!$meta['mixed'])
	return;	//value-less	

	$preType = getFriendlyType($content_type);	
	if(@$classmeta[$content_type]['isComplexType']) 
	$preType.="Ref";		
	if($meta['parent_meta']!=NULL)
	{
		$content = asFriendlyType($content_type); //HACK
		if(array_key_exists($content,$meta['parent_meta']['inline_elements']))
		$preType = '::'.$preType;			
	}
	//@: missing simple scalar types
	$content_typemeta = @$typemeta[$content_type];
	if(@$content_typemeta['isArray'])	
	echoCode("
	/**
	 * Gets the _value array.
	 * @return Returns a $preType reference of the _value array.
	 */
	$preType &getValue(){ return _value; }
	/**
	 * Gets the _value array.
	 * @return Returns a constant $preType reference of the _value array.
	 */
	const $preType &getValue()const{ return _value; }
	/**
	 * Sets the _value array.
	 * @param val The new value for the _value array.
	 */
	void setValue(const $preType &val){ _value = val; }
	");
	else if(@!$content_typemeta['isString'])
	echoCode("
	/**
	 * Gets the value of this element.
	 * @return Returns a $preType of the value.
	 */
	$preType getValue()const{ return _value; }
	/**
	 * Sets the value of this element.
	 * @param val The new value for this element.
	 */
	void setValue(const $preType &val){ _value = val; }
	"); 
	else //add xsString to help with back compatibility?
	if($content_type==='xs:anyURI')
	echoCode("
	/**
	 * Gets the value of this element.
	 * @return Returns a $preType referemce of the value.
	 */
	$preType &getValue(){ return _value; }
	/**
	 * Gets the value of this element.
	 * @return Returns a constant $preType referemce of the value.
	 */
	const $preType &getValue()const{ return _value; }
	/**
	 * Sets the _value of this element.
	 * @param val The new value for this element.
	 */
	void setValue(const $preType &val){ _value = val; }
	/**
	 * codeGen: adding for backward compatibility
	 */
	void setValue(xsString val){ _value = val; }
	"); 	
	else 
	echoCode("
	/**
	 * Gets the value of this element.
	 * @return Returns a $preType of the value.
	 */
	$preType getValue()const{ return _value; }
	/**
	 * Sets the value of this element.
	 * @param val The new value for this element.
	 */
	void setValue($preType val){ *($1StringRef*)&_value = val; }
	",$_globals['meta_prefix']); 
}

?>
