#include "asm_generator.h"
#include <assert.h>
#include <map>

map<ASM_REGISTER, string> asm_reg_to_str;
map<ASM_BIN_OPERATOR, string> asm_bin_op_to_str;
map<ASM_UN_OPERATOR, string> asm_un_op_to_str;

//------------------------------ASM_REGISTER_OPERAND-------------------------------------------

asm_reg_oprnd_t::asm_reg_oprnd_t(ASM_REGISTER reg) : reg(reg) {}

void asm_reg_oprnd_t::print(ostream& os) {
	os << asm_reg_to_str.at(reg);
}

//------------------------------ASM_CONSTANT_OPERAND-------------------------------------------

asm_const_oprnd_t::asm_const_oprnd_t(token_ptr constant) : constant(constant) {}

void asm_const_oprnd_t::print(ostream& os) {
	constant->short_print(os);
}

//------------------------------ASM_OPERATORS-------------------------------------------

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

void asm_cmd_list_t::add(ASM_REGISTER left, ASM_REGISTER right) {
	_push_bin_oprtr(ABO_ADD, left, right);
}

void asm_cmd_list_t::sub(ASM_REGISTER left, ASM_REGISTER right) {
	_push_bin_oprtr(ABO_SUB, left, right);
}

void asm_cmd_list_t::imul(ASM_REGISTER left, ASM_REGISTER right) {
	_push_bin_oprtr(ABO_IMUL, left, right);
}

void asm_cmd_list_t::xor_(ASM_REGISTER left, ASM_REGISTER right) {
	_push_bin_oprtr(ABO_XOR, left, right);
}

void asm_cmd_list_t::div(ASM_REGISTER reg) {
	_push_un_oprtr(AUO_DIV, reg);
}

void asm_cmd_list_t::push(ASM_REGISTER operand) {
	_push_un_oprtr(AUO_PUSH, operand);
}

void asm_cmd_list_t::push(token_ptr constant) {
	_push_un_oprtr(AUO_PUSH, constant);
}

void asm_cmd_list_t::pop(ASM_REGISTER operand) {
	_push_un_oprtr(AUO_POP, operand);
}

void asm_cmd_list_t::_push_un_oprtr(ASM_UN_OPERATOR op, asm_oprnd_ptr operand) {
	commands.push_back(asm_cmd_ptr(new asm_un_oprtr_t(op, operand)));
}

void asm_cmd_list_t::_push_un_oprtr(ASM_UN_OPERATOR op, ASM_REGISTER operand) {
	_push_un_oprtr(op, asm_oprnd_ptr(new asm_reg_oprnd_t(operand)));
}

void asm_cmd_list_t::_push_un_oprtr(ASM_UN_OPERATOR op, token_ptr operand) {
	_push_un_oprtr(op, asm_oprnd_ptr(new asm_const_oprnd_t(operand)));
}

void asm_cmd_list_t::_push_bin_oprtr(ASM_BIN_OPERATOR op, asm_oprnd_ptr left, asm_oprnd_ptr right) {
	commands.push_back(asm_cmd_ptr(new asm_bin_oprtr_t(op, left, right)));
}

void asm_cmd_list_t::_push_bin_oprtr(ASM_BIN_OPERATOR op, ASM_REGISTER left, ASM_REGISTER right) {
	_push_bin_oprtr(op, asm_oprnd_ptr(new asm_reg_oprnd_t(left)), asm_oprnd_ptr(new asm_reg_oprnd_t(right)));
}

void asm_cmd_list_t::print(ostream& os) {
	for each (auto cmd in commands) {
		cmd->print(os);
		os << endl;
	}
}

//------------------------------ASM_COMANNDS_LIST-------------------------------------------

asm_generator_t::asm_generator_t(asm_cmd_list_ptr cmd_list) : cmd_list(cmd_list) {}

void asm_generator_t::print(ostream& os) {
	os <<
		".386" << endl <<
		".model flat, C" << endl <<
		"option casemap : none" << endl <<
		"include \\masm32\\include\\msvcrt.inc" << endl <<
		"includelib \\masm32\\lib\\msvcrt.lib" << endl;

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
#define register_un_op(op_name) asm_un_op_to_str[AUO_##op_name] = lower_case(#op_name);
#define register_bin_op(op_name) asm_bin_op_to_str[ABO_##op_name] = lower_case(#op_name);
#include "asm_registers.h"
#include "asm_un_op.h"
#include "asm_bin_op.h"
#undef register_register
#undef register_un_op
#undef register_bin_op
}
