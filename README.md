A source-to-source compiler for a custom language called Kvantum. The transpiler generates C code.

1 The Kvantum Language

1.1 Types

1.1.1 Primitive Types
    Integer
    Float
    Boolean
    Char
    Void

1.1.2 Object Types

1.1.3 Array Type

1.1.4 Reference(Pointer) Type

1.2 Syntax

1.2.1 Modules

A Kvantum source file describes a module with the same name as the source file. A module consists of function definitions and object type definitions.
Modules can import other modules.

1.2.2 Functions

fn <IDENTIFIER>(<IDENTIFER>:<TYPENAME>,...) [ public | const | static | virtual | override ] -> <TYPENAME> {

}

fn <IDENTIFIER>(<IDENTIFER>:<TYPENAME>) {

}

fn <IDENTIFIER>(<IDENTIFER>:<TYPENAME>) => <EXPRESSION>;

fn <TYPENAME>.<IDENTIFIER>() {

}

1.2.3 Statements

<statement> := <assignment> | <return> | <s_function_call> | <if_else> | <while> | <statement_block> ;
<assignment> := let <identifier> := <expression>;
<return> := ret <expression>;
<s_function_call> := <function_call>;
<if_else> := if <expression>: <statement> <else>?
<else> := else <statement>
<while> := while <expression>: <statement>
<statement_block> := { <statement>... }

1.2.3 Expressions

<expression> := <variable> | <fcall> | <literal> | <array> | <field_access> |<bop> | <arr_index> | <take_reference> | <cast>
<variable> := <identifier> | <field_access>
<fcall> := <identifier>(<expression>,...)
<literal> := <integer> | <float> | <string> | <boolean>
<field_access> := <identifier>.<identifier>
<arr_index> := <expression>[<expression>]
<take_reference> := &<expression>
<cast> := <expression> as <typename>
<bop> := <expression> <binary_operator> <expression>
<binary_operator> := + | - | * | - | == | != | and | or


2 The transpiler

