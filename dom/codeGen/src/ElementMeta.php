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
	var $name, $doc; //UNUSED: watches?	
	
	var $bag; //antipattern	
	function& getMeta(){ return $this->bag;	}
	function ElementMeta(& $global_elements)
	{
		$bag = array(
		'has_id_attr'			=>false,
		'context'				=>'',
		'content_type'			=>'',		
		'documentation'			=>'', //array()
		'element_name'			=>'',
		'elements'				=>array(),
		'inline_elements'		=>array(),
		'ref_elements'			=>array(),
		//array of <xs:element> attributes
		'element_attrs'			=>array(),
		'attributes'			=>array(),
		//USED: although, COLLADA schema don't do "mixed"
		'mixed'					=>false,
		'complex_type'			=>false,
		//NOTE: COLLADA 1.4.1 alone uses abstract/substitution
		'abstract'				=>false,
		'substitutionGroup'		=>'',
		'substitutableWith'		=>array(),
		'element_documentation'	=>array(),
		//SCHEDULED OBSOLETE?
		//<COLLADA> and <technique> use this
		//however it needs to be converted into an <xs:attribute>
		//the schema itself uses <xs:appinfo>enable-xmlns</xs:appinfo>
		//IS enable-xmlns even legit?! xmlns seems like a universal feature
		'useXMLNS'				=>false,
		'hasChoice'				=>false,
		'group_elements'		=>array(), //UNUSED: of use??
		'isComplexType'			=>false,
		'isGroup'				=>false,
		'isExtension'			=>false,
		'isRestriction'			=>false,
		'base_type'				=>'', //extension/restriction/?
		//NULL doesn't show up in debugger
		'simple_type'			=>NULL,
		'parent_meta'			=>NULL,
		'has_any'				=>false,
		'content_model'			=>array(),
		);

		$this->bag =& $bag;
		$this->bag['global_elements'] =& $global_elements;
	}

	function addGroup(& $e) //UNUSED: of use??
	{
		$this->bag['group_elements'][] = $e->getAttribute('ref');
	}

	function setSubstitutionGroup($subGroup)
	{
		$this->bag['substitutionGroup'] = trim($subGroup);
	}

	function setComplexType($bool)
	{
		$this->bag['complex_type'] = $bool;
	}

	function setAbstract($bool)
	{
		$this->bag['abstract'] = $bool;
	}

	function setHasChoice($bool)
	{
		$this->bag['hasChoice'] = $bool;
	}

	function setIsComplexType($bool)
	{
		$this->bag['isComplexType'] = $bool;
	}

	function setIsGroup($bool)
	{
		$this->bag['isGroup'] = $bool;
	}

	function setMixed($bool)
	{
		$this->bag['mixed'] = $bool;
	}

	function setContentType($type)
	{
		//if type is non-empty, set it
		if(preg_match("/[^\s]+/",$type))
		$this->bag['content_type'] = trim($type);		
	}

	function setName($name)
	{
		$this->bag['element_name'] = $name;		
		$this->name = $name; //UNUSED: watch?	
	}

	function setHasID($bool)
	{
		$this->bag['has_id_attr'] = $bool;
	}
	
	function setHasAny($bool)
	{
		$this->bag['has_any'] = $bool;
	}

	function setDocumentation($doc)
	{
		$this->doc = $doc; //UNUSED: watch?
		$this->bag['documentation'] = trim($doc); //['en']
	}

	function setAppInfo($ap)
	{
		if(trim($ap)=='enable-xmlns')
		{
			//use xmlns pseudo-attribute
			$this->bag['useXMLNS'] = true;
		}
		else die("die(unhandled xs:appinfo: $ap)");
	}

	function setContext($context)
	{
		$this->bag['context'] = $context;
	}

	function addElement(& $e, $context)
	{
		$name = 'undefined';
		$ref_element = false;
		$_attributes = array();

		foreach($e->getAttributes() as $k=> $ea)
		{
			$_attributes[$k] = $ea;			
			if($k==='ref'){ $name = $ea; $ref_element = true; }
			else if($k==='name') $name = $ea;
		}

		//check if this element already exists this only applies if in a sequence.
		foreach($this->bag['elements'] as $ea) if($ea===$name)
		{
			//echo "found duplicate element upping max occurs";
			//if it does then update its max occurs and exit
			if(!$this->bag['hasChoice']||$_attributes['maxOccurs']===unbounded)
			{
				$maxO =& $this->bag['element_attrs'][$ea]; if($maxO!=unbounded) $maxO++;
			}			
			//echo " to ". $this->bag['element_attrs'][$ea]['maxOccurs'] ."\n";
			return;
		}

		//track the attrs on each sub-element
		$this->bag['element_attrs'][$name] =& $_attributes;

		//call the dom-recurse function on each new element
		if(!$ref_element)
		{
			$this->bag['elements'][] = $name;
			$meta2 =& $e->generate($this->bag['context'],$this->bag['global_elements']);
			$meta2['parent_meta'] =& $this->bag;
			$this->bag['inline_elements'][$name] =& $meta2;			
			$this->bag['element_documentation'][$name] = $meta2['documentation'];			
		}
		else
		{			
			$this->bag['elements'][] = $name;
			$this->bag['ref_elements'][] = $name;
			//check for documentation
			$a = $e->getElementsByType('xsAnnotation');
			if(!empty($a))
			{
				$d = $a[0]->getElementsByType('xsDocumentation');
				if(!empty($d))
				$this->bag['element_documentation'][$name] = $d[0]->get();				
			}
		}
	}

	function addAttribute(& $a)
	{
		$name = ''; $a_list = array();

		foreach($a->getAttributes() as $k=>$ea)
		{
			$a_list[$k] = $ea;
			if($k=='name')
			{
				$name = $ea; if($name=='id') $this->setHasId(true);
			}
			else if($k=='ref')
			{
				$name = $ea;
				//printf( "found an attribute ref for ". $name ."\n"); 
				if(strpos($name,':')!==FALSE)
				{
					$name[strpos($name,':')] = '_';
					//printf( "changed : to _ for ". $name ."\n" );
					$a_list['type']			 = 'xs:anyURI';
				}
			}
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
	const sequenceCMopening = 1, choiceCMopening = 2, groupCMcontinuing = 3, allCMopening = 4, anyCM = 5, CMclosure = 6;	
	//For elements name is the element name, for sequence name = 1, choice = 2, group = 3, all = 4, any = 5, end = 6
	function addContentModel($name,$minOccurs,$maxOccurs)
	{
		$this->bag['content_model'][] = array('name'=>$name,'minOccurs'=>$minOccurs,'maxOccurs'=>$maxOccurs);
		echo "adding content model name: $name minO: $minOccurs maxO: $maxOccurs\n";
	}
	function endContentModel()
	{
		$this->bag['content_model'][] = array('name'=>self::CMclosure,'minOccurs'=>0,'maxOccurs'=>0);
		echo "ending content model\n";
	}
}

?>