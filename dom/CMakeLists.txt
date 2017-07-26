if(CMAKE_SOURCE_DIR EQUAL CMAKE_CURRENT_SOURCE_DIR)
message(FATAL_ERROR "Please run cmake out of the top-level directory")
endif()

#Assuming outputting a shared library.
add_definitions(-DEXPORTING_COLLADA_DOM)

####EXTERNAL-LIBS############################

try_package(zlib ZLIB)
if(ZLIB_FOUND)	
	string(APPEND PC_LIBS " -lz") #OBSOLETE?
else()
	try_external_libs(z zlib/trunk ZLIB)
endif()

try_package(minizip MINIZIP)
if(MINIZIP_FOUND)	
	string(APPEND PC_LIBS " -lminizip") #OBSOLETE?
else()
	try_external_libs(minizip minizip/trunk MINIZIP)
endif()

if(COLLADA_DOM__LEGACY_TINYXML)
	try_external_lib(tinyxml tinyxml/trunk TINYXML)
	include_directories(${TINYXML_INCLUDE_DIRS})
endif()

if(COLLADA_DOM__LEGACY_LIBXML2)
	try_package(LibXml2 LIBXML2)
	if(LIBXML2_FOUND)
		string(APPEND PC_LIBS " -lxml2") #OBSOLETE?	
	else()
		#EXPERIMENTAL
		#libxml2 doesn't as of yet have a CMakeLists.txt
		#file, so one has been provided.
		try_external_libs(xml2 libxml2/cmake LIBXML2)	
		if(LIBXML2_INCLUDE_DIRS)
			string(APPEND LIBXML2_INCLUDE_DIRS "/../trunk/include")
			list(INSERT LIBXML2_LIBRARIES 0 PRIVATE)
		endif()
	endif()
endif()

if(LIBXML2_LIBRARIES)
	add_definitions(${LIBXML2_DEFINITIONS} -DBUILDING_IN_LIBXML)
else()
	set(LIBXML2_LIBRARIES "") #YUCK: CMake cries NOTFOUND :(
endif()

#HACK: The top-level script does this.
set(PC_LIBS "${PC_LIBS}" PARENT_SCOPE) #OBSOLETE?	

####BUILD/INSTALL############################

#old note: declare minizip/zlib before libxml2!
# (for some reason the precompiled libxml2 libraries have zlib.h/zconf.h)
include_directories(${MINIZIP_INCLUDE_DIRS} ${ZLIB_INCLUDE_DIRS} ${LIBXML2_INCLUDE_DIRS})

#Headers will never work with CMakePCHCompiler :(
file(GLOB files "src/dae/*.cpp") #"include/*.*" "include/dae/*.h" "include/modules/*.h") 

if(MSVC) 
	list(APPEND files projects/NatvisFile.natvis)
endif()

#Assuming SHARED for time being.
add_library(colladadom25 SHARED ${files} src/modules/LIBXMLPlugin/daeLIBXMLPlugin.cpp)

if(COLLADA_DOM__ENABLE_LIBRARY_PCH) #SOFT-DISABLING
#Historically I've used ColladaDOM.inl as the precompiled header, but it includes
#every single header, and so can be very confusing if users don't understand that
#changing the headers requires rebuilding the PCH file.
#SVN-EXTERNALS: cmake-modules/CMakePCHCompiler
target_precompiled_header(colladadom25 include/ColladaDOM.inl)
endif()

set_target_properties(colladadom25 PROPERTIES 
	POSITION_INDEPENDENT_CODE TRUE ##Assuming SHARED for time being.
	#"strings" are needed to guard against 'x called with incorrect number of arguments.' :(
	COMPILE_FLAGS "${_wincompile}"
	LINK_FLAGS "${_winlink}")

target_link_libraries(colladadom25 ${MINIZIP_LIBRARIES} ${ZLIB_LIBRARIES} ${LIBXML2_LIBRARIES})

#### Monkey see, monkey do??? ################

install(TARGETS colladadom25 EXPORT ${PROJECT_NAME}Targets 
	RUNTIME DESTINATION bin
	LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
	ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
	INCLUDES DESTINATION include/${PROJECT_NAME})

install(DIRECTORY include/dae 
	DESTINATION include/${PROJECT_NAME} COMPONENT Devel PATTERN ".svn")
install(FILES 
include/dae.h 
include/DAEP.h 
include/LINKAGE.HPP
include/WARNING.HPP
include/ColladaDOM.inl
include/modules/daeSTLDatabase.h DESTINATION include/${PROJECT_NAME} COMPONENT Devel)

