add_subdirectory(CUDD)

if(LLVM_LINK_LLVM_DYLIB)
    set(llvm_libs LLVM)
else()
    llvm_map_components_to_libnames(llvm_libs bitwriter core ipo irreader instcombine instrumentation target linker analysis scalaropts support transformutils)
endif()

file(GLOB SOURCES
     SVF-FE/*.cpp Graphs/*.cpp Util/*.cpp MemoryModel/*.cpp MSSA/*.cpp WPA/*.cpp SABER/*.cpp DDA/*.cpp
)

add_llvm_library(SVF STATIC ${SOURCES} LINK_LIBS CUDD)

link_directories(${CMAKE_BINARY_DIR}/lib/CUDD)

if(DEFINED IN_SOURCE_BUILD)
    add_dependencies(SVF intrinsics_gen)
endif()
