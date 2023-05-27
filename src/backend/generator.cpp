#include"backend/generator.h"
#include"./debug.h"
#include<assert.h>

using ir::Function;
using ir::Instruction;
using ir::Operator;
using ir::Operand;
using std::string;
using ir::GlobalVal;
using std::to_string;
using std::endl;
using std::pair;
using ir::Type;
using rv::rv_inst;
using rv::rvOPCODE;
using std::cout;
#define TODO assert(0 && "todo")

backend::Generator::Generator( ir::Program &p, std::ofstream &f ): program( p ), fout( f ) {}

void backend::Generator::gen() {
    set_reg.insert( "a0" );
    set_reg.insert( "s0" );
    set_reg.insert( "s1" );
    set_reg.insert( "s2" );
    set_reg.insert( "s3" );
    set_reg.insert( "s4" );
    set_reg.insert( "s5" );
    set_reg.insert( "s6" );
    set_reg.insert( "s7" );
    set_reg.insert( "s8" );
    set_reg.insert( "s9" );
    set_reg.insert( "s10" );
    set_reg.insert( "s11" );
    set_reg.insert( "a3" );
    set_reg.insert( "a4" );
    set_reg.insert( "a5" );
    set_reg.insert( "ra" );
    int i = 0;
    for ( Function function : program.functions ) {
        if ( function.name == "global_func" ) {
            gen_global_func( function );
        } else {
            gen_func( function, "_" + to_string( i ) );
        }
        i++;
    }
}

void backend::Generator::gen_global_func( const Function &function ) {
    fout << "   .file	\"ref.c\"\n";
    fout << "   .option nopic\n";
    fout << "   .text\n";
    for ( Instruction *inst : function.InstVec ) {
        if ( inst->op == Operator::def ) {
            fout << "   .globl	" << inst->des.name << '\n';
            fout << "   .data\n";
            fout << "   .align 2\n";
            fout << "   .type	" << inst->des.name << ", @object\n";
            fout << "   .size	" << inst->des.name << ", 4\n";
            fout << inst->des.name << ":\n";
            fout << "   .word	" << inst->op1.name << "\n";
        } else if ( inst->op == Operator::store && inst->op2.name == "0" ) {
            fout << "   .globl	" << inst->op1.name << '\n';
            fout << "   .data\n";
            fout << "   .align 2\n";
            fout << "   .type	" << inst->op1.name << ", @object\n";
            fout << "   .size	" << inst->op1.name << ", " << find_size( inst->op1.name ) << "\n";
            fout << inst->op1.name << ":\n";
            fout << "   .word	" << inst->des.name << "\n";
        } else if ( inst->op == Operator::store ) {
            fout << "   .word   " << inst->des.name << "\n";
        }
    }
    fout << "   .text" << endl;
    fout << "   .align	1" << endl;
}

string backend::Generator::find_size( const string &s ) {
    for ( GlobalVal global_val : program.globalVal ) {
        if ( global_val.val.name == s ) {
            return to_string( global_val.maxlen * 4 );
        }
    }
    assert( 0 );
}
void backend::Generator::gen_addi( string s, string s2, string s3 ) {
    //fout << " addi    " << s << "," << s2 << "," << s3 << endl;
    rv_inst rvinst;
    rvinst.rd = s;
    rvinst.rs1 = s2;
    rvinst.imm = s3;
    rvinst.op = rvOPCODE::ADDI;
    vec_rv_inst.push_back( rvinst );
    return;
}
void backend::Generator::gen_sw( string s, int is_reg, int is_sp ) {
    if ( set_reg.count( s ) ) {
        rv_inst rvinst;
        int offset = map_s_reg_to_addr[s];
        rvinst.rs2 = s;
        rvinst.rs1 = "s0";
        rvinst.imm = to_string( offset );
        rvinst.op = rvOPCODE::SW;
        vec_rv_inst.push_back( rvinst );
        return;
    }
    if ( is_tmp( s ) ) {
        rv_inst rvinst;
        rvinst.rd = map_tmp_register[s];
        int offset = map_s_reg_to_addr[rvinst.rd];
        rvinst.rs1 = "s0";
        rvinst.op = rvOPCODE::SW;
        rvinst.imm = to_string( offset );
        vec_rv_inst.push_back( rvinst );
        return;
    }
    return;
}
void backend::Generator::gen_lw( string s ) {
    if ( set_reg.count( s ) ) {
        rv_inst rvinst;
        int offset = map_s_reg_to_addr[s];
        rvinst.rd = s;
        rvinst.rs1 = "s0";
        rvinst.imm = to_string( offset );
        rvinst.op = rvOPCODE::LW;
        vec_rv_inst.push_back( rvinst );
        return;
    }
    if ( is_tmp( s ) ) {
        rv_inst rvinst;
        rvinst.rd = map_tmp_register[s];
        int offset = map_s_reg_to_addr[rvinst.rd];
        rvinst.rs1 = "s0";
        rvinst.op = rvOPCODE::LW;
        rvinst.imm = to_string( offset );
        vec_rv_inst.push_back( rvinst );
        return;
    }
    return;
}
void backend::Generator::add_store_s0() {
    rv_inst rvinst;
    rvinst.rs2 = "s0";
    rvinst.rs1 = "sp";
    rvinst.imm = "-4";
    rvinst.op = rvOPCODE::SW;
    vec_rv_inst.push_back( rvinst );
    rvinst.rs2 = "ra";
    rvinst.imm = "-8";
    vec_rv_inst.push_back( rvinst );
    return;
}
void backend::Generator::gen_func( const Function &function, string back ) {
    vec_rv_inst.clear();
    cal_seg( function );
    //debug
    out( map_seg );
    do_line_tmp( function );
    alloc_tmp_register( function );
    int offset = alloc_all_addr( function, back );
    out( varmap._table );
    out( map_tmp_register );
    fout << "   .globl	" << function.name << endl;
    fout << "   .type	" << function.name << ", @function" << endl;
    fout << function.name << ":" << endl;
    add_store_s0();
    gen_addi( "sp", "sp", to_string( offset ) );
    gen_addi( "s0", "sp", to_string( -offset ) );
    for ( int i = 0; i < function.InstVec.size(); i++ ) {
        Operator op = function.InstVec[i]->op;
        if ( op == Operator::call && ( ( ir::CallInst * )function.InstVec[i] )->op1.name == "global_func" ) continue;
        switch ( op ) {
        case Operator::def:
        case Operator::mov:
            do_move( *function.InstVec[i] );
            break;
        case Operator::sub:
        case Operator::add:
            // add && addi, for efficiency
            do_add( *function.InstVec[i], shift_to_rvopcode( op ) );
            break;
        case Operator::_return:
            do_return( *function.InstVec[i], offset );
            break;
        case Operator::store:
            do_store( *function.InstVec[i] );
            break;
        case Operator::load:
            do_load( *function.InstVec[i] );
            break;
        case Operator::mod:
        case Operator::div:
        case Operator::mul:
            do_mul( *function.InstVec[i], shift_to_rvopcode( op ) );
            break;
        case Operator::_goto:
            assert( 0 );
            break;
        case Operator::call:
        //do_call( *function.InstVec[i] );
        default:
            break;
        }
    }
    for ( rv_inst rvinst : vec_rv_inst ) {
        fout << rvinst.draw() << endl;
    }
    fout << "	.size	" << function.name << ", .-" << function.name << endl;
    fout << "	.ident	\"GCC: (GNU) 9.2.0\"" << endl;
    fout << "	.section	.note.GNU-stack,\"\",@progbits" << endl;
    return;
}
rvOPCODE backend::Generator::shift_to_rvopcode( Operator op ) {
    switch ( op ) {
    case Operator::add:
        return rvOPCODE::ADD;
        break;
    case Operator::sub:
        return rvOPCODE::SUB;
        break;
    case Operator::mul:
        return rvOPCODE::MUL;
        break;
    case Operator::div:
        return rvOPCODE::DIV;
        break;
    case Operator::mod:
        return rvOPCODE::REM;
        break;
    default:
        assert( 0 );
        break;
    }
}
void backend::Generator::add_fuck_in() {
    rv_inst rvinst;
    rvinst.op = rvOPCODE::FUCKIN;
    vec_rv_inst.push_back( rvinst );
}
void backend::Generator::add_fuck_out() {
    rv_inst rvinst;
    rvinst.op = rvOPCODE::FUCKOUT;
    vec_rv_inst.push_back( rvinst );
}
void backend::Generator::do_mul( Instruction inst, rvOPCODE rvopcode ) {
    Operand op1 = inst.op1;
    Operand op2 = inst.op2;
    rv_inst rvinst;
    if ( op1.type == Type::IntLiteral && op2.type == Type::IntLiteral )    {
        if ( rvopcode == rvOPCODE::MUL ) {
            Operand imm = {to_string( eval_int( op1.name ) * eval_int( op2.name ) ), Type::IntLiteral};
            do_move( Instruction( imm, {}, inst.des, Operator::mov ) );
        } else {
            Operand imm = {to_string( eval_int( op1.name ) / eval_int( op2.name ) ), Type::IntLiteral};
            do_move( Instruction( imm, {}, inst.des, Operator::mov ) );
        }
        return;
    }
    rvinst.op = rvopcode;
    rvinst.rd = "a5";
    if ( is_tmp( inst.des.name ) ) {
        rvinst.rd = map_tmp_register[inst.des.name];
    }
    rvinst.rs1 = get_reg_no_liter( op1, "a4" );
    rvinst.rs2 = get_reg_no_liter( op2, "a3" );
    if ( !is_tmp( inst.des.name ) ) {
        store_reg( inst.des.name, "a5" );
    }
    vec_rv_inst.push_back( rvinst );
    return;
}
string backend::Generator::get_reg_no_liter( Operand operand, string reg ) {
    if ( operand.type != Type::IntLiteral ) {
        return get_reg( operand, reg );;
    } else {
        Instruction mv = {operand, {}, {reg, Type::Int}, Operator::mov};
        do_move( mv );
        return reg;
    }
}
void backend::Generator::do_local_addr( Instruction inst ) {
    string reg = get_reg_no_liter( inst.op2, "a5" );
    // a4 = len * 4
    rv_inst rvinst_sll;
    rvinst_sll.op = rvOPCODE::SLLI;
    rvinst_sll.rd = "a4";
    rvinst_sll.rs1 = reg;
    rvinst_sll.imm = "2";
    vec_rv_inst.push_back( rvinst_sll );
    // a3 = offset + a4
    int offset = varmap._table[inst.op1.name];
    rv_inst rvinst_add;
    rvinst_add.op = rvOPCODE::ADDI;
    rvinst_add.rd = "a3";
    rvinst_add.imm =  to_string( offset );
    rvinst_add.rs1 = "a4";
    vec_rv_inst.push_back( rvinst_add );
    // a3 = a3 + s0
    rvinst_add.op = rvOPCODE::ADD;
    rvinst_add.rd = "a3";
    rvinst_add.rs1 =  "a3";
    rvinst_add.rs2 = "s0";
    vec_rv_inst.push_back( rvinst_add );
}
void backend::Generator::do_global_addr( Instruction inst ) {
    // global
    string reg = get_reg_no_liter( inst.op2, "a5" );
    // a4 = len * 4
    rv_inst rvinst_sll;
    rvinst_sll.op = rvOPCODE::SLLI;
    rvinst_sll.rd = "a4";
    rvinst_sll.rs1 = reg;
    rvinst_sll.imm = "2";
    vec_rv_inst.push_back( rvinst_sll );
    // a3 base
    get_global_addr( inst.op1.name, "a3" );
    // a3 = a3 + a4
    rv_inst rvinst_add;
    rvinst_add.op = rvOPCODE::ADD;
    rvinst_add.rd = "a3";
    rvinst_add.rs1 =  "a3";
    rvinst_add.rs2 = "a4";
    vec_rv_inst.push_back( rvinst_add );
}
void backend::Generator::do_load( Instruction inst ) {
    // tmp, local, global
    if ( varmap._table.count( inst.op1.name ) ) {
        // local
        do_local_addr( inst );
    } else {
        do_global_addr( inst );
    }
    // sw
    if ( is_tmp( inst.des.name ) ) {
        rv_inst rvinst_lw;
        rvinst_lw.op = rvOPCODE::LW;
        rvinst_lw.rd = get_reg( inst.des, "a5" );
        rvinst_lw.rs1 = "a3";
        rvinst_lw.imm = "0";
        vec_rv_inst.push_back( rvinst_lw );
    } else {
        // sw
        rv_inst rvinst_lw;
        rvinst_lw.op = rvOPCODE::LW;
        rvinst_lw.rs2 = "a5";
        rvinst_lw.rs1 = "a3";
        rvinst_lw.imm = "0";
        vec_rv_inst.push_back( rvinst_lw );
        store_reg( inst.des.name, "a5" );
    }
}
void backend::Generator::do_store( Instruction inst ) {
    // tmp, local, global
    if ( varmap._table.count( inst.op1.name ) ) {
        // local
        do_local_addr( inst );
    } else {
        // global
        do_global_addr( inst );
    }
    // sw
    string reg = get_reg_no_liter( inst.des, "a5" );
    rv_inst rvinst_sw;
    rvinst_sw.op = rvOPCODE::SW;
    rvinst_sw.rs2 = reg;
    rvinst_sw.rs1 = "a3";
    rvinst_sw.imm = "0";
    vec_rv_inst.push_back( rvinst_sw );
}
void backend::Generator::get_global_addr( string arr_name, string reg ) {
    rv_inst rvinst_lui;
    rvinst_lui.op = rvOPCODE::LUI;
    rvinst_lui.rd = reg;
    rvinst_lui.imm = "%hi(" + arr_name + ")";
    vec_rv_inst.push_back( rvinst_lui );
    rv_inst rvinst_addi;
    rvinst_addi.op = rvOPCODE::ADDI;
    rvinst_addi.rd = reg;
    rvinst_addi.rs1 = reg;
    rvinst_addi.imm = "%lo(" + arr_name + ")";
    vec_rv_inst.push_back( rvinst_addi );
}
void backend::Generator::do_jr_ra() {
    rv_inst rvinst;
    rvinst.op = rvOPCODE::J;
    vec_rv_inst.push_back( rvinst );
    return;
}
void backend::Generator::do_return( Instruction inst, int offset ) {
    Operand op1 = inst.op1;
    do_move( Instruction( op1, {}, {"a0", Type::Int}, Operator::mov ) );
    gen_lw( "ra" );
    gen_lw( "s0" );
    gen_addi( "sp", "sp", to_string( -offset ) );
    do_jr_ra();
    return;
}
void backend::Generator::do_add( Instruction inst, rvOPCODE rvopcode ) {
    Operand op1 = inst.op1;
    Operand op2 = inst.op2;
    rv_inst rvinst;
    switch ( op1.type ) {
    case Type::IntLiteral:
        switch ( op2.type ) {
        case Type::IntLiteral: {
            if ( rvopcode == rvOPCODE::ADD ) {
                Operand imm = {to_string( eval_int( op1.name ) + eval_int( op2.name ) ), Type::IntLiteral};
                do_move( Instruction( imm, {}, inst.des, Operator::mov ) );
            } else {
                Operand imm = {to_string( eval_int( op1.name ) - eval_int( op2.name ) ), Type::IntLiteral};
                do_move( Instruction( imm, {}, inst.des, Operator::mov ) );
            }
        }
        break;
        case Type::Int: {
            rvinst.rd = "a5";
            if ( is_tmp( inst.des.name ) ) rvinst.rd = map_tmp_register[inst.des.name];
            rvinst.rs1 = get_reg( op2, "a5" );
            if ( rvopcode == rvOPCODE::ADD ) rvinst.imm = to_string( eval_int( op1.name ) );
            else rvinst.imm = to_string( -eval_int( op1.name ) );
            rvinst.op = rvOPCODE::ADDI;
            vec_rv_inst.push_back( rvinst );
            if ( !is_tmp( inst.des.name ) ) store_reg( inst.des.name, rvinst.rd );
        }
        break;
        default:
            break;
        }
        break;
    case Type::Int:
        switch ( op2.type ) {
        case Type::IntLiteral:
            rvinst.rd = "a5";
            if ( is_tmp( inst.des.name ) ) rvinst.rd = map_tmp_register[inst.des.name];
            rvinst.rs1 = get_reg( op1, "a5" );
            if ( rvopcode == rvOPCODE::ADD ) rvinst.imm = to_string( eval_int( op2.name ) );
            else rvinst.imm = to_string( -eval_int( op2.name ) );
            rvinst.op = rvOPCODE::ADDI;
            vec_rv_inst.push_back( rvinst );
            if ( !is_tmp( inst.des.name ) ) store_reg( inst.des.name, rvinst.rd );
            break;
        case Type::Int:
            rvinst.rd = "a5";
            if ( is_tmp( inst.des.name ) ) {
                if ( map_tmp_register.count( inst.des.name ) == 0 ) assert( 0 );
                rvinst.rd = map_tmp_register[inst.des.name];
            };
            rvinst.rs1 = get_reg( op1, "a5" );
            rvinst.rs2 = get_reg( op2, "a4" );
            rvinst.op = rvopcode;
            vec_rv_inst.push_back( rvinst );
            if ( !is_tmp( inst.des.name ) ) store_reg( inst.des.name, rvinst.rd );
            break;
        default:
            break;
        }
        break;
    default:
        break;
    }
}
int backend::Generator::eval_int( std::string s ) {
    if ( s.size() >= 2 && ( s.substr( 0, 2 ) == "0b" || s.substr( 0, 2 ) == "0B" ) ) {
        return std::stoi( s.substr( 2, s.size() - 2 ), nullptr, 2 );
    } else if ( s.size() >= 2 && ( s.substr( 0, 2 ) == "0x" || s.substr( 0, 2 ) == "0X" ) ) {
        return std::stoi( s.substr( 2, s.size() - 2 ), nullptr, 16 );
    } else if ( s.size() > 1 && s.substr( 0, 1 ) == "0" ) {
        return std::stoi( s.substr( 1, s.size() - 1 ), nullptr, 8 );
    } else {
        return std::stoi( s );
    }
}
bool backend::Generator::is_tmp( string name ) {
    if ( name.size() > 7 && name.substr( 0, 7 ) == "tmp_int" ) return true;
    return false;
}
string backend::Generator::get_reg( Operand operand, string reg ) {
    string name = operand.name;
    if ( operand.type == Type::IntLiteral ) {
        return to_string( eval_int( operand.name ) );
    }
    if ( set_reg.count( name ) ) {
        return name;
    }
    if ( is_tmp( name ) ) return map_tmp_register[name];
    if ( !varmap._table.count( name ) ) {
        rv_inst rvinst;
        rvinst.op = rvOPCODE::LUI;
        rvinst.rd = reg;
        rvinst.imm = "%hi(" + name + ")";
        vec_rv_inst.push_back( rvinst );
        rvinst.op = rvOPCODE::LW;
        rvinst.rd = reg;
        rvinst.rs1 = reg;
        rvinst.imm = "%lo(" + name + ")";
        vec_rv_inst.push_back( rvinst );
        return reg;
    }
    int offset = varmap._table[name];
    rv_inst rvinst;
    rvinst.op = rvOPCODE::LW;
    rvinst.rd = reg;
    rvinst.rs1 = "s0";
    rvinst.imm = to_string( offset );
    vec_rv_inst.push_back( rvinst );
    return reg;
}
void backend::Generator::do_move( Instruction inst ) {
    Operand des = inst.des;
    Operand op1 = inst.op1;
    int if_tmp = 0;
    if ( des.name.size() > 7 && des.name.substr( 0, 7 ) == "tmp_int" ) if_tmp = 1;
    int if_reg = ( set_reg.count( des.name ) != 0 );
    int if_liter = 0;
    if ( op1.type == Type::IntLiteral ) if_liter = 1;
    rv_inst rvinst;
    switch ( if_tmp | if_reg ) {
    case 1:
        switch ( if_liter ) {
        case 1:
            rvinst.op = rvOPCODE::LI;
            rvinst.rd = get_reg( des, "a5" );
            rvinst.imm = get_reg( op1, "a5" );
            vec_rv_inst.push_back( rvinst );
            break;
        case 0:
            rvinst.op = rvOPCODE::MOV;
            rvinst.rd = get_reg( des, "a5" );
            rvinst.rs1 = get_reg( op1, "a5" );
            vec_rv_inst.push_back( rvinst );
            break;
        default:
            break;
        }
        break;
    case 0:
        switch ( if_liter ) {
        case 1:
            rvinst.op = rvOPCODE::LI;
            rvinst.rd = "a5";
            rvinst.imm = get_reg( op1, "a5" );
            vec_rv_inst.push_back( rvinst );
            store_reg( des.name, rvinst.rd );
            break;
        case 0:
            // 当 mov 的 op1 是tmp_int, 可以优化
            rvinst.op = rvOPCODE::MOV;
            rvinst.rd = "a5";
            rvinst.rs1 = get_reg( op1, "a5" );
            vec_rv_inst.push_back( rvinst );
            store_reg( des.name, rvinst.rd );
            break;
        default:
            break;
        }
        break;
    default:
        break;
    }
}
void backend::Generator::store_reg( string name, string reg ) {
    if ( is_tmp( name ) ) {
        rv_inst rvinst;
        rvinst.op = rvOPCODE::MOV;
        rvinst.rd = map_tmp_register[name];
        rvinst.rs1 = reg;
        vec_rv_inst.push_back( rvinst );
        return;
    }
    if ( !varmap._table.count( name ) ) {
        rv_inst rvinst;
        rvinst.op = rvOPCODE::LUI;
        rvinst.rd = "a5";
        rvinst.imm = "%hi(" + name + ")";
        vec_rv_inst.push_back( rvinst );
        rvinst.op = rvOPCODE::SW;
        rvinst.rd = reg;
        rvinst.rs1 = "a5";
        rvinst.imm = "%lo(" + name + ")";
        vec_rv_inst.push_back( rvinst );
        return;
    }
    if ( varmap._table.count( name ) ) {
        int offset = varmap._table[name];
        rv_inst rvinst;
        rvinst.op = rvOPCODE::SW;
        rvinst.rs1 = "s0";
        rvinst.imm = to_string( offset );
        rvinst.rs2 = reg;
        vec_rv_inst.push_back( rvinst );
        return;
    }
}
void backend::Generator::alloc_tmp_register( const Function &function ) {
    for ( int i = 1; i <= 11; i++ ) {
        string reg_s = "s" + to_string( i );
        unuse.insert( reg_s );
    }
    map_tmp_register.clear();
    // if exist risv code that reg can not be same
    for ( int line = 0; line < function.InstVec.size(); line++ ) {
        if ( map_line_tmp_create.count( line ) ) {
            for ( string tmp_name : map_line_tmp_create[line] ) {
                if ( unuse.size() == 0 ) assert( 0 );
                string reg = *unuse.begin();
                unuse.erase( unuse.begin() );
                map_tmp_register[tmp_name] = reg;
            }
        }
        if ( map_line_tmp_destroy.count( line ) ) {
            for ( string tmp_name : map_line_tmp_destroy[line] ) {
                unuse.insert( map_tmp_register[tmp_name] );
            }
        }
    }
}
void backend::Generator::do_line_tmp( const Function &function ) {
    map_line_tmp_create.clear();
    map_line_tmp_destroy.clear();
    for ( pair<string, pair<int, int>> kv : map_seg ) {
        string key = kv.first;
        pair<int, int> value = kv.second;
        map_line_tmp_create[value.first].insert( key );
        map_line_tmp_destroy[value.second].insert( key );
    }
}
int backend::Generator::alloc_all_addr( const Function &function, string back ) {
    varmap._table.clear();
    int offset = -8;
    map_s_reg_to_addr.clear();
    map_s_reg_to_addr["s0"] = -4;
    map_s_reg_to_addr["ra"] = -8;
    offset -= 4; // for ra register
    // alloc s reg
    for ( pair<string, string> kv : map_tmp_register ) {
        string tmp = kv.first;
        string reg = kv.second;
        map_s_reg_to_addr[reg] = offset;
        offset -= 4;
    }
    // alloc params
    for ( Operand operand : function.ParameterList ) {
        string name = operand.name;
        if ( name.size() >= back.size() && name.substr( name.size() - back.size(), back.size() ) == back && varmap._table.count( name ) == 0 ) {
            varmap._table[name] = offset;
            offset -= 4;
        }
    }
    // alloc local
    for ( const Instruction   *inst : function.InstVec ) {
        if ( inst->op == Operator::alloc ) {
            if ( varmap._table.count( inst->des.name ) == 0 ) {
                int len = eval_int( inst->op1.name );
                offset -= ( len - 1 ) * 4;
                varmap._table[inst->des.name] = offset;
                offset -= 4;
            }
            continue;
        }
        for ( Operand operand : {
                    inst->op1, inst->op2, inst->des
                } ) {
            string name = operand.name;
            if ( name.size() >= back.size() && name.substr( name.size() - back.size(), back.size() ) == back && varmap._table.count( name ) == 0 ) {
                varmap._table[name] = offset;
                offset -= 4;
            }
        }
    }
    return offset;
}
void backend::Generator::cal_seg( const Function &function ) {
    map_seg.clear();
    int instdex = 0;
    for ( const Instruction   *inst : function.InstVec ) {
        for ( Operand operand : {
                    inst->op1, inst->op2, inst->des
                } ) {
            string name = operand.name;
            if ( name.size() > 7 && name.substr( 0, 7 ) == "tmp_int" && map_seg.count( name ) == 0 ) {
                map_seg[name].first = instdex;
            } else if ( name.size() > 7 && name.substr( 0, 7 ) == "tmp_int" ) {
                map_seg[name].second = instdex;
            }
        }
        instdex++;
    }
}
string rv::rv_inst::draw() const {
    string ans = "    " + rv::toString( this->op );
// #define DEBUG
#ifdef DEBUG
    ans += "    rd=" + this->rd;
    ans += "    rs1=" + this->rs1;
    ans += "    rs2=" + this->rs2;
    ans += "    imm=" + this->imm;
    return ans;
#endif
    switch ( this->op ) {
    case rvOPCODE::SW:
        ans +=  "	" + this->rs2 + "," + this->imm + "(" + this->rs1 + ")";
        break;
    case rvOPCODE::ADDI:
        ans +=  "	" + this->rd + "," + this->rs1 + "," + this->imm;
        break;
    case rvOPCODE::LI:
        ans +=  "	" + this->rd + "," + this->imm;
        break;
    case rvOPCODE::LW:
        ans +=  "	" + this->rd + "," + this->imm + "(" + this->rs1 + ")";
        break;
    case rvOPCODE::LUI:
        ans += "    " + this->rd + "," + this->imm;
        break;
    case rvOPCODE::MOV:
        ans += "    " + this->rd + "," + this->rs1;
        break;
    case rvOPCODE::J:
        ans += "    ra";
        break;
    case rvOPCODE::REM:
    case rvOPCODE::SUB:
    case rvOPCODE::DIV:
    case rvOPCODE::ADD:
    case rvOPCODE::SLL:
    case rvOPCODE::MUL:
        ans += "    " + this->rd + "," + this->rs1 + "," + this->rs2;
        break;
    case rvOPCODE::SLLI:
        ans += "    " + this->rd + "," + this->rs1 + "," + this->imm;
        break;
    case rvOPCODE::FUCKIN:
    case rvOPCODE::FUCKOUT:
        break;
    default:
        assert( 0 );
        break;
    }
    return ans;
}
string rv::toString( rvOPCODE r ) {
    switch ( r ) {
    case rvOPCODE::ADD:
        return "add";
        break;
    case rvOPCODE::ADDI:
        return "addi";
        break;
    case rvOPCODE::LW:
        return "lw";
        break;
    case rvOPCODE::SW:
        return "sw";
        break;
    case rvOPCODE::MOV:
        return "mv";
        break;
    case rvOPCODE::LI:
        return "li";
        break;
    case rvOPCODE::LUI:
        return "lui";
        break;
    case rvOPCODE::J:
        return "jr";
        break;
    case rvOPCODE::MUL:
        return "mul";
        break;
    case rvOPCODE::SLL:
        return "sll";
        break;
    case rvOPCODE::SLLI:
        return "slli";
        break;
    case rvOPCODE::FUCKIN:
        return "in";
        break;
    case rvOPCODE::FUCKOUT:
        return "out";
        break;
    case rvOPCODE::DIV:
        return "div";
        break;
    case rvOPCODE::SUB:
        return "sub";
        break;
    case rvOPCODE::REM:
        return "rem";
        break;
    default:
        cout << int( r ) << endl;
        assert( 0 );
        break;
    }
}
