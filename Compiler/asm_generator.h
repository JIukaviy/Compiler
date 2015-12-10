#pragma once

#include "tokens.h"
#include "parser_base_node.h"
#include "var.h"
#include <memory>
#include <vector>
#include <iostream>
#include <algorithm>

using namespace std;

void asm_generator_init();

enum ASM_REGISTER {
#define register_register(reg_name, parent_reg_name, size) AR_##reg_name,
#include "asm_registers.h"
#undef register_register
};

enum ASM_UN_OPERATOR {
#define register_un_op(op_name, op_incode_name) AUO_##op_name,
#include "asm_un_op.h"
#undef register_un_op
};

enum ASM_BIN_OPERATOR {
#define register_bin_op(op_name, op_incode_name) ABO_##op_name,
#include "asm_bin_op.h"
#undef register_bin_op
};

enum ASM_MEM_TYPE {
#define register_mem_type(mt_name, size) AMT_##mt_name,
#include "asm_mem_type.h"
#undef register_mem_type
};

enum ASM_OPERAND_PREFIX {
	AOP_NONE,
	AOP_OFFSET,
#define register_mem_type(mt_name, size) AOP_##mt_name##_PTR,
#include "asm_mem_type.h"
#undef register_mem_type
};

class asm_t {
public:
	virtual void print(ostream& os) {};
};

class asm_cmd_t : public asm_t {
	
};

class asm_operator_t : public asm_cmd_t {

};

class asm_str_cmd_t : public asm_cmd_t {
	string str;
public:
	asm_str_cmd_t(string str);
	void print(ostream& os) override;
};

class asm_bin_oprtr_t : public asm_operator_t {
	ASM_BIN_OPERATOR op;
	asm_oprnd_ptr left_operand;
	asm_oprnd_ptr right_operand;
public:
	asm_bin_oprtr_t(ASM_BIN_OPERATOR op, asm_oprnd_ptr left_operand, asm_oprnd_ptr right_operand);
	asm_bin_oprtr_t(ASM_BIN_OPERATOR op);
	void print(ostream& os) override;
};

class asm_un_oprtr_t : public asm_operator_t {
	ASM_UN_OPERATOR op;
	asm_oprnd_ptr operand;
public:
	asm_un_oprtr_t(ASM_UN_OPERATOR op, asm_oprnd_ptr operand);
	asm_un_oprtr_t(ASM_UN_OPERATOR op);
	void print(ostream& os) override;
};

class asm_operand_t : public asm_t {
public:
	//void print(ostream& os) override;
};

class asm_reg_oprnd_t : public asm_operand_t {
	ASM_REGISTER reg;
public:
	asm_reg_oprnd_t(ASM_REGISTER reg);
	void print(ostream& os) override;
};

class asm_ident_operand_t : public asm_operand_t {
	string name;
public:
	asm_ident_operand_t(string name);
	void print(ostream& os) override;
};

class asm_addr_oprnd_t : public asm_operand_t {

};

class asm_addr_ident_oprnd_t : public asm_addr_oprnd_t {
protected:
	string name;
public:
	asm_addr_ident_oprnd_t(string name);
	void print(ostream& os);
};

class asm_addr_reg_oprnd_t : public asm_addr_oprnd_t {
protected:
	ASM_REGISTER reg;
	int offset;
public:
	asm_addr_reg_oprnd_t(ASM_REGISTER reg, int offset);
	void print(ostream& os);
};

class asm_deref_oprnd_t : public asm_operand_t {
protected:
	int offset;
	int scale;
public:
	asm_deref_oprnd_t(int offset, int scale);
};

class asm_deref_ident_oprnd_t : public asm_deref_oprnd_t {
protected:
	string name;
public:
	asm_deref_ident_oprnd_t(string name, int offset = 0, int scale = 0);
	void print(ostream& os);
};

class asm_deref_reg_oprnd_t : public asm_deref_oprnd_t {
protected:
	ASM_MEM_TYPE mtype;
	ASM_REGISTER reg;
public:
	asm_deref_reg_oprnd_t(ASM_MEM_TYPE mtype, ASM_REGISTER reg, int offset = 0, int scale = 0);
	void print(ostream& os);
};

class asm_const_oprnd_t : public asm_operand_t {
	var_ptr var;
public:
	asm_const_oprnd_t(var_ptr var);
	void print(ostream& os) override;
};

class asm_global_var_t {
	string name;
	ASM_MEM_TYPE type;
	vector<var_ptr> init_list;
	asm_cmd_list_ptr init_commands;
	int dup;
public:
	asm_global_var_t(string name, ASM_MEM_TYPE type, vector<var_ptr> init_list, int dup = 0);
	asm_global_var_t(string name, ASM_MEM_TYPE type, asm_cmd_list_ptr init_commands, int dup = 0);
	asm_global_var_t(string name, ASM_MEM_TYPE type, int dup = 0);
	void print_alloc(ostream& os);
	void print_init(ostream& os);
};

class asm_function_t : public asm_t {
	string name;
	asm_cmd_list_ptr cmd_list;
public:
	asm_function_t(string name, asm_cmd_list_ptr cmd_list);
	void print(ostream& os) override;
};

class asm_cmd_list_t : public asm_t {
protected:
	vector<asm_cmd_ptr> commands;
public:
#define register_un_op(op_name, op_incode_name) \
	void op_incode_name(); \
	void op_incode_name(ASM_REGISTER operand); \
	void op_incode_name(ASM_REGISTER operand, int operand_size); \
	void op_incode_name(var_ptr operand); \
	void op_incode_name(asm_oprnd_ptr operand); \
	void op_incode_name(string operand); \
	void op_incode_name##_addr(string operand); \
	void op_incode_name##_deref(ASM_REGISTER operand, ASM_MEM_TYPE mtype, int offset = 0, int scale = 0); \
	void op_incode_name##_deref(ASM_REGISTER operand, int operand_size, int offset = 0, int scale = 0);

#include "asm_un_op.h"
#undef register_un_op
#define register_bin_op(op_name, op_incode_name) \
	void op_incode_name(); \
	void op_incode_name(ASM_REGISTER left, ASM_REGISTER right); \
	void op_incode_name(ASM_REGISTER left, ASM_REGISTER right, int operand_size); \
	void op_incode_name(ASM_REGISTER left, var_ptr right); \
	void op_incode_name(ASM_REGISTER left, string right); \
	void op_incode_name##_raddr(ASM_REGISTER left, string right); \
	void op_incode_name(ASM_REGISTER left, string right, int offset, int scale = 0); \
	void op_incode_name(string left, ASM_REGISTER right); \
	void op_incode_name(string left, var_ptr right); \
	void op_incode_name##_lderef(ASM_REGISTER left, ASM_REGISTER right, ASM_MEM_TYPE mtype, int offset = 0, int scale = 0); \
	void op_incode_name##_rderef(ASM_REGISTER left, ASM_REGISTER right, ASM_MEM_TYPE mtype, int offset = 0, int scale = 0); \
	void op_incode_name##_lderef(ASM_REGISTER left, ASM_REGISTER right, int operand_size, int offset = 0, int scale = 0); \
	void op_incode_name##_rderef(ASM_REGISTER left, ASM_REGISTER right, int operand_size, int offset = 0, int scale = 0);
#include "asm_bin_op.h"
#undef register_bin_op

	void _push_un_oprtr(ASM_UN_OPERATOR op);
	void _push_un_oprtr(ASM_UN_OPERATOR op, asm_oprnd_ptr operand);
	void _push_un_oprtr(ASM_UN_OPERATOR op, ASM_REGISTER operand);
	void _push_un_oprtr(ASM_UN_OPERATOR op, ASM_REGISTER operand, int operand_size);
	void _push_un_oprtr(ASM_UN_OPERATOR op, var_ptr operand);
	void _push_un_oprtr(ASM_UN_OPERATOR op, string operand);
	void _push_un_oprtr_addr(ASM_UN_OPERATOR op, string operand);
	void _push_un_oprtr_deref(ASM_UN_OPERATOR op, string operand, int offset, int scale = 0);
	void _push_un_oprtr_deref(ASM_UN_OPERATOR op, ASM_REGISTER operand, ASM_MEM_TYPE mtype, int offset = 0, int scale = 0);
	void _push_un_oprtr_deref(ASM_UN_OPERATOR op, ASM_REGISTER operand, int operand_size, int offset = 0, int scale = 0);

	void _push_bin_oprtr(ASM_BIN_OPERATOR op);
	void _push_bin_oprtr(ASM_BIN_OPERATOR op, asm_oprnd_ptr left, asm_oprnd_ptr right);
	void _push_bin_oprtr(ASM_BIN_OPERATOR op, ASM_REGISTER left, ASM_REGISTER right);
	void _push_bin_oprtr(ASM_BIN_OPERATOR op, ASM_REGISTER left, ASM_REGISTER right, int operand_size);
	void _push_bin_oprtr(ASM_BIN_OPERATOR op, ASM_REGISTER left, var_ptr right);
	void _push_bin_oprtr(ASM_BIN_OPERATOR op, ASM_REGISTER left, string right);
	void _push_bin_oprtr_raddr(ASM_BIN_OPERATOR op, ASM_REGISTER left, string right);
	void _push_bin_oprtr(ASM_BIN_OPERATOR op, ASM_REGISTER left, string right, int offset, int scale = 0);
	void _push_bin_oprtr(ASM_BIN_OPERATOR op, string left, ASM_REGISTER right);
	void _push_bin_oprtr(ASM_BIN_OPERATOR op, string left, var_ptr right);
	void _push_bin_oprtr_lderef(ASM_BIN_OPERATOR op, ASM_REGISTER left, ASM_REGISTER right, ASM_MEM_TYPE mtype, int offset = 0, int scale = 0);
	void _push_bin_oprtr_lderef(ASM_BIN_OPERATOR op, ASM_REGISTER left, var_ptr right, ASM_MEM_TYPE mtype, int offset = 0, int scale = 0);
	void _push_bin_oprtr_rderef(ASM_BIN_OPERATOR op, ASM_REGISTER left, ASM_REGISTER right, ASM_MEM_TYPE mtype, int offset = 0, int scale = 0);
	void _push_bin_oprtr_lderef(ASM_BIN_OPERATOR op, ASM_REGISTER left, ASM_REGISTER right, int operand_size, int offset = 0, int scale = 0);
	void _push_bin_oprtr_rderef(ASM_BIN_OPERATOR op, ASM_REGISTER left, ASM_REGISTER right, int operand_size, int offset = 0, int scale = 0);

	void _push_copy_cmd(ASM_REGISTER src_reg, ASM_REGISTER dest_reg, int size, int src_offset = 0, int dst_offset = 0, bool copy_to_stack = false);
	void _push_copy_to_stack_cmd(ASM_REGISTER src_reg, int size, int src_offset = 0);
	void _push_alloc_cmd(int size);
	void _push_free_cmd(int size);
	void _push_int_to_double_cmd(ASM_REGISTER src_reg);
	void _push_double_to_int_cmd(ASM_REGISTER dst_reg);
	void _push_double_cmd(var_ptr var);

	void _push_str(string str);
	void print(ostream& os) override;
};

class asm_generator_t : public asm_t {
	void print_header(ostream& os);
	vector <shared_ptr<asm_global_var_t>> global_vars;
	vector <shared_ptr<asm_function_t>> functions;
	asm_cmd_list_ptr main_cmd_list;
public:
	void add_global_var(shared_ptr<asm_global_var_t> var);
	void add_global_var(string name, ASM_MEM_TYPE mem_type, int dup = 0);
	void add_global_var(string name, ASM_MEM_TYPE mem_type, asm_cmd_list_ptr init_cmd_list, int dup = 0);
	void add_function(string name, asm_cmd_list_ptr cmd_list);
	void set_main_cmd_list(asm_cmd_list_ptr cmd_list);
	void print(ostream& os);
	static int alignment(int size);
	static int align_size(int size);
	static ASM_REGISTER reg_by_size(ASM_REGISTER reg, int size);
	static ASM_REGISTER reg_by_mtype(ASM_REGISTER reg, ASM_MEM_TYPE mtype);
	static int size_of(ASM_REGISTER reg);
	static int size_of(ASM_MEM_TYPE mtype);
	static ASM_MEM_TYPE mtype_by_size(int size);
	static ASM_MEM_TYPE mtype_by_reg(ASM_REGISTER);
};