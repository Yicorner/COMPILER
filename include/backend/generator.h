#ifndef GENERARATOR_H
#define GENERARATOR_H

#include "backend/rv_def.h"
#include "ir/ir.h"
#include "backend/rv_inst_impl.h"

#include<map>
#include<string>
#include<vector>
#include<fstream>
#include<set>
using std::string;
using ir::Instruction;
namespace backend {
using rv::rvOPCODE;
// it is a map bewteen variable and its mem addr, the mem addr of a local variable can be identified by ($sp + off)
struct stackVarMap {
    std::map<string, int> _table;
    std::map<string, int> _ptrTable;
    /**
     * @brief find the addr of a ir::Operand
     * @return the offset
    */
    int find_operand( ir::Operand );

    /**
     * @brief add a ir::Operand into current map, alloc space for this variable in memory
     * @param[in] size: the space needed(in byte)
     * @return the offset
    */
    int add_operand( ir::Operand, uint32_t size = 4 );
};


struct Generator {
    const ir::Program &program;         // the program to gen
    std::ofstream &fout;                 // output file
    stackVarMap varmap;
    std::set<std::string> sets;
    std::map<int, std::set<std::string>> map_line_tmp_create;
    std::map<int, std::set<std::string>> map_line_tmp_destroy;
    std::map<std::string, std::pair<int, int>> map_seg;
    std::map<std::string, std::string> map_tmp_register;
    std::set<std::string> unuse;
    std::map<std::string, int> map_s_reg_to_addr;
    std::vector<rv::rv_inst> vec_rv_inst;
    std::set<std::string> set_reg;
    std::set<std::string> libfunc;
    std::map<int, std::string> map_line_label;
    Generator( ir::Program &, std::ofstream & );

    // reg allocate api
    // rv::rvREG getRd(ir::Operand);
    // rv::rvFREG fgetRd(ir::Operand);
    // rv::rvREG getRs1(ir::Operand);
    // rv::rvREG getRs2(ir::Operand);
    // rv::rvFREG fgetRs1(ir::Operand);
    // rv::rvFREG fgetRs2(ir::Operand);

    // generate wrapper function
    void gen();
    void gen_func( const ir::Function &, string back );
    void gen_instr( const ir::Instruction & );
    void gen_global_func( const ir::Function & );
    std::string find_size( const std::string & );
    int alloc_all_addr( const ir::Function &, string back );
    void do_line_tmp( const ir::Function & );
    void alloc_tmp_register( const ir::Function & );
    void cal_seg( const ir::Function & );
    void gen_addi( string, string, string );
    void gen_sw( string, int, int );
    int eval_int( std::string );
    string get_reg( ir::Operand, string );
    void store_reg( string, string );
    bool is_tmp( std::string );
    void do_add( Instruction, rvOPCODE );
    void do_return( Instruction, int );
    void do_move( Instruction );
    void gen_lw( string );
    void add_store_s0();
    void do_jr_ra();
    void do_store( Instruction );
    void do_load( Instruction );
    void do_mul( Instruction, rvOPCODE );
    void get_global_addr( string arr_name, string reg );
    void do_local_addr( Instruction );
    void do_global_addr( Instruction );
    string get_reg_no_liter( ir::Operand, string );
    void add_fuck_in();
    void add_fuck_out();
    rvOPCODE shift_to_rvopcode( ir::Operator op );
    void do_local_but_ptr( Instruction );
    void do_call( ir::CallInst );
    void get_reg_assign( ir::Operand, string );
    void get_ptr_addr_a5( ir::Operand );
    void call_func_name( string );
    void move_func_result( string );
    void alloc_label_function( const ir::Function & );
    void do_goto( Instruction, int );
    void do_lss( Instruction );
    void do_leq( Instruction );
    void do_gtr( Instruction );
    void do_geq( Instruction );
    void do_eq( Instruction );
    void do_neq( Instruction );
    void do_not( Instruction );
    void do_and( Instruction );
    void do_or( Instruction );
};



} // namespace backend


#endif