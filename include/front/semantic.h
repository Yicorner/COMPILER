/**
 * @file semantic.h
 * @author Yuntao Dai (d1581209858@live.com)
 * @brief
 * @version 0.1
 * @date 2023-01-06
 *
 * a Analyzer should
 * @copyright Copyright (c) 2023
 *
 */

#ifndef SEMANTIC_H
#define SEMANTIC_H

#include"ir/ir.h"
#include"front/abstract_syntax_tree.h"

#include<map>
#include<string>
#include<vector>
#include<queue>
using std::map;
using std::string;
using std::vector;
using ir::Function;
using ir::Instruction;
namespace frontend {
struct node {
    bool is_computable;
    string v;
    Type t;
};
// definition of symbol table entry
struct STE {
    ir::Operand operand;
    vector<int> dimension;
};

using map_str_ste = map<string, STE>;
// definition of scope infomation
struct ScopeInfo {
    int cnt;
    string name;
    map_str_ste table;
};

// surpport lib functions
map<std::string, ir::Function *> *get_lib_funcs();

// definition of symbol table
struct SymbolTable {
    vector<ScopeInfo> scope_stack;
    map<std::string, ir::Function *> functions;

    /**
     * @brief enter a new scope, record the infomation in scope stacks
     * @param node: a Block node, entering a new Block means a new name scope
     */
    void add_scope();

    /**
     * @brief exit a scope, pop out infomations
     */
    void exit_scope();

    /**
     * @brief Get the scoped name, to deal the same name in different scopes, we change origin id to a new one with scope infomation,
     * for example, we have these code:
     * "
     * int a;
     * {
     *      int a; ....
     * }
     * "
     * in this case, we have two variable both name 'a', after change they will be 'a' and 'a_block'
     * @param id: origin id
     * @return string: new name with scope infomations
     */
    string get_scoped_name(string id) const;
    void add_into_scope(string name, Type type,  vector<int> dimension);
    /**
     * @brief get the right operand with the input name
     * @param id identifier name
     * @return Operand
     */
    ir::Operand get_operand(string id);

    /**
     * @brief get the right ste with the input name
     * @param id identifier name
     * @return STE
     */
    STE get_ste(string id);
};


// singleton class
struct Analyzer {
    int tmp_cnt;
    vector<ir::Instruction *> g_init_inst;
    SymbolTable symbol_table;
    ir::Program program;
    std::priority_queue<std::pair<int, int>, std::vector<std::pair<int, int>>, std::greater<std::pair<int, int>>> tmp_int;
    std::priority_queue<std::pair<int, int>, std::vector<std::pair<int, int>>, std::greater<std::pair<int, int>>> tmp_float;

    /**
     * @brief constructor
     */
    Analyzer();
    string get_tmp_int();
    string get_tmp_float();
    void del_tmp_int(string id);
    void del_tmp_float(string id);
    void del_tmp(string id, Type type);
    int eval_int(std::string s);
    // analysis functions
    ir::Program get_ir_program(CompUnit *);
    void analyzeCompUnit(CompUnit *);
    void analyzeDecl(Decl *, Function *);
    void analyzeConstDecl(ConstDecl *, Function *);
    void analyzeVarDecl(VarDecl *, Function *);
    void analyzeBType(BType *, Function *);
    void analyzeConstDef(ConstDef *, Function *, Type); // using TokenType as BType
    void analyzeConstExp(ConstExp *, Function *);
    void analyzeConstInitVal(ConstInitVal *, Function *, string ident, int index);
    void analyzeVarDef(VarDef *, Function *, Type);
    void analyzeInitVal(InitVal *, Function *, string ident, int index);
    ir::Operand shift_type(Type, ir::Operand, Function *);
    ir::Operand do_operator(ir::Operand, ir::Operand, ir::Operator, Function *);
    void analyzeExp(Exp *, Function *);
    void analyzeAddExp(AddExp *, Function *); //
    void analyzeMulExp(MulExp *, Function *);
    void analyzeUnaryExp(UnaryExp *, Function *);
    void analyzePrimaryExp(PrimaryExp *, Function *);
    void analyzeNumber(Number *, Function *);
    void analyzeUnaryOp(UnaryOp *, Function *);

    ir::Operand shift_condition(ir::Operand, Function *);
    void analyzeFuncRParams(FuncRParams *, Function *, UnaryExp *, Type, string);
    void analyzeLVal(LVal *, Function *, int if_right);
    void analyzeStmt(Stmt *, Function *);
    void analyzeCond(Cond *, Function *);
    void analyzeLOrExp(LOrExp *, Function *, int);
    void analyzeLAndExp(LAndExp *, Function *, int);
    void analyzeEqExp(EqExp *, Function *);
    void analyzeRelExp(RelExp *, Function *);
    void analyzeBlock(Block *, Function *);
    void analyzeBlockItem(BlockItem *, Function *);

    void analyzeFuncDef(FuncDef *, Function *);
    void analyzeFuncType(FuncType *, Function *);
    void analyzeFuncFParams(FuncFParams *, Function *);
    void analyzeFuncFParam(FuncFParam *, Function *);
    // reject copy & assignment

    void log(AstNode *);
    void return_log(AstNode *);
    Analyzer(const Analyzer &) = delete;
    Analyzer &operator=(const Analyzer &) = delete;
};

} // namespace frontend

#endif