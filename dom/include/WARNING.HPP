		
//This file quiets warnings that are deemed unwarranted
//so that -Wall can be used.
//NOTE: It only applies to GCC because it can't spotfix
//warnings. 
//It is in the form of this file because it needs to be
//reusable since precompiled don't carry these settings
//over, and it's more flexible.

#ifdef __COLLADA_DOM__WARNING_HPP__
//"pop" the WARNING states
#undef __COLLADA_DOM__WARNING_HPP__
#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif
#else //"push"
#define __COLLADA_DOM__WARNING_HPP__
#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Winit-self" //useful
#pragma GCC diagnostic ignored "-Wparentheses" //bad advice
#pragma GCC diagnostic ignored "-Wuninitialized" //userful
#pragma GCC diagnostic ignored "-Wdelete-incomplete" //bad advice
#pragma GCC diagnostic ignored "-Wunused-local-typedefs" //useful
#pragma GCC diagnostic ignored "-Wnonnull-compare" //newer
#pragma GCC diagnostic ignored "-Wmisleading-indentation" //Huh?
#pragma GCC diagnostic ignored "-Waddress" //Is this a threat???
#endif
#endif //push/pop

/*C1071*/
