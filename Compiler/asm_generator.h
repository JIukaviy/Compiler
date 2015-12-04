#pragma once

#include "tokens.h"
#include "parser_base_node.h"
#include <memory>
#include <vector>
#include <iostream>
#include <algorithm>

using namespace std;

void asm_generator_init();

enum ASM_REGISTER {
#define register_register(reg_name) AR_##reg_name,
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
#define register_mem_type(mt_name) AMT_##mt_name,
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

class asm_str_cmd_t : asm_cmd_t {
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
	void print(ostream& os) override;
};

class asm_un_oprtr_t : public asm_operator_t {
	ASM_UN_OPERATOR op;
	asm_oprnd_ptr operand;
public:
	asm_un_oprtr_t(ASM_UN_OPERATOR op, asm_oprnd_ptr operand);
	void print(ostream& os) override;
};

class asm_operand_t : public asm_t {
public:
	
};

class asm_reg_oprnd_t : public asm_operand_t {
	ASM_REGISTER reg;
public:
	asm_reg_oprnd_t(ASM_REGISTER reg);
	void print(ostream& os) override;
};

class asm_addr_oprnd_t : public asm_operand_t {

};

class asm_const_oprnd_t : public asm_operand_t {
	token_ptr constant;
public:
	asm_const_oprnd_t(token_ptr constant);
	void print(ostream& os) override;
};

class asm_int_oprnd_t : public asm_operand_t {
	int val;
public:
	asm_int_oprnd_t(int val);
	void print(ostream& os) override;
};

class asm_global_vars_t {
public:
	asm_global_vars_t();
};

class asm_functions_t {
	
};

class asm_cmd_list_t : public asm_t {
protected:
	vector<asm_cmd_ptr> commands;
public:
#define register_bin_op(op_name, op_incode_name) \
	void op_incode_name(ASM_REGISTER left, ASM_REGISTER right); \
	void op_incode_name(ASM_REGISTER left, int right); \
	void op_incode_name(ASM_REGISTER left, token_ptr right);
#include "asm_bin_op.h"
#undef register_bin_op
#define register_un_op(op_name, op_incode_name) \
	void op_incode_name(ASM_REGISTER operand); \
	void op_incode_name(int operand); \
	void op_incode_name(token_ptr operand);
#include "asm_un_op.h"
#undef register_un_op

	void _push_un_oprtr(ASM_UN_OPERATOR op, asm_oprnd_ptr operand);
	void _push_un_oprtr(ASM_UN_OPERATOR op, ASM_REGISTER operand);
	void _push_un_oprtr(ASM_UN_OPERATOR op, token_ptr operand);
	void _push_un_oprtr(ASM_UN_OPERATOR op, int operand);

	void _push_bin_oprtr(ASM_BIN_OPERATOR op, asm_oprnd_ptr left, asm_oprnd_ptr right);
	void _push_bin_oprtr(ASM_BIN_OPERATOR op, ASM_REGISTER left, ASM_REGISTER right);
	void _push_bin_oprtr(ASM_BIN_OPERATOR op, ASM_REGISTER left, token_ptr right);
	void _push_bin_oprtr(ASM_BIN_OPERATOR op, ASM_REGISTER left, int right);

	void _push_str(string str);
	void print(ostream& os) override;
};

class asm_generator_t : public asm_t {
	asm_cmd_list_ptr cmd_list; 
	void print_header(ostream& os);
public:
	asm_generator_t(asm_cmd_list_ptr cmd_list);
	asm_cmd_list_ptr get_cmd_list();
	//void add_local_var(sym_ptr var);
	void print(ostream& os);
};