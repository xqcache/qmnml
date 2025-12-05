cmake_minimum_required(VERSION 3.23.0 FATAL_ERROR)

project(qmnml VERSION 1.0.0 LANGUAGES CXX)

set(MAIN_PROJECT OFF)

if(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_CURRENT_SOURCE_DIR})
    set(MAIN_PROJECT ON)
endif()

option(QMNML_BUILD_TESTS "" ${MAIN_PROJECT})

if(MSVC)
    add_compile_options(/utf-8)
endif()

set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 20)

add_subdirectory(source)

if(QMNML_BUILD_TESTS)
    add_subdirectory(tests)
endif()