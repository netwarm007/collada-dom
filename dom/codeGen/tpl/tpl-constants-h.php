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

#ifndef __$1_CONSTANTS_H__
#define __$1_CONSTANTS_H__

#include <$2/$2DomTypes.h>

",strtoupper($prefix),$meta_prefix);

$DLLSPEC_preString = 'extern DLLSPEC '.$meta_prefix.'String';

foreach($constStrings as $el=>$ea)
{
	////hack: [] = '\n'; style entries
	if(is_int($el)){ echo $ea; continue; }
	echo $DLLSPEC_preString, " $el;\n";
}
/*THERE'S NO POINT IN THIS. NONE AT ALL
echo "\n";
foreach($elementTypes as $el=>$ea)
echo $DLLSPEC_preString, " COLLADA_TYPE_$el;\n";
echo "\n";
foreach($elementNames as $el=>$ea)
echo $DLLSPEC_preString, " COLLADA_ELEMENT_$el;\n";*/

echoCode("
	
#endif //__$1_CONSTANTS_H__
",strtoupper($prefix));

?>

