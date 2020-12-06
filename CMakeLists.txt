cmake_minimum_required(VERSION 3.4.3)

project("SVF")

if(NOT COMMAND add_llvm_library)
    if(DEFINED LLVM_DIR)
        set(ENV{LLVM_DIR} "${LLVM_DIR}")
    endif()
    if(DEFINED ENV{LLVM_DIR})
        set(CMAKE_CXX_STANDARD 14)
	if(CMAKE_BUILD_TYPE MATCHES "Debug")
		set(CMAKE_CXX_FLAGS "-fPIC -std=gnu++2a -g -O0 -fno-rtti -Wno-deprecated")
	else()
		set (CMAKE_CXX_FLAGS "-fPIC -std=gnu++2a -O3 -fno-rtti -Wno-deprecated")
	endif()
	set(CMAKE_C_FLAGS "-fPIC")

	find_package(LLVM REQUIRED CONFIG)
        list(APPEND CMAKE_MODULE_PATH "${LLVM_CMAKE_DIR}")
        include(AddLLVM)
        add_definitions(${LLVM_DEFINITIONS})
        include_directories(SYSTEM ${LLVM_INCLUDE_DIRS})
    else()
        message(FATAL_ERROR "[-] WARNING: LLVM_DIR not found")
    endif()
else()
    set(IN_SOURCE_BUILD 1)
endif()

set(Z3_DIR $ENV{Z3_DIR})

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include
                    ${CMAKE_CURRENT_BINARY_DIR}/include
                    ${Z3_DIR}/include/)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/Test-Suite)

LINK_DIRECTORIES(${Z3_DIR}/bin)
add_subdirectory(lib)
add_subdirectory(tools)

INSTALL(
    DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/ ${CMAKE_CURRENT_BINARY_DIR}/include/ ${Z3_DIR}/include/
    COMPONENT devel
    DESTINATION include/svf
    FILES_MATCHING
    PATTERN "**/*.h"
)

enable_testing()
add_subdirectory(Test-Suite)
include(CTest)