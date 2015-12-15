#include "asm_code_optimnizer.h"
#include <vector>
#include <memory>
#include <algorithm>

vector<int(*)(asm_cmd_list_ptr, int)> optimizers;

shared_ptr<asm_operator_t> cast_to_op(asm_cmd_ptr cmd) {
	return static_pointer_cast<asm_operator_t>(cmd);
}

var_ptr cast_to_var(asm_oprnd_ptr op) {
	return static_pointer_cast<asm_const_oprnd_t>(op)->get_var();
}

shared_ptr<asm_reg_oprnd_t> cast_to_reg(asm_oprnd_ptr op) {
	return static_pointer_cast<asm_reg_oprnd_t>(op);
}

shared_ptr<asm_deref_reg_oprnd_t> cast_to_reg_deref(asm_oprnd_ptr op) {
	return static_pointer_cast<asm_deref_reg_oprnd_t>(op);
}

shared_ptr<asm_const_oprnd_t> new_const_oprnd(var_ptr var) {
	return shared_ptr<asm_const_oprnd_t>(new asm_const_oprnd_t(var));
}

bool reg_used_in_oprnd(asm_oprnd_ptr op, asm_oprnd_ptr reg) {
	return op->like(reg);
}

bool unused_reg(asm_cmd_list_ptr cmd_list, int i, asm_oprnd_ptr reg) {
	for (; i < cmd_list->_size(); i++) {
		if (cmd_list[i] == ACT_LABEL)
			continue;
		if (cmd_list[i] == AO_CALL)
			return true;
		if ((cmd_list[i] == AO_MOV || cmd_list[i] == AO_LEA) && 
			cmd_list->get_op(i)->get_left()->like(reg) &&
			!cmd_list->get_op(i)->get_right()->like(reg))
			return true;
		if ((cmd_list[i] == AO_XOR) &&
			cmd_list->get_op(i)->get_left()->like(reg) &&
			cmd_list->get_op(i)->get_right()->like(reg))
			return true;
		if (cmd_list[i] == AO_POP && cmd_list->get_op(i)->get_left()->like(reg))
			return true;
		if (cmd_list->get_op(i)->get_left() && cmd_list->get_op(i)->get_left()->like(reg) ||
			cmd_list->get_op(i)->get_right() && cmd_list->get_op(i)->get_right()->like(reg))
			return false;
	}
	return true;
}

ASM_REGISTER find_unused_reg(asm_cmd_list_ptr cmd_list, int i) {
	return AR_NONE;
}

int o1(asm_cmd_list_ptr cmd_list, int i) {
	if (cmd_list->_size() - i < 2)
		return 0;
	if (cmd_list[i] == AO_MOV &&
		cmd_list[i + 1] == AO_PUSH &&
		cmd_list->get_op(i)->get_left() == AR_EAX &&
		cmd_list->get_op(i+1)->get_left() == AR_EAX) 
	{
		cmd_list->get_op(i+1)->set_left(cmd_list->get_op(i)->get_right());
		cmd_list->_erase(i);
		return 1;
	}
	return 0;
}

int o2(asm_cmd_list_ptr cmd_list, int i) {
	if (cmd_list->_size() - i < 3)
		return 0;
	if (cmd_list[i] == AO_PUSH &&
		(cmd_list[i + 1] != AO_PUSH && cmd_list[i + 1] != AO_POP) &&
		cmd_list[i + 2] == AO_POP)
	{
		cmd_list->get_op(i)->set_op(AO_MOV);
		cmd_list->get_op(i)->set_right(cmd_list->get_op(i)->get_left());
		cmd_list->get_op(i)->set_left(cmd_list->get_op(i + 2)->get_left());
		cmd_list->_erase(i+2);
		return 1;
	}
	return 0;
}

int o3(asm_cmd_list_ptr cmd_list, int i) {
	if (cmd_list->_size() - i < 1)
		return 0;
	if ((cmd_list[i] == AO_ADD || cmd_list[i] == AO_SUB) &&
		cmd_list->get_op(i)->get_right() == AOT_VAR &&
		cast_to_var(cmd_list->get_op(i)->get_right())->is_null())
	{
		cmd_list->_erase(i);
		return 1;
	}
	return 0;
}

int o4(asm_cmd_list_ptr cmd_list, int i) {
	if (cmd_list->_size() - i < 3)
		return 0;
	if (cmd_list[i] == AO_MOV &&
		cmd_list[i+1] == AO_MOV &&
		cmd_list[i+2] == AO_XOR &&
		cmd_list->get_op(i)->get_left() == cmd_list->get_op(i+1)->get_right() &&
		cmd_list->get_op(i)->get_left() == cmd_list->get_op(i+2)->get_left() &&
		cmd_list->get_op(i)->get_left() == cmd_list->get_op(i+2)->get_right()) 
	{
		cmd_list->get_op(i + 1)->set_right(cmd_list->get_op(i)->get_right());
		cmd_list->_erase(i);
		return 1;
	}
	return 0;
}

int o5(asm_cmd_list_ptr cmd_list, int i) {
	if (cmd_list->_size() - i < 3)
		return 0;
	if (cmd_list[i] == AO_MOV &&
		cmd_list[i + 1] == AO_XOR &&
		cmd_list[i + 2] == AO_MOV &&
		cmd_list->get_op(i)->get_left() == AOT_REG &&
		cmd_list->get_op(i)->get_right() == AOT_VAR &&
		cmd_list->get_op(i + 1)->get_left() == cmd_list->get_op(i + 1)->get_right() &&
		cmd_list->get_op(i)->get_left()->like(cmd_list->get_op(i + 2)->get_right()) && 
		cmd_list->get_op(i+1)->get_left()->like(cmd_list->get_op(i + 2)->get_left()))
	{
		cmd_list->get_op(i)->set_left(cmd_list->get_op(i+1)->get_left());
		cmd_list->get_op(i)->set_right(new_const_oprnd(var_cast<int>(cast_to_var(cmd_list->get_op(i)->get_right())) % new_var<int>(256)));
		cmd_list->_erase(i+1);
		cmd_list->_erase(i+1);
		return 2;
	}
	return 0;
}

int o6(asm_cmd_list_ptr cmd_list, int i) {
	if (cmd_list->_size() - i < 2)
		return 0;
	if (cmd_list[i] == AO_MOV &&
		(cmd_list[i + 1] == AO_MOV || cmd_list[i + 1] == AO_ADD || cmd_list[i + 1] == AO_SUB) &&
		cmd_list->get_op(i)->get_left() == AOT_REG &&
		cmd_list->get_op(i)->get_right() == AOT_VAR &&
		cmd_list->get_op(i)->get_left()->like(cmd_list->get_op(i + 1)->get_right()) &&
		unused_reg(cmd_list, i + 2, cmd_list->get_op(i)->get_left()))
	{
		cmd_list->get_op(i+1)->set_right(cmd_list->get_op(i)->get_right());
		cmd_list->_erase(i);
		return 1;
	}
	return 0;
}

int o7(asm_cmd_list_ptr cmd_list, int i) {
	if (cmd_list->_size() - i < 6)
		return 0;
	if (cmd_list[i] == AO_CMP &&
		cmd_list[i + 2] == AO_XOR &&
		cmd_list[i + 3] == AO_MOV &&
		cmd_list[i + 4] == AO_TEST)
	{
		if (cmd_list[i + 5] == AO_JZ) {
			cmd_list->get_op(i + 1)->set_op(
				cmd_list[i + 1] == AO_SETE ? AO_JNE :
				cmd_list[i + 1] == AO_SETNE ? AO_JE :
				cmd_list[i + 1] == AO_SETL ? AO_JGE :
				cmd_list[i + 1] == AO_SETLE ? AO_JG :
				cmd_list[i + 1] == AO_SETG ? AO_JLE :
				cmd_list[i + 1] == AO_SETGE ? AO_JL :
				cmd_list[i + 1] == AO_SETB ? AO_JAE :
				cmd_list[i + 1] == AO_SETBE ? AO_JA :
				cmd_list[i + 1] == AO_SETA ? AO_JBE :
				AO_JB);
		} else {
			cmd_list->get_op(i + 1)->set_op(
				cmd_list[i + 1] == AO_SETE ? AO_JE :
				cmd_list[i + 1] == AO_SETNE ? AO_JNE :
				cmd_list[i + 1] == AO_SETL ? AO_JL :
				cmd_list[i + 1] == AO_SETLE ? AO_JLE :
				cmd_list[i + 1] == AO_SETG ? AO_JG :
				cmd_list[i + 1] == AO_SETGE ? AO_JGE :
				cmd_list[i + 1] == AO_SETB ? AO_JB :
				cmd_list[i + 1] == AO_SETBE ? AO_JBE :
				cmd_list[i + 1] == AO_SETA ? AO_JA :
				AO_JAE);
		}
		cmd_list->get_op(i + 1)->set_left(cmd_list->get_op(i + 5)->get_left());
		cmd_list->get_op(i + 1)->set_right(nullptr);
		cmd_list->_erase(i + 2);
		cmd_list->_erase(i + 2);
		cmd_list->_erase(i + 2);
		cmd_list->_erase(i + 2);
		return 4;
	}
	return 0;
}

int o8(asm_cmd_list_ptr cmd_list, int i) {
	if (cmd_list->_size() - i < 2)
		return 0;
	if (cmd_list[i] == AO_LEA &&
		cmd_list[i + 1] == AO_MOV &&
		cmd_list->get_op(i)->get_left() == cmd_list->get_op(i + 1)->get_right() &&
		unused_reg(cmd_list, i + 2, cmd_list->get_op(i + 1)->get_right()))
	{
		cmd_list->get_op(i)->set_left(cmd_list->get_op(i + 1)->get_left());
		cmd_list->_erase(i + 1);
		return 1;
	}
	return 0;
}

int o9(asm_cmd_list_ptr cmd_list, int i) {
	if (cmd_list->_size() - i < 3)
		return 0;
	if (cmd_list[i] == AO_MOV &&
		cmd_list[i + 1] == AO_XOR &&
		cmd_list[i + 2] == AO_MOV &&
		cmd_list->get_op(i)->get_left()->like(cmd_list->get_op(i + 2)->get_right()) &&
		cmd_list->get_op(i + 1)->get_left()->like(cmd_list->get_op(i + 2)->get_left()) &&
		cmd_list->get_op(i)->get_right() == AOT_DEREF &&
		!cmd_list->get_op(i)->get_right()->like(cmd_list->get_op(i + 1)->get_left()))
	{
		cast_to_reg_deref(cmd_list->get_op(i)->get_right())->set_op_size(cast_to_reg(cmd_list->get_op(i + 2)->get_left())->get_size());
		cmd_list->get_op(i + 2)->set_right(cmd_list->get_op(i)->get_right());
		cmd_list->_erase(i);
		return 1;
	}
	return 0;
}

int o10(asm_cmd_list_ptr cmd_list, int i) {
	if (cmd_list->_size() - i < 2)
		return 0;
	if (cmd_list[i] == AO_LEA &&
		cmd_list[i + 1] == AO_MOV &&
		cmd_list->get_op(i + 1)->get_left()->like(cmd_list->get_op(i)->get_left()) &&
		cmd_list->get_op(i + 1)->get_right() != AOT_DEREF)
	{
		cast_to_reg_deref(cmd_list->get_op(i)->get_right())->set_op_size(cast_to_reg_deref(cmd_list->get_op(i + 1)->get_left())->get_op_size());
		cast_to_reg_deref(cmd_list->get_op(i)->get_right())->add_offset(cast_to_reg_deref(cmd_list->get_op(i + 1)->get_left())->get_offset());
		cmd_list->get_op(i + 1)->set_left(cmd_list->get_op(i)->get_right());
		cmd_list->_erase(i);
		return 1;
	}
	return 0;
}

int o11(asm_cmd_list_ptr cmd_list, int i) {
	if (cmd_list->_size() - i < 1)
		return 0;
	if (cmd_list[i] == AO_XOR &&
		cmd_list->get_op(i + 1)->get_left() == cmd_list->get_op(i + 1)->get_right() &&
		unused_reg(cmd_list, i + 1, cmd_list->get_op(i)->get_left())) 
	{
		cmd_list->_erase(i);
		return 1;
	}
	return 0;
}

int o12(asm_cmd_list_ptr cmd_list, int i) {
	if (cmd_list->_size() - i < 2)
		return 0;
	if (cmd_list[i] == AO_MOV &&
		cmd_list[i + 1] == AO_MOV &&
		cmd_list->get_op(i)->get_right() == AOT_DEREF &&
		cmd_list->get_op(i + 1)->get_left() == AOT_REG &&
		cmd_list->get_op(i)->get_left()->like(cmd_list->get_op(i + 1)->get_right()) &&
		unused_reg(cmd_list, i + 2, cmd_list->get_op(i)->get_left())) 
	{
		cast_to_reg_deref(cmd_list->get_op(i)->get_right())->set_op_size(cast_to_reg(cmd_list->get_op(i + 1)->get_left())->get_size());
		cmd_list->get_op(i + 1)->set_right(cmd_list->get_op(i)->get_right());
		cmd_list->_erase(i);
		return 1;
	}
	return 0;
}

void init_asm_code_optimizer() {
	optimizers.push_back(o1);
	optimizers.push_back(o2);
	optimizers.push_back(o3);
	optimizers.push_back(o4);
	optimizers.push_back(o5);
	optimizers.push_back(o6);
	optimizers.push_back(o7);
	optimizers.push_back(o8);
	optimizers.push_back(o9);
	optimizers.push_back(o10);
	optimizers.push_back(o11);
	optimizers.push_back(o12);
}

void asm_optimize_code(asm_cmd_list_ptr cmd_list) {
	bool changed = true;
	while (changed) {
		changed = false;
		for (int i = 0; i < cmd_list->_size(); i++)
			for each (auto o in optimizers) {
				if (i < 0)
					break;
				int d = o(cmd_list, i);
				changed = changed || d;
				i -= d;
			}
	}
}
