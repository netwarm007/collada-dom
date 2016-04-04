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
		'listType'			=>'',
		//previously enum had parallel tracks:
		//'enum_documentation' and 'enum_value'
		//now the keys of the array are the enum
		//this is to be able to use foreach loops 
		'enum'				=>array(),
		'restrictions'		=>array(),
		'isExtension'		=>true,
		'isComplex'			=>false,
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
	static function& generateXMLSchemaTypes($lists, $listypes, $strings) 
	{
		$xs = new TypeMeta(); //antipattern
		foreach(explode(' ',$strings) as $ea)
		{ 
			$xs->TypeMeta(); $xs->setType($ea); $xs->bag['isString'] = true;
			self::$generated[$ea] = $xs->GetMeta(); 		
		}
		if(!is_array($lists))
		$lists = array_combine(explode(' ',$lists),explode(' ',$listypes));
		foreach($lists as $ea=>$ea_listype)
		{
			$xs->TypeMeta(); $xs->setType($ea);	$xs->setListType($ea_listype);
			self::$generated[$ea] = $xs->GetMeta();
		}//todo? add scalar types: eg. xs:double
		return self::$generated;
	}
	
	function setBase($base)
	{
		die('die(Sorry! See andFinally_setBase)');
	}
	
	function setListType($type)
	{
		$this->bag['listType'] = $type;
		$this->bag['isArray'] = true; //synonymous?
	}

	function setIsExtension($bool)
	{
		$this->bag['isExtension'] = $bool;
	}

	function setIsComplex($bool)
	{
		$this->bag['isComplex'] = $bool;
	}

	function setRestriction($name,$val)
	{
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
		$base = @self::$generated[$b]; if(!empty($base))
		{	
			if(empty($this->bag['enum'])||$this->bag['useConstStrings'])
			$this->bag['isString'] = $base['isString'];
			$this->bag['isArray'] = $base['isArray'];
		}//should be a scalar type then: eg. xs:double
		//else die('missing base type, or types appear out-of-order in the XSD schema');
	}
}

?>