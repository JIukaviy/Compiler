#ifdef CHECK_ARGS_FUNC_LIST
void check_args_printf(vector<expr_t*> &args, expr_t* func, ...) {
	if (args.empty())
		throw SemanticError("Printf operator requires at least one parameter", func->get_pos());
	if (args[0]->get_type() != ST_PTR)
		throw SemanticError("Printf operator requires pointer to char as first parameter", args[0]->get_pos());
}

void check_args_scanf(vector<expr_t*> &args, expr_t* func, ...) {
	if (args.size() < 2)
		throw SemanticError("Scanf operator requires at least two parameters", func->get_pos());
	for (int i = 0; i < args.size(); i++)
		if (args[i]->get_type() != ST_PTR)
			throw SemanticError("Scanf operator can pass only pointer type parameters", args[i]->get_pos());
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
reg_res_func(SCANF, scanf, crt_scanf, check_args_scanf, ST_INTEGER, 0)
reg_res_func(MALLOC, malloc, crt_malloc, check_args, ST_PTR, ST_INTEGER, 0)
#endif
