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
			
				$value = (int)$value; //Not sure what to do in this case??
				if($value>big_number) die('die(min/maxOccurs>big_number)');				
			}			
			$this->attributes[$name] = $value;
		}
		else 
		{
			//if('xmlns'!==substr($name,0,5))
			{
			//	die('die(unexpected attribute)'); //breakpoint
			}
			echo 'WARNING: Unhandled attribute: ', $name, "\n";			
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
		//$attrs doesn't seem to be used anywhere. $name is below.
		$this->elementMeta[$name] = $attrs;
	}

	function addElement(& $e)
	{
		if(!in_array($e->getType(),array_keys($this->elementMeta)))
		{
			echo "Unexpected element ", $e->getType(), "in ", $this->getType(), "\n";
			die('die(Where is log defined???)');
			$this->log("WARN: ".$e->getType()." not a valid member of ".$this->getType());
		}
		else $this->elements[] =& $e;
	}

	function& getElements()
	{
		return $this->elements;
	}

	function getElementsByType($type)
	{
		$list = array();
		foreach($this->elements as& $ea)
		if($ea->getType()==$type) $list[] =& $ea;
		return $list;
	}
	function getElementsByTypeAccordingToName($type)
	{
		$list = array();
		foreach($this->elements as& $ea)
		if($ea->getType()==$type) $list[$ea->attributes['name']] =& $ea;
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
	//READ: generateCM+isPlural. This is doing two things, and renamed as such
	//flatten choice/all/sequence groups into a single list of contained elements
	//NOTE: generateContentModel is tightly coupled to tpl-classes-cpp.php (formerly tpl-cpp-methods.php)
	function generateCM_isPlural(_elementSet $element, ElementMeta& $generator, $maxOccurs)
	{
		//WORKAROUND PHP Strict standards:  Only variables should be passed by reference in ...
		$st = array(true,1,array()); $this->generateContentModel2($st,$element,$generator,$maxOccurs);
	}
	function generateContentModel2(& $st, _elementSet $element, ElementMeta& $generator, $maxOccurs)
	{
		global $subgroups;
		
		//NEW: $st is used to properly assign 'isPlural'. $add is used to transclude groups.		
		$add =& $st[0]; $nonce =& $st[1]; $stack =& $st[2]; 
		$isPlural = function($name) use(&$st,$stack)
		{				
			$named =& $st[$name];
			//if so, save the name, and wait for any duplicates
			if(empty($named)){ $named = $stack; return false; }			
			//find the last common parent, and return true for sequences
			for($i=0,$n;@($n=$named[$i])===$stack[$i]&&!empty($n);$i++);
			return $stack[$i-1]<0;
		};		

		//echo "in generateContentModel ";
		foreach($element->getElements() as $ea) 
		{
			$type = $ea->getType(); if($type==='xsAttribute')
			{
				if(!$add) die('die(unexpected behavior)');
				
				$generator->addAttribute($ea); continue; //echo "found attribute!\n";
			}
			else if($type==='xsAnyAttribute') //NEW
			{
				$generator->setHasAnyAttribute(); continue;
			}
			
			$minO = $ea->getAttribute('minOccurs');	$maxO = $ea->getAttribute('maxOccurs');
					
			switch($type)
			{
			case 'xsAll':
				$generator->setHasAll();
				$cm = ElementMeta::allCMopening; 
				array_push($stack,$nonce++);
				goto seq_like;				
			case 'xsChoice':				
				//$generator->setHasChoice(); //UNUSED: could be a compile-time-constant?
				$cm = ElementMeta::choiceCMopening;
				array_push($stack,$nonce++);
				goto seq_like;
			case 'xsSequence': 				
				$cm = ElementMeta::sequenceCMopening; 
				array_push($stack,-$nonce++);
				
			seq_like: //goto seq_like;
					
				if($add) $generator->addContentModel($cm,$minO,$maxO);
				//propagate the maxOccurs down through choice hierarchy (while flattening)
				$local_max = $ea->getAttribute('maxOccurs');
				$this->generateContentModel2($st,$ea,$generator,max($maxOccurs,$local_max));				
				//END content model
				if($add) $generator->endContentModel(); //ElementMeta::CMclosure
				array_pop($stack);
				break;			
			case 'xsGroup': 
				$ref = $ea->getAttribute('ref');
				if($add) //NEW: This was encoded as two CM nodes. Just add 'ref' to groups
				{
					$PHP =& $generator->addContentModel(ElementMeta::groupCM,$minO,$maxO);
					$PHP['ref'] = $ref;
				}				
				$generator->transcludeRef($ref); //addGroup($ea);
				//propagate the maxOccurs down through choice hierarchy (while flattening)
				$local_max = $ea->getAttribute('maxOccurs');
				//HACK: find the <xs:group>
				global $_globals; $group = $_globals['group_types'][$ref];
				if(empty($group)) die('Testing xs:group transclusion');
				//Now the children are transcluded, by setting $add=false.
				$add = false;
				$this->generateContentModel2($st,$group,$generator,max($maxOccurs,$local_max));	
				$add = true;
				$generator->bag['#include'][] = $ref;
				break;
			case 'xsAny':
				echo "found an any\n";
				if($add)
				{
					$PHP =& $generator->addContentModel(ElementMeta::anyCM,$minO,$maxO);
					//Hack: this should be unpacked by echoContentModelCPP
					$any =& $PHP['any'];					
					$ns = $ea->getAttribute('namespace');
					$pc = $ea->getAttribute('processContents');					
					$any = '';
					if(!empty($pc)&&$pc!=='strict')	$any.= '.setProcessContents("'.$pc.'")';
					if(!empty($ns)&&$ns!=='##any') $any.= '.setNamespaceString("'.$ns.'")';					
				}
				//TODO: CAPTURE ANNOTATIONS
				//Note: if a group has this, it should propagate to where it's transcluded.
				$generator->setHasAny();
				break;
			case 'xsElement':
				//echo "found element!\n";				
				$name = $generator->addElement($ea,$add);
				//$add is treating group-transcluded elements as if references
				//(Otherwise they will be circular if inline defined)				
				if($add) $generator->addContentModel($name,$minO,$maxO);								
				//If not for $subgroups this is just the $isPlural logic below
				$els =& $generator->bag['elements'];
				$ea =& $els[$name];
				$names = array($name =>& $ea);
				if(!empty($subgroups[$name]))				
				{
					$inc =& $generator->bag['#include'];
					$el_docs =& $generator->bag['element_documentation'];
					$ea_doc = $el_docs[$name];
					foreach($subgroups[$name] as $k=>$ea2)
					{
						if(empty($els[$k]))
						{
							$inc[] = $k;
							$el =& $els[$k];
							$el_docs[$k] = $ea_doc; 
							foreach($ea as $k2=>$ea2)
							{
								//these mess with relateClass()
								if($k2!='ref'&&$k2!='type')
								$el[$k2] = $ea2;
							}
							$names[$k] =& $el;
						}
						else $names[$k] =& $els[$k];
					}
				}
				foreach($names as $name =>& $ea)
				{
					//if a containing element/group has a maxOccurs>1, then inherit it (will flag as array in code gen)
					//"isPlural" isn't an attribute, but neither is a composite "maxOccurs"
					//Note: this was being set prior to addElement so it would be indirectly included
					//if($maxOccurs>1) $ea->setAttribute('maxOccurs',$maxOccurs);								
					if($maxOccurs>1||$maxO>1||$isPlural($name)) $ea['isPlural'] = true;
					else if(!isset($ea['isPlural'])) $ea['isPlural'] = false;							
				}unset($ea);
			default: break; //annotations, etc.
			}			
		}
	}	
	
	//function that reads complex types.  will recurse complex type derived heirarchies.
	function generateComplexType(_elementSet $content, ElementMeta& $generator)
	{
		//TODO: determine if the extension is degenerate
		//for the library's purposes, and don't set this
		$generator->setRequiresClass();
				
		//echo "in generatecomplextype\n";
		$simple = $content->getElementsByType('xsSimpleContent');		
		if(empty($simple)) 
		$complex = $content->getElementsByType('xsComplexContent');
		$temp = $simple?:$complex;
		if(!empty($temp)) 
		{
			//Should only be one - now we now find out element's parent class
			$content = $temp[0]; $temp = $content->getElements();
			//Should either be an xsExtension or xsRestriction
			$content = $temp[0]; $base = $content->getAttribute('base');
		}
		if(!empty($simple)) //RETURNS
		{
			//echo "found simpleContent!\n";			
			//echo "setting extends to ". $base ."\n";
			//NEW: THIS WILL LATER BE CONVERTED INTO A SIMPLE-TYPE
			$generator->setContentType($base);
			if($content instanceof xsRestriction)
			$generator->bag['base_type'] = $generator->bag['baseTypeViaRestriction'] = $base;
			$temp = $content->getElementsByType('xsAttribute');
			foreach($temp as $ea) $generator->addAttribute($ea);
			return; //RETURNING
		}
		if(!empty($complex)) //RETURNS
		{
			//echo "found complexContent!\n";
			//ComplexContent specified means type is derived						
						
			if($content instanceof xsRestriction)
			$generator->bag['baseTypeViaRestriction'] = $base;
			//echo "setting extends to ". $base ."\n";
			$generator->bag['base_type'] = $base;
			
			//MAYBE THIS IS UNNECESSARY?
			//Generate the complex type this is derived from			
			global $_globals;
			//HACK: this is more or less how this has always been done
			if(@isset($_globals['complex_types'][$base]))
			{
				$generator->bag['#include'][] = $base;
			}
			else //wrapping a simple-type
			{
				//Shouldn't this $generator->setContentType($type)?
				die('die(Is this valid? Shouldn\'t the "value" be setup?)');			
			}
				
			//COMMENTING THIS OUT AS THE RESULT IS THE SAME???
			//parse element context
			//$this->generateCM_isPlural($content,$generator,$content->getAttribute('maxOccurs'));
			//return;
		}//ELSE
		//else echo "found nothing so doing complex content generateContentModel\n";
		//the alternative to xsSimpleContent is xsComplexContent - if it is not specified, it is implied			
		$this->generateCM_isPlural($content,$generator,$content->getAttribute('maxOccurs'));
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
		
			//UNUSED: //not clear this could ever be of any use
			//$generator->setIsExtension($el->getType()=='xsExtension');

			//Set base class //NEW: indirectly sets isString
			//HACK: saved for last so isString can reflect on enumeration/constant-strings
			//$generator->setBase($el->getAttribute('base'));

			//Look for enums
			$enums = $el->getElementsByType('xsEnumeration');
			
			//New: improve ColladaDOM 3 binary-search lookup.
			sort($enums); 
			
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
			$min = $el->getElementsByType('xsLength');
			if(empty($min))
			{			
				$min = $el->getElementsByType('xsMinLength');
				$max = $el->getElementsByType('xsMaxLength');
			}
			else $max = $min;
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
			$generator->setItemType($itemType);			
			break;
		
		case 'xsUnion':
		
			$generator->setUnionMembers($el->getAttribute('memberTypes'));
			break;
		
		default:
		
			die('die(Where is log defined???)');
			$this->log("WARN: unexpected element in xsSimpleType code generation");
		}
	}
}

?>