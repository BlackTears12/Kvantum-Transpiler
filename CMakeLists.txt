cmake_minimum_required(VERSION 3.16)

project(Kvantum-Transpiler LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(Kvantum-Transpiler
    include/ast/annotation.hpp
    include/ast/ast.hpp
    include/ast/AST_nodeoperation.hpp
    include/ast/expressionvisitor.hpp
    include/ast/statementvisitor.hpp
    include/ast/treevisitor.hpp
    include/c_codegen/c_ast.hpp
    include/c_codegen/c_codegen.hpp
    include/c_codegen/c_type.hpp
    include/codegen/c_codegenerator.hpp
    include/codegen/codeexecutorinterface.hpp
    include/codegen/codegenerator.hpp
    include/codegen/functioncodegenerator.hpp
    include/common/compiler.hpp
    include/common/diagnostics.hpp
    include/common/mainheader.hpp
    include/common/module.hpp
    include/common/token.hpp
    include/common/type.hpp
    include/interpreter/interpreter.hpp
    include/interpreter/value.hpp
    include/lexer/lexer.hpp
    include/lexer/scope.hpp
    include/parser/functiondefparser.hpp
    include/parser/parser.hpp

    include/parser/symbolstack.hpp
    include/parser/typechecker.hpp
    include/preproc/preprocessor.hpp
    source/ast/ast.cpp
    source/c_codegen/c_codegen.cpp
    source/c_codegen/c_type.cpp
    source/codegen/c_codegenerator.cpp
    source/codegen/codegenerator.cpp
    source/codegen/functioncodegenerator.cpp
    source/common/compiler.cpp
    source/common/generic.cpp
    source/common/module.cpp
    source/common/type.cpp
    source/interpreter/interpreter.cpp
    source/interpreter/value.cpp
    source/lexer/lexer.cpp
    source/main.cpp
    source/parser/functiondefparser.cpp
    source/parser/parser.cpp
    source/parser/parser_statement.cpp

    source/parser/typechecker.cpp
    source/preproc/preprocessor.cpp
)

include_directories(include)

include(GNUInstallDirs)
install(TARGETS Kvantum-Transpiler
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
