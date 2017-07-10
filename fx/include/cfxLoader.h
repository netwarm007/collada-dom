/*
 * Copyright 2006 Sony Computer Entertainment Inc.
 *
 * Licensed under the MIT Open Source License, for details please see license.txt or the website
 * http://www.opensource.org/licenses/mit-license.php
 *
 */
#ifndef __COLLADA_FX__LOADER_H__
#define __COLLADA_FX__LOADER_H__

#include "cfxTypes.h"
#include "cfxEffect.h"

COLLADA_(namespace)
{
	namespace FX
	{//-.
//<-----'

struct ColorSpace
{
	bool sRGB;
	operator bool()const{ return sRGB; }

	ColorSpace(bool sRGB=false):sRGB(sRGB){}
	ColorSpace(daeName space){ sRGB = space=="sRGB"; }
	ColorSpace(Collada05_XSD::fx_surface_format_hint_option_enum e)
	{
		sRGB = e==e.SRGB_GAMMA;
	}
};

class Loader
{
COLLADA_(public)
	
	daeArray<daeName,4> platform_Filter;

	CGcontext Cg; Loader():Cg(){}

	/**
	 * @c Load() calls this to let the client
	 * hook up world matrices and lights data.
	 * @see @c COLLADA::RT::Frame_FX.
	 */
	virtual void Load_ClientData(FX::NewParam*) = 0;
	/**INTERNAL
	 */
	inline void Copy_ClientData(FX::NewParam *np, FX::Data* &cp)
	{
		np->Apply(); Load_ClientData(np); cp = np->ClientData;
	}
	
	struct Load;
	/**LEGACY-SUPPORT
	 * @return Returns @c nullptr if the profile is not
	 * implemented or @c cg==nullptr and the profile is
	 * Cg for example.
	 * @note Historically only a small part of Cg could
	 * be support. In theory other "FX" profiles should
	 * be enabled, and better supported.
	 */
	FX::Effect *Load(Collada05::const_effect),*Load(Collada08::const_effect);
	/**LEGACY-SUPPORT
	 * Create an @c FX::Material for @c instance_effect.
	 */
	FX::Material *Load(Collada05::const_material,FX::Effect*,const daeDocument*);
	/**LEGACY-SUPPORT
	 * Create an @c FX::Material for @c instance_effect.
	 */
	FX::Material *Load(Collada08::const_material,FX::Effect*,const daeDocument*);

	//2017: THE RT COMPONENT IS PROVIDING THIS UNTIL IT CAN BE REMOVED.
	//SCHEDULED FOR REMOVAL
	//SCHEDULED FOR REMOVAL
	//SCHEDULED FOR REMOVAL
	static GLuint GetID_TexId(Collada05::const_image,FX::ColorSpace=false);
	static GLuint GetID_TexId(Collada08::const_image,FX::ColorSpace=false);
};

//-------.
	}//<-'
}

#endif //__COLLADA_FX__LOADER_H__
/*C1071*/