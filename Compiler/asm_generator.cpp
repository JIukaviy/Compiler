#include "asm_generator.h"
#include <assert.h>
#include <map>

map<ASM_REGISTER, string> asm_reg_to_str;
map<ASM_BIN_OPERATOR, string> asm_bin_op_to_str;
map<ASM_UN_OPERATOR, string> asm_un_op_to_str;
map<ASM_MEM_TYPE, string> asm_mt_to_str;

//------------------------------ASM_REGISTER_OPERAND-------------------------------------------

asm_reg_oprnd_t::asm_reg_oprnd_t(ASM_REGISTER reg) : reg(reg) {}

void asm_reg_oprnd_t::print(ostream& os) {
	os << asm_reg_to_str.at(reg);
}

//------------------------------ASM_CONSTANT_OPERAND-------------------------------------------

asm_const_oprnd_t::asm_const_oprnd_t(var_ptr var) : var(var) {}

void asm_const_oprnd_t::print(ostream& os) {
	var->print(os);
}

//------------------------------ASM_COMMANDS-------------------------------------------

//------------------------------ASM_STR_COMMAND-------------------------------------------

asm_str_cmd_t::asm_str_cmd_t(string str) : str(str) {}

void asm_str_cmd_t::print(ostream& os) {
	os << str;
}

//------------------------------ASM_UNARY_OPERATOR-------------------------------------------

asm_un_oprtr_t::asm_un_oprtr_t(ASM_UN_OPERATOR op, asm_oprnd_ptr operand) : op(op), operand(operand) {}

void asm_un_oprtr_t::print(ostream& os) {
	os << asm_un_op_to_str.at(op);
	os << ' ';
	operand->print(os);
}

//------------------------------ASM_BINARY_OPERATOR-------------------------------------------

asm_bin_oprtr_t::asm_bin_oprtr_t(ASM_BIN_OPERATOR op, asm_oprnd_ptr left_operand, asm_oprnd_ptr right_operand) : 
	op(op), left_operand(left_operand), right_operand(right_operand) {}

void asm_bin_oprtr_t::print(ostream& os) {
	os << asm_bin_op_to_str.at(op);
	os << ' ';
	left_operand->print(os);
	os << ", ";
	right_operand->print(os);
}


//------------------------------ASM_COMANNDS_LIST-------------------------------------------

#define register_bin_op(op_name, op_incode_name) \
	void asm_cmd_list_t::op_incode_name(ASM_REGISTER left, ASM_REGISTER right) { \
		_push_bin_oprtr(ABO_##op_name, left, right); \
	} \
	void asm_cmd_list_t::op_incode_name(ASM_REGISTER left, var_ptr right) { \
		_push_bin_oprtr(ABO_##op_name, left, right); \
	}
#include "asm_bin_op.h"
#undef register_bin_op
#define register_un_op(op_name, op_incode_name) \
	void asm_cmd_list_t::op_incode_name(ASM_REGISTER operand) { \
		_push_un_oprtr(AUO_##op_name, operand); \
	} \
	void asm_cmd_list_t::op_incode_name(var_ptr operand) { \
		_push_un_oprtr(AUO_##op_name, operand); \
	}
#include "asm_un_op.h"
#undef register_un_op

void asm_cmd_list_t::_push_un_oprtr(ASM_UN_OPERATOR op, asm_oprnd_ptr operand) {
	commands.push_back(asm_cmd_ptr(new asm_un_oprtr_t(op, operand)));
}

void asm_cmd_list_t::_push_un_oprtr(ASM_UN_OPERATOR op, ASM_REGISTER operand) {
	_push_un_oprtr(op, asm_oprnd_ptr(new asm_reg_oprnd_t(operand)));
}

void asm_cmd_list_t::_push_un_oprtr(ASM_UN_OPERATOR op, var_ptr operand) {
	_push_un_oprtr(op, asm_oprnd_ptr(new asm_const_oprnd_t(operand)));
}

void asm_cmd_list_t::_push_bin_oprtr(ASM_BIN_OPERATOR op, asm_oprnd_ptr left, asm_oprnd_ptr right) {
	commands.push_back(asm_cmd_ptr(new asm_bin_oprtr_t(op, left, right)));
}

void asm_cmd_list_t::_push_bin_oprtr(ASM_BIN_OPERATOR op, ASM_REGISTER left, ASM_REGISTER right) {
	_push_bin_oprtr(op, asm_oprnd_ptr(new asm_reg_oprnd_t(left)), asm_oprnd_ptr(new asm_reg_oprnd_t(right)));
}

void asm_cmd_list_t::_push_bin_oprtr(ASM_BIN_OPERATOR op, ASM_REGISTER left, var_ptr right) {
	_push_bin_oprtr(op, asm_oprnd_ptr(new asm_reg_oprnd_t(left)), asm_oprnd_ptr(new asm_const_oprnd_t(right)));
}

void asm_cmd_list_t::_push_str(string str) {
	commands.push_back(asm_cmd_ptr(new asm_str_cmd_t(str)));
}

void asm_cmd_list_t::print(ostream& os) {
	for each (auto cmd in commands) {
		cmd->print(os);
		os << endl;
	}
}

//------------------------------ASM_COMANNDS_LIST-------------------------------------------

asm_generator_t::asm_generator_t(asm_cmd_list_ptr cmd_list) : cmd_list(cmd_list) {}

asm_cmd_list_ptr asm_generator_t::get_cmd_list() {
	return cmd_list;
}

void asm_generator_t::print_header(ostream& os) {
	os <<
		".386" << endl <<
		".model flat, C" << endl <<
		"option casemap : none" << endl <<
		"include \\masm32\\include\\msvcrt.inc" << endl <<
		"includelib \\masm32\\lib\\msvcrt.lib" << endl;
}

void asm_generator_t::print(ostream& os) {
	print_header(os);

	os <<
		".DATA" << endl <<
		"printf_format_str BYTE \"%d\", 10, 13, 0" << endl;

	os << ".CODE" << endl;

	os << "main:" << endl;
	cmd_list->print(os);
	os <<
		"pop eax" << endl <<
		"invoke crt_printf, ADDR printf_format_str, eax" << endl <<
		"invoke crt__exit, 0" << endl;
	os << "end main" << endl;
}

static string lower_case(char cstr[]) {
	string res(cstr);
	transform(res.begin(), res.end(), res.begin(), tolower);
	return res;
}

void asm_generator_init() {
#define register_register(reg_name) asm_reg_to_str[AR_##reg_name] = lower_case(#reg_name);
#define register_un_op(op_name, op_incode_name) asm_un_op_to_str[AUO_##op_name] = lower_case(#op_name);
#define register_bin_op(op_name, op_incode_name) asm_bin_op_to_str[ABO_##op_name] = lower_case(#op_name);
#define register_mem_type(mt_name) asm_mt_to_str[AMT_##mt_name] = #mt_name;
#include "asm_registers.h"
#include "asm_un_op.h"
#include "asm_bin_op.h"
#include "asm_mem_type.h"
#undef register_register
#undef register_un_op
#undef register_bin_op
#undef register_mem_type
}
