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

#ifndef __$1_h__$2
#define __$1_h__$2"
,$meta['element_name'],$include_guard);
//HACK. As long as localy types are inside the
//global classes, there's no way to workaround
//circular-dependencies.
$ob_len = ob_get_length();
echoInclude_dependents($meta,$meta['element_name']);
if($ob_len===ob_get_length())
echo "
#include \"$target_namespace.h\"";	
echo "
COLLADA_H(__FILE__)
COLLADA_(namespace)
{
    COLLADA_($target_namespace,namespace)
    {//-.
//<-----'
";
echo applyTemplate('class-h-def',$meta);
echo "
//-------.
    }//<-'
}

";
echo applyTemplate('class-h-inc',$meta);
if(inline_CM)
echo applyTemplate('classes-cpp',$meta);
echoCode("
#endif //__$1_h__$2",
$meta['element_name'],$include_guard);

?>/*C1071*/