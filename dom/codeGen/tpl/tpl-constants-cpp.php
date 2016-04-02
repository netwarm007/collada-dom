<?php
/*
 * Copyright 2006 Sony Computer Entertainment Inc.
 *
 * Licensed under the MIT Open Source License, for details please see license.txt or the website
 * http://www.opensource.org/licenses/mit-license.php
 *
 */

echoCode("
$copyright

#include <$1/$1Constants.h>
",$prefix);

$DLLSPEC_preString = 'DLLSPEC '.$meta_prefix.'String';

foreach($constStrings as $el=>$ea)
{
	//hack: [] = '\n'; style entries
	if(is_int($el)){ echo $ea; continue; }
	echo $DLLSPEC_preString, " $el = $ea"; //;\n
}
/*THERE'S NO POINT IN THIS. NONE AT ALL
echo "\n";
foreach($elementTypes as $el=>$ea)
echo $DLLSPEC_preString, " COLLADA_TYPE_$name= \"$ea\";\n";
echo "\n";
foreach($elementNames as $el=>$ea) 
echo $DLLSPEC_preString, " COLLADA_ELEMENT_$name = \"$ea\";\n";*/

?>
