#pragma once

#include <memory>
#include <vector>
#include <ostream>

using namespace std;

class asm_cmd_t;
class asm_cmd_list_t;

typedef shared_ptr<asm_cmd_t> asm_cmd_ptr;
typedef shared_ptr<asm_cmd_list_t> asm_cmd_list_ptr;

enum ASM_REGISTERS {
	AR_EAX,
	AR_EBX,
	AR_ECX,
	AR_EDX,
	AR_EBP,
	AR_ESP
};

enum ASM_UN_OPERATORS {
	AUO_PUSH,
	AUO_POP
};

enum ASM_BIN_OPERATORS {
	ABO_PUSH,
	ABO_POP,
	ABO_ADD,
	ABO_SUB,
	ABO_IMUL,
	ABO_DIV
};

class asm_t {
public:
	virtual void print(ostream& os) = 0;
};

class asm_operator_t : public asm_t {

};

class asm_bin_optr_t : public asm_operator_t {

};

class asm_un_optr_t : public asm_operator_t {

};

class asm_operand_t : public asm_t {

};

class asm_reg_oprnd_t : public asm_operand_t {

};

class asm_addr_oprnd_t : public asm_operand_t {

};

class asm_int_oprnd_t : public asm_operand_t {

};

class asm_var_oprnd_t : public asm_operand_t {

};

class asm_global_vars : public asm_t {

};

class asm_local_vars : public asm_t {

};

class asm_cmd_list_t : public asm_t {
public:
	void add();
	void sub();
	void push();
	void pop();
	void print(ostream& os) override;
};