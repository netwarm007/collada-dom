<?php

/*
 * Copyright 2006 Sony Computer Entertainment Inc.
 *
 * Licensed under the MIT Open Source License, for details please see license.txt or the website
 * http://www.opensource.org/licenses/mit-license.php
 *
 */

// INCLUDES

//clear duplicate-entry filter
if($meta['parent_meta']==NULL); $include_list = array(); 

//NEW: check for : excludes imports
//(could also in_array($import_list))
foreach($meta['ref_elements'] as $ea)
if(!in_array($ea,$include_list)&&strpos($ea,':')===false)
{
	$include_list[] = $ea; 
	echo "#include <$prefix/$prefix", ucfirst($ea), ".h>\n";
}//one more to go?
if(@$classmeta[$meta['content_type']]['isComplexType'])
{
	$ea = $meta['content_type']; 
	if(!in_array($ea,$include_list)&&strpos($ea,':')===false)
	{
		$include_list[] = $ea;
		echo "#include <$prefix/$prefix", ucfirst($ea), ".h>\n";
	}
}

foreach($meta['inline_elements'] as $ea) echo applyTemplate('class-h-inc',$ea);

?>