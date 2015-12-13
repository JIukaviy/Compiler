#include "asm_generator.h"
#include <assert.h>
#include <map>

#define DOUBLE_BUFF_NAME string("_double")
#define INT_BUFF_NAME string("_int")

map<ASM_REGISTER, string> asm_reg_to_str;
map<ASM_REGISTER, int> size_of_reg;
map<ASM_REGISTER, ASM_REGISTER> parent_of;
map<ASM_REGISTER, map<int, ASM_REGISTER>> child_by_size;
map<ASM_OPERATOR, string> asm_op_to_str;
map<ASM_MEM_TYPE, string> asm_mt_to_str;
map<ASM_MEM_TYPE, int> size_of_mtype;
map<int, ASM_MEM_TYPE> mem_type_by_size;
map<ASM_OPERAND_PREFIX, string> asm_aop_to_str;

static string lower_case(char cstr[]) {
	string res(cstr);
	transform(res.begin(), res.end(), res.begin(), tolower);
	return res;
}

void asm_generator_init() {
#define register_asm_op(op_name, op_incode_name) asm_op_to_str[AO_##op_name] = lower_case(#op_name);
#include "asm_op.h"
#undef register_asm_op

#define register_mem_type(mt_name, size) \
	asm_mt_to_str[AMT_##mt_name] = #mt_name; \
	size_of_mtype[AMT_##mt_name] = size; \
	mem_type_by_size[size] = AMT_##mt_name;
#include "asm_mem_type.h"
#undef register_mem_type

#define register_register(reg_name, parent_reg_name, size) \
	asm_reg_to_str[AR_##reg_name] = lower_case(#reg_name); \
	size_of_reg[AR_##reg_name] = size; \
	parent_of[AR_##reg_name] = AR_##parent_reg_name; \
	child_by_size[AR_##parent_reg_name][size] = AR_##reg_name;
#include "asm_registers.h"
#undef register_register

	asm_aop_to_str[AOP_NONE] = "";
	asm_aop_to_str[AOP_OFFSET] = "OFFSET";
#define register_mem_type(mt_name) asm_aop_to_str[AOP_##mt_name##_PTR] = string(#mt_name) + " PTR";
#include "asm_mem_type.h"
#undef register_mem_type
}

//------------------------------ASM_REGISTER_OPERAND-------------------------------------------

asm_reg_oprnd_t::asm_reg_oprnd_t(ASM_REGISTER reg) : reg(reg) {}
asm_reg_oprnd_t::asm_reg_oprnd_t(ASM_REGISTER reg, int reg_size) : reg(asm_gen_t::reg_by_size(reg, reg_size)) {}

asm_reg_oprnd_t::asm_reg_oprnd_t(ASM_REGISTER reg, ASM_MEM_TYPE reg_size) : reg(asm_gen_t::reg_by_mtype(reg, reg_size)) {}

void asm_reg_oprnd_t::print(ostream& os) {
	os << asm_reg_to_str.at(reg);
}

//------------------------------ASM_IDENT_OPERAND-------------------------------------------

asm_ident_operand_t::asm_ident_operand_t(string name) : name(name) {}

void asm_ident_operand_t::print(ostream& os) {
	os << name;
}

//------------------------------ASM_CONSTANT_OPERAND-------------------------------------------

asm_const_oprnd_t::asm_const_oprnd_t(var_ptr var) : var(var) {}

void asm_const_oprnd_t::print(ostream& os) {
	var->asm_print(os);
}

//------------------------------ASM_ADRESS_OPERAND-------------------------------------------

asm_addr_ident_oprnd_t::asm_addr_ident_oprnd_t(string name) : name(name) {}

void asm_addr_ident_oprnd_t::print(ostream& os) {
	os << "OFFSET " << name;
}

asm_addr_reg_oprnd_t::asm_addr_reg_oprnd_t(ASM_REGISTER reg, int offset) : reg(reg), offset(offset) {}

void asm_addr_reg_oprnd_t::print(ostream& os) {
	os << asm_reg_to_str.at(reg) << " + " << offset;
}

//------------------------------ASM_DEREFERENCE_OPERAND-------------------------------------------

asm_deref_oprnd_t::asm_deref_oprnd_t(int offset, int scale) : offset(offset), scale(scale) {}

asm_deref_reg_oprnd_t::asm_deref_reg_oprnd_t(ASM_MEM_TYPE mtype, ASM_REGISTER reg, int offset, ASM_REGISTER offset_reg, int scale) : 
	asm_deref_oprnd_t(offset, scale), mtype(mtype), reg(reg), offset_reg(offset_reg) {
	assert(reg);
}

void asm_deref_reg_oprnd_t::print(ostream& os) {
	os << asm_mt_to_str.at(mtype) << " PTR [" << asm_reg_to_str.at(reg);
	if (offset_reg != AR_NONE)
		os << " + " << asm_reg_to_str.at(offset_reg);
	if (scale)
		os << " * " << scale;
	if (offset)
		os << " + " << offset;
	os << ']';
}

//------------------------------ASM_GLOBAL_VAR-------------------------------------------

asm_global_var_t::asm_global_var_t(string name, ASM_MEM_TYPE type, asm_cmd_list_ptr init_commands, int dup) : 
	name(name), type(type), init_commands(init_commands), dup(dup) {}

asm_global_var_t::asm_global_var_t(string name, ASM_MEM_TYPE type, int dup) :
	name(name), type(type), dup(dup) {}

void asm_global_var_t::print_alloc(ostream& os) {
	os << name << ' ' << asm_mt_to_str.at(type);
	if (dup)
		os << ' ' << dup << " dup (0)";
	else
		os << " 0";
	os << endl;
}

void asm_global_var_t::print_init(ostream& os) {
	init_commands->print(os);
}

//------------------------------ASM_FUNCTION-------------------------------------------

asm_function_t::asm_function_t(string name, asm_cmd_list_ptr cmd_list) : name(name), cmd_list(cmd_list) {}

void asm_function_t::print(ostream& os) {
	os << name << " PROC" << endl;
	cmd_list->print(os);
	os << "ret" << endl;
	os << name << " ENDP" << endl;
}

//------------------------------ASM_COMMANDS-------------------------------------------

//------------------------------ASM_STR_COMMAND-------------------------------------------

asm_str_cmd_t::asm_str_cmd_t(string str) : str(str) {}

void asm_str_cmd_t::print(ostream& os) {
	os << str;
}

//------------------------------ASM_OPERATOR-------------------------------------------

asm_oprtr_t::asm_oprtr_t(ASM_OPERATOR op, asm_oprnd_ptr left_operand, asm_oprnd_ptr right_operand) : 
	op(op), left_operand(left_operand), right_operand(right_operand) {}

asm_oprtr_t::asm_oprtr_t(ASM_OPERATOR op, asm_oprnd_ptr left_operand) : op(op), left_operand(left_operand) {}

asm_oprtr_t::asm_oprtr_t(ASM_OPERATOR op) : op(op) {}

void asm_oprtr_t::print(ostream& os) {
	os << asm_op_to_str.at(op);
	if (left_operand) {
		os << ' ';
		left_operand->print(os);
	}
	if (right_operand) {
		os << ", ";
		right_operand->print(os);
	}
}

//------------------------------ASM_COMANNDS_LIST-------------------------------------------

#define register_asm_op(op_name, op_incode_name)\
	void asm_cmd_list_t::op_incode_name() \
	{_add_op(AO_##op_name);}\
	void asm_cmd_list_t::op_incode_name(ASM_REGISTER operand, int operand_size)\
	{_add_op(AO_##op_name, operand, operand_size);}\
	void asm_cmd_list_t::op_incode_name(var_ptr operand)\
	{_add_op(AO_##op_name, operand);}\
	void asm_cmd_list_t::op_incode_name(string operand)\
	{_add_op(AO_##op_name, operand);}\
	void asm_cmd_list_t::op_incode_name##_addr(string operand)\
	{_add_op_addr(AO_##op_name, operand);}\
	void asm_cmd_list_t::op_incode_name##_deref(ASM_REGISTER operand, ASM_MEM_TYPE mtype, int offset, ASM_REGISTER offset_reg, int scale)\
	{_add_op_deref(AO_##op_name, operand, mtype, offset, offset_reg, scale);}\
	void asm_cmd_list_t::op_incode_name##_deref(ASM_REGISTER operand, int operand_size, int offset, ASM_REGISTER offset_reg, int scale)\
	{_add_op_deref(AO_##op_name, operand, operand_size, offset, offset_reg, scale);}\
\
	void asm_cmd_list_t::op_incode_name(ASM_REGISTER left, ASM_REGISTER right, int operand_size)\
	{_add_op(AO_##op_name, left, right, operand_size);}\
	void asm_cmd_list_t::op_incode_name(ASM_REGISTER left, var_ptr right, int operand_size)\
	{_add_op(AO_##op_name, left, right, operand_size);}\
	void asm_cmd_list_t::op_incode_name(ASM_REGISTER left, string right, int operand_size)\
	{_add_op(AO_##op_name, left, right, operand_size);}\
\
	void asm_cmd_list_t::op_incode_name(string left, var_ptr right)\
	{_add_op(AO_##op_name, left, right);}\
	void asm_cmd_list_t::op_incode_name(string left, ASM_REGISTER right, int operand_size)\
	{_add_op(AO_##op_name, left, right, operand_size);}\
\
	void asm_cmd_list_t::op_incode_name##_raddr(ASM_REGISTER left, string right)\
	{_add_op_raddr(AO_##op_name, left, right);}\
	void asm_cmd_list_t::op_incode_name##_raddr(string left, string right)\
	{_add_op_raddr(AO_##op_name, left, right);}\
\
	void asm_cmd_list_t::op_incode_name##_lderef(ASM_REGISTER left, ASM_REGISTER right, ASM_MEM_TYPE mtype, int offset, ASM_REGISTER offset_reg, int scale)\
	{_add_op_lderef(AO_##op_name, left, right, mtype, offset, offset_reg, scale);}\
	void asm_cmd_list_t::op_incode_name##_lderef(ASM_REGISTER left, ASM_REGISTER right, int operand_size, int offset, ASM_REGISTER offset_reg, int scale)\
	{_add_op_lderef(AO_##op_name, left, right, operand_size, offset, offset_reg, scale);}\
	void asm_cmd_list_t::op_incode_name##_lderef_ls(ASM_REGISTER left, ASM_REGISTER right, ASM_MEM_TYPE mtype, int offset, ASM_REGISTER offset_reg, int scale)\
	{_add_op_lderef_ls(AO_##op_name, left, right, mtype, offset, offset_reg, scale);}\
	void asm_cmd_list_t::op_incode_name##_lderef_ls(ASM_REGISTER left, ASM_REGISTER right, int operand_size, int offset, ASM_REGISTER offset_reg, int scale)\
	{_add_op_lderef_ls(AO_##op_name, left, right, operand_size, offset, offset_reg, scale);}\
	void asm_cmd_list_t::op_incode_name##_lderef(ASM_REGISTER left, string right, ASM_MEM_TYPE mtype, int offset, ASM_REGISTER offset_reg, int scale)\
	{_add_op_lderef(AO_##op_name, left, right, mtype, offset, offset_reg, scale);}\
	void asm_cmd_list_t::op_incode_name##_lderef(ASM_REGISTER left, string right, int operand_size, int offset, ASM_REGISTER offset_reg, int scale)\
	{_add_op_lderef(AO_##op_name, left, right, operand_size, offset, offset_reg, scale);}\
	void asm_cmd_list_t::op_incode_name##_lderef(ASM_REGISTER left, var_ptr right, ASM_MEM_TYPE mtype, int offset, ASM_REGISTER offset_reg, int scale)\
	{_add_op_lderef(AO_##op_name, left, right, mtype, offset, offset_reg, scale);}\
	void asm_cmd_list_t::op_incode_name##_lderef(ASM_REGISTER left, var_ptr right, int operand_size, int offset, ASM_REGISTER offset_reg, int scale)\
	{_add_op_lderef(AO_##op_name, left, right, operand_size, offset, offset_reg, scale);}\
\
	void asm_cmd_list_t::op_incode_name##_rderef(ASM_REGISTER left, ASM_REGISTER right, ASM_MEM_TYPE mtype, int offset, ASM_REGISTER offset_reg, int scale)\
	{_add_op_rderef(AO_##op_name, left, right, mtype, offset, offset_reg, scale);}\
	void asm_cmd_list_t::op_incode_name##_rderef(ASM_REGISTER left, ASM_REGISTER right, int operand_size, int offset, ASM_REGISTER offset_reg, int scale)\
	{_add_op_rderef(AO_##op_name, left, right, operand_size, offset, offset_reg, scale);}\
	void asm_cmd_list_t::op_incode_name##_rderef(string left, ASM_REGISTER right, ASM_MEM_TYPE mtype, int offset, ASM_REGISTER offset_reg, int scale)\
	{_add_op_rderef(AO_##op_name, left, right, mtype, offset, offset_reg, scale);}\
	void asm_cmd_list_t::op_incode_name##_rderef(string left, ASM_REGISTER right, int operand_size, int offset, ASM_REGISTER offset_reg, int scale)\
	{_add_op_rderef(AO_##op_name, left, right, operand_size, offset, offset_reg, scale);}
#include "asm_op.h"
#undef register_op

void asm_cmd_list_t::_add_op(ASM_OPERATOR op, asm_oprnd_ptr operand) {
	commands.push_back(asm_cmd_ptr(new asm_oprtr_t(op, operand)));
}

void asm_cmd_list_t::_add_op(ASM_OPERATOR op, ASM_REGISTER operand, int operand_size) {
	_add_op(op, asm_oprnd_ptr(new asm_reg_oprnd_t(operand, operand_size)));
}

void asm_cmd_list_t::_add_op(ASM_OPERATOR op, var_ptr operand) {
	_add_op(op, asm_oprnd_ptr(new asm_const_oprnd_t(operand)));
}

void asm_cmd_list_t::_add_op(ASM_OPERATOR op, string operand) {
	_add_op(op, asm_oprnd_ptr(new asm_ident_operand_t(operand)));
}

void asm_cmd_list_t::_add_op_addr(ASM_OPERATOR op, string operand) {
	_add_op(op, asm_oprnd_ptr(new asm_addr_ident_oprnd_t(operand)));
}

void asm_cmd_list_t::_add_op_deref(ASM_OPERATOR op, ASM_REGISTER operand, ASM_MEM_TYPE mtype, int offset, ASM_REGISTER offset_reg, int scale) {
	_add_op(op, asm_oprnd_ptr(new asm_deref_reg_oprnd_t(mtype, operand, offset, offset_reg, scale)));
}

void asm_cmd_list_t::_add_op_deref(ASM_OPERATOR op, ASM_REGISTER operand, int operand_size, int offset, ASM_REGISTER offset_reg, int scale) {
	_add_op_deref(op, operand, asm_gen_t::mtype_by_size(operand_size), offset, offset_reg, scale);
}

void asm_cmd_list_t::_add_op(ASM_OPERATOR op) {
	commands.push_back(asm_cmd_ptr(new asm_oprtr_t(op)));
}

void asm_cmd_list_t::_add_op(ASM_OPERATOR op, asm_oprnd_ptr left, asm_oprnd_ptr right) {
	commands.push_back(asm_cmd_ptr(new asm_oprtr_t(op, left, right)));
}

void asm_cmd_list_t::_add_op(ASM_OPERATOR op, ASM_REGISTER left, ASM_REGISTER right, int operand_size) {
	_add_op(op, asm_oprnd_ptr(new asm_reg_oprnd_t(left, operand_size)), asm_oprnd_ptr(new asm_reg_oprnd_t(right, operand_size)));
}

void asm_cmd_list_t::_add_op(ASM_OPERATOR op, ASM_REGISTER left, var_ptr right, int operand_size) {
	_add_op(op, asm_oprnd_ptr(new asm_reg_oprnd_t(left, operand_size)), asm_oprnd_ptr(new asm_const_oprnd_t(right)));
}

void asm_cmd_list_t::_add_op(ASM_OPERATOR op, ASM_REGISTER left, string right, int operand_size) {
	_add_op(op, asm_oprnd_ptr(new asm_reg_oprnd_t(left, operand_size)), asm_oprnd_ptr(new asm_ident_operand_t(right)));
}

void asm_cmd_list_t::_add_op_raddr(ASM_OPERATOR op, ASM_REGISTER left, string right) {
	_add_op(op, asm_oprnd_ptr(new asm_reg_oprnd_t(left)), asm_oprnd_ptr(new asm_addr_ident_oprnd_t(right)));
}

void asm_cmd_list_t::_add_op_raddr(ASM_OPERATOR op, string left, string right) {
	_add_op(op, asm_oprnd_ptr(new asm_ident_operand_t(left)), asm_oprnd_ptr(new asm_addr_ident_oprnd_t(right)));
}

void asm_cmd_list_t::_add_op(ASM_OPERATOR op, string left, ASM_REGISTER right, int operand_size) {
	_add_op(op, asm_oprnd_ptr(new asm_ident_operand_t(left)), asm_oprnd_ptr(new asm_reg_oprnd_t(right, operand_size)));
}

void asm_cmd_list_t::_add_op(ASM_OPERATOR op, string left, var_ptr right) {
	_add_op(op, asm_oprnd_ptr(new asm_ident_operand_t(left)), asm_oprnd_ptr(new asm_const_oprnd_t(right)));
}

void asm_cmd_list_t::_add_op_lderef(ASM_OPERATOR op, ASM_REGISTER left, ASM_REGISTER right, ASM_MEM_TYPE mtype, int offset, ASM_REGISTER offset_reg, int scale) {
	_add_op(op, asm_oprnd_ptr(new asm_deref_reg_oprnd_t(mtype, left, offset, offset_reg, scale)), asm_oprnd_ptr(new asm_reg_oprnd_t(right, mtype)));
}

void asm_cmd_list_t::_add_op_lderef(ASM_OPERATOR op, ASM_REGISTER left, ASM_REGISTER right, int operand_size, int offset, ASM_REGISTER offset_reg, int scale) {
	_add_op_lderef(op, left, right, asm_gen_t::mtype_by_size(operand_size), offset, offset_reg, scale);
}

void asm_cmd_list_t::_add_op_lderef_ls(ASM_OPERATOR op, ASM_REGISTER left, ASM_REGISTER right, ASM_MEM_TYPE mtype, int offset, ASM_REGISTER offset_reg, int scale) {
	_add_op(op, asm_oprnd_ptr(new asm_deref_reg_oprnd_t(mtype, left, offset, offset_reg, scale)), asm_oprnd_ptr(new asm_reg_oprnd_t(right)));
}

void asm_cmd_list_t::_add_op_lderef_ls(ASM_OPERATOR op, ASM_REGISTER left, ASM_REGISTER right, int operand_size, int offset, ASM_REGISTER offset_reg, int scale) {
	_add_op_lderef_ls(op, left, right, asm_gen_t::mtype_by_size(operand_size), offset, offset_reg, scale);
}

void asm_cmd_list_t::_add_op_lderef(ASM_OPERATOR op, ASM_REGISTER left, var_ptr right, ASM_MEM_TYPE mtype, int offset, ASM_REGISTER offset_reg, int scale) {
	_add_op(op, asm_oprnd_ptr(new asm_deref_reg_oprnd_t(mtype, left, offset, offset_reg, scale)), asm_oprnd_ptr(new asm_const_oprnd_t(right)));
}

void asm_cmd_list_t::_add_op_lderef(ASM_OPERATOR op, ASM_REGISTER left, var_ptr right, int operand_size, int offset, ASM_REGISTER offset_reg, int scale) {
	_add_op_lderef(op, left, right, asm_gen_t::mtype_by_size(operand_size), offset, offset_reg, scale);
}

void asm_cmd_list_t::_add_op_lderef(ASM_OPERATOR op, ASM_REGISTER left, string right, ASM_MEM_TYPE mtype, int offset, ASM_REGISTER offset_reg, int scale) {
	_add_op(op, asm_oprnd_ptr(new asm_deref_reg_oprnd_t(mtype, left, offset, offset_reg, scale)), asm_oprnd_ptr(new asm_ident_operand_t(right)));
}

void asm_cmd_list_t::_add_op_lderef(ASM_OPERATOR op, ASM_REGISTER left, string right, int operand_size, int offset, ASM_REGISTER offset_reg, int scale) {
	_add_op_lderef(op, left, right, asm_gen_t::mtype_by_size(operand_size), offset, offset_reg, scale);
}

void asm_cmd_list_t::_add_op_rderef(ASM_OPERATOR op, ASM_REGISTER left, ASM_REGISTER right, ASM_MEM_TYPE mtype, int offset, ASM_REGISTER offset_reg, int scale) {
	_add_op(op, asm_oprnd_ptr(new asm_reg_oprnd_t(left, mtype)), asm_oprnd_ptr(new asm_deref_reg_oprnd_t(mtype, right, offset, offset_reg, scale)));
}

void asm_cmd_list_t::_add_op_rderef(ASM_OPERATOR op, ASM_REGISTER left, ASM_REGISTER right, int operand_size, int offset, ASM_REGISTER offset_reg, int scale) {
	_add_op_rderef(op, left, right, asm_gen_t::mtype_by_size(operand_size), offset, offset_reg, scale);
}

void asm_cmd_list_t::_add_op_rderef(ASM_OPERATOR op, string left, ASM_REGISTER right, ASM_MEM_TYPE mtype, int offset, ASM_REGISTER offset_reg, int scale) {
	_add_op(op, asm_oprnd_ptr(new asm_ident_operand_t(left)), asm_oprnd_ptr(new asm_deref_reg_oprnd_t(mtype, right, offset, offset_reg, scale)));
}

void asm_cmd_list_t::_add_op_rderef(ASM_OPERATOR op, string left, ASM_REGISTER right, int operand_size, int offset, ASM_REGISTER offset_reg, int scale) {
	_add_op_rderef(op, left, right, asm_gen_t::mtype_by_size(operand_size), offset, offset_reg, scale);
}

void asm_cmd_list_t::_copy_to_mem(ASM_REGISTER src_reg, ASM_REGISTER dest_reg, int size, int src_offset, int dst_offset) {
	for (int i = 0; i < size;) {
		int d = size - i;
		if (d < asm_gen_t::size_of(AMT_DWORD)) {
			if (d < asm_gen_t::size_of(AMT_WORD))
				d = 1;
			else
				d = asm_gen_t::size_of(AMT_WORD);
		} else
			d = min(d, asm_gen_t::size_of(AMT_DWORD));
		mov_rderef(AR_EDX, src_reg, d, i + src_offset);
		mov_lderef(dest_reg, AR_EDX, d, i + dst_offset);
		i += d;
	}
}

void asm_cmd_list_t::_copy_to_stack(ASM_REGISTER src_reg, int size, int src_offset) {
	size = asm_gen_t::alignment(size) - asm_gen_t::size_of(AMT_DWORD);
	for (int i = size; i >= 0; i -= asm_gen_t::size_of(AMT_DWORD))
		push_deref(src_reg, AMT_DWORD, i + src_offset);
}

void asm_cmd_list_t::_alloc_in_stack(int size) {
	sub(AR_ESP, new_var<int>(asm_gen_t::alignment(size)));
}

void asm_cmd_list_t::_free_in_stack(int size) {
	add(AR_ESP, new_var<int>(asm_gen_t::alignment(size)));
}

void asm_cmd_list_t::_cast_int_to_double(ASM_REGISTER src_reg) {
	mov(INT_BUFF_NAME, src_reg);
	fild(INT_BUFF_NAME);
}

void asm_cmd_list_t::_cast_double_to_int(ASM_REGISTER dst_reg, bool keep_val) {
	if (keep_val)
		fist(INT_BUFF_NAME);
	else
		fistp(INT_BUFF_NAME);
	mov(dst_reg, INT_BUFF_NAME);
}

void asm_cmd_list_t::_cast_char_to_int(ASM_REGISTER src_reg, ASM_REGISTER dst_reg) {
	xor_(dst_reg, dst_reg);
	mov(dst_reg, src_reg, asm_gen_t::size_of(AMT_BYTE));
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

//------------------------------ASM_GENERATOR-------------------------------------------

void asm_gen_t::print_header(ostream& os) {
	os <<
		".386" << endl <<
		".model flat, C" << endl <<
		"option casemap : none" << endl <<
		"include \\masm32\\include\\msvcrt.inc" << endl <<
		"includelib \\masm32\\lib\\msvcrt.lib" << endl <<
		"R8 macro value:req" << endl <<
		"LOCAL lbl" << endl << 
		".data" << endl <<
		"lbl REAL8 value" << endl <<
		".code" << endl <<
		"EXITM <lbl>" << endl <<
		"endm" << endl <<
		"STR_LITERAL macro value : req" << endl <<
		"LOCAL lbl" << endl <<
		".data" << endl <<
		"lbl BYTE value, 0" << endl <<
		".code" << endl <<
		"EXITM <lbl>" << endl <<
		"endm" << endl <<
		".DATA" << endl <<
		DOUBLE_BUFF_NAME << " QWORD 0" << endl <<
		INT_BUFF_NAME << " DWORD 0" << endl;
}

void asm_gen_t::add_global_var(string name, ASM_MEM_TYPE mem_type, int dup) {
	global_vars.push_back(shared_ptr<asm_global_var_t>(new asm_global_var_t(name, mem_type, dup)));
}

void asm_gen_t::add_global_var(string name, ASM_MEM_TYPE mem_type, asm_cmd_list_ptr init_cmd_list, int dup) {
	global_vars.push_back(shared_ptr<asm_global_var_t>(new asm_global_var_t(name, mem_type, init_cmd_list, dup)));
}

void asm_gen_t::add_function(string name, asm_cmd_list_ptr cmd_list) {
	functions.push_back(shared_ptr<asm_function_t>(new asm_function_t(name, cmd_list)));
}

void asm_gen_t::set_main_cmd_list(asm_cmd_list_ptr cmd_list) {
	main_cmd_list = cmd_list;
}

void asm_gen_t::print(ostream& os) {
	print_header(os);

	for each (auto var in global_vars)
		var->print_alloc(os);

	os << ".CODE" << endl;
	for each (auto func in functions)
		func->print(os);
	os << "start:" << endl;
	for each (auto var in global_vars)
		var->print_init(os);
	main_cmd_list->print(os);
	os << "invoke crt__exit, 0" << endl;
	os << "end start" << endl;
}

int asm_gen_t::alignment(int size) {
	return size + (size_of(AMT_DWORD) - size % size_of(AMT_DWORD)) % size_of(AMT_DWORD);
}

int asm_gen_t::align_size(int size) {
	return 
		size >= size_of(AMT_DWORD) ? alignment(size) : 
		size > size_of(AMT_WORD) ? size_of(AMT_DWORD) :
		size == size_of(AMT_WORD) ? size : 
		size > size_of(AMT_BYTE) ? size_of(AMT_WORD) :
		size;
}

ASM_REGISTER asm_gen_t::reg_by_size(ASM_REGISTER reg, int size) {
	return size ? child_by_size.at(parent_of.at(reg)).at(size) : reg;
}

ASM_REGISTER asm_gen_t::reg_by_mtype(ASM_REGISTER reg, ASM_MEM_TYPE mtype) {
	return reg_by_size(reg, size_of(mtype));
}

int asm_gen_t::size_of(ASM_REGISTER reg) {
	return size_of_reg.at(reg);
}

int asm_gen_t::size_of(ASM_MEM_TYPE mtype) {
	return size_of_mtype.at(mtype);
}

ASM_MEM_TYPE asm_gen_t::mtype_by_size(int size) {
	return mem_type_by_size.at(size);
}
