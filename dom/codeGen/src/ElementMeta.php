<?php
/*
 * Copyright 2006 Sony Computer Entertainment Inc.
 *
 * Licensed under the MIT Open Source License, for details please see license.txt or the website
 * http://www.opensource.org/licenses/mit-license.php
 *
 */

class ElementMeta //antipattern
{
	var $name, $doc, $requiresClass;
	
	var $bag; //antipattern	
	function& getMeta(){ return $this->bag;	}
	function ElementMeta(& $global_elements)
	{
		//TODO: WHY ARE THESE NOT VARIABLES?
		$bag = array(
		'context'				=>array(),
		//This is the "value" simple-type member.
		//It had be used to name a C++ base class.
		//flattenInheritanceModel is converting it.
		'content_type'			=>'',			
		//This/content_type/mixed are mutually exclusive
		'content_model'			=>array(),
		//This/content_type/content_model are mutually exclusive
		'mixed'					=>false,
		//no longer uses country code
		'documentation'			=>'', //array()
		'element_name'			=>'',
		'attributes'			=>array(),		
		//array of <xs:element> attributes (and also isPlural)
		//Note: the pre-2016 generator calls this 'element_attrs',
		//-and in it, 'elements' is identical to that array's keys,
		//-so instead they're merged here.
		'elements'				=>array(),				
		'element_documentation'	=>array(),
		//this was 'ref_elements', but #include is its purpose
		'#include'				=>array(),
		//'classes' replaces 'inline_elements'
		//Degenerate name/type classes are discarded.
		'classes'				=>array(), //'inline_elements'		
		//NOTE: COLLADA 1.4.1 alone uses abstract/substitution
		'abstract'				=>false,
		'transcludes'			=>array(), //'groupElements'
		'substitutionGroup'		=>'',				
		//'hasChoice'				=>false, //UNUSED
		//NEW: Replacing 'complex_type',
		//'isExtension' & 'isRestriction',
		//where complex_type meant not a simple-type wrapper.		
		'requiresClass'			=>false,
		'base_type'				=>'', //extension/restriction/?
		//NOTE: 1.5.0 has <author_email>
		//NULL doesn't show up in debugger
		//This is a TypeMeta used for <xs:simpleContent> with a
		//local definition. The COLLADA schema have not of this.
		//'simple_type'			=>NULL,
		'parent_meta'			=>NULL,		
		//NEW: see isGroup, etc. below. Replacing 'isGroup' and so on.
		'flags'					=>0,		
		);

		$this->bag =& $bag;
		$this->bag['global_elements'] =& $global_elements;
	}
	//NEW: dispensing with some array keys. These are DAEP.h values
	const isGroup = 8, isAbstract = 16, hasAny = 32, hasAnyAttribute = 64, hasAll = 128;

	function transcludeRef($ref) //TODO: documentation tags
	{
		$this->bag['transcludes'][$ref] = 'TRANSCLUDED';
	}
  
	function setSubstitutionGroup($subGroup)
	{
		$this->bag['substitutionGroup'] = trim($subGroup);
	}

	function setRequiresClass()
	{
		$this->bag['requiresClass'] = true;
	}

	function setAbstract()
	{
		$this->bag['flags']|=self::isAbstract;
		
		if(empty($name)) die('die(empty($name))');
		//this assumes that abstract is for substitutionGroups
		//and so all children and types will have global names
		global $abstract; $abstract[$name] = true;
	}

	function setHasChoice() //UNUSED
	{
		die('setHasChoice()'); //$this->bag['hasChoice'] = true;
	}

	function setIsGroup()
	{
		$this->bag['flags']|=self::isGroup;
	}

	function setMixed()
	{
		$this->bag['mixed'] = true;
	}

	function setContentType($type)
	{
		//NOTE: THIS CAN BE AN ELEMENT
		//IT'S LATER CONVERTED TO A SIMPLE-TYPE
		//if type is non-empty, set it
		//if(preg_match("/[^\s]+/",$type))
		{
			//$type = trim($type); //Really??
			assert(!empty($type));
			$this->bag['content_type'] = $type;
			includeTypeInSchema($type); //HACK			
		}
	}

	function setName($name)
	{
		$this->bag['element_name'] = $name;		
		$this->name = $name; //UNUSED: watch?	
	}

	function setHasAny()
	{
		$this->bag['flags']|=self::hasAny;
	}
	
	function setHasAnyAttribute()
	{
		$this->bag['flags']|=self::hasAnyAttribute;
	}
	
	function setHasAll()
	{
		$this->bag['flags']|=self::hasAll;
	}

	function setDocumentation($doc)
	{
		$this->doc.=$doc; //UNUSED: watch?
		$this->bag['documentation'] = trim($doc); //['en']
	}

	function setAppInfo($ap)
	{
		//NOTE: enable-xmlns may be COLLADA's schema quirk.
		if(trim($ap)=='enable-xmlns')
		{
			//use xmlns pseudo-attribute
			//THIS IS HOW THIS HAS HISTORICALLY WORKED
			//DAEP:Element CAN PROBABLY USE AN xmlns MEMBER
			$a = array('name'=>'xmlns','type'=>'xs:anyURI');
			$a['documentation'] = 'This element may specify its own xmlns.';
			$this->bag['attributes']['xmlns'] = $a;
		}
		else die("die(unhandled xs:appinfo: $ap)");
	}

	function setContext($context)
	{
		$this->bag['context'] = $context;
	}

	//HACK $add=false when transcluding group content.
	//returns $name, so it doesn't have to be prefetched
	function addElement($e, $add=true)
	{
		$attribs = array();		
		$ref_element = true;
		$attribs = $e->getAttributes();
		if($ref_element=!isset($attribs['name']))
		{
			if(@!empty($attribs['ref'])) $name = $attribs['ref'];		
		}
		else $name = $attribs['name'];
		
		$el =& $this->bag['elements'][$name]; if(isset($el))
		{
			//this doesn't consider transcluded local types
			//those are caught by tpl-ColladaDOM-3.php though
			if(isset($attribs['type'])&&$attribs['type']!=$el['type'])
			die('die(Element has ambiguous type; This is not implemented.)');				
			
			return $name; //GO NO FURTHER IF A CHILD NAMED $name EXISTS
		}		
		$el = $attribs; //track the attributes on each sub-element
		
		$inc =& $this->bag['#include'];
		
		if(!$ref_element&&$add) //recurse
		{			
			$meta2 =& $e->generate($this->bag['context'],$this->bag['global_elements']);
			
			//this includes simple name/type elements, but they can be discarded			
			$this->bag['element_documentation'][$name] = $meta2['documentation'];
			
			//HACK. historically this is how this worked
			$inc = array_merge($inc,$meta2['#include']);			
			//NEW: DISCARD DEGENERATE NAME/TYPE ELEMENTS
			if($meta2['requiresClass']||empty($attribs['type']))
			{				
				$el['isLocal'] = true;
				$meta2['parent_meta'] =& $this->bag;
				$this->bag['classes'][$name] =& $meta2;						
				//does local defaults where typically deriving from simple types only
				//FYI: it's nonobvious globals have defaults. An O'Reilly book says yes			
				$def =@@ $el['default']; if(!empty($def)) $meta2['default'] = $def;
				
				//EXPERIMENTAL
				//this is an effort to merge local/simple types
				$s = $meta2['content_type']; if(!empty($s)&&1)
				{
					//REMINDER 
					//this contradicts the ordering of the schema
					//PHP will not have the array itself as a key
					if(!empty($meta2['attributes']))
					{
						$k =& $meta2['attributes']; 
						foreach($k as& $ea) ksort($ea); ksort($k);
						unset($ea);						
						$s.=serialize($k);						
					}unset($k); global $synthetics;					
					$synthetics[$name][$s][] =& $meta2;					
				}
			}
		}
		else
		{	
			if($add) $inc[] = $name;
			
			$doc =& $this->bag['element_documentation'][$name];
			if(empty($doc)) $doc = '';
			
			//check for documentation
			$a = $e->getElementsByType('xsAnnotation');
			if(!empty($a))
			{
				$d = $a[0]->getElementsByType('xsDocumentation');
				if(!empty($d)) $doc.=$d[0]->get();				
			}
		}
		return $name;
	}

	function addAttribute(& $a)
	{
		$name = ''; $a_list = array();

		foreach($a->getAttributes() as $k=>$ea)
		{
			$a_list[$k] = $ea;
			if($k=='name')
			{
				$name = $ea;
			}
			else if($k=='ref')
			{				
				$name = $ea;
				//printf( "found an attribute ref for ". $name ."\n"); 
				/*THIS is pre 2.5 code. Whatever it's doing, the keys
				//must be pristine. It's causing xml:base to have the
				//wrong name under <COLLADA>.
				if(strpos($name,':')!==FALSE)
				{
					$name[strpos($name,':')] = '_';
					//printf( "changed : to _ for ". $name ."\n" );
					$a_list['type'] = 'xs:anyURI';
				}*/if(strpos($name,':')!==false) //TRYING THIS INSTEAD
				{
					$a_list['type'] = $ea;		
					
					includeTypeInSchema($name); //HACK
				}
			}
			else if($k=='type') includeTypeInSchema($ea); //HACK
		}
		//check for documentation
		$e = $a->getElementsByType('xsAnnotation');
		if(!empty($e))
		{
			$d = $e[0]->getElementsByType('xsDocumentation');
			if(!empty($d)) $a_list['documentation'] = $d[0]->get();			
		}

		$this->bag['attributes'][$name] =& $a_list;

		//echo "adding attribute ". $name ."\n";
	}

	//0 is not used any longer, as it is error-prone/had caused a major bug in the past
	//NEW: these are to make code more clear, but also to catch problems resulting from future changes to the CM procedures
	//opening requires closure. continuing is a prefix, in front of a named element. Undecorated (anyCM) is like an element
	const sequenceCMopening = 1, choiceCMopening = 2, groupCM = 3, allCMopening = 4, anyCM = 5, CMclosure = 6;	
	//For elements name is the element name, for sequence name = 1, choice = 2, group = 3, all = 4, any = 5, end = 6
	function& addContentModel($name,$minOccurs,$maxOccurs)
	{
		$back =& $this->bag['content_model'][];
		$back = array('name'=>$name,'minOccurs'=>$minOccurs,'maxOccurs'=>$maxOccurs);
		echo "adding content model name: $name minO: $minOccurs maxO: $maxOccurs\n"; return $back;		
	}
	function endContentModel()
	{
		$this->bag['content_model'][] = array('name'=>self::CMclosure,'minOccurs'=>0,'maxOccurs'=>0);
		echo "ending content model\n";
	}
}

?>