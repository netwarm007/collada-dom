<?php

/*
 * Copyright 2006 Sony Computer Entertainment Inc.
 *
 * Licensed under the MIT Open Source License, for details please see license.txt or the website
 * http://www.opensource.org/licenses/mit-license.php
 *
 */

define('unbounded',PHP_INT_MAX);
require_once('src/TypeMeta.php');
require_once('src/ElementMeta.php');

class _type 
{
	var $type = array();

	function _type()
	{
		$this->type[] = "MotherOfAllTypes";
	}

	function isOfType($type)
	{
		return $type==$this->type[count($this->type)-1];
	}

	function getType()
	{
		$type = "NULL";
		if(!empty($this->type))	$type = $this->type[0]; return $type;
	}

	function isAncestor($type)
	{
		return in_array($type,$this->type);
	}

}

class _typedData extends _type
{
	var $data;
	var $attributeMeta	 = array();
	var $attributes		 = array();

	function _typedData()
	{
		$this->type[] = "TypedData";
		parent::_type();
	}

	function _addAttribute($name, $meta)
	{
		$this->attributeMeta[$name] = $meta;
	}

	function setAttribute($name, $value)
	{
		//make sure we know about the attribute before setting it
		if(isset($this->attributeMeta[$name]))
		{
			if($name[0]==='m'&&!is_int($value))
			if($name==='minOccurs'||$name==='maxOccurs') switch($value)
			{
			case '': $value = 0; break; case 'unbounded': $value = unbounded; break;
			
			default: if(!ctype_digit($value)) die("die(min/maxOccurs is NaN? $value)");
			
				$value = (int)$value;
			}			
			$this->attributes[$name] = $value;
		}
	}

	function getAttribute($name)
	{
		return isset($this->attributeMeta[$name])&&isset($this->attributes[$name])
		?$this->attributes[$name]:'';
	}

	function& getAttributes()
	{
		return $this->attributes;
	}

	function set(& $buffer)
	{
		$this->data = $buffer;
	}

	function append(& $buffer)
	{
		$this->data.=$buffer;
	}

	function get()
	{
		return $this->data;
	}
}

class _elementSet extends _typedData
{
	var $elementMeta = array();	var $elements = array();

	function _elementSet()
	{
		$this->_addAttribute('minOccurs',array('type'=>'xs:integer'));
		$this->setAttribute('minOccurs',1);
		$this->_addAttribute('maxOccurs',array('type'=>'xs:integer'));
		$this->setAttribute('maxOccurs',1);
		$this->type[] = "ElementSet"; parent::_typedData();
	}

	function _addElement($name,$attrs)
	{
		$this->elementMeta[$name] = $attrs;
	}

	function addElement(& $e)
	{
		if(!in_array($e->getType(),array_keys($this->elementMeta)))
		{
			echo "Invalid element ", $e->getType(), "in ", $this->getType(), "\n";
			$this->log("WARN: ".$e->getType()." not a valid member of ".$this->getType());
		}
		else $this->elements[] =& $e;
	}

	function& getElements()
	{
		return $this->elements;
	}

	function& getElementsByType($type)
	{
		$list = array();
		foreach($this->elements as& $ea)
		if($ea->getType()==$type) $list[] =& $ea;
		return $list;
	}

	function setElement($name,& $value)
	{
		$this->elements[$name] = $value;
	}

	function exists($name)
	{
		return isset($this->elements[$name]);
	}

	function delete($name)
	{
		unset($this->elements[$name]);
	}

	function _delete($name)
	{
		$this->delete($name);
		unset($this->elementMeta[$name]);
	}

	function getCount()
	{
		return count($this->elements);
	}
}

//NEW: pulling out duplicated methods
class _elementSet2 extends _elementSet
{
	function _elementSet2(){ parent::_elementSet(); }
	
	//FUNCTIONALLY STATIC	
	//flatten choice/all/sequence groups into a single list of contained elements
	//NOTE: generateContentModel is tightly coupled to tpl-classes-cpp.php (formerly tpl-cpp-methods.php)
	function generateContentModel(_elementSet& $element, ElementMeta& $generator, & $context, $maxOccurs)
	{
		//echo "in generateContentModel ";
		foreach($element->getElements() as $ea) 
		{
			$type = $ea->getType(); if($type==='xsAttribute')
			{
				$generator->addAttribute($ea); continue; //echo "found attribute!\n";
			}
			
			$minO = $ea->getAttribute('minOccurs');	$maxO = $ea->getAttribute('maxOccurs');
			
			switch($type)
			{
			case 'xsChoice':
				$generator->setHasChoice(true);
				$generator->addContentModel(ElementMeta::choiceCMopening,$minO,$maxO);
				//propagate the maxOccurs down through choice hierarchy (while flattening)
				$local_max = $ea->getAttribute('maxOccurs');
				if($maxOccurs>$local_max)
				$this->generateContentModel($ea,$generator,$context,$maxOccurs);
				else $this->generateContentModel($ea,$generator,$context,$local_max);			
				break;
			case 'xsSequence':
				$generator->addContentModel(ElementMeta::sequenceCMopening,$minO,$maxO);
				//propagate the maxOccurs down through choice hierarchy (while flattening)
				$local_max = $ea->getAttribute('maxOccurs');
				if($maxOccurs>$local_max)
				$this->generateContentModel($ea,$generator,$context,$maxOccurs);			
				else $this->generateContentModel($ea,$generator,$context,$local_max);			
				break;
			case 'xsAll': //like xsSequence, but in no particular order and listed children must be 1-or-none
				$generator->addContentModel(ElementMeta::allCMopening,$minO,$maxO);
				//propagate the maxOccurs down through choice hierarchy (while flattening)
				$local_max = $ea->getAttribute('maxOccurs');
				if($maxOccurs>$local_max)
				$this->generateContentModel($ea,$generator,$context,$maxOccurs);			
				else $this->generateContentModel($ea,$generator,$context,$local_max);			
				break;
			case 'xsGroup':
				$generator->addContentModel(ElementMeta::groupCMcontinuing,$minO,$maxO);
				$generator->addGroup($ea);
				//break; //look out below!
			case 'xsElement':
				$nm = $ea->getAttribute('name')?:$ea->getAttribute('ref');
				if(empty($nm)) die('die(name is empty)');
				$generator->addContentModel($nm,$minO,$maxO);
				//echo "found element!\n";
				//if a containing element/group has a maxOccurs>1, then inherit it (will flag as array in code gen)
				if($maxOccurs>1) $ea->setAttribute('maxOccurs',$maxOccurs);			
				$generator->addElement($ea,$context);
				//NEW: tpl-classes-cpp.php behaves as if there is no END terminator
				continue 2;
			case 'xsAny':
				echo "found an any\n";
				$generator->addContentModel(ElementMeta::anyCM,$minO,$maxO);
				$generator->setHasAny(true);
				//break? OR would return; be more appropriate? 
				//ANYWAY, the code-generator must agree with whichever way it should be
				//break; //bug? see tpl/tpl-classes-cpp.php
				continue 2; //the present code behaves as if there is no END terminator
			default: continue 2; //break; 
			}
			//END content model - There will be one extra on every element
			$generator->endContentModel(); //ElementMeta::CMclosure
		}
	}	
	
	//function that reads complex types.  will recurse complex type derived heirarchies.
	function generateComplexType(_elementSet $content, ElementMeta& $generator, & $context)
	{
		//echo "in generatecomplextype\n";
		$temp = $content->getElementsByType('xsSimpleContent');		
		if(!empty($temp)) //returns
		{
			//echo "found simpleContent!\n";			
			$content = $temp[0]; // Should only be one - now we now find out element's parent class
			$temp =& $content->getElements();
			$content = $temp[0]; // Should either be an xsExtension or xsRestriction
			$type = $content->getAttribute('base');
			//echo "setting extends to ". $type ."\n";
			$generator->setContentType($type);
			if($content instanceof xsRestriction)
			$generator->bag['base_type'] = $generator->bag['baseTypeViaRestriction'] = $type;
			$temp =& $content->getElementsByType('xsAttribute');
			foreach($temp as $ea) $generator->addAttribute($ea);
			return;
		}//ELSE
		$temp = $content->getElementsByType('xsComplexContent');
		if(!empty($temp)) //returns 
		{
			//echo "found complexContent!\n";
			//ComplexContent specified means type is derived			
			$content = $temp[0]; // Should only be one - now we now find out element's parent class
			$temp =& $content->getElements();
			$content = $temp[0]; // Should either be an xsExtension or xsRestriction
			if($content->getType()=='xsExtension')
			$generator->bag['isExtension'] = true;
			if($content->getType()=='xsRestriction')
			$generator->bag['isRestriction'] = true;			
			$type = $content->getAttribute('base');
			if($content instanceof xsRestriction)
			$generator->bag['baseTypeViaRestriction'] = $type;
			//echo "setting extends to ". $type ."\n";
			$generator->bag['base_type'] = $type;
			//Generate the complex type this is derived from
			//*************CHANGE NEEDED HERE 8-25 !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
			//HACK: there must be a better way than $GLOBALS
			foreach($GLOBALS['_globals']['complex_types'] as& $ea)
			{
				if($type==$ea->getAttribute('name'))
				{
					$generator->setComplexType(true);
					$generator->bag['ref_elements'][] = $type;
					//$this->generateComplexType($ea, $generator, $context );
					break;
				}
			}

			//parse element context
			$this->generateContentModel($content,$generator,$element_context,$content->getAttribute('maxOccurs'));
			return;
		}//ELSE
		//echo "found nothing so doing complex content generateContentModel\n";
		//the alternative to xsSimpleContent is xsComplexContent - if it is not specified, it is implied			
		$this->generateContentModel($content,$generator,$element_context,$content->getAttribute('maxOccurs'));
	}
	
	//function that generates the inline simpleType
	function generateSimpleType(_elementSet $content, TypeMeta& $generator)
	{
		$e = $content->getElements();		
		$generator->setType($this->getAttribute('name'));
		//echo $this->getAttribute('name'), " has a simpletype\n";
		$a = $content->getElementsByType('xsAnnotation');
		if(!empty($a))
		{
			//echo "found annotation for ", $this->getAttribute('name'), "!\n";
			$d = $a[0]->getElementsByType('xsDocumentation');
			if(!empty($d))
			{
				//echo "found documentation for ", $this->getAttribute('name'), "!\n";
				$generator->setDocumentation($d[0]->get());
			}
			$ap = $a[0]->getElementsByType('xsAppinfo');
			if(!empty($ap))
			{
				$generator->setAppInfo($ap[0]->get());
			}
		}//THIS LOOKS SUSPICIOUS: 0 or 1? 
		$el = $e[$e[0]->getType()=='xsAnnotation'?1:0];		
		switch($el->getType())
		{
		case 'xsRestriction': case 'xsExtension':
		
			$generator->setIsExtension($el->getType()=='xsExtension');

			//Set base class //NEW: indirectly sets isString
			//HACK: saved for last so isString can reflect on enumeration/constant-strings
			//$generator->setBase($el->getAttribute('base'));

			// Look for enums
			$enums = $el->getElementsByType('xsEnumeration');
			foreach($enums as $ea)
			{
				$enum = $ea->getAttribute('value');
				$generator->addEnum($enum);
				//echo $ea->getAttribute( 'value' );
				foreach($ea->getElementsByType('xsAnnotation') as $ea2)
				{
					foreach($ea2->getElementsByType('xsDocumentation') as $ea3)
					$generator->addEnumDoc($enum,$ea3->get());
					foreach($ea2->getElementsByType('xsAppinfo') as $ea3)
					$generator->addEnumAppInfo($enum,$ea3->get());
				}
			}

			// Look for max/mins
			$min   = $el->getElementsByType('xsMinLength');
			$max   = $el->getElementsByType('xsMaxLength');
			$minIn = $el->getElementsByType('xsMinInclusive');
			$maxIn = $el->getElementsByType('xsMaxInclusive');
			$minEx = $el->getElementsByType('xsMinExclusive');
			$maxEx = $el->getElementsByType('xsMaxExclusive');

			if(!empty($min))			
			$generator->setRestriction('minLength',$min[0]->getAttribute('value'));
			if(!empty($max))			
			$generator->setRestriction('maxLength',$max[0]->getAttribute('value'));
			if(!empty($minIn))			
			$generator->setRestriction('minInclusive',$minIn[0]->getAttribute('value'));			
			if(!empty($maxIn))
			$generator->setRestriction('maxInclusive',$maxIn[0]->getAttribute('value'));
			if(!empty($minEx))
			$generator->setRestriction('minExclusive',$minEx[0]->getAttribute('value'));
			if(!empty($maxEx))
			$generator->setRestriction('maxExclusive',$maxEx[0]->getAttribute('value'));
			
			//HACK: saved for last so isString can reflect on enumeration/constant-strings
			$generator->andFinally_setBase($el->getAttribute('base'));
			break;			
		
		case 'xsList':
		
			//$extends = "xsList";
			$itemType = $el->getAttribute('itemType');
			$generator->setListType($itemType);			
			break;
		
		case 'xsUnion':
		
			$generator->setUnionMembers($el->getAttribute('memberTypes'));
			break;
		
		default:
		
			$this->log("WARN: unexpected element in xsSimpleType code generation");
		}
	}
}

?>