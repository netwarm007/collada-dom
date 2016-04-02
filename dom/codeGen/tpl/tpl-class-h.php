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

#ifndef __$2$1_h__
#define __$2$1_h__

#include <$3/$3Document.h>
#include <$2/$2Types.h>
#include <$2/$2Elements.h>",
ucfirst($meta['element_name']),$prefix,$meta_prefix);			
echo applyTemplate('class-h-inc',$meta);
echo "\n";
echo applyTemplate('class-h-def',$meta);
echoCode("
#endif //__$2$1_h__
",ucfirst($meta['element_name']),$prefix,$meta_prefix);
