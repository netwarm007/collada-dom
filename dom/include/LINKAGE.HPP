		
#ifdef __COLLADA_DOM__LINKAGE_HPP__
//"pop" the LINKAGE macros
#undef __COLLADA_DOM__LINKAGE_HPP__
#undef LINKAGE
#undef NOALIAS_LINKAGE
#undef SNIPPET
#else //"push"
#define __COLLADA_DOM__LINKAGE_HPP__

#ifdef LINKAGE
#error LINKAGE.HPP: LINKAGE is already defined. Non-portable push/pop required. (Collada-DOM 2.5)
#endif
#ifdef SNIPPET
#error LINKAGE.HPP: SNIPPET is already defined. Non-portable push/pop required. (Collada-DOM 2.5)
#endif

	//These are shorthand for heavy use inside class defintions
	//COLLADA_DOM is necessary as Windows clients use dllimport.
	#define LINKAGE COLLADA_DOM_LINKAGE
	//This is supposed to help the compiler reorder/cache calls.
	//The documentation on _declspec(noalias) is not very clear.
	//Whether it works or not, it communicates/documents intent. 
	#define NOALIAS_LINKAGE COLLADA_NOALIAS COLLADA_DOM_LINKAGE

	//SNIPPET can be removed if "noline" can be demonstrated to
	//ALWAYS import methods with inline linkage on all platforms
	//(it's used because anything is better than implementing one-liners inside CPP files. ANYTHING!)
	#ifdef BUILDING_COLLADA_DOM
	//note {} could be omitted, but writing ({ }) seems busy-work
	#define SNIPPET(...) { __VA_ARGS__ }
	#else
	#define SNIPPET(...) ;
	#endif

#endif //push/pop

/*C1071*/