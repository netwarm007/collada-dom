if(CMAKE_SOURCE_DIR EQUAL CMAKE_CURRENT_SOURCE_DIR)
message(FATAL_ERROR "Please run cmake out of the top-level directory")
endif()

#This is how RT knows it's building itself.
add_definitions(-DPRECOMPILING_COLLADA_RT)

####BULLET################################

if(NOT BULLET_FOUND)
	try_external_libs(bullet bullet/trunk BULLET)
	if(BULLET_INCLUDE_DIRS)
		string(APPEND BULLET_INCLUDE_DIRS "/src")
		if(NOT COLLADA_DOM__SINGLE_PRECISION)
			set(USE_DOUBLE_PRECISION ON) #Bullet's option.
		endif()		
	else()
		add_definitions(-DNO_BULLET)
		message(STATUS "No Bullet physics library; Means no physics simulations?")
	endif()
endif()

####DEVIL##################################

try_package(IL IL) #legacy image library
if(NOT IL_FOUND)
	try_external_libs(IL devil/trunk/DevIL/src-IL IL)
	if(IL_INCLUDE_DIRS)
		string(APPEND IL_INCLUDE_DIRS "/include")
	endif()
endif()
if(NOT IL_INCLUDE_DIRS)
	message("No DevIL image library, try 'ln -s /usr/local/lib/pkgconfig/IL.pc /lib/pkgconfig/IL.pc'?")
	message("No DevIL image library, maybe try https://unix.stackexchange.com/questions/18182/tell-pkg-config-to-look-pc-files-also-in-usr-local-lib-pkgconfig-system-wide")
	#For the PlayStation 3?
	add_definitions("-DNO_IL")
	message(STATUS "No DevIL image library. TGA images only mode?")
endif()

####BUILD/NO-INSTALL#########################

include_directories(include ../dom/include ../fx/include)

include_directories(${BULLET_INCLUDE_DIRS} ${IL_INCLUDE_DIRS})

#Headers will never work with CMakePCHCompiler :(
file(GLOB files "src/*.cpp") #"include/*.h") 

add_library(colladadomRT STATIC ${files})
add_compile_options(${_wincompile}) #${_winlink}

#SVN-EXTERNALS: cmake-modules/CMakePCHCompiler
target_precompiled_header(colladadomRT include/RT.pch.h)