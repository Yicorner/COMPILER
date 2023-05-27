#include"front/syntax.h"

#include<iostream>
#include<cassert>

using namespace frontend;
// #define DEBUG_PARSER
#define TODO assert(0 && "todo")
#define CUR_TOKEN_IS(tk_type) (token_stream[index].type == TokenType::tk_type)
#define PARSE_TOKEN(tk_type) root->children.push_back(parseTerm(root,  TokenType::tk_type))
#define PARSE(name, type) auto name = new type(root); assert(parse##type(name)); root->children.push_back(name);


Parser::Parser(const std::vector<Token> &tokens): index(0), token_stream(tokens) {}

Parser::~Parser() {}
bool Parser::isfunctype() {
    return ((token_stream[index].type == TokenType::VOIDTK) ||
            (token_stream[index].type == TokenType::INTTK) ||
            (token_stream[index].type == TokenType::FLOATTK)) &&
           ((int)index + 1 < (int)token_stream.size() &&
            token_stream[index + 1].type == TokenType::IDENFR) &&
           ((int)index + 2 < (int)token_stream.size() &&
            token_stream[index + 2].type == TokenType::LPARENT);
}
bool Parser::isbtype_or_const() {
    return (CUR_TOKEN_IS(INTTK) || CUR_TOKEN_IS(FLOATTK) || CUR_TOKEN_IS(CONSTTK));
}
Term *Parser::parseTerm(AstNode *parent, TokenType expected) {
    if (!(token_stream[index].type == expected)) assert(0);
    Term *term = new Term(token_stream[index++], parent);
    return term;
}
// CompUnit -> (Decl | FuncDef) [CompUnit]
frontend::CompUnit *Parser::get_abstract_syntax_tree() {
    CompUnit *root = new CompUnit();
    if (isfunctype()) {
        PARSE(funcdef, FuncDef);
    } else {
        PARSE(decl, Decl);
    }
    if ((int)index != (int)token_stream.size()) {
        PARSE(compunit, CompUnit);
    }
    return root;
}
bool Parser::parseCompUnit(AstNode *root) {
    log(root);
    if (isfunctype()) {
        PARSE(funcdef, FuncDef);
    } else {
        PARSE(decl, Decl);
    }
    if ((int)index != (int)token_stream.size()) {
        PARSE(compunit, CompUnit);
    }
    return true;
}
// Decl -> ConstDecl | VarDecl
bool Parser::parseDecl(frontend::AstNode *root) {
    log(root);
    if (token_stream[index].type == TokenType::CONSTTK) {
        PARSE(constdecl, ConstDecl);
    } else {
        PARSE(vardecl, VarDecl);
    }
    return true;
}
// FuncDef -> FuncType Ident '(' [FuncFParams] ')' Block
bool Parser::parseFuncDef(AstNode *root) {
    log(root);
    PARSE(functype, FuncType);
    PARSE_TOKEN(IDENFR);
    PARSE_TOKEN(LPARENT);
    if (token_stream[index].type != TokenType::RPARENT) {
        PARSE(funcfparams, FuncFParams);
    }
    PARSE_TOKEN(RPARENT);
    PARSE(block, Block);
    return true;
}
// ConstDecl -> 'const' BType ConstDef { ',' ConstDef } ';'
bool Parser::parseConstDecl(AstNode *root) {
    log(root);
    PARSE_TOKEN(CONSTTK);
    PARSE(btype, BType);
    PARSE(constdef, ConstDef);
    while (1) {
        if (!CUR_TOKEN_IS(COMMA)) break;
        PARSE_TOKEN(COMMA);
        PARSE(constdef, ConstDef);
    }
    PARSE_TOKEN(SEMICN);
    return true;
}
// BType -> 'int' | 'float'
bool Parser::parseBType(AstNode *root) {
    log(root);
    if (CUR_TOKEN_IS(INTTK)) {
        PARSE_TOKEN(INTTK);
    } else {
        PARSE_TOKEN(FLOATTK);
    }
    return true;
}
// ConstDef -> Ident { '[' ConstExp ']' } '=' ConstInitVal
bool Parser::parseConstDef(AstNode *root) {
    log(root);
    PARSE_TOKEN(IDENFR);
    while (1) {
        if (!CUR_TOKEN_IS(LBRACK)) break;
        PARSE_TOKEN(LBRACK);
        PARSE(constexp, ConstExp);
        PARSE_TOKEN(RBRACK);
    }
    PARSE_TOKEN(ASSIGN);
    PARSE(constinitval, ConstInitVal);
    return true;
}
// ConstInitVal -> ConstExp | '{' [ ConstInitVal { ',' ConstInitVal } ] '}'
bool Parser::parseConstInitVal(AstNode *root) {
    log(root);
    if (CUR_TOKEN_IS(LBRACE)) {
        PARSE_TOKEN(LBRACE);
        if (!CUR_TOKEN_IS(RBRACE)) {
            PARSE(constinitval, ConstInitVal);
            while (1) {
                if (!CUR_TOKEN_IS(COMMA)) break;
                PARSE_TOKEN(COMMA);
                PARSE(constinitval, ConstInitVal);
            }
        }
        PARSE_TOKEN(RBRACE);
        return true;
    } else {
        PARSE(constexp, ConstExp);
        return true;
    }
    return true;
}
// VarDecl -> BType VarDef { ',' VarDef } ';'
bool Parser::parseVarDecl(AstNode *root) {
    log(root);
    PARSE(btype, BType);
    PARSE(vardef, VarDef);
    while (1) {
        if (!CUR_TOKEN_IS(COMMA)) break;
        PARSE_TOKEN(COMMA);
        PARSE(vardef, VarDef);
    }
    PARSE_TOKEN(SEMICN);
    return true;
}
// VarDef -> Ident { '[' ConstExp ']' } [ '=' InitVal ]
bool Parser::parseVarDef(AstNode *root) {
    log(root);
    PARSE_TOKEN(IDENFR);
    while (1) {
        if (!CUR_TOKEN_IS(LBRACK)) break;
        PARSE_TOKEN(LBRACK);
        PARSE(constexp, ConstExp);
        PARSE_TOKEN(RBRACK);
    }
    if (CUR_TOKEN_IS(ASSIGN)) {
        PARSE_TOKEN(ASSIGN);
        PARSE(initval, InitVal);
    }
    return true;
}
// InitVal -> Exp | '{' [ InitVal { ',' InitVal } ] '}'
bool Parser::parseInitVal(AstNode *root) {
    log(root);
    if (CUR_TOKEN_IS(LBRACE)) {
        PARSE_TOKEN(LBRACE);
        if (!CUR_TOKEN_IS(RBRACE)) {
            PARSE(initval, InitVal);
            while (1) {
                if (!CUR_TOKEN_IS(COMMA)) break;
                PARSE_TOKEN(COMMA);
                PARSE(initval, InitVal);
            }
        }
        PARSE_TOKEN(RBRACE);
        return true;
    } else {
        PARSE(exp, Exp);
        return true;
    }
    return true;
}
// FuncType -> 'void' | 'int' | 'float'
bool Parser::parseFuncType(AstNode *root) {
    log(root);
    if (CUR_TOKEN_IS(VOIDTK)) PARSE_TOKEN(VOIDTK);
    else if (CUR_TOKEN_IS(INTTK)) PARSE_TOKEN(INTTK);
    else if (CUR_TOKEN_IS(FLOATTK)) PARSE_TOKEN(FLOATTK);
    return true;
}
// FuncFParam -> BType Ident ['[' ']' { '[' Exp ']' }]
bool Parser::parseFuncFParam(AstNode *root) {
    log(root);
    PARSE(btype, BType);
    PARSE_TOKEN(IDENFR);
    if (CUR_TOKEN_IS(LBRACK)) {
        PARSE_TOKEN(LBRACK);
        PARSE_TOKEN(RBRACK);
        while (1) {
            if (!CUR_TOKEN_IS(LBRACK)) break;
            PARSE_TOKEN(LBRACK);
            PARSE(exp, Exp);
            PARSE_TOKEN(RBRACK);
        }
    }
    return true;
}
// FuncFParams -> FuncFParam { ',' FuncFParam }
bool Parser::parseFuncFParams(AstNode *root) {
    log(root);
    PARSE(funcfparm, FuncFParam);
    while (1) {
        if (!CUR_TOKEN_IS(COMMA)) break;
        PARSE_TOKEN(COMMA);
        PARSE(funcfparm, FuncFParam);
    }
    return true;
}
// Block -> '{' { BlockItem } '}'
bool Parser::parseBlock(AstNode *root) {
    log(root);
    PARSE_TOKEN(LBRACE);
    while (1) {
        if (CUR_TOKEN_IS(RBRACE)) break;
        PARSE(blockitem, BlockItem);
    }
    PARSE_TOKEN(RBRACE);
    return true;
}
// BlockItem -> Decl | Stmt
bool Parser::parseBlockItem(AstNode *root) {
    log(root);
    if (isbtype_or_const()) {
        PARSE(decl, Decl);
    } else {
        PARSE(stmt, Stmt);
    }
    return true;
}
// Stmt -> LVal '=' Exp ';' | Block | 'if' '(' Cond ')' Stmt [ 'else' Stmt ] |
// 'while' '(' Cond ')' Stmt | 'break' ';' | 'continue' ';' | 'return' [Exp] ';' |
// [Exp] ';'
bool Parser::parseStmt(AstNode *root) {
    log(root);
    if (CUR_TOKEN_IS(LBRACE)) {
        PARSE(block, Block);
    } else if (CUR_TOKEN_IS(IFTK)) {
        PARSE_TOKEN(IFTK);
        PARSE_TOKEN(LPARENT);
        PARSE(cond, Cond);
        PARSE_TOKEN(RPARENT);
        PARSE(stmt, Stmt);
        if (CUR_TOKEN_IS(ELSETK)) {
            PARSE_TOKEN(ELSETK);
            PARSE(stmt, Stmt);
        }
    } else if (CUR_TOKEN_IS(WHILETK)) {
        PARSE_TOKEN(WHILETK);
        PARSE_TOKEN(LPARENT);
        PARSE(cond, Cond);
        PARSE_TOKEN(RPARENT);
        PARSE(stmt, Stmt);
    } else if (CUR_TOKEN_IS(BREAKTK)) {
        PARSE_TOKEN(BREAKTK);
        PARSE_TOKEN(SEMICN);
    } else if (CUR_TOKEN_IS(CONTINUETK)) {
        PARSE_TOKEN(CONTINUETK);
        PARSE_TOKEN(SEMICN);
    } else if (CUR_TOKEN_IS(RETURNTK)) {
        PARSE_TOKEN(RETURNTK);
        if (!CUR_TOKEN_IS(SEMICN)) {
            PARSE(exp, Exp);
        }
        PARSE_TOKEN(SEMICN);
    }  else {
        int next = 0;
        if (!CUR_TOKEN_IS(IDENFR)) next |= 1;
        int i = index + 1;
        int cnt = 0;
        while (1) {
            if (cnt == 0 && i < (int)token_stream.size() && token_stream[i].type != TokenType::LBRACK) break;
            if (token_stream[i].type == TokenType::LBRACK) cnt++;
            if (token_stream[i].type == TokenType::RBRACK) cnt--;
            i++;
        }
        if (i < token_stream.size() && token_stream[i].type != TokenType::ASSIGN) next |= 1;
        if (next) {
            if (!CUR_TOKEN_IS(SEMICN)) {
                PARSE(exp, Exp);
            }
            PARSE_TOKEN(SEMICN);
        } else {
            PARSE(lval, LVal);
            PARSE_TOKEN(ASSIGN);
            PARSE(exp, Exp);
            PARSE_TOKEN(SEMICN);
        }
    }
    return true;
}
// Exp -> AddExp
bool Parser::parseExp(AstNode *root) {
    log(root);
    PARSE(addexp, AddExp);
    return true;
}
// Cond -> LOrExp
bool Parser::parseCond(AstNode *root) {
    log(root);
    PARSE(lorexp, LOrExp);
    return true;
}
// LVal -> Ident {'[' Exp ']'}
bool Parser::parseLVal(AstNode *root) {
    log(root);
    PARSE_TOKEN(IDENFR);
    while (1) {
        if (!CUR_TOKEN_IS(LBRACK)) break;
        PARSE_TOKEN(LBRACK);
        PARSE(exp, Exp);
        PARSE_TOKEN(RBRACK);
    }
    return true;
}
// Number -> IntConst | floatConst
bool Parser::parseNumber(AstNode *root) {
    log(root);
    if (CUR_TOKEN_IS(INTLTR)) {
        PARSE_TOKEN(INTLTR);
    } else {
        PARSE_TOKEN(FLOATLTR);
    }
    return true;
}
// PrimaryExp -> '(' Exp ')' | LVal | Number
bool Parser::parsePrimaryExp(AstNode *root) {
    log(root);
    if (CUR_TOKEN_IS(LPARENT)) {
        PARSE_TOKEN(LPARENT);
        PARSE(exp, Exp);
        PARSE_TOKEN(RPARENT);
    } else if (CUR_TOKEN_IS(IDENFR)) {
        // std::cout << "fuck" << std::endl;
        PARSE(lval, LVal);
    } else {
        PARSE(number, Number);
    }
    return true;
}
// UnaryExp -> PrimaryExp | Ident '(' [FuncRParams] ')' | UnaryOp UnaryExp
bool Parser::parseUnaryExp(AstNode *root) {
    log(root);
    if (CUR_TOKEN_IS(PLUS) || CUR_TOKEN_IS(MINU) || CUR_TOKEN_IS(NOT)) {
        PARSE(unaryop, UnaryOp);
        PARSE(unaryexp, UnaryExp);
    } else if (CUR_TOKEN_IS(IDENFR) && (int)index + 1 < (int)token_stream.size() && token_stream[index + 1].type == TokenType::LPARENT) {
        PARSE_TOKEN(IDENFR);
        PARSE_TOKEN(LPARENT);
        if (!CUR_TOKEN_IS(RPARENT)) {
            PARSE(funcrparams, FuncRParams);
        }
        PARSE_TOKEN(RPARENT);
    } else {
        // std::cout << "fuck" << std::endl;
        PARSE(primaryexp, PrimaryExp);
    }
    return true;
}
//UnaryOp -> '+' | '-' | '!'
bool Parser::parseUnaryOp(AstNode *root) {
    log(root);
    if (CUR_TOKEN_IS(PLUS)) {
        PARSE_TOKEN(PLUS);
    } else if (CUR_TOKEN_IS(MINU)) {
        PARSE_TOKEN(MINU);
    } else {
        PARSE_TOKEN(NOT);
    }
    return true;
}
// FuncRParams -> Exp { ',' Exp }
bool Parser::parseFuncRParams(AstNode *root) {
    log(root);
    PARSE(exp, Exp);
    while (1) {
        if (!CUR_TOKEN_IS(COMMA)) break;
        PARSE_TOKEN(COMMA);
        PARSE(exp, Exp);
    }
    return true;
}
// MulExp -> UnaryExp { ('*' | '/' | '%') UnaryExp }
bool Parser::parseMulExp(AstNode *root) {
    log(root);
    PARSE(unaryexp, UnaryExp);
    while (1) {
        int next = 0;
        if (CUR_TOKEN_IS(MULT)) PARSE_TOKEN(MULT), next = 1;
        else if (CUR_TOKEN_IS(DIV)) PARSE_TOKEN(DIV), next = 1;
        else if (CUR_TOKEN_IS(MOD)) PARSE_TOKEN(MOD), next = 1;
        if (next) {
            PARSE(unaryexp, UnaryExp);
        } else break;
    }
    return true;
}
// AddExp -> MulExp { ('+' | '-') MulExp }
bool Parser::parseAddExp(AstNode *root) {
    log(root);
    PARSE(mulexp, MulExp);
    while (1) {
        int next = 0;
        if (CUR_TOKEN_IS(PLUS)) PARSE_TOKEN(PLUS), next = 1;
        else if (CUR_TOKEN_IS(MINU)) PARSE_TOKEN(MINU), next = 1;
        if (next) {
            PARSE(mulexp2, MulExp);
        } else break;
    }
    return true;
}
// RelExp -> AddExp { ('<' | '>' | '<=' | '>=') AddExp }
bool Parser::parseRelExp(AstNode *root) {
    log(root);
    PARSE(addexp, AddExp);
    while (1) {
        int next = 0;
        if (CUR_TOKEN_IS(LSS)) PARSE_TOKEN(LSS), next = 1;
        else if (CUR_TOKEN_IS(GTR)) PARSE_TOKEN(GTR), next = 1;
        else if (CUR_TOKEN_IS(LEQ)) PARSE_TOKEN(LEQ), next = 1;
        else if (CUR_TOKEN_IS(GEQ)) PARSE_TOKEN(GEQ), next = 1;
        if (next) {
            PARSE(addexp, AddExp);
        } else break;
    }
    return true;
}
// EqExp -> RelExp { ('==' | '!=') RelExp }
bool Parser::parseEqExp(AstNode *root) {
    log(root);
    PARSE(relexp, RelExp);
    while (1) {
        int next = 0;
        if (CUR_TOKEN_IS(EQL)) PARSE_TOKEN(EQL), next = 1;
        else if (CUR_TOKEN_IS(NEQ)) PARSE_TOKEN(NEQ), next = 1;
        if (next) {
            PARSE(relexp, RelExp);
        } else break;
    }
    return true;
}
// LAndExp -> EqExp [ '&&' LAndExp ]
bool Parser::parseLAndExp(AstNode *root) {
    log(root);
    PARSE(eqexp, EqExp);
    if (CUR_TOKEN_IS(AND)) {
        PARSE_TOKEN(AND);
        PARSE(landexp, LAndExp);
    }
    return true;
}
// LOrExp -> LAndExp [ '||' LOrExp ]
bool Parser::parseLOrExp(AstNode *root) {
    log(root);
    PARSE(landexp, LAndExp);
    if (CUR_TOKEN_IS(OR)) {
        PARSE_TOKEN(OR);
        PARSE(lorexp, LOrExp);
    }
    return true;
}
// ConstExp -> AddExp
bool Parser::parseConstExp(AstNode *root) {
    log(root);
    PARSE(addexp, AddExp);
    return true;
}
void Parser::log(AstNode *node) {
#ifdef DEBUG_PARSER
    std::cout << "in parse" << toString(node->type) << ", cur_token_type::" << toString(token_stream[index].type) << ", token_val::" << token_stream[index].value << '\n';
#endif
}
