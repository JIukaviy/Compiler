#include "asm_generator.h"
#include <assert.h>

//------------------------------ASM_REGISTER_OPERAND-------------------------------------------

asm_reg_oprnd_t::asm_reg_oprnd_t(ASM_REGISTER reg) : reg(reg) {}

void asm_reg_oprnd_t::print(ostream& os) {
	os << (
		reg == AR_EAX ? "eax" :
		reg == AR_EBX ? "ebx" :
		reg == AR_ECX ? "ecx" :
		reg == AR_EDX ? "edx" :
		reg == AR_EBP ? "ebp" :
		reg == AR_ESP ? "esp" : (assert(false), nullptr));
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
	os << (
		op == AUO_PUSH ? "push" :
		op == AUO_POP ? "pop" : 
		op == AUO_DIV ? "div" : (assert(false), nullptr));
	os << ' ';
	operand->print(os);
}

//------------------------------ASM_BINARY_OPERATOR-------------------------------------------

asm_bin_oprtr_t::asm_bin_oprtr_t(ASM_BIN_OPERATOR op, asm_oprnd_ptr left_operand, asm_oprnd_ptr right_operand) : 
	op(op), left_operand(left_operand), right_operand(right_operand) {}

void asm_bin_oprtr_t::print(ostream& os) {
	os << (
		op == ABO_ADD ? "add" :
		op == ABO_SUB ? "sub" :
		op == ABO_IMUL ? "imul" :
		op == ABO_XOR? "xor" : (assert(false), nullptr));
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

