cmake_minimum_required(VERSION 3.8)
project(PL0)

set(CMAKE_CXX_STANDARD 17)

set(SOURCE_FILES
        src/assembler.cpp
        src/assembler.h
        src/bytecode.h
        src/lexer.cpp
        src/lexer.h
        src/main.cpp
        src/parser.cpp
        src/parser.h
        src/scope.h
        src/symbol.h
        src/token.h
        src/util.h
        src/vm.cpp
        src/vm.h src/ast.h src/ast.cpp src/pretty-printer.cpp src/pretty-printer.h)

add_executable(PL0 ${SOURCE_FILES})