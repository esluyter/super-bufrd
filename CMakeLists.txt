cmake_minimum_required (VERSION 2.8)
set(PROJECT "SuperBufRd") # set project name manually
message(STATUS "Project name is ${PROJECT}")
project (${PROJECT})

include_directories(${SC_PATH}/include/plugin_interface)
include_directories(${SC_PATH}/include/common)
include_directories(${SC_PATH}/common)

set(CMAKE_SHARED_MODULE_PREFIX "")
if(APPLE OR WIN32)
set(CMAKE_SHARED_MODULE_SUFFIX ".scx")
endif()

option(SUPERNOVA "Build plugins for supernova" OFF)
if (SUPERNOVA)
    include_directories(${SC_PATH}/external_libraries/nova-tt)
    # actually just boost.atomic
    include_directories(${SC_PATH}/external_libraries/boost)
    include_directories(${SC_PATH}/external_libraries/boost_lockfree)
    include_directories(${SC_PATH}/external_libraries/boost-lockfree)
endif()

option(CPP11 "Build with c++11." ON)

if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
	set(CMAKE_COMPILER_IS_CLANG 1)
endif()

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_COMPILER_IS_CLANG)
    add_definitions(-fvisibility=hidden)

    include (CheckCCompilerFlag)
    include (CheckCXXCompilerFlag)

    CHECK_C_COMPILER_FLAG(-msse HAS_SSE)
    CHECK_CXX_COMPILER_FLAG(-msse HAS_CXX_SSE)

    if (HAS_SSE)
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -msse")
    endif()
    if (HAS_CXX_SSE)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -msse")
    endif()

    CHECK_C_COMPILER_FLAG(-msse2 HAS_SSE2)
    CHECK_CXX_COMPILER_FLAG(-msse2 HAS_CXX_SSE2)

    if (HAS_SSE2)
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -msse2")
    endif()
    if (HAS_CXX_SSE2)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -msse2")
    endif()

    CHECK_C_COMPILER_FLAG(-mfpmath=sse HAS_FPMATH_SSE)
    CHECK_CXX_COMPILER_FLAG(-mfpmath=sse HAS_CXX_FPMATH_SSE)

    if (HAS_FPMATH_SSE)
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -mfpmath=sse")
    endif()
    if (HAS_CXX_FPMATH_SSE)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mfpmath=sse")
    endif()

    if(NATIVE)
        add_definitions(-march=native)
    endif()

    if(CPP11)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
        if(CMAKE_COMPILER_IS_CLANG)
            set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++")
        endif()
    endif()
endif()
if(MINGW)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -mstackrealign")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mstackrealign")
endif()

add_library("SuperBufRd" MODULE "SuperBufRd.cpp")
add_library("SuperPoll" MODULE "SuperPoll.cpp")
add_library("SuperBinaryOpUGen" MODULE "SuperBinaryOpUGen.cpp")
if(SUPERNOVA)
    add_library("SuperBufRd_supernova" MODULE "SuperBufRd.cpp")
    add_library("SuperPoll_supernova" MODULE "SuperPoll.cpp")
    add_library("SuperBinaryOpUGen_supernova" MODULE "SuperBinaryOp.cpp")
    set_property(TARGET "SuperBufRd_supernova"
                 PROPERTY COMPILE_DEFINITIONS SUPERNOVA)
    set_property(TARGET "SuperPoll_supernova"
                 PROPERTY COMPILE_DEFINITIONS SUPERNOVA)
    set_property(TARGET "SuperBinaryOpUGen_supernova"
                 PROPERTY COMPILE_DEFINITIONS SUPERNOVA)
endif()
