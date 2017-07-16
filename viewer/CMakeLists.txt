if(CMAKE_SOURCE_DIR EQUAL CMAKE_CURRENT_SOURCE_DIR)
message(FATAL_ERROR "Please run cmake out of the top-level directory")
endif()

#This is done separately because it can't use the precompiled header.
add_library(colladadom3D-big-obj OBJECT src/viewer_base.lite.cpp)
#_bigobj is -Wa,-mbig-obj -g0 -O1 for GCC. See its definition in
#the top-level script for more thoughts.
set_source_files_properties(src/viewer_base.lite.cpp PROPERTIES
	COMPILE_FLAGS "${_bigobj} ${_wincompile} ${_Wno-unused-but-set-variable}") 

#-idirafter is needed for fx/include to hide its gl/glext.h file from
#the include path, but it's GCC specific so just hoping glext.h is cross
#platform; written in terms of gl.h.
include_directories(src ../fx/include ../rt/include)

find_package(GLUT QUIET) #GLUT
if(NOT GLUT_FOUND)
message("No GLUT? Assuming PlayStation 3 build (probably not.)")
add_definitions("-DNO_GLUT")
endif()

#EXPERIMENTAL
#..\..\external-libs\GLUI\CMake\Debug\glui_staticd.lib
#find_package(GLUI QUIET)
add_definitions(-DNO_GLUI) #GLUI?

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
	LINK_FLAGS ${_winlink})
target_link_libraries(colladadom3D #${_winlink}
	${_whole-archive} colladadomRT colladadomFX colladadom25 ${_no-whole-archive})

#RT components
target_link_libraries(colladadom3D ${BULLET_LIBRARIES} -L/usr/local/lib ${IL_LIBRARIES})

#3D components
target_link_libraries(colladadom3D ${OPENGL_LIBRARIES} ${GLUT_LIBRARIES} ${GLUI_LIBRARIES} ) 

#Special care is taken to avoid converting the Windows path to a Cygwin path.
#add_definitions("-I\"$ENV{CG_INC_PATH}\"") just works. -l is just different.
if(4 EQUAL CMAKE_SIZEOF_VOID_P) #==
STRING(REPLACE "\\" "/" CG_LIB_PATH "$ENV{CG_LIB_PATH}")  
else()
STRING(REPLACE "\\" "/" CG_LIB_PATH "$ENV{CG_LIB64_PATH}")  
endif()
target_link_libraries(colladadom3D -L"${CG_LIB_PATH}" cg cgGL)

#TODO: Create symlink to colladadom3D-X.Y?
set_property(TARGET colladadom3D PROPERTY OUTPUT_NAME colladadom3d)
install(TARGETS colladadom3D RUNTIME DESTINATION bin)
