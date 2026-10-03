cmake_minimum_required(VERSION 3.20)

project(quark LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif()

find_package(LLVM REQUIRED CONFIG)

message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")

add_executable(quark)

target_sources(quark PRIVATE
    src/main.cpp
    src/Optimizer.cpp
)

add_subdirectory(src/frontend)
add_subdirectory(src/nodes)
add_subdirectory(src/passes)
add_subdirectory(src/utils)
add_subdirectory(src/visitors)

target_include_directories(quark PRIVATE
    include
    tabulate/include
    ${LLVM_INCLUDE_DIRS}
)

target_compile_definitions(quark PRIVATE
    ${LLVM_DEFINITIONS}
)

target_compile_options(quark PRIVATE
    -Wall
)

target_link_libraries(quark PRIVATE
    LLVM-22
)

set_target_properties(quark PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}
)
