#include "asm_code_optimnizer.h"
#include <vector>
#include <memory>
#include <algorithm>

vector<int(*)(asm_cmd_list_ptr, int)> optimizers;

shared_ptr<asm_operator_t> cast_to_op(asm_cmd_ptr cmd) {
	return dynamic_pointer_cast<asm_operator_t>(cmd);
}

var_ptr cast_to_var(asm_oprnd_ptr op) {
	return dynamic_pointer_cast<asm_const_oprnd_t>(op)->get_var();
}

shared_ptr<asm_reg_oprnd_t> cast_to_reg(asm_oprnd_ptr op) {
	return dynamic_pointer_cast<asm_reg_oprnd_t>(op);
}

shared_ptr<asm_deref_reg_oprnd_t> cast_to_reg_deref(asm_oprnd_ptr op) {
	return dynamic_pointer_cast<asm_deref_reg_oprnd_t>(op);
}

shared_ptr<asm_const_oprnd_t> new_const_oprnd(var_ptr var) {
	return shared_ptr<asm_const_oprnd_t>(new asm_const_oprnd_t(var));
}

bool reg_used_in_oprnd(asm_oprnd_ptr op, asm_oprnd_ptr reg) {
	return op ? op->like(reg) : false;
}

bool reg_used_in_oprnd(asm_oprnd_ptr op, ASM_REGISTER reg) {
	return op ? op->like(reg) : false;
}

enum REG_USAGE {
	RU_USED,
	RU_FREED,
	RU_UNUSED
};

REG_USAGE check_reg_use(asm_cmd_ptr cmd, ASM_REGISTER reg) {
	if (cmd == ACT_LABEL)
		RU_UNUSED;
	if (cmd == AO_CALL)
		RU_UNUSED;

	if (cmd == AO_MOV &&
		cast_to_op(cmd)->get_left()->like(reg) &&
		cast_to_op(cmd)->get_left() != AOT_DEREF)
		return RU_FREED;
	if (cmd == AO_LEA &&
		cast_to_op(cmd)->get_left()->like(reg))
		return RU_FREED;
	if (cmd == AO_XOR &&
		cast_to_op(cmd)->get_left()->like(reg) &&
		cast_to_op(cmd)->get_right()->like(reg))
		return RU_FREED;
	if (cmd == AO_POP &&
		cast_to_op(cmd)->get_left()->like(reg))
		return RU_FREED;

	if (cmd == AO_RET && reg == AR_EAX)
		return RU_USED;

	if (reg_used_in_oprnd(cast_to_op(cmd)->get_left(), reg) ||
		reg_used_in_oprnd(cast_to_op(cmd)->get_right(), reg))
		return RU_USED;
}

bool unused_reg(asm_cmd_list_ptr cmd_list, int i, asm_oprnd_ptr reg) {
	for (; i < cmd_list->_size(); i++) {
		if (cmd_list[i] == ACT_LABEL)
			continue;
		if (cmd_list[i] == AO_CALL)
			return true;
		REG_USAGE reg_usage = check_reg_use(cmd_list[i], cast_to_reg(reg)->get_reg());
		if (reg_usage == RU_UNUSED)
			continue;
		return reg_usage == RU_FREED;
	}
	return true;
}

void erase_reg_from_set(set<ASM_REGISTER>& regs, asm_oprnd_ptr op) {
	if (!op)
		return;
	if (op == AOT_REG)
		regs.erase(cast_to_reg(op)->get_reg());
	else if (op == AOT_DEREF) {
		regs.erase(cast_to_reg_deref(op)->get_reg());
		regs.erase(cast_to_reg_deref(op)->get_offset_reg());
	}
}

ASM_REGISTER find_unused_reg_until_pop(asm_cmd_list_ptr cmd_list, int i) {
	set<ASM_REGISTER> regs;
	regs.insert(AR_EAX);
	regs.insert(AR_EBX);
	regs.insert(AR_ECX);
	regs.insert(AR_EDX);
	//regs.insert(AR_ESI);
	//regs.insert(AR_EDI);
	for (; i < cmd_list->_size(); i++) {
		if (cmd_list[i] == ACT_LABEL)
			continue;
		if (cmd_list[i] == AO_CALL)
			return AR_NONE;
		erase_reg_from_set(regs, cmd_list->get_op(i)->get_left());
		erase_reg_from_set(regs, cmd_list->get_op(i)->get_right());
	}
	return regs.empty() ? AR_NONE : *regs.begin();
}

void replace_reg_in_operand(asm_oprnd_ptr op, ASM_REGISTER from, ASM_REGISTER to) {
	if (!op)
		return;
	if (op == AOT_REG && cast_to_reg(op)->get_reg() == from)
		cast_to_reg(op)->set_reg(to);
	else if (op == AOT_DEREF) {
		if (cast_to_reg_deref(op)->get_reg() == from)
			cast_to_reg_deref(op)->set_reg(to);
		if (cast_to_reg_deref(op)->get_offset_reg() == from)
			cast_to_reg_deref(op)->set_offset_reg(to);
	}
}

void replace_register(asm_cmd_list_ptr cmd_list, int i, ASM_REGISTER from, ASM_REGISTER to) {
	for (; i < cmd_list->_size(); i++) {
		if (cmd_list[i] == ACT_LABEL)
			continue;
		if (cmd_list[i] == AO_CALL)
			return;
		if (check_reg_use(cmd_list[i], from) == RU_FREED)
			return;
		replace_reg_in_operand(cmd_list->get_op(i)->get_left(), from, to);
		replace_reg_in_operand(cmd_list->get_op(i)->get_right(), from, to);
	}
}

void replace_reg_to_operand(asm_cmd_list_ptr cmd_list, int i, ASM_REGISTER from, asm_oprnd_ptr to) {
	for (; i < cmd_list->_size(); i++) {
		if (cmd_list[i] == ACT_LABEL)
			continue;
		if (cmd_list[i] == AO_CALL)
			return;
		if (check_reg_use(cmd_list[i], from) == RU_FREED)
			return;

		if (cmd_list->get_op(i)->get_left() && 
			cmd_list->get_op(i)->get_left() == AOT_REG &&
			cmd_list->get_op(i)->get_left()->like(from))
				cmd_list->get_op(i)->set_left(to);
		
		if (cmd_list->get_op(i)->get_right() &&
			cmd_list->get_op(i)->get_right() == AOT_REG &&
			cmd_list->get_op(i)->get_right()->like(from))
				cmd_list->get_op(i)->set_right(to);
	}
}

void combine_reg_derefs(asm_oprnd_ptr src, asm_oprnd_ptr add, ASM_REGISTER reg) {
	if (src != AOT_DEREF || add != AOT_DEREF)
		return;
	auto src_d = cast_to_reg_deref(src);
	auto add_d = cast_to_reg_deref(add);
	src_d->add_offset(add_d->get_offset());
	if (src_d->get_reg() == reg)
		src_d->set_reg(add_d->get_reg());
	if (src_d->get_offset_reg() == reg)
		src_d->set_offset_reg(add_d->get_offset_reg());
}

void combine_all_reg_derefs(asm_cmd_list_ptr cmd_list, int i, ASM_REGISTER reg, asm_oprnd_ptr deref) {
	for (; i < cmd_list->_size(); i++) {
		if (cmd_list[i] == ACT_LABEL)
			continue;
		if (cmd_list[i] == AO_CALL)
			return;

		if (cmd_list[i] == AO_MOV &&
			cmd_list->get_op(i)->get_left() == AOT_REG &&
			cmd_list->get_op(i)->get_left()->like(reg) &&
			cmd_list->get_op(i)->get_right() == AOT_DEREF &&
			(cast_to_reg_deref(cmd_list->get_op(i)->get_right())->get_reg() == reg ||
			cast_to_reg_deref(cmd_list->get_op(i)->get_right())->get_offset_reg() == reg))
		{
			combine_reg_derefs(cmd_list->get_op(i)->get_right(), deref, reg);
			return;
		}

		if (check_reg_use(cmd_list[i], reg) == RU_FREED)
			return;

		if (cmd_list->get_op(i)->get_left() &&
			cmd_list->get_op(i)->get_left() == AOT_DEREF &&
			(cast_to_reg_deref(cmd_list->get_op(i)->get_left())->get_reg() == reg ||
			cast_to_reg_deref(cmd_list->get_op(i)->get_left())->get_offset_reg() == reg))
				combine_reg_derefs(cmd_list->get_op(i)->get_left(), deref, reg);

		if (cmd_list->get_op(i)->get_right() &&
			cmd_list->get_op(i)->get_right() == AOT_DEREF &&
			(cast_to_reg_deref(cmd_list->get_op(i)->get_right())->get_reg() == reg ||
			cast_to_reg_deref(cmd_list->get_op(i)->get_right())->get_offset_reg() == reg))
				combine_reg_derefs(cmd_list->get_op(i)->get_right(), deref, reg);
	}
}

bool used_only_as_deref_base(asm_cmd_list_ptr cmd_list, int i, ASM_REGISTER reg) {
	for (; i < cmd_list->_size(); i++) {
		if (cmd_list[i] == ACT_LABEL)
			continue;
		if (cmd_list[i] == AO_CALL)
			return true;
		if (cmd_list[i] == AO_RET && reg == AR_EAX)
			return false;
		if (check_reg_use(cmd_list[i], reg) == RU_FREED)
			return true;
		if (cmd_list->get_op(i)->get_left() &&
			cmd_list->get_op(i)->get_left() != AOT_DEREF &&
			cmd_list->get_op(i)->get_left()->like(reg))
			return false;
		if (cmd_list->get_op(i)->get_right() &&
			cmd_list->get_op(i)->get_right() != AOT_DEREF &&
			cmd_list->get_op(i)->get_right()->like(reg))
			return false;
	}
	return true;
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
		unused_reg(cmd_list, i + 2, cmd_list->get_op(i)->get_left()))
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
	if (cmd_list[i] == AO_LEA) {
		int j;
		bool erased = false;
		for (j = i + 1; j < cmd_list->_size() &&
			(cmd_list[j] == AO_MOV || cmd_list[j] == AO_PUSH) &&
			cmd_list->get_op(j)->get_left()->like(cmd_list->get_op(i)->get_left()) &&
			cmd_list->get_op(j)->get_left() == AOT_DEREF &&
			cmd_list->get_op(j)->get_right() != AOT_DEREF; j++) 
		{
			cast_to_reg_deref(cmd_list->get_op(j)->get_left())->add_offset(cast_to_reg_deref(cmd_list->get_op(i)->get_right())->get_offset());
			cast_to_reg_deref(cmd_list->get_op(j)->get_left())->set_reg(cast_to_reg_deref(cmd_list->get_op(i)->get_right())->get_reg());
			erased = true;
		}
		if (erased)
			cmd_list->_erase(i);
		return erased;
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
		!cmd_list->get_op(i)->get_right()->like(cmd_list->get_op(i)->get_left()) &&
		unused_reg(cmd_list, i + 2, cmd_list->get_op(i)->get_left())) 
	{
		asm_oprnd_ptr moved_to_reg = cmd_list->get_op(i)->get_left();
		cast_to_reg_deref(cmd_list->get_op(i)->get_right())->set_op_size(cast_to_reg(cmd_list->get_op(i + 1)->get_left())->get_size());
		cmd_list->get_op(i + 1)->set_right(cmd_list->get_op(i)->get_right());
		cmd_list->get_op(i)->set_op(AO_XOR);
		cmd_list->get_op(i)->set_left(moved_to_reg);
		cmd_list->get_op(i)->set_right(moved_to_reg);
		return 0;
	}
	return 0;
}

int o13(asm_cmd_list_ptr cmd_list, int i) {
	if (cmd_list->_size() - i < 2)
		return 0;
	if (cmd_list[i] == AO_MOV &&
		cmd_list[i + 1] == AO_MOV &&
		cmd_list->get_op(i + 1)->get_left() == AOT_DEREF &&
		cmd_list->get_op(i)->get_left() == AOT_REG &&
		cmd_list->get_op(i)->get_right() == AOT_REG &&
		cmd_list->get_op(i)->get_left() != AR_EBP &&
		cmd_list->get_op(i)->get_right() == AR_ESP &&
		cast_to_reg_deref(cmd_list->get_op(i + 1)->get_left())->like(cmd_list->get_op(i)->get_left()) &&
		unused_reg(cmd_list, i + 2, cmd_list->get_op(i)->get_right())) 
	{
		cast_to_reg_deref(cmd_list->get_op(i + 1)->get_left())->set_reg(cast_to_reg(cmd_list->get_op(i)->get_right())->get_reg());
		cmd_list->_erase(i);
		return 1;
	}
	return 0;
}

int o14(asm_cmd_list_ptr cmd_list, int i) {
	if (cmd_list->_size() - i < 2)
		return 0;
	if (cmd_list[i + 0] == AO_ADD &&
		cmd_list[i + 1] == AO_MOV &&
		cmd_list->get_op(i)->get_left() == AOT_REG &&
		cmd_list->get_op(i)->get_right() == AOT_REG &&
		cmd_list->get_op(i + 1)->get_left() == AOT_DEREF &&
		cmd_list->get_op(i)->get_left() == cast_to_reg_deref(cmd_list->get_op(i + 1)->get_left())->get_reg() &&
		unused_reg(cmd_list, i + 2, cmd_list->get_op(i)->get_left()))
	{
		cast_to_reg_deref(cmd_list->get_op(i + 1)->get_left())->set_reg(cast_to_reg(cmd_list->get_op(i)->get_right())->get_reg());
		cmd_list->_erase(i);
		return 1;
	}
	return 0;
}

int o15(asm_cmd_list_ptr cmd_list, int i) {
	if (cmd_list->_size() - i < 2)
		return 0;
	int var_val;
	if (cmd_list[i] == AO_IMUL &&
		cmd_list[i + 1] == AO_MOV &&
		cmd_list->get_op(i)->get_left() == AOT_REG &&
		cmd_list->get_op(i)->get_right() == AOT_VAR &&
		cmd_list->get_op(i + 1)->get_left() == AOT_DEREF &&
		((var_val = var_pointer_cast<int>(cast_to_var(cmd_list->get_op(i)->get_right()))->get_val()) == 1 ||
		(var_val == 2) || (var_val == 4) || (var_val == 8)) &&
		cmd_list->get_op(i)->get_left() == cast_to_reg_deref(cmd_list->get_op(i + 1)->get_left())->get_reg() &&
		unused_reg(cmd_list, i + 2, cmd_list->get_op(i)->get_left())) 
	{
		ASM_REGISTER base_reg = cast_to_reg_deref(cmd_list->get_op(i + 1)->get_left())->get_reg();
		cast_to_reg_deref(cmd_list->get_op(i + 1)->get_left())->set_reg(cast_to_reg_deref(cmd_list->get_op(i + 1)->get_left())->get_offset_reg());
		cast_to_reg_deref(cmd_list->get_op(i + 1)->get_left())->set_offset_reg(base_reg);
		cast_to_reg_deref(cmd_list->get_op(i + 1)->get_left())->set_scale(var_val);
		cmd_list->_erase(i);
		return 1;
	}
	return 0;
}

int o16(asm_cmd_list_ptr cmd_list, int i) {
	if (cmd_list->_size() - i < 2)
		return 0;
	int var_val;
	if (cmd_list[i] == AO_LEA &&
		cmd_list[i + 1] == AO_LEA &&
		cast_to_reg(cmd_list->get_op(i+1)->get_left())->get_reg() == cast_to_reg_deref(cmd_list->get_op(i + 1)->get_right())->get_reg())
	{
		cast_to_reg_deref(cmd_list->get_op(i)->get_right())->add_offset(cast_to_reg_deref(cmd_list->get_op(i + 1)->get_right())->get_offset());
		cmd_list->_erase(i + 1);
		return 1;
	}
	return 0;
}

int o17(asm_cmd_list_ptr cmd_list, int i) {
	if (cmd_list->_size() - i < 3)
		return 0;
	if (cmd_list[i] != AO_XOR &&
		cmd_list[i + 1] == AO_MOV &&
		cmd_list[i + 2] == AO_MOV &&
		cmd_list->get_op(i)->get_right() == AOT_DEREF &&
		cmd_list->get_op(i + 1)->get_left() == AOT_REG &&
		cmd_list->get_op(i)->get_left()->like(cmd_list->get_op(i + 1)->get_right()) &&
		!cmd_list->get_op(i)->get_right()->like(cmd_list->get_op(i)->get_left()) &&
		unused_reg(cmd_list, i + 2, cmd_list->get_op(i)->get_left()))
	{
		asm_oprnd_ptr moved_to_reg = cmd_list->get_op(i)->get_left();
		cast_to_reg_deref(cmd_list->get_op(i)->get_right())->set_op_size(cast_to_reg(cmd_list->get_op(i + 1)->get_left())->get_size());
		cmd_list->get_op(i + 1)->set_right(cmd_list->get_op(i)->get_right());
		cmd_list->get_op(i)->set_op(AO_XOR);
		cmd_list->get_op(i)->set_left(moved_to_reg);
		cmd_list->get_op(i)->set_right(moved_to_reg);
		return 0;
	}
	return 0;
}

int o18(asm_cmd_list_ptr cmd_list, int i) {
	if (cmd_list->_size() - i < 1)
		return 0;
	if (cmd_list[i] == AO_LEA &&
		used_only_as_deref_base(cmd_list, i + 1, cast_to_reg(cmd_list->get_op(i)->get_left())->get_reg())) 
	{
		combine_all_reg_derefs(cmd_list, i + 1, cast_to_reg(cmd_list->get_op(i)->get_left())->get_reg(), cmd_list->get_op(i)->get_right());
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
	//optimizers.push_back(o12);
	optimizers.push_back(o13);
	//optimizers.push_back(o14);
	optimizers.push_back(o15);
	optimizers.push_back(o16);
	//optimizers.push_back(o17);
	//optimizers.push_back(o18);
}

void asm_optimize_code(asm_cmd_list_ptr cmd_list) {
	bool changed = true;
	while (changed) {
		changed = false;
		for (int i = 0; i < cmd_list->_size(); i++) {
			for each (auto o in optimizers) {
				if (i < 0)
					break;
				int d = o(cmd_list, i);
				changed = changed || d;
				i -= d;
			}
			if (i < 0)
				break;
		}
	}
}
