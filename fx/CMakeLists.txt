if(CMAKE_SOURCE_DIR EQUAL CMAKE_CURRENT_SOURCE_DIR)
message(FATAL_ERROR "Please run cmake out of the top-level directory")
endif()

#This is how FX knows it's building itself.
add_definitions(-DPRECOMPILING_COLLADA_FX)

#### Assuming parent directory set up OpenGL/Cg #########

include_directories(include)

#Headers will never work with CMakePCHCompiler :(
file(GLOB files "src/*.cpp") #"include/*.h") 

add_library(colladadomFX STATIC ${files})
add_compile_options(${_wincompile}) #${_winlink}

#SVN-EXTERNALS: cmake-modules/CMakePCHCompiler
target_precompiled_header(colladadomFX include/FX.pch.h)
