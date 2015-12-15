#include "asm_code_optimnizer.h"
#include <vector>
#include <memory>

vector<int(*)(asm_cmd_list_ptr, int)> optimizers;

shared_ptr<asm_operator_t> cast_to_op(asm_cmd_ptr cmd) {
	return static_pointer_cast<asm_operator_t>(cmd);
}

int o1(asm_cmd_list_ptr cmd_list, int i) {
	if (cmd_list[i] == AO_MOV &&
		cmd_list[i + 1] == AO_PUSH &&
		cast_to_op(cmd_list[i])->get_left() == AR_EAX &&
		cast_to_op(cmd_list[i+1])->get_left() == AR_EAX) 
	{
		cast_to_op(cmd_list[i+1])->set_left(cast_to_op(cmd_list[i])->get_right());
		cmd_list->_erase(i);
		return 1;
	}
	return 0;
}

void init_asm_code_optimizer() {
	optimizers.push_back(o1);
}

void asm_optimize_code(asm_cmd_list_ptr cmd_list) {
	for (int i = 0; i < cmd_list->_size(); i++)
		for each (auto var in optimizers)
			i -= var(cmd_list, i);
}
