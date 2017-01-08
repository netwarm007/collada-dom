<?php
/*
 * Copyright 2006 Sony Computer Entertainment Inc.
 *
 * Licensed under the MIT Open Source License, for details please see license.txt or the website
 * http://www.opensource.org/licenses/mit-license.php
 *
 */

if(inline_CM)
{
	echo '//Generator inline_CM==true put class definitions in their h files.';
	return;
}	
if(2!=$COLLADA_DOM)
{
	echo '//ColladaDOM 3 doesn\'t require this file. It\'s legacy only anyway.';
	return;
}
	
echoCode("
$copyright

#ifndef __$1_CONSTANTS_H__$2
#define __$1_CONSTANTS_H__$2",
strtoupper($prefix),$include_guard);

echo "
COLLADA_(namespace)
{
    COLLADA_($target_namespace,namespace)
    {//-.
//<-----'
";

$DLLSPEC_preString = 'extern DLLSPEC daeString';

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
}} //COLLADA_($target_namespace,namespace)

#endif //__$1_CONSTANTS_H__$2",strtoupper($prefix),$include_guard);

?>/*C1071*/