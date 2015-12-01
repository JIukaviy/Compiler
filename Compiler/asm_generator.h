#pragma once

#include "tokens.h"
#include <memory>
#include <vector>
#include <ostream>

using namespace std;

class asm_cmd_t;
class asm_cmd_list_t;
class asm_operand_t;

typedef shared_ptr<asm_cmd_t> asm_cmd_ptr;
typedef shared_ptr<asm_operand_t> asm_oprnd_ptr;
typedef shared_ptr<asm_cmd_list_t> asm_cmd_list_ptr;

enum ASM_REGISTER {
	AR_EAX,
	AR_EBX,
	AR_ECX,
	AR_EDX,
	AR_EBP,
	AR_ESP
};

enum ASM_UN_OPERATOR {
	AUO_PUSH,
	AUO_POP,
	AUO_DIV,
};

enum ASM_BIN_OPERATOR {
	ABO_ADD,
	ABO_SUB,
	ABO_IMUL,
	ABO_XOR
};

class asm_t {
public:
	virtual void print(ostream& os) {};
};

class asm_cmd_t : public asm_t {

};

class asm_operator_t : public asm_cmd_t {

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

class asm_global_vars : public asm_t {

};

class asm_local_vars : public asm_t {

};

class asm_cmd_list_t : public asm_t {
protected:
	vector<asm_cmd_ptr> commands;
public:
	void add(ASM_REGISTER left, ASM_REGISTER right);
	void sub(ASM_REGISTER left, ASM_REGISTER right);
	void imul(ASM_REGISTER left, ASM_REGISTER right);
	void xor_(ASM_REGISTER left, ASM_REGISTER right);
	void div(ASM_REGISTER reg);
	void push(ASM_REGISTER reg);
	void push(token_ptr constant);
	void pop(ASM_REGISTER reg);

	void _push_un_oprtr(ASM_UN_OPERATOR op, asm_oprnd_ptr operand);
	void _push_un_oprtr(ASM_UN_OPERATOR op, ASM_REGISTER operand);
	void _push_un_oprtr(ASM_UN_OPERATOR op, token_ptr operand);

	void _push_bin_oprtr(ASM_BIN_OPERATOR op, asm_oprnd_ptr left, asm_oprnd_ptr right);
	void _push_bin_oprtr(ASM_BIN_OPERATOR op, ASM_REGISTER left, ASM_REGISTER right);
	void print(ostream& os) override;
};