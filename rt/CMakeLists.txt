if(CMAKE_SOURCE_DIR EQUAL CMAKE_CURRENT_SOURCE_DIR)
message(FATAL_ERROR "Please run cmake out of the top-level directory")
endif()

#This is how RT knows it's building itself.
add_definitions(-DPRECOMPILING_COLLADA_RT)

####BULLET################################

if(NOT BULLET_FOUND)
	#Disable various clutter for end-user's sake :)
	set(BUILD_BULLET2_DEMOS OFF CACHE INTERNAL "" FORCE)
	set(BUILD_BULLET3 OFF CACHE INTERNAL "" FORCE)
	set(BUILD_CLSOCKET OFF CACHE INTERNAL "" FORCE)
	set(BUILD_CPU_DEMOS OFF CACHE INTERNAL "" FORCE)
	set(BUILD_ENET OFF CACHE INTERNAL "" FORCE)
	set(BUILD_EXTRAS OFF CACHE INTERNAL "" FORCE)
	set(BUILD_OPENGL3_DEMOS OFF CACHE INTERNAL "" FORCE)
	set(BUILD_PYBULLET OFF CACHE INTERNAL "" FORCE)
	set(BUILD_UNIT_TESTS OFF CACHE INTERNAL "" FORCE)
	#Disabling this would be nice, but CMake makes it difficult.
	#cmake-gui's Advanced menu is the better way to do this.
	set(USE_MSVC_RUNTIME_LIBRARY_DLL ON CACHE INTERNAL "" FORCE)
	if(NOT COLLADA_DOM__SINGLE_PRECISION)
	set(USE_DOUBLE_PRECISION ON CACHE BOOL "" FORCE)
	endif()
	try_external_libs(bullet bullet/trunk BULLET)
	if(BULLET_LIBRARIES)
		string(APPEND BULLET_INCLUDE_DIRS "/src")	
		set(BULLET_LIBRARIES "LinearMath;BulletCollision;BulletDynamics")
	else()
		add_definitions(-DNO_BULLET)
		message(STATUS "No Bullet physics library; Means no physics simulations?")
	endif()
endif()
#HACK: Pass to colladadom3D	
set(BULLET_LIBRARIES "${BULLET_LIBRARIES}" PARENT_SCOPE) 

####DEVIL##################################

try_package(IL IL) #legacy image library
if(NOT IL_FOUND)
	add_definitions("-DIL_STATIC_LIB")
	try_external_libs(IL devil/trunk/DevIL/src-IL IL)
	if(IL_LIBRARIES)
		string(APPEND IL_INCLUDE_DIRS "/../include")			
	endif()
endif()
if(NOT IL_LIBRARIES)
	message("No DevIL image library, try 'ln -s /usr/local/lib/pkgconfig/IL.pc /lib/pkgconfig/IL.pc'?")
	message("No DevIL image library, maybe try https://unix.stackexchange.com/questions/18182/tell-pkg-config-to-look-pc-files-also-in-usr-local-lib-pkgconfig-system-wide")
	#For the PlayStation 3?
	add_definitions("-DNO_IL")
	message(STATUS "No DevIL image library. TGA images only mode?")
endif()
#HACK: Pass to colladadom3D	
set(IL_LIBRARIES "${IL_LIBRARIES}" PARENT_SCOPE) 

####BUILD/NO-INSTALL#########################

include_directories(include ../fx/include)

include_directories(${BULLET_INCLUDE_DIRS} ${IL_INCLUDE_DIRS})

#Headers will never work with CMakePCHCompiler :(
file(GLOB files "src/*.cpp") #"include/*.h") 

add_library(colladadomRT STATIC ${files})
add_compile_options(${_wincompile}) #${_winlink}

#SVN-EXTERNALS: cmake-modules/CMakePCHCompiler
target_precompiled_header(colladadomRT include/RT.pch.h)