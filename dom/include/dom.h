/*
 * Copyright 2006 Sony Computer Entertainment Inc.
 *
 * Licensed under the MIT Open Source License, for details please see license.txt or the website
 * http://www.opensource.org/licenses/mit-license.php
 *
 */
#ifndef __DOM__
#define __DOM__

class DAE;
class daeMetaElement;

#ifdef COLLADA_DOM_SUPPORT150
namespace ColladaDOM150 {
extern daeString COLLADA_VERSION;
extern daeString COLLADA_NAMESPACE;

// Register all types
void DLLSPEC registerDomTypes(DAE& dae);

// Register all elements
daeMetaElement* DLLSPEC registerDomElements(DAE& dae);

daeInt DLLSPEC colladaTypeCount();

}
#endif

#ifdef COLLADA_DOM_SUPPORT141
namespace ColladaDOM141 {
extern daeString COLLADA_VERSION;
extern daeString COLLADA_NAMESPACE;

// Register all types
void DLLSPEC registerDomTypes(DAE& dae);

// Register all elements
daeMetaElement* DLLSPEC registerDomElements(DAE& dae);

daeInt DLLSPEC colladaTypeCount();

}
#endif

#endif // __DOM_INTERFACE__
