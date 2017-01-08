cmake_minimum_required (VERSION 2.6.0)
project (collada-dom)
set( CMAKE_ALLOW_LOOSE_LOOP_CONSTRUCTS TRUE )

# Define here the needed parameters
set (COLLADA_DOM_VERSION_MAJOR 2)
set (COLLADA_DOM_VERSION_MINOR 4)
set (COLLADA_DOM_VERSION_PATCH 0)
set (COLLADA_DOM_VERSION ${COLLADA_DOM_VERSION_MAJOR}.${COLLADA_DOM_VERSION_MINOR}.${COLLADA_DOM_VERSION_PATCH})
set (COLLADA_DOM_SOVERSION ${COLLADA_DOM_VERSION_MAJOR}.${COLLADA_DOM_VERSION_MINOR})

message(STATUS "Compiling Collada DOM Version ${COLLADA_DOM_VERSION}")
message(STATUS "Using cmake version ${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION}" )

# http://www.cmake.org/cmake/help/cmake-2.6.html#policy:CMP0002
cmake_policy(SET CMP0002 NEW)
# http://www.cmake.org/cmake/help/cmake-2.6.html#policy:CMP0003
cmake_policy(SET CMP0003 NEW)

# Use, i.e. don't skip the full RPATH for the build tree
set(CMAKE_SKIP_BUILD_RPATH  FALSE)

# When building, don't use the install RPATH already
# (but later on when installing)
set(CMAKE_BUILD_WITH_INSTALL_RPATH FALSE)

# The RPATH to be used when installing
set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib${LIB_SUFFIX}")

# Add the automatically determined parts of the RPATH
# which point to directories outside the build tree to the install RPATH
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

message(STATUS "installing to ${CMAKE_INSTALL_PREFIX}")

##############################################################################
# Custom CMake options
##############################################################################

option(OPT_COMPILE_VIEWER "Enable collada viewer compilation" OFF)
option(OPT_COMPILE_FX "Enable collada fx compilation" OFF)
option(OPT_COMPILE_RT "Enable collada rt compilation" OFF)
option(OPT_COMPILE_TESTS "Compile collada tests" OFF)
option(OPT_BUILD_PACKAGES "Set to ON to generate CPack configuration files and packaging targets" OFF)
option(OPT_BUILD_PACKAGE_DEFAULT "Set to ON to generate a default openrave package that creates symlinks" ON)
option(OPT_DOUBLE_PRECISION "Use double precision for everything (daeFloat included)" ON)

set(COLLADA_DOM_EXTERNAL_FLAGS "${COLLADA_DOM_EXTERNAL_FLAGS}")

set(CPACK_DEBIAN_PACKAGE_NAME collada-dom${COLLADA_DOM_SOVERSION})
if(OPT_DOUBLE_PRECISION)
  set(COLLADA_PRECISION "dp")
  set(CPACK_DEBIAN_PACKAGE_NAME "${CPACK_DEBIAN_PACKAGE_NAME}-dp")
  set(COLLADA_DOM_EXTERNAL_FLAGS "${COLLADA_DOM_EXTERNAL_FLAGS} -DCOLLADA_DOM_FLOAT=double") # daeFloat is 64 bit
  message(STATUS "compiling with double float precision")
else()
  set(COLLADA_PRECISION "sp")
  set(CPACK_DEBIAN_PACKAGE_NAME "${CPACK_DEBIAN_PACKAGE_NAME}-sp")
  message(STATUS "compiling with single float precision")
endif()


if( OPT_COMPILE_VIEWER )
  message(WARNING "compiling collada viewer currently not supported")
endif()
if( OPT_COMPILE_FX )
  message(WARNING "compiling collada fx currently not supported")
endif()
if( OPT_COMPILE_RT )
  message(WARNING "compiling collada rt currently not supported")
endif()

set(PACKAGE_VERSION "0" CACHE STRING "the package-specific version used for uploading the sources")

set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake-modules")
set(COMPONENT_PREFIX "${CPACK_DEBIAN_PACKAGE_NAME}")
string(TOUPPER ${COMPONENT_PREFIX} COMPONENT_PREFIX_UPPER)
set(CPACK_COMPONENTS_ALL ${COMPONENT_PREFIX}-base ${COMPONENT_PREFIX}-dev)

if( MSVC )
  if( MSVC70 OR MSVC71 )
    set(MSVC_PREFIX "vc70")
  elseif( MSVC80 )
    set(MSVC_PREFIX "vc80")
  elseif( MSVC90 )
    set(MSVC_PREFIX "vc90")
  else()
    set(MSVC_PREFIX "vc100")
  endif()
  set(COLLADA_DOM_LIBRARY_SUFFIX "${COLLADA_DOM_SOVERSION}-${COLLADA_PRECISION}-${MSVC_PREFIX}-mt")
else()
  set(COLLADA_DOM_LIBRARY_SUFFIX "${COLLADA_DOM_SOVERSION}-${COLLADA_PRECISION}")
endif()

include(CheckIncludeFile)
include(CheckIncludeFileCXX)
include(CheckLibraryExists)
include(CheckFunctionExists)
include(CheckCXXSourceCompiles)
#include(CheckCXXSourceRuns)
#include(CheckCXXCompilerFlag)
include(CheckTypeSize)
find_package(PkgConfig)

if( CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUCXX )
  add_definitions("-fno-strict-aliasing -Wall")
endif()

if( CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUCXX )
  set(CMAKE_CXX_FLAGS_RELEASE "-O2")# -DNDEBUG -D_SECURE_SCL=0") # this practically removes all checks making it a very dangerous options to play with
  set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "-O2 -g")
  set(CMAKE_CXX_FLAGS_DEBUG "-g -D_DEBUG")
endif()

if( UNIX OR CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUCXX )
  set(STDC_LIBRARY stdc++)
else()
  set(STDC_LIBRARY)
endif()

if( APPLE OR ${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
  # apple doesn't have 64bit versions of file opening functions, so add them
  add_definitions("-Dfopen64=fopen -Dfseeko64=fseeko -Dfseek64=fseek -Dftell64=ftell -Dftello64=ftello")
endif()

set(COLLADA_DOM_INCLUDE_INSTALL_DIR "include/collada-dom${COLLADA_DOM_SOVERSION}")

set(COLLADA_DOM_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/dom/include)
set(COLLADA_DOM_LINK_DIRS "")

if(CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUCXX)
  set(EXTRA_COMPILE_FLAGS "${COLLADA_DOM_EXTERNAL_FLAGS} -fPIC")
else()
  set(EXTRA_COMPILE_FLAGS "${COLLADA_DOM_EXTERNAL_FLAGS}")
endif()

find_package(ZLIB)
if( NOT ZLIB_FOUND )
  message(STATUS "compiling zlib from souces and linking statically")
  # compile from sources
  add_subdirectory(dom/external-libs/zlib-1.2.5)
  set(ZLIB_FOUND 1)
  set(ZLIB_LIBRARIES zlib)
  set(ZLIB_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/dom/external-libs/zlib-1.2.5)
endif()

find_package(LibXml2)
if( LIBXML2_FOUND )
  include_directories(${LIBXML2_INCLUDE_DIR})
  add_definitions(${LIBXML2_DEFINITIONS} -DBUILDING_IN_LIBXML)
  message(STATUS "libxml2 found")
else()

pkg_check_modules(minizip minizip)
if(minizip_FOUND)
  set(MINIZIP_INCLUDE_DIR ${minizip_INCLUDE_DIRS})
else()
  message(STATUS "compiling minizip from sources and linking statically")
  add_subdirectory(dom/external-libs/minizip-1.1)
  set(MINIZIP_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/dom/external-libs/minizip-1.1 ${ZLIB_INCLUDE_DIR})
endif()

# declare minizip/zlib before libxml2! (for some reason the precompiled libxml2 libraries have zlib.h/zconf.h)
include_directories(BEFORE ${MINIZIP_INCLUDE_DIR})

link_directories(${COLLADA_DOM_LINK_DIRS})

add_subdirectory(dom)

if(UNIX)
  configure_file("${CMAKE_CURRENT_SOURCE_DIR}/collada-dom.pc.in" "${CMAKE_CURRENT_BINARY_DIR}/collada-dom.pc" @ONLY IMMEDIATE)
  install(FILES ${CMAKE_CURRENT_BINARY_DIR}/collada-dom.pc DESTINATION lib${LIB_SUFFIX}/pkgconfig COMPONENT ${COMPONENT_PREFIX}-dev)
endif()

configure_file("${CMAKE_CURRENT_SOURCE_DIR}/collada_dom-config.cmake.in" "${CMAKE_CURRENT_BINARY_DIR}/collada_dom-config.cmake" @ONLY IMMEDIATE)
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/collada_dom-config-version.cmake.in" "${CMAKE_CURRENT_BINARY_DIR}/collada_dom-config-version.cmake" @ONLY IMMEDIATE)
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/collada_dom-config.cmake" "${CMAKE_CURRENT_BINARY_DIR}/collada_dom-config-version.cmake" DESTINATION "lib${LIB_SUFFIX}/cmake/collada_dom-${COLLADA_DOM_SOVERSION}" COMPONENT ${COMPONENT_PREFIX}-dev)

# add make uninstall capability
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/cmake-modules/cmake_uninstall.cmake.in" "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake" IMMEDIATE @ONLY)

add_custom_target(uninstall "${CMAKE_COMMAND}" -P "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake")

if(CMAKE_CPACK_COMMAND AND UNIX AND OPT_BUILD_PACKAGES)
  # Packing information
  set(CPACK_PACKAGE_NAME collada-dom${COLLADA_DOM_SOVERSION})
  set(CPACK_PACKAGE_CONTACT "" CACHE STRING "Package maintainer and PGP signer.")
  set(CPACK_PACKAGE_VENDOR "http://sourceforge.net/projects/collada-dom")
  set(CPACK_PACKAGE_DISPLAY_NAME "collada-dom ${COLLADA_DOM_VERSION}")
  set(CPACK_PACKAGE_DESCRIPTION_SUMMARY " - COLLADA")
  if( OPT_DOUBLE_PRECISION )
    set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "${CPACK_PACKAGE_DESCRIPTION_SUMMARY} using double precision")
  else()
    set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "${CPACK_PACKAGE_DESCRIPTION_SUMMARY} using single precision")
  endif()
  set(CPACK_PACKAGE_VERSION ${COLLADA_DOM_VERSION}.${PACKAGE_VERSION})
  set(CPACK_PACKAGE_VERSION_MAJOR ${COLLADA_DOM_VERSION_MAJOR})
  set(CPACK_PACKAGE_VERSION_MINOR ${COLLADA_DOM_VERSION_MINOR})
  set(CPACK_PACKAGE_VERSION_PATCH ${COLLADA_DOM_VERSION_PATCH})
  set(CPACK_PACKAGE_INSTALL_DIRECTORY "collada-dom${COLLADA_DOM_SOVERSION}")
  set(CPACK_PACKAGE_DESCRIPTION_FILE ${CMAKE_CURRENT_SOURCE_DIR}/description)
  set(CPACK_RESOURCE_FILE_LICENSE ${CMAKE_CURRENT_SOURCE_DIR}/dom/license.txt)

  set(CPACK_COMPONENT_${COMPONENT_PREFIX_UPPER}-BASE_DISPLAY_NAME "core libraries")
  set(CPACK_COMPONENT_${COMPONENT_PREFIX_UPPER}-DEV_DISPLAY_NAME "C++ headers and build tools")
  set(CPACK_COMPONENT_${COMPONENT_PREFIX_UPPER}-DEV_DEPENDS ${COMPONENT_PREFIX}-base)
  
  set(CPACK_DEBIAN_BUILD_DEPENDS debhelper cmake libxml2-dev zlib1g-dev pkg-config)

  # debian
  set(CPACK_DEBIAN_PACKAGE_PRIORITY optional)
  set(CPACK_DEBIAN_PACKAGE_SECTION libs)
  set(CPACK_DEBIAN_PACKAGE_DEPENDS ${CPACK_COMPONENTS_ALL})
  set(CPACK_DEBIAN_PACKAGE_SUGGESTS)
  set(CPACK_DEBIAN_CMAKE_OPTIONS "-DOPT_DOUBLE_PRECISION=${OPT_DOUBLE_PRECISION} -DOPT_BUILD_PACKAGE_DEFAULT=${OPT_BUILD_PACKAGE_DEFAULT}")
  set(CPACK_DEBIAN_PACKAGE_REMOVE_SOURCE_FILES viewer fx rt Makefile.linux Makefile dom/codeGen dom/Makefile dom/external-libs/minizip dom/external-libs/zlib-1.2.5 dom/test dom/release dom/projects dom/make)
  set(CPACK_DEBIAN_PACKAGE_SOURCE_COPY svn export --force)
  set(CPACK_DEBIAN_CHANGELOG)
  execute_process(COMMAND lsb_release -is
    OUTPUT_VARIABLE _lsb_distribution OUTPUT_STRIP_TRAILING_WHITESPACE
    RESULT_VARIABLE _lsb_release_failed)
  set(CPACK_DEBIAN_DISTRIBUTION_NAME ${_lsb_distribution} CACHE STRING "Name of the distrubiton")
  string(TOLOWER ${CPACK_DEBIAN_DISTRIBUTION_NAME} CPACK_DEBIAN_DISTRIBUTION_NAME)
  if( ${CPACK_DEBIAN_DISTRIBUTION_NAME} STREQUAL "ubuntu" )
    set(CPACK_DEBIAN_DISTRIBUTION_RELEASES lucid maverick natty oneiric precise CACHE STRING "Release code-names of the distrubiton release")
  endif()
  set(CPACK_COMPONENT_COLLADA-DOM_DEPENDS ${CPACK_COMPONENTS_ALL})

  if( OPT_BUILD_PACKAGE_DEFAULT )
    set(CPACK_COMPONENT_COLLADA-DOM-DEV_DEPENDS ${CPACK_DEBIAN_PACKAGE_NAME}-dev)
    set(CPACK_COMPONENT_COLLADA-DOM-DEV_DISPLAY_NAME ${CPACK_COMPONENT_${COMPONENT_PREFIX_UPPER}-DEV_DISPLAY_NAME})
    set(CPACK_COMPONENTS_ALL ${CPACK_COMPONENTS_ALL} collada-dom-dev)
  endif()

  set(DPUT_HOST "" CACHE STRING "PPA repository to upload the debian sources")
  include(CPack)
  include(DebSourcePPA)
endif()
