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
if($meta['parent_meta']==NULL) $include_list = array(); 

foreach($meta['#include'] as $ea)
echoInclude($include_list,$ea);
//implicates COLLADA 1.4.1
foreach($meta['elements'] as $k=>$ea) 
{
	$sub =@@ $classmeta[$k]['substitutableWith'];
	if(!empty($sub))
	foreach($sub as $k=>$ea2) echoInclude($include_list,$k);
}

foreach($meta['classes'] as $ea)
echo applyTemplate('class-h-inc',$ea);

?>