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

file(GLOB SRC_FILES
    src/*.cpp
    src/frontend/scanner/*.cpp
    src/frontend/parser/*.cpp
    src/nodes/*.cpp
    src/nodes/expression/*.cpp
    src/nodes/external/*.cpp
    src/nodes/statement/*.cpp
    src/nodes/function/*.cpp
    src/visitors/*.cpp
    src/passes/*.cpp
    src/utils/*.cpp
)

add_executable(quark ${SRC_FILES})

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
