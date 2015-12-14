#ifdef CHECK_ARGS_FUNC_LIST
void check_args_printf(vector<expr_t*> &args, expr_t* func, ...) {
	if (args.empty())
		throw SemanticError("Printf operator requires at least one parameter", func->get_pos());
	if (args[0]->get_type() != ST_PTR)
		throw SemanticError("Printf operator requires pointer to char as first parameter", args[0]->get_pos());
}

void check_args(vector<expr_t*> &args, expr_t* func, ...) {
	vector<type_ptr> func_arg_types;
	SYM_TYPE* curr_type = (SYM_TYPE*)(&func);
	while (*(++curr_type))
		func_arg_types.push_back(type_t::make_type(*curr_type));
	if (func_arg_types.size() != args.size())
		throw IncorrectNumberOfArguments(args.size(), func_arg_types.size(), func->get_pos());
	for (int i = 0; i < func_arg_types.size(); i++)
		args[i] = auto_convert(args[i], func_arg_types[i]);
}
#endif

#ifdef reg_res_func
reg_res_func(PRINTF, printf, crt_printf, check_args_printf, ST_INTEGER, 0)
#endif
