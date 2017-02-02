<?php
/*
 * Copyright 2006 Sony Computer Entertainment Inc.
 *
 * Licensed under the MIT Open Source License, for details please see license.txt or the website
 * http://www.opensource.org/licenses/mit-license.php
 *
 */

require_once('om/base-types.php');

/*HINT: MOST OF THIS FILE SEEMS TO BE HARD-CODING THE XML SCHEMA-SCHEMA*/

//REMINDER: _addElement seems like it doesn't use the minOccurs/maxOccurs stuff.
//It's not even clear that it's set up correctly or is meaningful in many cases.

class xsAll extends _elementSet //require_once('om/xsAll.php');
{
	function xsAll()
	{
		$this->_addElement('xsElement',array('minOccurs'=>0,'maxOccurs'=>unbounded));
		$this->_addElement('xsAttribute',array('minOccurs'=>0,'maxOccurs'=>unbounded));
		$this->type[] = "xsAll"; parent::_elementSet();
	}//UNUSED
	function addAllElement(& $e){ $this->addElement($e); } 
}
class xsAnnotation extends _elementSet //require_once('om/xsAnnotation.php');
{
	function xsAnnotation()
	{
		$this->_addElement('xsDocumentation',array('minOccurs'=>0,'maxOccurs'=>unbounded));
		$this->_addElement('xsAppinfo',array('minOccurs'=>0,'maxOccurs'=>unbounded));
		$this->type[] = "xsAnnotation";	parent::_elementSet();
		//Set bounds on number of elements allowable in annotation element
		$this->setAttribute('minOccurs',0); $this->setAttribute('maxOccurs',unbounded);
	}//UNUSED
	function addAnnotationElement(& $e){ $this->addElement($e); }
}
class xsAny extends _elementSet //_typedData //require_once('om/xsAny.php');
{
	function xsAny()
	{
		$this->_addAttribute('namespace',array('type'=>'xs:anyURI'));
		$this->_addAttribute('processContents',array('type'=>'xs:string'));
		$this->_addAttribute('minOccurs',array('type'=>'xs:integer'));
		$this->setAttribute('minOccurs',1);
		$this->_addAttribute('maxOccurs',array('type'=>'xs:integer'));
		$this->setAttribute('maxOccurs',1);
		$this->type[] = 'xsAny'; parent::_elementSet();
	}
}
class xsAnyAttribute extends _elementSet //NEW
{
	function xsAnyAttribute()
	{
		$this->_addAttribute('namespace',array('type'=>'xs:anyURI'));
		$this->_addAttribute('processContents',array('type'=>'xs:string'));		
		$this->type[] = 'xsAnyAttribute'; parent::_elementSet();
	}
}
class xsAttribute extends _elementSet //require_once('om/xsAttribute.php');
{
	function xsAttribute()
	{
		$this->_addAttribute('name',array('type'=>'xs:string'));
		$this->_addAttribute('type',array('type'=>'xs:string'));
		$this->_addAttribute('use',array('type'=>'xs:string'));
		$this->_addAttribute('default',array('type'=>'xs:string'));
		$this->_addAttribute('ref',array('type'=>'xs:string'));
		$this->_addElement('xsAnnotation',array('minOccurs'=>0,'maxOccurs'=>unbounded));
		$this->type[] = 'xsAttribute'; parent::_typedData();
	}
}
class xsChoice extends _elementSet //require_once('om/xsChoice.php');
{
	function xsChoice()
	{
		$this->_addElement('xsElement',array('minOccurs'=>0,'maxOccurs'=>unbounded));
		$this->_addElement('xsAttribute',array('minOccurs'=>0,'maxOccurs'=>unbounded));
		$this->_addElement('xsChoice',array('minOccurs'=>0,'maxOccurs'=>unbounded));
		$this->_addElement('xsSequence',array('minOccurs'=>0,'maxOccurs'=>unbounded));
		$this->_addElement('xsGroup',array('minOccurs'=>0,'maxOccurs'=>unbounded));
		$this->_addElement('xsAnnotation',array('minOccurs'=>0,'maxOccurs'=>unbounded));
		$this->type[] = "xsChoice";	parent::_elementSet();
	}
	function addChoiceElement(& $e){ $this->addElement($e);	}
}
class xsComplexContent extends _elementSet //require_once('om/xsComplexContent.php');
{
	function xsComplexContent()
	{
		$this->_addElement('xsRestriction',array('minOccurs'=>1,'maxOccurs'=>1));
		$this->_addElement('xsExtension',array('minOccurs'=>1,'maxOccurs'=>1));
		//$this->_addAttribute('name',array('type'=>'xs:string'));
		$this->type[] = 'xsComplexContent';	parent::_elementSet();
	}
}
class xsComplexType extends _elementSet2 //require_once('om/xsComplexType.php');
{
	function xsComplexType()
	{
		$this->_addElement('xsAnnotation',array('minOccurs'=>0,'maxOccurs'=>unbounded));
		$this->_addElement('xsAnyAttribute',array('minOccurs'=>0,'maxOccurs'=>unbounded));	
		$this->_addElement('xsChoice',array('minOccurs'=>1,'maxOccurs'=>1));
		$this->_addElement('xsAttribute',array('minOccurs'=>1,'maxOccurs'=>1));
		$this->_addElement('xsSequence',array('minOccurs'=>1,'maxOccurs'=>1));
		$this->_addElement('xsAll',array('minOccurs'=>1,'maxOccurs'=>1));
		$this->_addElement('xsGroup',array('minOccurs'=>1,'maxOccurs'=>1));
		$this->_addElement('xsSimpleContent',array('minOccurs'=>1,'maxOccurs'=>1));
		$this->_addElement('xsComplexContent',array('minOccurs'=>1,'maxOccurs'=>1));
		$this->_addAttribute('name',array('type'=>'xs:string'));
		$this->_addAttribute('mixed',array('type'=>'xs:string','default'=>'false'));
		$this->type[] = 'xsComplexType'; parent::_elementSet2();
	}
	function& generate($element_context,& $global_elements)
	{
		$name = $this->getAttribute('name');
		$element_context[] = $name;
		echo implode(",",$element_context)."\n";
		//get new factory
		$generator = new ElementMeta($global_elements);
		//load the class name and a context pre-fix (in case we're inside another element)
		$generator->setName($name);
		$generator->setContext($element_context);
		//extract any documentation for this node
		$a = $this->getElementsByType('xsAnnotation');
		if(!empty($a))
		{
			$d = $a[0]->getElementsByType('xsDocumentation');
			if(!empty($d))			
			$generator->setDocumentation($d[0]->get());			
			$ap = $a[0]->getElementsByType('xsAppinfo');
			if(!empty($ap))
			$generator->setAppInfo($ap[0]->get());			
		}
		if($this->getAttribute('mixed')=='true')
		$generator->setMixed();		

		//should only be one
		$this->generateComplexType($this,$generator);
		$meta =& $generator->getMeta();
		if(1===count($element_context))
		$global_elements[$element_context[0]] =& $meta;	return $meta;
	}
}
class xsDocumentation extends _typedData //require_once('om/xsDocumentation.php');
{
	function xsDocumentation(){ $this->type[] = "xsDocumentation"; parent::_typedData(); } 	
}
class xsAppinfo extends _typedData //require_once('om/xsAppinfo.php');
{
	function xsAppinfo(){ $this->type[] = "xsAppinfo"; parent::_typedData(); } 
}
class xsElement extends _elementSet2 //require_once('om/xsElement.php');
{
	function xsElement()
	{
		$this->_addElement('xsAnnotation',array('minOccurs'=>0,'maxOccurs'=>unbounded));
		$this->_addElement('xsComplexType',array('minOccurs'=>0,'maxOccurs'=>1));
		$this->_addElement('xsSimpleType',array('minOccurs'=>0,'maxOccurs'=>1));
		$this->_addAttribute('ref',array('type'=>'xs:string'));
		$this->_addAttribute('name',array('type'=>'xs:string'));
		$this->_addAttribute('type',array('type'=>'xs:string'));
		$this->_addAttribute('abstract',array('type'=>'xs:bool'));
		$this->_addAttribute('substitutionGroup',array('type'=>'xs:string'));
		$this->_addAttribute('default',array('type'=>'xs:string'));
		//$this->_addAttribute('maxOccurs',array('type'=>'xs:integer'));
		//$this->_addAttribute('minOccurs',array('type'=>'xs:integer'));
		$this->type[] = 'xsElement'; parent::_elementSet2();
	}
	function& generate($element_context,& $global_elements)
	{
		$element_context[] = $this->getAttribute("name");
		echo implode(",",$element_context)."\n";
		//get new factory
		$generator = new ElementMeta($global_elements);
		//load the class name and a context pre-fix (in case we're inside another element)
		$generator->setName($this->getAttribute('name'));
		$generator->setContext($element_context);
		$subGroup = $this->getAttribute('substitutionGroup');
		if(!empty($subGroup))
		{
			//echo "found a subGroup ". $subGroup ."!\n";
			$generator->setSubstitutionGroup($subGroup);
			$generator->bag['#include'][] = $subGroup;
		}
		if('true'===$this->getAttribute('abstract'))
		$generator->setAbstract();

		//extract any documentation for this node
		$a = $this->getElementsByType('xsAnnotation');
		if(!empty($a))
		{
			$d = $a[0]->getElementsByType('xsDocumentation');
			if(!empty($d))
			$generator->setDocumentation($d[0]->get());			
			$ap = $a[0]->getElementsByType('xsAppinfo');
			if(!empty($ap))
			$generator->setAppInfo($ap[0]->get());			
		}

		//******************************************************************************************/
		//$generator->setContentType($this->getAttribute('type'));
		$type = $this->getAttribute('type');
		$generator->bag['base_type'] = $type;
		//check if this type equals a complex type    
		//echo "element ", $this->getAttribute('name'), " is of type ", $type, "!\n";
		if(!empty($type))
		{
			global $_globals;
			//HACK: this is more or less how this has always been done
			if(@isset($_globals['complex_types'][$type]))
			{
				//echo "found a match for ". $type ."\n";				
				$generator->bag['#include'][] = $type;
			}
			else //wrapping a simple-type
			{
				$generator->setContentType($type);
				$generator->setRequiresClass();
				
				includeTypeInSchema($type); //HACK
			}			
		}
		//*******************************************************************************************/
		// Inspect the semantic structure of this node and extract the elements/attributes
		$temp = $this->getElementsByType('xsComplexType');

		if(!empty($temp))
		{
			if($temp[0]->getAttribute('mixed')=='true')
			{
				$generator->setMixed();
			}

			$content = $temp[0]; //Should only be one
			$this->generateComplexType($content,$generator);
		}
		else 
		{
			$temp = $this->getElementsByType('xsSimpleType');
			
			if(!empty($temp))
			{
				if($generator->name!=='author_email')
				die('COLLADA doesn\'t do this. This facility requires work.');				
				
				$generator->setRequiresClass();
		
				//inline simple type definition
				//right now handle as string but needs to be fixed
				$generator->bag['simple_type'] = new TypeMeta();
				$this->generateSimpleType($temp[0],$generator->bag['simple_type']);
				if(!empty($generator->bag['simple_type']->bag['enum']))
				{
					die('die(Is this COLLADA 1.4.1??)'); //suffixes are a bad idea!!
					$generator->setContentType($this->getAttribute('name')."_type");
				}
				else $generator->setContentType($generator->bag['simple_type']->bag['base']);
			}
		}

		$meta =& $generator->getMeta();
		if(1===count($element_context)) $global_elements[$element_context[0]] =& $meta;
		return $meta;
	}
}
class xsEnumeration extends _elementSet //require_once('om/xsEnumeration.php');
{
	function xsEnumeration()
	{
		$this->_addAttribute('value',array('type'=>'xs:integer'));
		$this->_addElement('xsAnnotation',array('minOccurs'=>0,'maxOccurs'=>unbounded));
		$this->type[] = 'xsEnumeration'; parent::_typedData();
	}
}
class xsExtension extends _elementSet //require_once('om/xsExtension.php');
{
	function xsExtension()
	{
		$this->_addElement('xsAttribute',array('minOccurs'=>0,'maxOccurs'=>unbounded));
		$this->_addElement('xsAnyAttribute',array('minOccurs'=>0,'maxOccurs'=>unbounded));	
		$this->_addElement('xsSequence',array('minOccurs'=>0,'maxOccurs'=>unbounded));
		$this->_addAttribute('base',array('type'=>'xs:string'));
		$this->type[] = 'xsExtension'; parent::_elementSet();
	}
}
class xsGroup extends _elementSet2 //require_once('om/xsGroup.php');
{
	function xsGroup()
	{
		$this->_addElement('xsAnnotation',array('minOccurs'=>0,'maxOccurs'=>unbounded));
		//Don't think these are allowed in groups.
		//$this->_addElement('xsElement',array('minOccurs'=>0,'maxOccurs'=>unbounded));		
		//$this->_addElement('xsAttribute',array('minOccurs'=>0,'maxOccurs'=>unbounded));
		$this->_addElement('xsAll',array('minOccurs'=>0,'maxOccurs'=>unbounded));
		$this->_addElement('xsChoice',array('minOccurs'=>0,'maxOccurs'=>unbounded));
		$this->_addElement('xsSequence',array('minOccurs'=>0,'maxOccurs'=>unbounded));
		$this->_addElement('xsGroup',array('minOccurs'=>0,'maxOccurs'=>unbounded));
		$this->_addAttribute('ref',array('type'=>'xs:string'));
		$this->_addAttribute('name',array('type'=>'xs:string'));
		$this->type[] = "xsGroup"; parent::_elementSet2();
	}
	function addChoiceElement(& $e){ $this->addElement($e); }
	function& generate($element_context,& $global_elements)
	{
		$element_context[] = $this->getAttribute("name");
		echo 'Generator context: ', implode(",",$element_context), "\n";
		//get new factory
		$generator = new ElementMeta($global_elements);
		$generator->setIsGroup();
		//load the class name and a context pre-fix (in case we're inside another element)
		$generator->setName($this->getAttribute('name'));
		$generator->setContext($element_context);
		//extract any documentation for this node
		$a = $this->getElementsByType('xsAnnotation');
		if(!empty($a))
		{
			$d = $a[0]->getElementsByType('xsDocumentation');
			if(!empty($d))
			$generator->setDocumentation($d[0]->get());			
		}

		//inspect the semantic structure of this node and extract the elements/attributes
		$this->generateCM_isPlural($this,$generator,$this->getAttribute('maxOccurs'));

		$meta =& $generator->getMeta();
		if(1===count($element_context))
		$global_elements[$element_context[0]] =& $meta;	return $meta;
	}	
}
class xsLength extends _typedData //NEW
{
	function xsLength()
	{
		$this->_addAttribute('value',array('type'=>'xs:integer'));
		$this->type[] = 'xsLength'; parent::_typedData();
	}
}
class xsList extends _typedData //require_once('om/xsList.php');
{
	var $minLength, $maxLength;

	function xsList()
	{
		$this->minLength = 0; $this->maxLength = unbounded;
		$this->_addAttribute('itemType',array('type'=>'xs:string'));
		$this->setAttribute('itemType','TypedData');
		$this->type[] = 'xsList'; parent::_typedData();
	}//to save the heavyweight object-per-data-point approach, allow a list type
	//to parse the buffer into a single array (say what?)
	function set(& $buffer)
	{
		eval('$type = new '.$this->getAttribute('itemType').'();');
		$this->data =& $type->parse($buffer);
		//for($i=0;trim($buffer)!=""&&$i<$this->maxLength;$i++)
		//{
		//	eval('$this->data[$i] = new ' . $this->getAttribute('itemType').'();');
		//	$this->data[$i]->set($buffer);
		//}
	}
	function getCount(){ return count($this->data); }
}
class xsMaxExclusive extends _typedData //require_once('om/xsMaxExclusive.php');
{
	function xsMaxExclusive()
	{
		$this->_addAttribute('value',array('type'=>'xs:float'));
		$this->type[] = 'xsMaxExclusive'; parent::_typedData();
	}
}
class xsMaxInclusive extends _typedData //require_once('om/xsMaxInclusive.php');
{
	function xsMaxInclusive()
	{
		$this->_addAttribute('value',array('type'=>'xs:float'));
		$this->type[] = 'xsMaxInclusive'; parent::_typedData();
	}
}
class xsMaxLength extends _typedData //require_once('om/xsMaxLength.php');
{
	function xsMaxLength()
	{
		$this->_addAttribute('value',array('type'=>'xs:integer'));
		$this->type[] = 'xsMaxLength'; parent::_typedData();
	}
}
class xsMinExclusive extends _typedData //require_once('om/xsMinExclusive.php');
{
	function xsMinExclusive()
	{
		$this->_addAttribute('value',array('type'=>'xs:float'));
		$this->type[] = 'xsMinExclusive'; parent::_typedData();
	}
}
class xsMinInclusive extends _typedData //require_once('om/xsMinInclusive.php');
{
	function xsMinInclusive()
	{
		$this->_addAttribute('value',array('type'=>'xs:float'));
		$this->type[] = 'xsMinInclusive'; parent::_typedData();
	}
}
class xsMinLength extends _typedData //require_once('om/xsMinLength.php');
{
	function xsMinLength()
	{
		$this->_addAttribute('value',array('type'=>'xs:integer'));
		$this->type[] = 'xsMinLength'; parent::_typedData();
	}
}
class xsPattern extends _elementSet //require_once('om/xsPattern.php');
{
	function xsPattern()
	{
		$this->_addAttribute('value',array('type'=>'xs:string'));
		$this->_addElement('xsAnnotation',array('minOccurs'=>0,'maxOccurs'=>unbounded));
		$this->type[] = 'xsPattern'; parent::_typedData();
	}
}
class xsRestriction extends _elementSet //require_once('om/xsRestriction.php');
{
	function xsRestriction()
	{
		$this->_addElement('xsAttribute',array('minOccurs'=>0,'maxOccurs'=>unbounded));
		$this->_addElement('xsAnyAttribute',array('minOccurs'=>0,'maxOccurs'=>unbounded));	
		$this->_addElement('xsLength',array('minOccurs'=>0,'maxOccurs'=>unbounded));
		$this->_addElement('xsMinLength',array('minOccurs'=>0,'maxOccurs'=>unbounded));
		$this->_addElement('xsMaxLength',array('minOccurs'=>0,'maxOccurs'=>unbounded));
		$this->_addElement('xsMinInclusive',array('minOccurs'=>0,'maxOccurs'=>unbounded));
		$this->_addElement('xsMaxInclusive',array('minOccurs'=>0,'maxOccurs'=>unbounded));
		$this->_addElement('xsMinExclusive',array('minOccurs'=>0,'maxOccurs'=>unbounded));
		$this->_addElement('xsMaxExclusive',array('minOccurs'=>0,'maxOccurs'=>unbounded));
		$this->_addElement('xsEnumeration',array('minOccurs'=>0,'maxOccurs'=>unbounded));
		$this->_addElement('xsWhiteSpace',array('minOccurs'=>0,'maxOccurs'=>unbounded));
		$this->_addElement('xsPattern',array('minOccurs'=>0,'maxOccurs'=>unbounded));
		$this->_addElement('totalDigits',array('minOccurs'=>0,'maxOccurs'=>unbounded));
		$this->_addElement('fractionDigits',array('minOccurs'=>0,'maxOccurs'=>unbounded));
		$this->_addAttribute('base',array('type'=>'xs:string'));
		$this->type[] = 'xsRestriction'; parent::_elementSet();
	}
}
class xsSchema extends _elementSet //require_once('om/xsSchema.php');
{
	function xsSchema()
	{
		$this->_addElement('xsAnnotation',array('minOccurs'=>0,'maxOccurs'=>unbounded));
		$this->_addElement('xsElement',array('minOccurs'=>0,'maxOccurs'=>unbounded));
		$this->_addElement('xsSimpleType',array('minOccurs'=>0,'maxOccurs'=>unbounded));
		$this->_addElement('xsComplexType',array('minOccurs'=>0,'maxOccurs'=>unbounded));
		$this->_addElement('xsGroup',array('minOccurs'=>0,'maxOccurs'=>unbounded));
		$this->_addElement('xsImport',array('minOccurs'=>0,'maxOccurs'=>unbounded));
		$this->_addAttribute('targetNamespace',array('type'=>'xs:string'));
		$this->_addAttribute('elementFormDefault',array('type'=>'xs:string'));
		$this->_addAttribute('xmlns',array('type'=>'xs:string'));
		//Just letting these go out as WARNING: Unhandled attribute: 
		//$this->_addAttribute('xmlns:xs',array('type'=>'xs:string'));
		//$this->_addAttribute('xmlns:xsi',array('type'=>'xs:string'));
		//$this->_addAttribute('xml:lang',array('type'=>'xs:string'));
		//$this->_addAttribute('xsi:schemaLocation',array('type'=>'xs:string'));
		$this->_addAttribute('version',array('type'=>'xs:string'));
		$this->type[] = 'xsSchema';	parent::_elementSet();
	}
}
class xsSequence extends _elementSet //require_once('om/xsSequence.php');
{
	function xsSequence()
	{
		$this->_addElement('xsElement',array('minOccurs'=>0,'maxOccurs'=>unbounded));
		$this->_addElement('xsAttribute',array('minOccurs'=>0,'maxOccurs'=>unbounded));
		$this->_addElement('xsChoice',array('minOccurs'=>0,'maxOccurs'=>unbounded));
		$this->_addElement('xsSequence',array('minOccurs'=>0,'maxOccurs'=>unbounded));
		$this->_addElement('xsGroup',array('minOccurs'=>0,'maxOccurs'=>unbounded));
		$this->_addElement('xsAny',array('minOccurs'=>0,'maxOccurs'=>unbounded));
		$this->_addElement('xsAnnotation',array('minOccurs'=>0,'maxOccurs'=>unbounded));
		$this->type[] = "xsSequence"; parent::_elementSet();
	}//UNUSED
	function addSequenceElement(& $e){ $this->addElement($e); }

}
class xsSimpleContent extends _elementSet //require_once('om/xsSimpleContent.php');
{
	function xsSimpleContent()
	{
		$this->_addElement('xsRestriction',array('minOccurs'=>1,'maxOccurs'=>1));
		$this->_addElement('xsExtension',array('minOccurs'=>1,'maxOccurs'=>1));
		//$this->_addAttribute('name',array('type'=>'xs:string'));
		$this->type[] = 'xsSimpleContent'; parent::_elementSet();
	}
}
class xsSimpleType extends _elementSet2 //require_once('om/xsSimpleType.php');
{
	function xsSimpleType()
	{
		$this->_addElement('xsRestriction',array('minOccurs'=>1,'maxOccurs'=>1));
		$this->_addElement('xsExtension',array('minOccurs'=>1,'maxOccurs'=>1));
		$this->_addElement('xsList',array('minOccurs'=>1,'maxOccurs'=>1));
		$this->_addElement('xsUnion',array('minOccurs'=>1,'maxOccurs'=>1));
		$this->_addElement('xsAnnotation',array('minOccurs'=>1,'maxOccurs'=>1));
		$this->_addAttribute('name',array('type'=>'xs:string'));
		$this->type[] = 'xsSimpleType';	parent::_elementSet2();
	}
	function& generate()
	{
		$generator = new TypeMeta(); $this->generateSimpleType($this,$generator);		
		return $generator->getMeta();
	}
}
class xsUnion extends _typedData //require_once('om/xsUnion.php');
{
	function xsUnion()
	{
		$this->_addAttribute('memberTypes',array('type'=>'xs:string'));
		$this->type[] = 'xsUnion'; parent::_typedData();
	}
}
class xsWhiteSpace extends _typedData //require_once('om/xsWhiteSpace.php');
{
	function xsWhiteSpace()
	{
		$this->_addAttribute('value',array('type'=>'xs:string'));
		$this->type[] = 'xsWhiteSpace'; parent::_typedData();
	}
}
class xsImport extends _typedData //require_once('om/xsImport.php');
{
	function xsImport()
	{
		$this->_addAttribute('namespace',array('type'=>'xs:string'));
		$this->_addAttribute('schemaLocation',array('type'=>'xs:string'));
		$this->type[] = 'xsImport';	parent::_typedData();
	}
}

?>