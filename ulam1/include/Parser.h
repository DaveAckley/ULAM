/**                                        -*- mode:C++ -*-
 * Parser.h -  Basic Parse handling for ULAM
 *
 * Copyright (C) 2014-2015 The Regents of the University of New Mexico.
 * Copyright (C) 2014-2015 Ackleyshack LLC.
 *
 * This file is part of the ULAM programming language compilation system.
 *
 * The ULAM programming language compilation system is free software:
 * you can redistribute it and/or modify it under the terms of the GNU
 * General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option)
 * any later version.
 *
 * The ULAM programming language compilation system is distributed in
 * the hope that it will be useful, but WITHOUT ANY WARRANTY; without
 * even the implied warranty of MERCHANTABILITY or FITNESS FOR A
 * PARTICULAR PURPOSE.  See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License
 * along with the ULAM programming language compilation system
 * software.  If not, see <http://www.gnu.org/licenses/>.
 *
 * @license GPL-3.0+ <http://spdx.org/licenses/GPL-3.0+>
 */

/**
  \file Parser.h -  Basic Parse handling for ULAM
  \author Elenas S. Ackley.
  \author David H. Ackley.
  \date (C) 2014-2015 All rights reserved.
  \gpl
*/


#ifndef PARSER_H
#define PARSER_H

#include <string>
#include <vector>
#include "itype.h"
#include "CompilerState.h"
#include "Node.h"
#include "NodeBinaryOpEqual.h"
#include "NodeBinaryOp.h"
#include "NodeBlock.h"
#include "NodeConditionalAs.h"
#include "NodeConstantDef.h"
#include "NodeFunctionCall.h"
#include "NodeStatements.h"
#include "NodeSquareBracket.h"
#include "NodeTypeBitsize.h"
#include "NodeUnaryOp.h"
#include "Symbol.h"
#include "SymbolFunction.h"
#include "Tokenizer.h"

namespace MFM{
  /**
      Preparser wrapper class for tokenizer's (e.g. Lexer), within CompilerState.
   */
  class Parser
  {
  public:
    Parser(Tokenizer * izer, CompilerState & state);
    ~Parser();

    /**
	<LIBRARY> := <PROGRAM_DEF>* + <EOF>

	Ends when each subject of compile (i.e. startstr) has been parsed
	(takes an optional error output arg), until EOF; Returns number of errors.
     */
    u32 parseProgram(std::string startstr, File * errOutput = NULL);

  private:
    // owned by parent, e.g. Compiler object. Has Tables of Classes.
    CompilerState & m_state;

    // owned by parent, e.g. Compiler object.  used to get Tokens
    Tokenizer * m_tokenizer;

    /**
	<PROGRAM_DEF> := <QUARK_OR_UNION_DEF> | <ELEMENT_DEF>
	<ELEMENT_DEF> := 'element' + <TYPE_IDENT> + <CLASS_BLOCK>
	<QUARK_OR_UNION_DEF> := ('quark' | 'union')   + <TYPE_IDENT> + <CLASS_BLOCK>
    */
    bool parseThisClass();

    /**
	<CLASS_BLOCK> := '{' + <DATA_MEMBERS> + '}'
    */
    NodeBlockClass * parseClassBlock(SymbolClass * csym);

    void parseRestOfClassParameters(SymbolClass * csym);

    /**
       <DATA_MEMBERS> := ( 0 | <FUNC_DEF> | ('element' | 0) + <DECL> + ';' | <TYPE_DEF> + ';'| <CONST_DEF> + ';' )*
     */
    bool parseDataMember(NodeStatements *& nextNode);

    /**
	<BLOCK> := '{' + <STATEMENTS> + '}'
    */
    Node * parseBlock();

    /**
	<STATEMENTS> := NULL | <STATEMENT> + <STATEMENTS>
    */
    Node * parseStatements();

    /**
	<STATEMENT> := <SIMPLE_STATEMENT> | <CONTROL_STATEMENT> | <BLOCK>
     */
    Node * parseStatement();

    /**
	<CONTROL_STATEMENT> := <IF_STATEMENT> | <WHILE_STATEMENT> | <FOR_STATEMENT> |
                               <BREAK_STATEMENT> | <CONTINUE_STATEMENT>
	<BREAK_STATEMENT> := 'break' + ';'
	<CONTINUE_STATEMENT> := 'continue' + ';'
    */
    Node * parseControlStatement();

    /**
	<IF_STATEMENT> := 'if' + '(' + <CONDITIONAL_EXPR> + ')' + <STATEMENT> + <OPT_ELSE_STATEMENT>
	<OPT_ELSE_STATEMENT> := 0 | 'else' + <STATEMENT>
    */
    Node * parseControlIf(Token ifTok);

    /**
       <WHILE_STATEMENT> := 'while' + '(' + <CONDITIONAL_EXPR> + ')' + <STATEMENT>
       => equiv to a parse tree shaped like: while(true) { if(! <CONDITIONAL_EXPR) break; <STATEMENT> }
    */
    Node * parseControlWhile(Token wTok);

    /**
       <FOR_STATEMENT> := 'for' + '(' + ( 0 | <STATEMENT_DECL>) + ';' + ( 0 | <CONDITIONAL_EXPRESSION>)
                                + ';' + ( 0 | <ASSIGN_EXPRESSION>) + ')' + <STATEMENT>
			=> equiv to a parse tree shaped like: { <STATEMENT_DECL>  while ( <CONDITIONAL_EXPR> ) { <STATEMENT> <ASSIGN_EXPRESSION> } }
     */
    Node * parseControlFor(Token fTok);

    /**
       <CONDITIONAL_EXPR> := <SIMPLE_COND_DECL> | <ASSIGN_EXPR>
       <SIMPLE_COND_DECL> := <IDENT_EXPR> + 'as' + <TYPE_IDENT>
    */
    Node * parseConditionalExpr();

    /**
       (helper for 'as' condition in if/while)
    */
    Node * setupAsConditionalBlockAndParseStatements(NodeConditionalAs * asNode);

    /**
	<SIMPLE_STATEMENT> := ( 0 | <STATEMENT_DECL> | <TYPE_DEF> | <CONST_DEF> | <ASSIGN_EXPR> |
                                <RETURN_STATEMENT> ) + ';'
     */
    Node * parseSimpleStatement();

    /**
       <TYPE_NAME> := 'Int' | 'Unsigned' | 'Bool' | 'Unary' | 'Bits | <TYPE_IDENT> | <Type_IdENT> + ( '.' + <TYPE_IDENT>)*
       <TYPE> := <TYPE_NAME> | <TYPE_NAME> + '(' + <EXPRESSION> + ')'
       <TYPE_IDENT> := /^[A-Z][A-Za-z0-9\_]*
       <TYPEDEF> := 'typedef' + <TYPE> + <TYPE_EXPRESSION>
       <TYPE_EXPRESSION> := ( <TYPE_IDENT> | <TYPE_IDENT> + '[' + <EXPRESSION> + ']')
    */
    Node * parseTypedef();

    /**
       <CONST_DEF> := 'constant' + <TYPE> + <IDENT> + '=' + <EXPRESSION>
    */
    Node * parseConstdef(bool assignOK = true);

    /**
       <DECL> := <TYPE> + <VAR_DECLS>
       <VAR_DECLS> := <VAR_DECL> | <VAR_DECL> + ',' + <VAR_DECLS>
       <VAR_DECL> := <LVAL_EXPR>

       <STATEMEMT_DEC> := <TYPE> + <VAR_STATEMENT_DECLS>
       <VAR_STATEMENT_DECLS> := <VAR_STATEMENT_DECL> | <VAR_STATEMENT_DECL> + ',' + <VAR_STATEMENT_DECLS>
       <VAR_STATEMENT_DECL> := <LVAL_EXPR> + ( 0 | '=' + ASSIGN_EXPR>)

       (when flag is true stops after one decl for function parameters).
    */
    Node * parseDecl(bool parseSingleDecl = false);



    UTI parseClassArguments(Token& typeTok);
    void parseRestOfClassArguments(SymbolClass * csym, SymbolClassName * cnsym, u32& parmIdx);

    /** helper for parsing type; returns bitsize, or UNKNOWNSIZE and node with constant expression */
    NodeTypeBitsize * parseTypeBitsize(Token& typeTok, s32& typebitsize, s32& arraysize);

    /** helper for parsing type; returns bitsize, or UNKNOWNSIZE */
    void parseTypeFromAnotherClassesTypedef(Token & typeTok, s32& rtnbitsize, s32& rtnarraysize);

    /**
       <RETURN_STATMENT> := 'return' + (0 | <ASSIGN_EXPR>)
    */
    Node * parseReturn();

    /**
       <ASSIGNEXPR> := <EXPRESSION> | <LVAL_EXPRESSION> + <ASSIGN_OP> + <ASSIGNEXPR>
       <ASSIGN_OP> := '=' | '+=' | '-=' | '*=' | '&=' | '|=' | '^='
    */
    Node * parseAssignExpr();

    /**
       <LVAL_EXPRESSION> := <IDENT> | <IDENT> + '[' + <EXPRESSION> + ']'
       <IDENT> := /^[a-z][A-Za-z0-9\_]*
    */
    Node * parseLvalExpr(Token identTok);

    /**
       <IDENT_EXPRESSION> := <LVAL_EXPRESSION> | <MEMBER_SELECT_EXPRESSION> | <FUNC_CALL>
    */
    Node * parseIdentExpr(Token identTok);

    /**
	<MEMBER_SELECT_EXPRESSION> := <IDENT_EXPRESSION> + '.' + <IDENT_EXPRESSION>
    */
    Node * parseMemberSelectExpr(Token memberTok);

    Node * parseRestOfMemberSelectExpr(Node * classInstanceNode);

    Node * parseMinMaxSizeofType(Token memberTok, UTI utype);

    /**
       <FUNC_CALL> := <IDENT> + '(' + <ARGS> + ')'
    */
    Node * parseFunctionCall(Token identTok);

    /**
       <ARGS>    := 0 | <ARG> | <ARG> + ',' + <ARGS>
       <ARG>     := <ASSIGNEXPR>
    */
    bool parseRestOfFunctionCallArguments(NodeFunctionCall * callNode);

    /**
       <EXPRESSION> := <LOGICAL_EXPRESSION> | <EXPRESSION> <LOGICALOP> <LOGICAL_EXPRESSION>
       <LOGICALOP> := '&&' | '||'
    */
    Node * parseExpression();

    /**
       <LOGICAL_EXPRESSION> := <BIT_EXPRESSION> | <LOGICAL_EXPRESSION> <BITOP> <BIT_EXPRESSION>
       <BITOP> := '&' | '|' | '^'
    */
    Node * parseLogicalExpression();

    /**
       <BIT_EXPRESSION> := <EQ_EXPRESSION> | <BIT_EXPRESSION> <EQOP> <EQ_EXPRESSION>
       <EQOP> := '==' | '!='
    */
    Node * parseBitExpression();

    /**
       <EQ_EXPRESSION> := <COMPARE_EXPRESSION> | <EQ_EXPRESSION> <COMPOP> <COMPARE_EXPRESSION>
       <COMPOP> := '<' | '>' | '<=' | '>='
    */
    Node * parseEqExpression();

    /**
       <COMPARE_EXPRESSION> := <SHIFT_EXPRESSION> | <COMPARE_EXPRESSION> <SHIFTOP> <SHIFT_EXPRESSION>
       <SHIFTOP> := '<<' | '>>'
    */
    Node * parseCompareExpression();

    /**
       <SHIFT_EXPRESSION> := <TERM> | <SHIFT_EXPRESSION> <ADDOP> <TERM>
       <ADDOP> := '+' | ' -'
    */
    Node * parseShiftExpression();

    /**
	<TERM> := <FACTOR> | <TERM> <MULOP> <FACTOR>
	<MULOP> := '*' | '/' | '%'
    */
    Node * parseTerm();

    /**
       <FACTOR> := <IDENT_EXPRESSION> | <NUMBER> | '(' + <ASSIGN_EXPR> + ')' | <UNOP_EXPRESSION>
       <UNOP_EXPRESSION> := <UNOP> + <FACTOR> | <IDENT_EXPRES> + ('is' | 'has') + <TYPE_IDENT>
       <UNOP> := '-' | '+' | '!' | <CAST>
     */
    Node * parseFactor();

    Node * parseRestOfFactor(Node * leftNode);

    Node * parseRestOfCastOrExpression();

    Node * parseRestOfTerm(Node * leftNode);

    Node * parseRestOfShiftExpression(Node * leftNode);

    Node * parseRestOfCompareExpression(Node * leftNode);

    Node * parseRestOfEqExpression(Node * leftNode);

    Node * parseRestOfBitExpression(Node * leftNode);

    Node * parseRestOfLogicalExpression(Node * leftNode);

    Node * parseRestOfExpression(Node * leftNode);

    Node * parseRestOfLvalExpr(Node * leftNode);

    Node * parseRestOfAssignExpr(Node * leftNode);

    Node * parseRestOfDecls(Token typeTok, u32 typebitsize, s32 arraysize, Token identTok, Node * dNode, bool assignOK = true);

    Node * parseRestOfDeclAssignment(Token typeTok, u32 typebitsize, s32 arraysize, Token identTok, Node * dNode);

    NodeConstantDef * parseRestOfConstantDef(NodeConstantDef * constNode, bool assignOK = true);

    /**
	<FUNC_DEF>  := <ULAM_FUNC_DEF> | <NATIVE_FUNC_DEF>
	<ULAM_FUNC_DEF> := <FUNC_DECL> + <BLOCK>
	<NATIVE_FUNC_DEF> := <FUNC_DECL> + 'native' + ';'

	<FUNC_DECL> := <TYPE> + <IDENT> + '(' + <FUNC_PARAM_DECLS> + ')'
	<FUNC_PARAM_DECLS> := 0 | '...' | <FUNC_PARAMS> | <FUNC_PARAMS> + ',' + '...'
	<FUNC_PARAMS> := <FUNC_PARAM> | <FUNC_PARAM> + ',' + <FUNC_PARAMS>
	<FUNC_PARAM>  := <TYPE> + <VAR_DECL>
     */
    NodeBlockFunctionDefinition * makeFunctionBlock(Token typeTok, u32 typebitsize, Token identTok, NodeTypeBitsize * constExprForBitSize);

    void parseRestOfFunctionParameters(SymbolFunction * sym);

    /**
	helper method for function definition, populates funcNode,
	returns true if body parsed
    */
    bool parseFunctionBody(NodeBlockFunctionDefinition *& funcNode);


    /** helper for parseDataMember */
    Node * makeFunctionSymbol(Token typeTok, u32 typebitsize, Token identTok, NodeTypeBitsize * constExprForBitSize);

    /** helper for parseDecl and parseRestOfDecls */
    Node * makeVariableSymbol(Token typeTok, u32 typebitsize, s32 arraysize, Token identTok, NodeTypeBitsize * constExprForBitSize=NULL);

    /** helper for parseTypedef */
    Node * makeTypedefSymbol(Token typeTok, u32 typebitsize, s32 arraysize, Token identTok, NodeTypeBitsize * constExprForBitSize);

    /** helper for parseConstdef */
    Node * makeConstdefSymbol(Token typeTok, u32 typebitsize, s32 arraysize, Token identTok, NodeTypeBitsize * constExprForBitSize, bool assignOK = true);

    /** helper method for parseConditionalExpr */
    Node * makeConditionalExprNode(Node * leftNode);

    /**
       helper method to make assigment nodes
    */
    NodeBinaryOpEqual * makeAssignExprNode(Node * leftNode);

    /**
       helper methods to make binary expression nodes
    */
    NodeBinaryOp * makeExpressionNode(Node * leftNode);
    NodeBinaryOp * makeLogicalExpressionNode(Node * leftNode);
    NodeBinaryOp * makeBitExpressionNode(Node * leftNode);
    NodeBinaryOp * makeEqExpressionNode(Node * leftNode);
    NodeBinaryOp * makeCompareExpressionNode(Node * leftNode);
    NodeBinaryOp * makeShiftExpressionNode(Node * leftNode);

    /**
       helper method to make binary term nodes
    */
    NodeBinaryOp * makeTermNode(Node * leftNode);

    /**
       helper method to make unary factor nodes
    */
    Node * makeFactorNode();

    /** helper for parseRestOfCastOrExpression via parseFactor*/
    Node * makeCastNode(Token typeTok);

    /**
       helper method to make a terminal node
       for a constant value known at parse time (e.g. one for ++/--)
    */
    Node * makeTerminal(Token& locTok, s32 val, ULAMTYPE etype);

    Node * makeTerminal(Token& locTok, u32 val, ULAMTYPE etype);

    /**
       helper method to save subtrees for unknown UTIs
    */
    void linkOrFreeConstantExpressions(UTI auti, NodeTypeBitsize * ceForBitSize, NodeSquareBracket * ceForArraySize);

    /** helper, gets CLOSE_PAREN for <FACTOR>, CLOSE_SQUARE rest of LVal */
    bool getExpectedToken(TokenType eTokType, Token & myTok, bool quietly = false);
    bool getExpectedToken(TokenType closeTokType, bool quietly = false);

    /** helper , passes through to tokenizer */
    bool getNextToken(Token & tok);

    /** helper , passes through to tokenizer */
    void unreadToken();

    /**
	helper, bypasses token until end reached
	if EOF reached, it will unread it before returning
     */
    void getTokensUntil(TokenType lastTok);

    /**
	initializes primitive UlamTypes into classBlock Symbol Table
     */
    void initPrimitiveUlamTypes();
  };

} //MFM

#endif //end PARSER_H
