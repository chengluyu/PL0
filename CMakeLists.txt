cmake_minimum_required(VERSION 3.8)
project(PL0)

set(CMAKE_CXX_STANDARD 17)

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --std=c++17")
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --std=c++17")
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /std:c++17")
endif()

set(PARSING_SOURCE_FILES
        src/parsing/lexer.cpp
        src/parsing/lexer.h
        src/parsing/parser.cpp
        src/parsing/parser.h
        src/parsing/scope.h
        src/parsing/symbol.h
        src/parsing/token.h)

set(AST_SOURCE_FILES
        src/ast/ast.h
        src/ast/ast.cpp
        src/ast/pretty-printer.cpp
        src/ast/pretty-printer.h
        src/ast/dot-generator.cpp
        src/ast/dot-generator.h)

set(BYTECODE_SOURCE_FILES
        src/bytecode/assembler.cpp
        src/bytecode/assembler.h
        src/bytecode/bytecode.h
        src/bytecode/compiler.cpp
        src/bytecode/compiler.h)

set(SOURCE_FILES
        ${PARSING_SOURCE_FILES}
        ${AST_SOURCE_FILES}
        ${BYTECODE_SOURCE_FILES}
        src/main.cpp
        src/util.h
        src/vm.cpp
        src/vm.h)

add_executable(PL0 ${SOURCE_FILES})