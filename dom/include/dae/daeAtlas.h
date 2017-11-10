/*
 * Copyright 2006 Sony Computer Entertainment Inc.
 *
 * Licensed under the MIT Open Source License, for details please see license.txt or the website
 * http://www.opensource.org/licenses/mit-license.php
 *
 */

#ifndef __COLLADA_DOM__DAE_ATLAS_H__
#define __COLLADA_DOM__DAE_ATLAS_H__

#include "daeIOPlugin.h"
  
COLLADA_(namespace)
{//-.
//<-'

/**ZAE
 * @c daeImage is a virtual file; such as an uncompressed
 * part of a ZIP archive that is modified in memory until
 * the entire archive is ready to be written out together.
 *
 * @note Users are largely responsible for creating these
 * objects. It can be implemented with @c std::deque or a
 * @c FILE or @c std::vector or combination or other ways.
 */
class daeImage : public daeContainedObject
{
COLLADA_(private)
	/**
	 * @c _name is set for life.
	 */
	daeClientString _name;

COLLADA_(public) //OPERATORS

	COLLADA_DOM_OBJECT_OPERATORS(daeImage)

COLLADA_(public)
	/**
	 * Constructor
	 */
	daeImage(daeClientString name):_name(name)
	{}
	/**
	 * Alternative Constructor with parentage. 
	 */
	daeImage(daeClientString name, const DAEP::Object *c)
	:daeContainedObject(c),_name(name)
	{}

	/**WARNING
	 * This is a raw string identifiying this image that 
	 * doubles as a numeric (pointer) association with a
	 * real image.
	 *
	 * @warning This string is normally owned by atlases.
	 * It's garbage if the image lingers after the atlas.
	 */
	inline daeClientString getName()const{ return _name; }	
		
	/**
	 * "Deletes" this image. To clarify: When an archive
	 * is opened it's not possible to remove its entries.
	 * Because they refer to the original. By setting it
	 * to be "deleted" when the archive is rewritten the
	 * corresponding image will no longer be part of the
	 * original or derivative archive.
	 *
	 * @note @c clear() does not delete an entry from an
	 * an archive, just as there can be 0 sized files on
	 * a file-system. It must be marked deleted. Also to
	 * "delete" does not make the image 0 sized.
	 *
	 * @warning When a "daeAtlasValue" enum is specified
	 * it could include a way to delete images without a
	 * dummy @c daeImage.
	 */
	inline void setIsDeleted(bool f=true)
	{
		if(f!=getIsDeleted()) _getClassTag()^=1;
	}
	/**
	 * Tells if @c this image's data is "Deleted."
	 * @see @c setIsDeleted() doxygentation.
	 */
	inline bool getIsDeleted()const{ return (_getClassTag()&1)!=0; }

	/**
	 * Tells if @c this image's data has been compressed.
	 * It's likely uncompressed, modified & recompressed.
	 */
	inline bool getIsCompressed()const{ return (_getClassTag()&2)!=0; }

	/**
	 * Tells if @c this image's data is a memory pointer.
	 */
	inline bool getIsContiguous()const{ return (_getClassTag()&4)!=0; }

COLLADA_(public) //daeImage interfaces
	/**MUTABLE
	 * This interface is responsible for most operations.
	 *
	 * @a in is either a buffer to draw from or to fill.
	 * If @c in==nullptr insertion mode fills with junk.
	 *
	 * Erasing is done if only @a range2 is provided.
	 * Insertion is done if @a in and @a range1 is provided.
	 * Erasing+Insertion is done if all three are provided.
	 * Reading is done if @a in and @a range2 is provided.
	 *
	 * @a range1 and @a range2 should be set to the affective
	 * range of @c char data. Or should be emptied on error.
	 * 
	 * @return Returns the modified size of @c this image.
	 */
	virtual size_t data(const void *in, daeIO::Range *range1, daeIO::Range *range2=nullptr)const = 0;
	
	/**MUTABLE
	 * @return Returns @c nullptr if a pointer cannot be facilitated
	 * or locking is not-implemented.
	 *
	 * @a mode is 0 to unlock; 'r' to get a shared read lock; and 'w' 
	 * to get an unshared write lock. Partial locks are not possible.
	 * @note Such locks simply ward off changes to the memory and it
	 * is implied that a 'w' lock changes the memory and 'r' doesn't.
	 *
	 * @a mode 0 may still return a pointer to memory that is useful.
	 * But it should only be used with private images. Writing to it
	 * is undefined-behavior.
	 */
	virtual char *lock(int mode)const{ return nullptr; }
	/**MUTABLE
	 * Called to unlock after using @c lock(). 
	 *
	 * @return Returns @c lock(0) which can be a pointer without any
	 * guarantees.
	 */
	inline char *unlock()const{ return lock(0); }	

COLLADA_(public) //HELPERS
	/**	
	 * Reads a range of @c char sized memory.
	 */
	inline size_t copy(size_t begin, size_t end, void *cp)const
	{
		daeIO::Range r = {begin,end}; 
		data(cp,nullptr,&r); return r.size();
	}
	/**	
	 * Erase a range of @c char sized memory.
	 */
	inline size_t erase(size_t begin, size_t end)
	{
		daeIO::Range r = {begin,end}; 
		data(nullptr,nullptr,&r); return r.size();
	}
	/**	
	 * Inserts a range of @c char sized memory.
	 */
	inline size_t insert(size_t begin, size_t end, const void *cp)
	{
		daeIO::Range r = {begin,end}; 
		data(cp,&r,nullptr); return r.size();
	}
	/**	
	 * Writes a range of @c char sized memory.
	 */
	inline size_t replace(size_t begin, size_t end, const void *cp)
	{
		daeIO::Range r = {begin,end}; 
		data(cp,&r,&r); return r.size();
	}
	/**	
	 * Replaces a range of @c char sized memory.
	 */
	inline size_t replace(size_t begin, size_t end, const void *cp, size_t len)
	{
		daeIO::Range r = {begin,end}; 
		daeIO::Range r2 = {begin,begin+len}; 
		data(cp,&r2,&r); return r.size();
	}
	/**	
	 * Replaces a range of @c char sized memory.
	 */
	inline size_t assign(const void *cp, size_t len)
	{
		daeIO::Range r = {0,-1}; 
		daeIO::Range r2 = {0,0+len}; 
		data(cp,&r2,&r); return r.size();
	}
	/**	
	 * Replaces a range of @c char sized memory.
	 */
	inline size_t append(const void *cp, size_t len)
	{
		size_t n = size();
		daeIO::Range r = {n,n+len};
		data(cp,&r,nullptr); return r.size();
	}
	/**	
	 * Resizes, padding with undefined memory.
	 */
	inline size_t resize(size_t len)
	{
		size_t n = size();
		daeIO::Range r = {len,-1};
		daeIO::Range r2 = {n,len};
		data(nullptr,n<len?&r2:nullptr,&r); return r.size();
	}

COLLADA_(public) //quasi Standard Library compatibility layer	

	inline size_t size()const{ return data(nullptr,nullptr); }

	inline size_t clear(){ return erase(0,size_t(-1)); }

	inline bool empty()const{ return size()==0; }
};

#ifdef NDEBUG
#error SHOULD THE ATLAS RETAIN A REMOTE URI IN CASE THEY
#error CHANGE? OR CAN USERS/PLUGINS KEEP A HANDLE ON IT?
#endif
/**ZAE
 * @c daeAtlas is a virtual file system; such as a compressed
 * archive.
 *
 * PARENTAGE
 * @note daeAtlas is a ref-counted object, however it doesn't
 * belong to a @c daeDOM or a @c daeDatabase. Usually atlases
 * are unparented so that when their @c daeArchive goes, they
 * will go with it. Usually there is only one archive, but it
 * can be that multiple archives in multiple DOMs share these.
 * FURTHERMORE @c daeContainedObject was amended so images do
 * not "strong" reference their atlas...
 *
 * IMAGES
 * Use @c daeContainerObject::contain() to assign @c daeImage
 * objects to @c this atlas. They must be created with a name
 * pointer (that is not just a string) that the atlas owns or
 * they will not be recognized by the atlas. NOTE THAT images
 * are a completely optional feature... or that by default an
 * atlas will not have images since the data is drawn from an
 * I/O channel until for some reason it must be cached or got
 * overridden.
 */
class daeAtlas 
:
public daeContainerObject<0,daeImage>, public daeIOController
{
COLLADA_(public)

	virtual ~daeAtlas(){} //C4624

COLLADA_(public) //OPERATORS

	COLLADA_DOM_OBJECT_OPERATORS(daeAtlas)

COLLADA_(public) //daeAtlas interfaces 
	/**
	 * @note Here you are editing an archive for some application.
	 *
	 * When an atlas is created it typically already has many names
	 * that're derived from a ZIP-like archive that was just opened.
	 * This API adds all new subimages. 
	 *
	 * @return Returns @c nullptr if a new region cannot be created.
	 * @param error retrieves an error code if it is not @c nullptr.
	 * The returned pointer can be used to create a @c daeImage and
	 * add it to @c this atlas via @c daeContainerObject::contain().
	 *
	 * @remark There is no way to remove names. This is because the
	 * original source of the archive would have to remove its name.
	 * To remove the name from future write operations, @c daeImage
	 * has a @c setIsDeleted() method. A "property" framework might
	 * allow the name to be stricken without creating a @c daeImage.
	 */
	virtual daeClientString addName(daeName name, daeOK *error=nullptr) 
	{
		if(error!=nullptr) *error = DAE_ERR_NOT_IMPLEMENTED; return nullptr;
	}

	/**
	 * Gets a sorted list of raw region "names."
	 *
	 * @note "raw" here means that an atlas can store names as it
	 * likes. It may be necessary to translate from raw to pretty
	 * names. It can be discovered with @c daeModel.
	 *
	 * @note @c daeImage objects should be added to the container
	 * array in the same order as the names.
	 *
	 * @param wc is a substring to match against. All matches are
	 * outputted. A name is a match if it starts with @c wc. This
	 * is like specifying a directory. Normally a filename cannot
	 * also be a directory; in which case @a wc can translate any
	 * @c daeString into its "interned" pointer/string equivalent.
	 */
	virtual daeOK getNames(daeArray<daeClientString>&o, daeName wc=nullptr)const
	{
		o.clear(); return DAE_ERR_NOT_IMPLEMENTED; (void)wc;
	}	 	
	
	//EXPERIMENTAL ("daeAtlasValue" still has to be implemented.)
	/**
	 * If @a values is empty it is filled with all available values.
	 * If @a values is nonempty it is @c daeAtlasValue keys to fill.
	 *
	 * @return Returns a mixed list of @c daeAtlasValue @c enum key
	 * values and their corresponding value-values which might span
	 * multiple @c int items according to the key-value's semantics.
	 */
	virtual daeOK getValues(daeArray<int> &values, daeClientString named=nullptr)const
	{
		return DAE_ERR_NOT_IMPLEMENTED;
	}
	/**
	 * @param values is a list of @c daeAtlasValue properties to be 
	 * assigned to the region @a named or @c this @c named==nullptr.
	 */
	virtual daeOK setValues(const int *values, size_t valuesN, daeClientString named=nullptr)
	{
		return DAE_ERR_NOT_IMPLEMENTED;
	}
};

//---.
}//<-'

#endif //__COLLADA_DOM__DAE_ATLAS_H__
/*C1071*/
