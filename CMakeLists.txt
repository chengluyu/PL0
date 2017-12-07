cmake_minimum_required(VERSION 3.8)
project(PL0)

set(CMAKE_CXX_STANDARD 17)

set(SOURCE_FILES
        src/assembler.cpp
        src/bytecode/assembler.h
        src/bytecode/bytecode.h
        src/parsing/lexer.cpp
        src/parsing/lexer.h
        src/main.cpp
        src/parsing/parser.cpp
        src/parsing/parser.h
        src/parsing/scope.h
        src/parsing/symbol.h
        src/parsing/token.h
        src/util.h
        src/vm.cpp
        src/vm.h src/ast/ast.h src/ast/ast.cpp src/ast/pretty-printer.cpp src/ast/pretty-printer.h src/ast/dot-generator.cpp src/ast/dot-generator.h src/bytecode/compiler.cpp src/bytecode/compiler.h)

add_executable(PL0 ${SOURCE_FILES})