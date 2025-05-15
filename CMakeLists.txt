cmake_minimum_required(VERSION 3.16)

project(Kvantum-Transpiler LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(Kvantum-Transpiler
    ast/annotation.hpp
    ast/ast.hpp
    ast/ast.cpp
    ast/AST_nodeoperation.hpp
    ast/expressionvisitor.hpp
    ast/statementvisitor.hpp
    ast/treevisitor.hpp
    c_codegen/c_ast.hpp
    c_codegen/c_codegen.hpp
    c_codegen/c_codegen.cpp
    c_codegen/c_type.hpp
    c_codegen/c_type.cpp
    codegen/c_codegenerator.hpp
    codegen/c_codegenerator.cpp
    codegen/codeexecutorinterface.hpp
    codegen/codegenerator.hpp
    codegen/codegenerator.cpp
    codegen/functioncodegenerator.hpp
    codegen/functioncodegenerator.cpp
    common/compiler.hpp
    common/compiler.cpp
    common/diagnostics.hpp
    common/generic.hpp
    common/generic.cpp
    common/mainheader.hpp
    common/module.hpp
    common/module.cpp
    common/token.hpp
    common/type.hpp
    common/type.cpp
    interpreter/interpreter.hpp
    interpreter/interpreter.cpp
    interpreter/value.hpp
    interpreter/value.cpp
    lexer/lexer.hpp
    lexer/lexer.cpp
    lexer/scope.hpp
    main.cpp
    parser/functiondefparser.hpp
    parser/functiondefparser.cpp
    parser/parser.hpp
    parser/parser.cpp
    parser/parser_statement.cpp
    parser/symbolstack.hpp
    parser/typechecker.hpp
    parser/typechecker.cpp
    preproc/preprocessor.hpp
    preproc/preprocessor.cpp
)
include_directories(ast codegen c_codegen common interpreter lexer parser preproc)

include(GNUInstallDirs)
install(TARGETS Kvantum-Transpiler
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
