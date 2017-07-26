if(CMAKE_SOURCE_DIR EQUAL CMAKE_CURRENT_SOURCE_DIR)
message(FATAL_ERROR "Please run cmake out of the top-level directory")
endif()

####GLUT##################################

if(NOT COLLADA_DOM__TARGET_PS3)
	try_package(GLUT GLUT)
	if(NOT GLUT_FOUND)
		try_external_libs(freeglut freeglut/trunk/freeglut/freeglut GLUT)
		if(GLUT_INCLUDE_DIRS)
			string(APPEND GLUT_INCLUDE_DIRS "/include")
		endif()
	endif()
	#GLUI? #EXPERIMENTAL	
	add_definitions(-DNO_GLUI) 
endif(NOT COLLADA_DOM__TARGET_PS3)

#This is done separately because it can't use the precompiled header.
add_library(colladadom3D-big-obj OBJECT src/viewer_base.lite.cpp)
#_bigobj is -Wa,-mbig-obj -g0 -O1 for GCC. See its definition in
#the top-level script for more thoughts.
target_compile_options(colladadom3D-big-obj PRIVATE
	${_winbigobj} ${_gccbigobj} ${_Wno-unused-but-set-variable}) 

#-idirafter is needed for fx/include to hide its gl/glext.h file from
#the include path, but it's GCC specific so just hoping glext.h is cross
#platform; written in terms of gl.h.
include_directories(src ../fx/include ../rt/include)

if(COLLADA_DOM__TARGET_PS3)
file(GLOB main 
	projects/vc8-PS3/PS3_main.cpp
	#PS3_BrowserUI.h
	PS3_BrowserUI.cpp)
elseif(WIN32)
set(main projects/vc10-Win/Windows_main.cpp)
elseif(APPLE)
set(main projects/xcode/OSX_main.cpp)
else()
set(main projects/linux/Linux_main.cpp)
endif()

#Headers will never work with CMakePCHCompiler :(
#file(GLOB src "src/*.*") 

add_executable(colladadom3D ${main} $<TARGET_OBJECTS:colladadom3D-big-obj>) #${src}

#SVN-EXTERNALS: cmake-modules/CMakePCHCompiler
target_precompiled_header(colladadom3D src/viewer_base.pch.h)

add_compile_options(${_wincompile})
set_property(TARGET colladadom3D PROPERTY
	#"strings" are needed to guard against 'x called with incorrect number of arguments.' :(
	LINK_FLAGS "${_winlink}")
target_link_libraries(colladadom3D #${_winlink}
	${_whole-archive} colladadomRT colladadomFX colladadom25 ${_no-whole-archive})

#RT components
target_link_libraries(colladadom3D -L/usr/local/lib ${IL_LIBRARIES} ${BULLET_LIBRARIES})

#3D components
target_link_libraries(colladadom3D ${GLUI_LIBRARIES} ${GLUT_LIBRARIES} ${OPENGL_LIBRARIES}) 

#Special care is taken to avoid converting the Windows path to a Cygwin path.
#add_definitions("-I\"$ENV{CG_INC_PATH}\"") just works. -l is just different.
if(4 EQUAL CMAKE_SIZEOF_VOID_P) #==
STRING(REPLACE "\\" "/" CG_LIB_PATH "$ENV{CG_LIB_PATH}")  
else()
STRING(REPLACE "\\" "/" CG_LIB_PATH "$ENV{CG_LIB64_PATH}")  
endif()
target_link_libraries(colladadom3D ${_libpath}${CG_LIB_PATH} Cg CgGL) ##${CG_LIB_PATH} #Must be done up above.

#Various standard libraries. Windows 10 Linux wants these. Just add what's missing :(
if(NOT WIN32 AND NOT CYGWIN)
	target_link_libraries(colladadom3D stdc++ m) #m is math
endif()

#TODO: Create symlink to colladadom3D-X.Y?
set_property(TARGET colladadom3D PROPERTY OUTPUT_NAME colladadom3d)
install(TARGETS colladadom3D RUNTIME DESTINATION bin)
