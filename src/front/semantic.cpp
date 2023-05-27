#include"front/semantic.h"
#include<cassert>
#include<iostream>
using ir::Instruction;
using ir::Function;
using ir::Operand;
using ir::Operator;
using std::cout;
using std::endl;
#define NOTDELTMP
#define TODO assert(0 && "TODO");
#define PII std::pair<int, int>
#define GET_CHILD_PTR(node, type, index) auto node = dynamic_cast<type*>(root->children[index]); assert(node)
#define ANALYSIS(node, type, index) auto node = dynamic_cast<type*>(root->children[index]); assert(node)
#define ANALYSIS2(node, type, index) auto node = dynamic_cast<type*>(root->children[index]); assert(node); analyze##type(node, function_ptr)
#define COPY_EXP_NODE(from, to) to->is_computable = from->is_computable; to->v = from->v; to->t = from->t;
#define NODE(s) {s->is_computable, s->v, s->t}
#define GIVESYMBOLVALUE(ident, value)  symbol_table.scope_stack.back().table[ident].operand.name = value
#define GIVESYMBOLTYPE(ident, itype) symbol_table.scope_stack.back().table[ident].operand.type = itype
void printnode(frontend::node a) {
    cout << "node->v = " << a.v << " node->t = " << ir::toString(a.t) << " node->is_computable = " << a.is_computable << endl;
}
map<std::string, ir::Function *> *frontend::get_lib_funcs() {
    static map<std::string, ir::Function *> lib_funcs = {
        {"getint", new Function("getint", Type::Int)},
        {"getch", new Function("getch", Type::Int)},
        {"getfloat", new Function("getfloat", Type::Float)},
        {"getarray", new Function("getarray", {Operand("arr", Type::IntPtr)}, Type::Int)},
        {"getfarray", new Function("getfarray", {Operand("arr", Type::FloatPtr)}, Type::Int)},
        {"putint", new Function("putint", {Operand("i", Type::Int)}, Type::null)},
        {"putch", new Function("putch", {Operand("i", Type::Int)}, Type::null)},
        {"putfloat", new Function("putfloat", {Operand("f", Type::Float)}, Type::null)},
        {"putarray", new Function("putarray", {Operand("n", Type::Int), Operand("arr", Type::IntPtr)}, Type::null)},
        {"putfarray", new Function("putfarray", {Operand("n", Type::Int), Operand("arr", Type::FloatPtr)}, Type::null)},
    };
    return &lib_funcs;
}
void frontend::SymbolTable::add_scope() {
    scope_stack.push_back(ScopeInfo());
}
void frontend::SymbolTable::exit_scope() {
    scope_stack.pop_back();
}
string frontend::SymbolTable::get_scoped_name(string id) const {
    string scoped_name = id;
    for (int i = scope_stack.size() - 1; i >= 0; i--) {
        const map_str_ste &mp = scope_stack[i].table;
        if (mp.find(id) != mp.end()) {
            scoped_name += "_" + std::to_string(i);
            break;
        }
    }
    return scoped_name;
}
void frontend::SymbolTable::add_into_scope(string name, Type type, vector<int> dimension) {
    map_str_ste &mp = scope_stack.back().table;
    mp[name].operand = Operand(name, type);
    mp[name].dimension = dimension;
}
Operand frontend::SymbolTable::get_operand(string id) {
    for (int i = scope_stack.size() - 1; i >= 0; i--) {
        map_str_ste &mp = scope_stack[i].table;
        if (mp.find(id) != mp.end()) {
            Operand tmp =  mp[id].operand;
            tmp.name = get_scoped_name(tmp.name);
            return tmp;
        }
    }
    assert(0);
}
frontend::STE frontend::SymbolTable::get_ste(string id) {
    for (int i = scope_stack.size() - 1; i >= 0; i--) {
        map_str_ste &mp = scope_stack[i].table;
        if (mp.find(id) != mp.end()) {
            return mp[id];
        }
    }
    assert(0 && "can't find STE");
}
frontend::Analyzer::Analyzer(): tmp_cnt(0), symbol_table() {
    for (int i = 0; i < 4000; i++) {
        tmp_int.push({0, i});
    }
    for (int i = 0; i < 4000; i++) {
        tmp_float.push({0, i});
    }
}
string frontend::Analyzer::get_tmp_int() {
    assert(tmp_int.size() != 0);
    PII tmp = tmp_int.top();
    tmp_int.pop();
    if (tmp.first == 1) assert(0);
    string ans = "tmp_int_" + std::to_string(tmp.second);
    return ans;
}
string frontend::Analyzer::get_tmp_float() {
    assert(tmp_float.size() != 0);
    PII tmp = tmp_float.top();
    tmp_float.pop();
    if (tmp.first == 1) assert(0);
    string ans = "tmp_float_" + std::to_string(tmp.second);
    return ans;
};
void frontend::Analyzer::del_tmp_int(string id) {
#ifdef NOTDELTMP
    return;
#endif
    if (id.size() < 8 || id.substr(0, 8) != "tmp_int_") {
        return;
    }
    string tmp = id.substr(8, id.size() - 8);
    int int_tmp = eval_int(tmp);
    tmp_int.push({0, int_tmp});
}
void frontend::Analyzer::del_tmp_float(string id) {
#ifdef NOTDELTMP
    return;
#endif
    if (id.size() < 10 || id.substr(0, 10) != "tmp_float_") {
        return;
    }
    string tmp = id.substr(10, id.size() - 10);
    int int_tmp = eval_int(tmp);
    tmp_float.push({0, int_tmp});
}
void frontend::Analyzer::del_tmp(string id, Type type) {
#ifdef NOTDELTMP
    return;
#endif
    if (type == Type::IntLiteral || type == Type::FloatLiteral) return;
    if (type == Type::Float) del_tmp_float(id);
    else del_tmp_int(id);
}
// 全局变量， 加入global val， 将赋初值加入g_init_inst
// 根据function加指令
#define IFNODETYPE(i, s) if (i < root->children.size() && root->children[i]->type == s)
#define IFNOTNODETYPE(i, s) if (i >= root->children.size() || root->children[i]->type != s)
#define IFTERMTYPE(i, s) if (((Term *)(root->children[i]))->token.type == s)
#define IFNOTTERMTYPE(i, s) if (((Term *)(root->children[i]))->token.type != s)
#define GETTERMIDENT(ident, i) string ident = ((Term *)(root->children[i]))->token.value
#define GETTERMTYPE(ident, i) TokenType ident = ((Term *)(root->children[i]))->token.TokenType
#define GETIDENTTYPE(ident) symbol_table.get_operand(ident).type
#define GETIDENTSIZE(ident) symbol_table.scope_stack.back().table[ident].dimension.size()
#define SCOPE_NAME(ident) symbol_table.get_scoped_name(ident)
ir::Program frontend::Analyzer::get_ir_program(CompUnit *root) {
    log(root);
    symbol_table.add_scope();
    symbol_table.functions = *get_lib_funcs();
    Function *global_function = new Function("global_func", Type::null);
    program.addFunction(*global_function);
    analyzeCompUnit(root);
    program.functions[0].addInst(new Instruction(Operand(), Operand(), Operand(), Operator::_return));
    symbol_table.exit_scope();
    return_log(root);
    return program;
}
void frontend::Analyzer::analyzeCompUnit(CompUnit *root) {
    log(root);

    IFNODETYPE(0, NodeType::DECL) {
        ANALYSIS(decl, Decl, 0);
        analyzeDecl(decl, &program.functions[0]);
    } else {
        ANALYSIS(funcdef, FuncDef, 0);
        analyzeFuncDef(funcdef, NULL);
    }
    IFNODETYPE(1, NodeType::COMPUINT) {

        ANALYSIS(compunit, CompUnit, 1);
        analyzeCompUnit(compunit);
    }

    return_log(root);
}
void frontend::Analyzer::analyzeDecl(Decl *root, Function *function_ptr) {
    log(root);

    IFNODETYPE(0, NodeType::CONSTDECL) {
        ANALYSIS2(constdef, ConstDecl, 0);
    } else {
        ANALYSIS2(vardef, VarDecl, 0);
    }
    return_log(root);
}
void frontend::Analyzer::analyzeConstDecl(ConstDecl *root, Function *function_ptr) {
    log(root);
    ANALYSIS2(btype, BType, 1);
    ANALYSIS(constdef, ConstDef, 2);
    analyzeConstDef(constdef, function_ptr, btype->t);
    int index = 3;
    while (1) {
        IFTERMTYPE(index, TokenType::SEMICN) break;
        ANALYSIS(constdef, ConstDef, index + 1);
        analyzeConstDef(constdef, function_ptr, btype->t);
        index += 2;
    }
    return_log(root);
}
void frontend::Analyzer::analyzeBType(BType *root, Function *function_ptr) {
    log(root);
    IFTERMTYPE(0, TokenType::INTTK) {
        root->t = Type::Int;
    } else IFTERMTYPE(0, TokenType::FLOATTK) {
        root->t = Type::Float;
    } else assert(0);
    return_log(root);
}
void frontend::Analyzer::analyzeConstDef(ConstDef *root, Function *function_ptr, Type btype) { // using Type as BType
    log(root);
    GETTERMIDENT(ident, 0);
    int index = 2;
    vector<int> demension;
    int maxlen = 1;
    while (1) {
        IFNOTNODETYPE(index, NodeType::CONSTEXP) break;
        ANALYSIS2(constexp, ConstExp, index);
        index += 3;
        int dimen = eval_int(constexp->v);
        demension.push_back(dimen);
        maxlen *= dimen;
    }
    if (maxlen != 1 && btype == Type::Int) btype = Type::IntPtr;
    if (maxlen != 1 && btype == Type::Float) btype = Type::FloatPtr;
    symbol_table.add_into_scope(ident, btype, demension);
    string scope_name = symbol_table.get_scoped_name(ident);
    if (function_ptr->name != "global_func") {
        if (maxlen != 1) {
            function_ptr->addInst(new Instruction(Operand(std::to_string(maxlen), Type::IntLiteral),
                                                  Operand(), Operand(scope_name, btype), Operator::alloc));
        }
    } else {
        if (maxlen != 1) program.globalVal.push_back(ir::GlobalVal(Operand(scope_name, btype), maxlen));
        else program.globalVal.push_back(ir::GlobalVal(Operand(scope_name, btype)));
    }
    if (index < root->children.size()) {
        ANALYSIS(constinitval, ConstInitVal, index);
        analyzeConstInitVal(constinitval, function_ptr, ident, 0);
    } else {
        if (btype == Type::Int) {
            function_ptr->addInst(new Instruction(Operand("0", Type::IntLiteral),
                                                  Operand(), Operand(scope_name, btype), Operator::mov));
            GIVESYMBOLVALUE(ident, "0");
            GIVESYMBOLTYPE(ident, Type::IntLiteral);
        } else if (btype == Type::Float) {
            function_ptr->addInst(new Instruction(Operand("0", Type::FloatLiteral),
                                                  Operand(), Operand(scope_name, btype), Operator::fmov));
            GIVESYMBOLVALUE(ident, "0");
            GIVESYMBOLTYPE(ident, Type::FloatLiteral);
        } else assert(0);
    }
    return_log(root);
}
void frontend::Analyzer::analyzeConstInitVal(ConstInitVal *root, Function *function_ptr, string ident, int index) {
    log(root);
    string scope_name = symbol_table.get_scoped_name(ident);
    Type type = GETIDENTTYPE(ident);
    Type LiterType;
    Operator op;
    if (type == Type::Int || type == Type::IntPtr) LiterType = Type::IntLiteral, op = Operator::def;
    else if (type == Type::Float || type == Type::FloatPtr) LiterType = Type::FloatLiteral, op = Operator::fdef;

    Instruction *InitInst;
    IFNODETYPE(0, NodeType::CONSTEXP) {
        ANALYSIS2(constexp, ConstExp, 0);
        if (GETIDENTSIZE(ident) == 0) {
            // 非数组， 只考虑 int a = 3, 未考虑 int a = 3.5
            InitInst = new Instruction(Operand(constexp->v, LiterType), Operand(),
                                       Operand(scope_name, type), op);
            GIVESYMBOLVALUE(ident, constexp->v);
            GIVESYMBOLTYPE(ident, LiterType);
        } else {
            // 数组
            InitInst = new Instruction(Operand(scope_name, type), Operand(std::to_string(index), Type::IntLiteral),
                                       Operand(constexp->v, LiterType), Operator::store);
            // assert(0 && "const no array");
        }
        function_ptr->addInst(InitInst);
    } else {
        // 处理 {2，3，4}
        int index2 = 1;
        while (1) {
            IFTERMTYPE(index2, TokenType::RBRACE) break;
            ANALYSIS(constinitval, ConstInitVal, index2);
            analyzeConstInitVal(constinitval, function_ptr, ident, index);
            IFTERMTYPE(index2 + 1, TokenType::RBRACE) break;
            index++, index2 += 2;
        }
    }
    return_log(root);
}
void frontend::Analyzer::analyzeVarDecl(VarDecl *root, Function *function_ptr) {
    log(root);
    ANALYSIS2(btype, BType, 0);
    int index = 2;
    while (1) {
        IFTERMTYPE(index, TokenType::SEMICN) break;
        ANALYSIS(vardef, VarDef, index - 1);
        analyzeVarDef(vardef, function_ptr, btype->t);
        index += 2;
    }
    ANALYSIS(vardef, VarDef, index - 1);
    analyzeVarDef(vardef, function_ptr, btype->t);
    return_log(root);
}
void frontend::Analyzer::analyzeVarDef(VarDef *root, Function *function_ptr, Type btype) {
    log(root);
    GETTERMIDENT(ident, 0);
    int index = 2;
    vector<int> demension;
    int maxlen = 1;
    bool if_array = false;
    while (1) {
        IFNOTNODETYPE(index, NodeType::CONSTEXP) break;
        ANALYSIS2(constexp, ConstExp, index);
        if_array = true;
        index += 3;
        int dimen = eval_int(constexp->v);
        demension.push_back(dimen);
        maxlen *= dimen;
    }
    if (if_array && btype == Type::Int) btype = Type::IntPtr;
    if (if_array && btype == Type::Float) btype = Type::FloatPtr;
    symbol_table.add_into_scope(ident, btype, demension);
    string scope_name = symbol_table.get_scoped_name(ident);
    if (function_ptr->name != "global_func") {
        if (if_array)
            function_ptr->addInst(new Instruction(Operand(std::to_string(maxlen), Type::IntLiteral),
                                                  Operand(), Operand(scope_name, btype), Operator::alloc));
    } else {
        if (if_array) program.globalVal.push_back(ir::GlobalVal(Operand(scope_name, btype), maxlen));
        else program.globalVal.push_back(ir::GlobalVal(Operand(scope_name, btype)));
    }
    if (index < root->children.size()) {
        ANALYSIS(initval, InitVal, index);
        analyzeInitVal(initval, function_ptr, ident, 0);
    } else {
        if (btype == Type::Int) {
            function_ptr->addInst(new Instruction(Operand("0", Type::IntLiteral),
                                                  Operand(), Operand(scope_name, btype), Operator::mov));
        } else if (btype == Type::Float) {
            function_ptr->addInst(new Instruction(Operand("0", Type::FloatLiteral),
                                                  Operand(), Operand(scope_name, btype), Operator::fmov));
        } else if (btype == Type::IntPtr) {
            for (int i = 0; i < maxlen; i++) {
                Instruction *InitInst = new Instruction(Operand(scope_name, btype), Operand(std::to_string(i), Type::IntLiteral),
                                                        Operand("0", Type::IntLiteral), Operator::store);
                function_ptr->addInst(InitInst);
            }
        } else if (btype == Type::FloatPtr) {
            for (int i = 0; i < maxlen; i++) {
                Instruction *InitInst = new Instruction(Operand(scope_name, btype), Operand(std::to_string(i), Type::IntLiteral),
                                                        Operand("0", Type::FloatLiteral), Operator::store);
                function_ptr->addInst(InitInst);
            }
        }
    }
    return_log(root);
}
void frontend::Analyzer::analyzeInitVal(InitVal *root, Function *function_ptr, string ident, int index) {
    log(root);
    string scope_name = symbol_table.get_scoped_name(ident);
    Type type = GETIDENTTYPE(ident);
    Type LiterType;
    Operator op;
    if (type == Type::Int || type == Type::IntPtr) LiterType = Type::IntLiteral, op = Operator::def;
    else if (type == Type::Float || type == Type::FloatPtr) LiterType = Type::FloatLiteral, op = Operator::fdef;

    Instruction *InitInst;
    IFNODETYPE(0, NodeType::EXP) {
        ANALYSIS2(exp, Exp, 0);
        if (!exp->is_computable && LiterType == Type::IntLiteral) LiterType = Type::Int;
        if (!exp->is_computable && LiterType == Type::FloatLiteral) LiterType = Type::Float;
        if (GETIDENTSIZE(ident) == 0) {
            // 非数组， 只考虑 int a = 3, 未考虑 int a = 3.5
            InitInst = new Instruction(Operand(SCOPE_NAME(exp->v), LiterType), Operand(),
                                       Operand(scope_name, type), op);
        } else {
            // 数组
            InitInst = new Instruction(Operand(scope_name, type), Operand(std::to_string(index), Type::IntLiteral),
                                       Operand(SCOPE_NAME(exp->v), LiterType), Operator::store);
        }
        function_ptr->addInst(InitInst);
    } else {
        // 处理 {2，3，4}
        int index2 = 1;
        while (1) {
            IFTERMTYPE(index2, TokenType::RBRACE) break;
            ANALYSIS(initval, InitVal, index2);
            analyzeInitVal(initval, function_ptr, ident, index);
            IFTERMTYPE(index2 + 1, TokenType::RBRACE) break;
            index++, index2 += 2;
        }
        int len = 1;
        vector<int> dimension = symbol_table.scope_stack.back().table[ident].dimension;
        for (int i = 0; i < dimension.size(); i++) len *= dimension[i];
        for (int i = index + 1; i < len; i++) {
            InitInst = new Instruction(Operand(scope_name, type), Operand(std::to_string(i), Type::IntLiteral),
                                       Operand("0", LiterType), Operator::store);
            function_ptr->addInst(InitInst);
        }
    }
    return_log(root);
}

void frontend::Analyzer::analyzeConstExp(ConstExp *root, Function *function_ptr) {
    log(root);
    ANALYSIS2(addexp, AddExp, 0);
    COPY_EXP_NODE(addexp, root);
    return_log(root);
}
void frontend::Analyzer::analyzeAddExp(AddExp *root, Function *function_ptr) {
    log(root);
    ANALYSIS2(mulexp, MulExp, 0);
    vector<Operand> son_info;
    son_info.push_back({mulexp->v, mulexp->t});
    vector<Operator> op;
    int index = 2;
    while (1) {
        if (root->children.size() <= index) break;
        ANALYSIS2(mulexp, MulExp, index);
        son_info.push_back({mulexp->v, mulexp->t});
        IFTERMTYPE(index - 1, TokenType::MINU) op.push_back(Operator::sub);
        else op.push_back(Operator::add);
        index += 2;
    }
    // check if float
    Operand add_front = son_info[0];
    for (int i = 0; i < op.size(); i++) {
        Operand add_result = do_operator(add_front, son_info[i + 1], op[i], function_ptr);
        add_front = add_result;
        cout << add_front.name << endl;
    }
    root->v = add_front.name, root->t = add_front.type;
    root->is_computable = (root->t == Type::IntLiteral || root->t == Type::FloatLiteral);
    return_log(root);
}
void frontend::Analyzer::analyzeMulExp(MulExp *root, Function *function_ptr) {
    log(root);
    ANALYSIS2(Unaryexp, UnaryExp, 0);
    vector<Operand> son_info;
    son_info.push_back({Unaryexp->v, Unaryexp->t});
    vector<Operator> op;
    int index = 2;
    while (1) {
        if (root->children.size() <= index) break;
        ANALYSIS2(Unaryexp, UnaryExp, index);
        son_info.push_back({Unaryexp->v, Unaryexp->t});
        IFTERMTYPE(index - 1, TokenType::MULT) op.push_back(Operator::mul);
        else IFTERMTYPE(index - 1, TokenType::DIV) op.push_back(Operator::div);
        else IFTERMTYPE(index - 1, TokenType::MOD) op.push_back(Operator::mod);
        else assert(0 && "undefined op type");
        index += 2;
    }
    Operand mul_front = son_info[0];
    for (int i = 0; i < op.size(); i++) {
        Operand mul_result = do_operator(mul_front, son_info[i + 1], op[i], function_ptr);
        mul_front = mul_result;
    }
    root->v = mul_front.name, root->t = mul_front.type;
    root->is_computable = (root->t == Type::IntLiteral || root->t == Type::FloatLiteral);
    return_log(root);
}
void frontend::Analyzer::analyzeExp(Exp *root, Function *function_ptr) {
    log(root);
    ANALYSIS2(addexp, AddExp, 0);
    COPY_EXP_NODE(addexp, root);
    return_log(root);
}
void frontend::Analyzer::analyzeUnaryExp(UnaryExp *root, Function *function_ptr) {
    log(root);
    IFNODETYPE(0, NodeType::PRIMARYEXP) {
        ANALYSIS2(primaryexp, PrimaryExp, 0);
        COPY_EXP_NODE(primaryexp, root);
    } else IFNODETYPE(0, NodeType::UNARYOP) {
        ANALYSIS2(unaryop, UnaryOp, 0);
        ANALYSIS2(unaryexp, UnaryExp, 1);
        COPY_EXP_NODE(unaryexp, root);
        if (unaryexp->is_computable == true) {
            if (unaryop->op == TokenType::MINU) {
                if (unaryexp->t == Type::IntLiteral) {
                    root->v = std::to_string(eval_int(unaryexp->v) * -1), root->t = Type::IntLiteral, root->is_computable = true;
                } else {
                    root->v = std::to_string(stof(unaryexp->v) * -1), root->t = Type::FloatLiteral, root->is_computable = true;
                }
            }
            if (unaryop->op == TokenType::PLUS) {
                if (unaryexp->t == Type::IntLiteral) {
                    root->v = std::to_string(eval_int(unaryexp->v) * 1), root->t = Type::IntLiteral, root->is_computable = true;
                } else {
                    root->v = std::to_string(stof(unaryexp->v) * 1), root->t = Type::FloatLiteral, root->is_computable = true;
                }
            }
            if (unaryop->op == TokenType::NOT && abs(std::stof(root->v)) > 1e-9) {
                root->v = "0", root->t = Type::IntLiteral, root->is_computable = true;
            }
            if (unaryop->op == TokenType::NOT && abs(std::stof(root->v)) < 1e-9) {
                root->v = "1", root->t = Type::IntLiteral, root->is_computable = true;
            }
        } else {
            del_tmp(unaryexp->v, unaryexp->t);
            root->is_computable = false;
            if (unaryexp->t == Type::Float) root->t = Type::Float;
            else root->t = Type::Int;
            if (unaryop->op == TokenType::MINU) {
                Operand ans = do_operator({unaryexp->v, unaryexp->t}, {"-1", Type::IntLiteral}, Operator::mul, function_ptr);
                root->v = ans.name;
            }
            if (unaryop->op == TokenType::PLUS) {
                Operand ans = do_operator({unaryexp->v, unaryexp->t}, {"1", Type::IntLiteral}, Operator::mul, function_ptr);
                root->v = ans.name;
            }
            if (unaryop->op == TokenType::NOT) {
                root->v = get_tmp_int();
                root->t = Type::Int;
                Instruction *eq = new Instruction(Operand("0", Type::IntLiteral), Operand(SCOPE_NAME(unaryexp->v), Type::Int), Operand(root->v, Type::Int), Operator::eq);
                function_ptr->addInst(eq);
            }
        }
    } else {
        GETTERMIDENT(func_name, 0);
        Type type = symbol_table.functions[func_name]->returnType;
        // cout << "***************" << root->children.size() << endl;
        // cout << "***************" << func_name << endl;
        // for (int i = 0; i < root->children.size(); i++) {
        //     cout << frontend::toString(root->children[i]->type) << endl;
        // }
        if (root->children.size() > 3) {
            ANALYSIS(funcrparams, FuncRParams, 2);
            analyzeFuncRParams(funcrparams, function_ptr, root, type, func_name);
        } else {
            string return_tmp;
            if (type == Type::Int) return_tmp = get_tmp_int();
            else if (type == Type::Float) return_tmp = get_tmp_float();
            else return_tmp = "void_tmp";
            ir::CallInst *callinst = new ir::CallInst(Operand(func_name, type),
                    vector<Operand>(), Operand(return_tmp, type));
            function_ptr->addInst(callinst);
            root->v = return_tmp;
        }
        root->t = type, root->is_computable = false;
    }
    return_log(root);
}
void frontend::Analyzer::analyzePrimaryExp(PrimaryExp *root, Function *function_ptr) {
    log(root);
    IFTERMTYPE(0, TokenType::LPARENT) {
        ANALYSIS2(exp, Exp, 1);
        COPY_EXP_NODE(exp, root);
    } else IFNODETYPE(0, NodeType::NUMBER) {
        ANALYSIS2(number, Number, 0);
        COPY_EXP_NODE(number, root);
    } else {
        // LVAL
        ANALYSIS(lval, LVal, 0);
        analyzeLVal(lval, function_ptr, 1);
        COPY_EXP_NODE(lval, root);
    }
    return_log(root);
}
void frontend::Analyzer::analyzeNumber(Number *root, Function *function_ptr) {
    log(root);
    IFTERMTYPE(0, TokenType::INTLTR) {
        GETTERMIDENT(tmp, 0);
        root->v = tmp;
        root->t = Type::IntLiteral;
        root->is_computable = true;
    } else IFTERMTYPE(0, TokenType::FLOATLTR) {
        GETTERMIDENT(tmp, 0);
        root->v = tmp;
        root->t = Type::FloatLiteral;
        root->is_computable = true;
    } else assert(0 && "Number no const");
    return_log(root);
}
void frontend::Analyzer::analyzeUnaryOp(UnaryOp *root, Function *function_ptr) {
    log(root);
    IFTERMTYPE(0, TokenType::MINU) {
        root->op = TokenType::MINU;
    } else IFTERMTYPE(0, TokenType::PLUS) {
        root->op = TokenType::PLUS;
    } else IFTERMTYPE(0, TokenType::NOT) {
        root->op = TokenType::NOT;
    }
    return_log(root);
}

Operand frontend::Analyzer::shift_condition(Operand condition, Function *function_ptr) {
    if (condition.type == Type::FloatLiteral) {
        if (stof(condition.name) == 0) condition = {"0", Type::IntLiteral};
        else condition = {"1", Type::IntLiteral};
    } else if (condition.type == Type::Float) {
        string cond_tmp = get_tmp_float();
        string cond_int = get_tmp_int();
        function_ptr->addInst(new Instruction(Operand("0", Type::FloatLiteral), Operand(SCOPE_NAME(condition.name), Type::Float),
                                              Operand(cond_tmp, Type::Float), Operator::fneq));
        del_tmp_float(cond_tmp);
        function_ptr->addInst(new Instruction(Operand(cond_tmp, Type::Float), Operand(),
                                              Operand(cond_int, Type::Int), Operator::cvt_f2i));
        condition = {cond_int, Type::Int};
    } else if (condition.type == Type::Int || condition.type == Type::IntLiteral) {
        Operand cond_tmp = {get_tmp_int(), Type::Int};
        function_ptr->addInst(new Instruction(Operand(SCOPE_NAME(condition.name), condition.type), Operand(),
                                              cond_tmp, Operator::mov));
        condition = cond_tmp;
    }
    return condition;
}
void frontend::Analyzer::analyzeFuncRParams(FuncRParams *root, Function *function_ptr, UnaryExp *father_root, Type return_type, string func_name) {
    log(root);
    ANALYSIS2(exp, Exp, 0);
    vector<Operand> argumentList;
    Operand origin_argument = {SCOPE_NAME(exp->v), exp->t};
    int argu_index = 0;
    // cout << "**************" << endl;
    // cout << (function_ptr->ParameterList.size()) << endl;
    // cout << (function_ptr->name) << endl;
    vector<Operand> ref;
    for (int i = 0; i < program.functions.size(); i++) {
        if (program.functions[i].name == func_name) {
            ref = program.functions[i].ParameterList;
            break;
        }
    }
    if (ref.size() == 0) ref = symbol_table.functions[func_name]->ParameterList;
    // cout << "*****************" << endl;
    // cout << ref.size() << endl;
    // cout << func_name << endl;
    // cout << symbol_table.functions[func_name]->ParameterList.size() << endl;
    Operand argument = shift_type(ref[argu_index++].type, origin_argument, function_ptr);
    argumentList.push_back(argument);
    // argumentList.push_back(origin_argument);
    int index = 2;
    while (1) {
        if (index >= root->children.size()) break;
        ANALYSIS2(exp, Exp, index);
        origin_argument = {SCOPE_NAME(exp->v), exp->t};
        argument = shift_type(ref[argu_index++].type, origin_argument, function_ptr);
        argumentList.push_back(argument);
        // argumentList.push_back(origin_argument);
        index += 2;
    }
    string return_tmp;
    if (return_type == Type::Float) return_tmp = get_tmp_float();
    else return_tmp = get_tmp_int();
    ir::CallInst *callinst = new ir::CallInst(Operand(func_name, return_type),
            argumentList, Operand(return_tmp, return_type));
    function_ptr->addInst(callinst);
    father_root->v = return_tmp;
    return_log(root);
}
void frontend::Analyzer::analyzeLVal(LVal *root, Function *function_ptr, int if_right) {
    log(root);
    int index = 2;
    vector<std::pair<string, Type>> dimension;
    while (1) {
        if (index >= root->children.size()) break;
        ANALYSIS2(exp, Exp, index);
        dimension.push_back({exp->v, exp->t});
        index += 3;
    }
    for (int i = 0; i < dimension.size(); i++) {
        del_tmp(dimension[i].first, dimension[i].second);
    }
    GETTERMIDENT(ident, 0);
    vector<int> dimension2 = symbol_table.get_ste(ident).dimension;
    Type type = GETIDENTTYPE(ident);

    string ste_ident = symbol_table.get_ste(ident).operand.name;
    if (root->children.size() == 1) {
        if (if_right) {
            root->v = ste_ident, root->t = type, root->is_computable = (type == Type::IntLiteral || type == Type::FloatLiteral);
        } else {
            assert(type != ir::Type::IntPtr &&
                   type != ir::Type::FloatPtr &&
                   type != ir::Type::IntLiteral &&
                   type != ir::Type::FloatLiteral &&
                   "Can NOT be the left value of assignment");
            root->v = ste_ident, root->t = type, root->is_computable = false;
        }
        return_log(root);
        return;
    }
    // 数组, 且【】【】
    string arr_index = get_tmp_int();
    if (dimension.size() == 2) {
        function_ptr->addInst(new Instruction(Operand(std::to_string(dimension2[1]), Type::IntLiteral),
                                              Operand(SCOPE_NAME(dimension[0].first), dimension[0].second), Operand(arr_index, Type::Int), Operator::mul));
        function_ptr->addInst(new Instruction(Operand(arr_index, Type::Int),
                                              Operand(SCOPE_NAME(dimension[1].first), dimension[1].second), Operand(arr_index, Type::Int), Operator::add));
    } else if (dimension.size() == 1) {
        function_ptr->addInst(new Instruction(Operand(SCOPE_NAME(dimension[0].first), dimension[0].second), Operand(), Operand(arr_index, Type::Int), Operator::mov));
    }
    node node_lval;
    if (type == Type::FloatPtr) node_lval.v  = get_tmp_float(), node_lval.t = Type::Float;
    else if (type == Type::IntPtr) node_lval.v  = get_tmp_int(), node_lval.t = Type::Int;
    else {
        assert(0);
    }
    del_tmp_int(arr_index);
    if (if_right) {
        function_ptr->addInst(new Instruction(symbol_table.get_operand(ident),
                                              Operand(arr_index, Type::Int), {node_lval.v, node_lval.t}, Operator::load));
        root->v = node_lval.v, root->t = node_lval.t, root->is_computable = false;
    } else {
        del_tmp(node_lval.v, type);
        root->v = ident, root->t = type, root->is_computable = false, root->i = arr_index;
    }
    return_log(root);
}
Operand frontend::Analyzer::shift_type(Type type, Operand origin, Function *function_ptr) {
    Operand ans;
    // cout << "**************" << endl;
    if (type == Type::Int) {
        if (origin.type == Type::Float || origin.type == Type::FloatLiteral) {
            ans = {get_tmp_int(), Type::Int};
            function_ptr->addInst(new Instruction(origin, {}, ans, Operator::cvt_f2i));
        } else ans = origin;
    } else if (type == Type::Float) {
        if (origin.type == Type::Int || origin.type == Type::IntLiteral) {
            ans = {get_tmp_float(), Type::Float};
            function_ptr->addInst(new Instruction(origin, {}, ans, Operator::cvt_i2f));
        } else ans = origin;
    } else {
        ans = origin;
    }
    return ans;
}
void frontend::Analyzer::analyzeStmt(Stmt *root, Function *function_ptr) {
    log(root);
    IFNODETYPE(0, NodeType::EXP) {
        ANALYSIS2(exp, Exp, 0);
    } else IFNODETYPE(0, NodeType::BLOCK) {
        symbol_table.add_scope();
        ANALYSIS2(block, Block, 0);
        symbol_table.exit_scope();
    } else IFTERMTYPE(0, TokenType::RETURNTK) {
        IFTERMTYPE(1, TokenType::SEMICN) {
            function_ptr->addInst(new Instruction(Operand(), Operand(), Operand(), Operator::_return));
        } else {
            ANALYSIS2(exp, Exp, 1);
            Operand origin = {SCOPE_NAME(exp->v), exp->t};
            Type type = function_ptr->returnType;
            Operand ans = shift_type(type, origin, function_ptr);
            function_ptr->addInst(new Instruction(ans, {}, {}, Operator::_return));
        }
    } else IFNODETYPE(0, NodeType::LVAL) {
        ANALYSIS(lval, LVal, 0);
        analyzeLVal(lval, function_ptr, 0);
        ANALYSIS2(exp, Exp, 2);
        if (lval->t == Type::Int) {
            function_ptr->addInst(new Instruction({SCOPE_NAME(exp->v), exp->t}, Operand(), Operand({SCOPE_NAME(lval->v), lval->t}), Operator::mov));
        } else if (lval->t == Type::Float) {
            function_ptr->addInst(new Instruction({SCOPE_NAME(exp->v), exp->t}, Operand(), Operand({SCOPE_NAME(lval->v), lval->t}), Operator::fmov));
        } else if (lval->t == Type::IntPtr || lval->t == Type::FloatPtr) {
            function_ptr->addInst(new Instruction({SCOPE_NAME(lval->v), lval->t}, {lval->i, Type::Int}, Operand({SCOPE_NAME(exp->v), exp->t}), Operator::store));
        } else {
            assert(0 && "undefined lavl->t type");
        }
    } else IFTERMTYPE(0, TokenType::IFTK) {
        ANALYSIS2(cond, Cond, 2);
        Operand condition = {cond->v, cond->t};
        Instruction *gotoinst = new Instruction({SCOPE_NAME(condition.name), condition.type}, {}, {"2", Type::IntLiteral}, Operator::_goto);
        function_ptr->addInst(gotoinst);
        Instruction *gotoinst2 = new Instruction({}, {}, {}, Operator::_goto);
        function_ptr->addInst(gotoinst2);
        int siz = function_ptr->InstVec.size();
        ANALYSIS2(stmt, Stmt, 4);
        int siz2 = function_ptr->InstVec.size();
        int diff = siz2 - siz;
        gotoinst2->des = {std::to_string(diff + 1), Type::IntLiteral};
        if (root->children.size() > 5) {
            gotoinst2->des = {std::to_string(diff + 2), Type::IntLiteral};
            Instruction *gotoinst3 = new Instruction({}, {}, {}, Operator::_goto);
            function_ptr->addInst(gotoinst3);
            ANALYSIS2(stmt, Stmt, 6);
            int siz3 = function_ptr->InstVec.size();
            gotoinst3->des = {std::to_string(siz3 - siz2), Type::IntLiteral};
        }
    } else IFTERMTYPE(0, TokenType::WHILETK) {
        int siz0 = function_ptr->InstVec.size();
        ANALYSIS2(cond, Cond, 2);
        Operand condition = {cond->v, cond->t};
        assert(cond->t == Type::Int || cond->t == Type::IntLiteral);
        Instruction *gotoinst = new Instruction({SCOPE_NAME(condition.name), condition.type}, {}, {"2", Type::IntLiteral}, Operator::_goto);
        Instruction *gotoinst2 = new Instruction({}, {}, {}, Operator::_goto);
        function_ptr->addInst(gotoinst);
        function_ptr->addInst(gotoinst2);
        int siz = function_ptr->InstVec.size();
        ANALYSIS2(stmt, Stmt, 4);
        Instruction *gotoinst3 = new Instruction({}, {}, {}, Operator::_goto);
        function_ptr->addInst(gotoinst3);
        int siz2 = function_ptr->InstVec.size();
        gotoinst2->des = {std::to_string(siz2 - siz + 1), Type::IntLiteral};
        gotoinst3->des = {std::to_string(siz0 - siz2 + 1), Type::IntLiteral};
        for (int i = siz; i < siz2; i++) {
            Instruction *unuse = function_ptr->InstVec[i];
            if (unuse->op != Operator::__unuse__) continue;
            unuse->op = Operator::_goto;
            if (unuse->des.name == "0") {
                unuse->des = {std::to_string(siz2 - i), Type::IntLiteral};
            } else if (unuse->des.name == "1") {
                unuse->des = {std::to_string(siz0 - i), Type::IntLiteral};
            } else assert(0);
        }
    } else IFTERMTYPE(0, TokenType::BREAKTK) {
        function_ptr->addInst(new Instruction({}, {}, {"0", Type::IntLiteral}, Operator::__unuse__));
    } else IFTERMTYPE(0, TokenType::CONTINUETK) {
        function_ptr->addInst(new Instruction({}, {}, {"1", Type::IntLiteral}, Operator::__unuse__));
    }
    return_log(root);
}
void frontend::Analyzer::analyzeCond(Cond *root, Function *function_ptr) {
    log(root);
    ANALYSIS(lorexp, LOrExp, 0);
    analyzeLOrExp(lorexp, function_ptr, 0);
    COPY_EXP_NODE(lorexp, root);
    return_log(root);
}
void frontend::Analyzer::analyzeLOrExp(LOrExp *root, Function *function_ptr, int index) {
    log(root);
    ANALYSIS(landexp, LAndExp, 0);
    analyzeLAndExp(landexp, function_ptr, 0);
    Operand land_int = {landexp->v, landexp->t};
    if (root->children.size() > 1) {
        Instruction *gotoinst = new Instruction({SCOPE_NAME(land_int.name), land_int.type}, {}, {}, Operator::_goto);
        function_ptr->addInst(gotoinst);
        int siz = function_ptr->InstVec.size();
        ANALYSIS(lorexp, LOrExp, 2);
        analyzeLOrExp(lorexp, function_ptr, index + 1);
        Operand lor_int = {lorexp->v, lorexp->t};
        del_tmp_int(lor_int.name);
        Instruction *orinst = new Instruction({SCOPE_NAME(land_int.name), land_int.type}, {SCOPE_NAME(lor_int.name), lor_int.type}, land_int, Operator::_or);
        function_ptr->addInst(orinst);
        int siz2 = function_ptr->InstVec.size();
        gotoinst->des = {std::to_string(siz2 - siz + 1 + index + 2), Type::IntLiteral};
    }
    root->v = land_int.name, root->t = land_int.type;
    root->is_computable = (root->t == Type::IntLiteral || root->t == Type::FloatLiteral);
    return_log(root);
}
void frontend::Analyzer::analyzeLAndExp(LAndExp *root, Function *function_ptr, int index) {
    ANALYSIS2(eqexp, EqExp, 0);
    Operand eq_int = {eqexp->v, eqexp->t};
    if (root->children.size() > 1) {
        Operand not_eq_int = {get_tmp_int(), Type::Int};
        Instruction *notinst = new Instruction({SCOPE_NAME(eq_int.name), eq_int.type}, {}, not_eq_int, Operator::_not);
        function_ptr->addInst(notinst);
        del_tmp_int(not_eq_int.name);
        int siz = function_ptr->InstVec.size();
        Instruction *gotoinst = new Instruction({SCOPE_NAME(not_eq_int.name), not_eq_int.type}, {}, {}, Operator::_goto);
        function_ptr->addInst(gotoinst);
        ANALYSIS(landexp, LAndExp, 2);
        analyzeLAndExp(landexp, function_ptr, index + 1);
        Operand land_int = {landexp->v, landexp->t};
        del_tmp_int(land_int.name);
        Instruction *andinst = new Instruction({SCOPE_NAME(land_int.name), land_int.type}, eq_int, eq_int, Operator::_and);
        function_ptr->addInst(andinst);
        int siz2 = function_ptr->InstVec.size();
        gotoinst->des = {std::to_string(siz2 - siz + 1 + index), Type::IntLiteral};
    }
    root->v = eq_int.name, root->t = eq_int.type;
    root->is_computable = (root->t == Type::IntLiteral || root->t == Type::FloatLiteral);
    return_log(root);
}
int frontend::Analyzer::eval_int(std::string s) {
    if (s.size() >= 2 && (s.substr(0, 2) == "0b" || s.substr(0, 2) == "0B")) {
        return std::stoi(s.substr(2, s.size() - 2), nullptr, 2);
    } else if (s.size() >= 2 && (s.substr(0, 2) == "0x" || s.substr(0, 2) == "0X")) {
        return std::stoi(s.substr(2, s.size() - 2), nullptr, 16);
    } else if (s.size() > 1 && s.substr(0, 1) == "0") {
        return std::stoi(s.substr(1, s.size() - 1), nullptr, 8);
    } else {
        return std::stoi(s);
    }
}
Operand frontend::Analyzer::do_operator(Operand op1, Operand op2, Operator op, Function *function_ptr) {
    Operator fop;
    Operand tmp_int = {get_tmp_int(), Type::Int};
    Operand tmp_float = {get_tmp_float(), Type::Float};
    op1 = {SCOPE_NAME(op1.name), op1.type};
    op2 = {SCOPE_NAME(op2.name), op2.type};
    switch (op) {
    case Operator::lss:
        fop = Operator::flss;
        break;
    case Operator::gtr:
        fop = Operator::fgtr;
        break;
    case Operator::leq:
        fop = Operator::fleq;
        break;
    case Operator::geq:
        fop = Operator::fgeq;
        break;
    case Operator::eq:
        fop = Operator::feq;
        break;
    case Operator::neq:
        fop = Operator::fneq;
        break;
    case Operator::mul:
        fop = Operator::fmul;
        break;
    case Operator::div:
        fop = Operator::fdiv;
        break;
    case Operator::add:
        fop = Operator::fadd;
        break;
    case Operator::sub:
        fop = Operator::fsub;
        break;
    default:
        break;
    }
    if (op1.type == Type::IntLiteral && op2.type == Type::IntLiteral) {
        if (op == Operator::mul)
            return {std::to_string(eval_int(op1.name) * eval_int(op2.name)), Type::IntLiteral};
        if (op == Operator::div)
            return {std::to_string(eval_int(op1.name) / eval_int(op2.name)), Type::IntLiteral};
        if (op == Operator::mod)
            return {std::to_string(eval_int(op1.name) % eval_int(op2.name)), Type::IntLiteral};
        if (op == Operator::add)
            return {std::to_string(eval_int(op1.name) + eval_int(op2.name)), Type::IntLiteral};
        if (op == Operator::sub)
            return {std::to_string(eval_int(op1.name) - eval_int(op2.name)), Type::IntLiteral};
    }
    if (op1.type == Type::IntLiteral && op2.type == Type::FloatLiteral) {
        if (op == Operator::mul)
            return {std::to_string(eval_int(op1.name) * stof(op2.name)), Type::FloatLiteral};
        if (op == Operator::div)
            return {std::to_string(eval_int(op1.name) / stof(op2.name)), Type::FloatLiteral};
        if (op == Operator::add)
            return {std::to_string(eval_int(op1.name) + stof(op2.name)), Type::FloatLiteral};
        if (op == Operator::sub)
            return {std::to_string(eval_int(op1.name) - stof(op2.name)), Type::FloatLiteral};
    }
    if (op1.type == Type::FloatLiteral && op2.type == Type::IntLiteral) {
        if (op == Operator::mul)
            return {std::to_string(stof(op1.name) * eval_int(op2.name)), Type::FloatLiteral};
        if (op == Operator::div)
            return {std::to_string(stof(op1.name) / eval_int(op2.name)), Type::FloatLiteral};
        if (op == Operator::add)
            return {std::to_string(stof(op1.name) + eval_int(op2.name)), Type::FloatLiteral};
        if (op == Operator::sub)
            return {std::to_string(stof(op1.name) - eval_int(op2.name)), Type::FloatLiteral};
    }
    if (op1.type == Type::FloatLiteral && op2.type == Type::FloatLiteral) {
        if (op == Operator::mul)
            return {std::to_string(stof(op1.name) * stof(op2.name)), Type::FloatLiteral};
        if (op == Operator::div)
            return {std::to_string(stof(op1.name) / stof(op2.name)), Type::FloatLiteral};
        if (op == Operator::add)
            return {std::to_string(stof(op1.name) + stof(op2.name)), Type::FloatLiteral};
        if (op == Operator::sub)
            return {std::to_string(stof(op1.name) - stof(op2.name)), Type::FloatLiteral};
    }
    switch (op1.type) {
    case Type::IntLiteral:
    case Type::Int:
        switch (op2.type) {
        case Type::IntLiteral:
        case Type::Int:
            function_ptr->addInst(new Instruction(op1, op2, tmp_int, op));
            return tmp_int;
            break;
        case Type::FloatLiteral:
        case Type::Float:
            function_ptr->addInst(new Instruction(op1, {}, tmp_float, Operator::cvt_i2f));
            function_ptr->addInst(new Instruction(tmp_float, op2, tmp_float, fop));
            return tmp_float;
        default:
            break;
        }
        break;
    case Type::FloatLiteral:
    case Type::Float:
        switch (op2.type) {
        case Type::IntLiteral:
        case Type::Int:
            function_ptr->addInst(new Instruction(op2, {}, tmp_float, Operator::cvt_i2f));
            function_ptr->addInst(new Instruction(op1, tmp_float, tmp_float, fop));
            return tmp_float;
            break;
        case Type::FloatLiteral:
        case Type::Float:
            function_ptr->addInst(new Instruction(op1, op2, tmp_float, fop));
            return tmp_float;
        default:
            break;
        }
        break;
    default:
        break;
    }
}
void frontend::Analyzer::analyzeEqExp(EqExp *root, Function *function_ptr) {
    log(root);
    ANALYSIS2(relexp, RelExp, 0);
    Operand rel_int = {relexp->v, relexp->t};
    int index = 2;
    bool is_shift = false;
    while (1) {
        if (index >= root->children.size()) break;
        ANALYSIS2(relexp2, RelExp, index);
        Operand rel2_int = {relexp2->v, relexp2->t};
        del_tmp_int(rel2_int.name);
        Operator op;
        IFTERMTYPE(1, TokenType::EQL) op = Operator::eq;
        else IFTERMTYPE(1, TokenType::NEQ) op = Operator::neq;
        Operand ans = do_operator(rel_int, rel2_int, op, function_ptr);
        ans = shift_condition(ans, function_ptr);
        rel_int = ans;
        index += 2;
        is_shift = true;
    }
    if (!is_shift) rel_int = shift_condition(rel_int, function_ptr);
    root->v = rel_int.name, root->t = rel_int.type;
    root->is_computable = (root->t == Type::IntLiteral || root->t == Type::FloatLiteral);
    return_log(root);
}
void frontend::Analyzer::analyzeRelExp(RelExp *root, Function *function_ptr) {
    log(root);
    ANALYSIS2(addexp, AddExp, 0);
    Operand add_int = {addexp->v, addexp->t};
    int index = 2;
    while (1) {
        if (index >= root->children.size()) break;
        ANALYSIS2(addexp, AddExp, index);
        Operand add2_int = {addexp->v, addexp->t};
        Operator op;
        IFTERMTYPE(index - 1, TokenType::LSS) op = Operator::lss;
        else IFTERMTYPE(index - 1, TokenType::GTR) op = Operator::gtr;
        else IFTERMTYPE(index - 1, TokenType::GEQ) op = Operator::geq;
        else IFTERMTYPE(index - 1, TokenType::LEQ) op = Operator::leq;
        Operand ans = do_operator(add_int, add2_int, op, function_ptr);
        ans = shift_condition(ans, function_ptr);
        add_int = ans;
        index += 2;
    }
    root->v = add_int.name, root->t = add_int.type;
    root->is_computable = (root->t == Type::IntLiteral || root->t == Type::FloatLiteral);
    return_log(root);
}
void frontend::Analyzer::analyzeBlock(Block *root, Function *function_ptr) {
    log(root);
    int index = 1;
    while (1) {
        if (index >= root->children.size() - 1) break;
        ANALYSIS2(blockitem, BlockItem, index);
        index++;
    }
    return_log(root);
}
void frontend::Analyzer::analyzeBlockItem(BlockItem *root, Function *function_ptr) {
    log(root);
    IFNODETYPE(0, NodeType::DECL) {
        ANALYSIS2(decl, Decl, 0);
    } else IFNODETYPE(0, NodeType::STMT) {
        ANALYSIS2(stmt, Stmt, 0);
    } else assert(0 && "no BlockItem type");
    return_log(root);
}

void frontend::Analyzer::analyzeFuncDef(FuncDef *root, Function *function_ptr) {
    log(root);
    ANALYSIS2(functype, FuncType, 0);
    GETTERMIDENT(func_name, 1);
    program.addFunction(Function(func_name, functype->t));
    function_ptr = &program.functions[program.functions.size() - 1];
    // if (func_name == "main") {
    //     cout << symbol_table.functions["float_abs"]->name << endl;
    //     // assert(0);
    // }
    if (func_name == "main") {
        string tmp_return = get_tmp_int();
        ir::CallInst *callglobal = new ir::CallInst(Operand("global_func", Type::null),
                vector<Operand>(), Operand(tmp_return, Type::Int));
        del_tmp_int(tmp_return);
        function_ptr->addInst((Instruction *)callglobal);
    }
    symbol_table.functions[func_name] = function_ptr;

    if (root->children.size() > 5) {
        symbol_table.add_scope();
        ANALYSIS2(funcfparams, FuncFParams, 3);
        ANALYSIS2(block, Block, 5);
        symbol_table.exit_scope();
    } else {
        symbol_table.add_scope();
        ANALYSIS2(block, Block, 4);
        symbol_table.exit_scope();
    }

    // if (func_name == "float_abs") {
    //     cout << "??????????????????????" << endl;
    //     cout << symbol_table.functions[func_name]->ParameterList.size() << endl;
    //     // assert(0);
    // }
    if (functype->t == Type::Int) function_ptr->addInst(new Instruction({"0", Type::IntLiteral}, {}, {}, Operator::_return));
    else if (functype->t == Type::Float) function_ptr->addInst(new Instruction({"0", Type::FloatLiteral}, {}, {}, Operator::_return));
    else function_ptr->addInst(new Instruction({}, {}, {}, Operator::_return));
    return_log(root);
}
void frontend::Analyzer::analyzeFuncType(FuncType *root, Function *function_ptr) {
    log(root);
    IFTERMTYPE(0, TokenType::VOIDTK) {
        root->t = Type::null;
    } else IFTERMTYPE(0, TokenType::FLOATTK) {
        root->t = Type::Float;
    } else  IFTERMTYPE(0, TokenType::INTTK) {
        root->t = Type::Int;
    } else assert(0 && "undefined FuncType");
    return_log(root);
}
void frontend::Analyzer::analyzeFuncFParams(FuncFParams *root, Function *function_ptr) {
    log(root);
    int index = 0;
    while (1) {
        if (index >= root->children.size()) break;
        ANALYSIS2(funcfparam, FuncFParam, index);
        index += 2;
    }
    return_log(root);
}
void frontend::Analyzer::analyzeFuncFParam(FuncFParam *root, Function *function_ptr) {
    log(root);
    ANALYSIS2(btype, BType, 0);
    GETTERMIDENT(ident, 1);
    if (root->children.size() > 2) {
        Type type;
        if (btype->t == Type::Int) type = Type::IntPtr;
        else type = Type::FloatPtr;
        symbol_table.add_into_scope(ident, type, vector<int>());
        function_ptr->ParameterList.push_back(symbol_table.get_operand(ident));
    } else {
        symbol_table.add_into_scope(ident, btype->t, vector<int>());
        function_ptr->ParameterList.push_back(symbol_table.get_operand(ident));
    }
    return_log(root);
}
#define DEBUG_SEMANTIC
void frontend::Analyzer::log(AstNode *node) {
#ifdef DEBUG_SEMANTIC
    std::cout << "in " << toString(node->type) << std::endl;
#endif
}
void frontend::Analyzer::return_log(AstNode *node) {
#ifdef DEBUG_SEMANTIC
    std::cout << "out " << toString(node->type) << std::endl;
#endif
}