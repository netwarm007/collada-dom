<?php
/*
 * Copyright 2006 Sony Computer Entertainment Inc.
 *
 * Licensed under the MIT Open Source License, for details please see license.txt or the website
 * http://www.opensource.org/licenses/mit-license.php
 *
 */

class TypeMeta //antipattern
{
	var $bag; //antipattern
	function& getMeta(){ return $this->bag; }
	function TypeMeta()
	{
		$bag = array(
		'type'				=>'',
		'base'				=>'',
		'itemType'			=>'',
		//previously enum had parallel tracks:
		//'enum_documentation' and 'enum_value'
		//now the keys of the array are the enum
		//this is to be able to use foreach loops 
		'enum'				=>array(),
		'restrictions'		=>array(),
		//not clear this could ever be of any use
		//'isExtension'		=>true, //UNUSED
		'useConstStrings'	=>false,
		'documentation'		=>'', //array()		
		'union_type'		=>false,
		'union_members'		=>'',
		//UNLISTED
		///there is also an inner 'simple_type'
		//that is itself a TypeMeta: xsElement.php
		//OUTPUT MODIFIERS
		//these refer to the C++ types instead of XML 
		//how could the names reflect that difference??		
		//(NOTE: It'd be cool to target non-C++ languages)
		'isArray'			=>false, 
		'isString'			=>false, //NEW
		//Hack: includeTypeInSchema()
		'inSchema'			=>false, //NEW
		);
		$this->bag =& $bag;
	}

	function setType($t)
	{
		$this->bag['type'] = $t;
	}

	//SCHEDULED OBSOLETE
	//give setBase access to $typemeta
	static $generated = array(); //$string/listtypes moved to gen.php
	static function& generateXMLSchemaTypes($lists, $listypes, $strings, $scalars) 
	{
		$s = '/\s+/';
		$xs = new TypeMeta(); //antipattern
		foreach(preg_split($s,$strings) as $ea)
		{ 
			$xs->TypeMeta(); $xs->setType($ea); $xs->bag['isString'] = true;
			self::$generated[$ea] = $xs->GetMeta(); 		
		}
		if(!is_array($lists))
		$lists = array_combine(preg_split($s,$lists),preg_split($s,$listypes));
		foreach($lists as $ea=>$ea_itemType)
		{
			$xs->TypeMeta(); $xs->setType($ea);
			//$xs->setItemType($ea_itemType); //HACK: Non-schema...
			$xs->bag['itemType'] = $ea_itemType; $xs->bag['isArray'] = true;
			self::$generated[$ea] = $xs->GetMeta();
		}
		foreach(preg_split($s,$scalars) as $ea)
		{ 
			$xs->TypeMeta(); $xs->setType($ea); 
			self::$generated[$ea] = $xs->GetMeta(); 		
		}
		return self::$generated;
	}
	
	function setBase($base)
	{
		die('die(Sorry! See andFinally_setBase)');
	}
	
	function setItemType($type)
	{
		$this->bag['itemType'] = $type;
		$this->bag['isArray'] = true; //synonymous?
		
		//HACK: see andFinally_setBase() explanation
		self::$generated[$type]['inSchema'] = true;
	}

	//function setIsExtension($bool) //UNUSED
	//{
	//	$this->bag['isExtension'] = $bool;
	//}

	function setRestriction($name,$val)
	{
		if($val=='unbounded') $val = unbounded;
		$this->bag['restrictions'][$name] = $val;
	}

	function setAppInfo($ap)
	{
		if(trim($ap)=='constant-strings')
		{
			$this->bag['isString'] = true; //NEW
			$this->bag['useConstStrings'] = true;		
		}
		else die("die(unhandled xs:appinfo: $ap)");
	}

	function setDocumentation($doc)
	{
		$this->doc = $doc;
		$this->bag['documentation'] = $doc; //['en']
	}

	function addEnum($val)
	{
		$enum =& $this->bag['enum'][$val];
		if(empty($enum)) $enum = array();
	}
	function addEnumDoc($enum,$add)
	{
		$doc =& $this->bag['enum'][$enum]['documentation'];
		if(!empty($doc)) $doc.='\n'; $doc.=trim($add);
	}
	function addEnumAppInfo($enum,$ai)
	{
		$kv = explode('=',$ai);
		$val =& $this->bag['enum'][$enum][trim($kv[0])];
		if(!empty($val)){ $val.='\n'; die("die(unexpected double-value)"); }
		$val = trim($kv[1]);
		//echo "found app info\n";
		//echo "its in the correct format\n";		
		//echo $kv[0], ' is ', $kv[1], '\n'; //previously assumed 'value is'
		if($kv[0]!='value') die('die(is this not \"the correct format\"??)');
	}

	function setUnionMembers($um)
	{
		$this->bag['union_type'] = true;
		$this->bag['union_members'] = $um;
	}
	
	//HACK: isString needs to reflect on enum/useConstStrings
	function andFinally_setBase($b) 
	{
		$this->bag['base'] = $b;		
		//NEW: simplifying inheritance model by exposing $typemeta global
		$base =& self::$generated[@$b]; if(!empty($base))
		{	
			if(empty($this->bag['enum'])||$this->bag['useConstStrings'])
			$this->bag['isString'] = $base['isString'];
			if($this->bag['isArray']=$base['isArray'])
			{	
				$r2 = $base['restrictions']; if(!empty($r2))
				{
					$r =& $this->bag['restrictions']; 
					if(!isset($r['minLength'])) $r['minLength'] =@@ $r2['minLength'];
					if(!isset($r['maxLength'])) $r['maxLength'] =@@ $r2['maxLength'];
				}
			}		
		}//should be a scalar type then: eg. xs:double
		//else die('missing base type, or types appear out-of-order in the XSD schema');		
		//Hack. add a dummy type for any imported base types
		//Note, they'll need to be before their derived type
		//Hack. omit unused TypeMeta::generateXMLSchemaTypes
		self::$generated[$b]['inSchema'] = true;
	}
}

?>