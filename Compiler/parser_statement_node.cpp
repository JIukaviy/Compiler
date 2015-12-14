#include "parser_statement_node.h"
#include "type_conversion.h"

stmt_expr_t::stmt_expr_t(expr_t * expression) : expression(expression) {}
stmt_block_t::stmt_block_t() {}
stmt_block_t::stmt_block_t(const vector<stmt_ptr>& statements, sym_table_ptr  sym_table) : statements(statements), sym_table(sym_table) {}
stmt_decl_t::stmt_decl_t(sym_ptr symbol) : symbol(symbol) {}
stmt_if_t::stmt_if_t(expr_t* condition, stmt_ptr then_stmt) : condition(condition), then_stmt(then_stmt), else_stmt(0) {}
stmt_if_t::stmt_if_t(expr_t* condition, stmt_ptr then_stmt, stmt_ptr else_stmt) : condition(condition), then_stmt(then_stmt), else_stmt(else_stmt) {}
stmt_loop_t::stmt_loop_t(stmt_ptr stmt) : stmt(stmt) {}
stmt_loop_t::stmt_loop_t() : stmt(0) {}
stmt_while_t::stmt_while_t(expr_t* condition, stmt_ptr stmt) : condition(condition), stmt_loop_t(stmt) {}
stmt_while_t::stmt_while_t(expr_t* condition) : condition(condition) {}
stmt_for_t::stmt_for_t(expr_t* init_expr, expr_t* condition, expr_t* expr, stmt_ptr stmt) : init_expr(init_expr), condition(condition), expr(expr), stmt_loop_t(stmt) {}
stmt_for_t::stmt_for_t(expr_t* init_expr, expr_t* condition, expr_t* expr) : init_expr(init_expr), condition(condition), expr(expr) {}

void statement_t::asm_generate_code(asm_cmd_list_ptr cmd_list, int offset) {
	asm_gen_entry_code(cmd_list, offset);
	asm_gen_internal_code(cmd_list, offset);
	asm_gen_exit_code(cmd_list);
}

void statement_t::asm_gen_entry_code(asm_cmd_list_ptr cmd_list, int offset) {}

void statement_t::asm_gen_internal_code(asm_cmd_list_ptr cmd_list, int offset) {
	assert(false);
}

void statement_t::asm_gen_exit_code(asm_cmd_list_ptr cmd_list) {}

void stmt_block_t::print_l(ostream& os, int level) {
	os << '{' << endl;
	if (!sym_table->empty()) {
		sym_table->print_l(os, level + 1);
		if (!statements.empty())
			os << endl;
	}
	for each (auto var in statements) {
		print_level(os, level + 1);
		var->print_l(os, level + 1);
		os << endl;
	}
	print_level(os, level);
	os << '}';
}

void stmt_block_t::asm_generate_code(asm_cmd_list_ptr cmd_list, int offset) {
	if (statements.empty())
		return;
	asm_gen_entry_code(cmd_list, offset);
	asm_gen_internal_code(cmd_list);
	asm_gen_exit_code(cmd_list);
}

void stmt_block_t::asm_gen_entry_code(asm_cmd_list_ptr cmd_list, int offset) {
	vars_size = asm_gen_t::alignment(sym_table->get_local_vars_size());
	sym_table->asm_set_offset_for_local_vars(offset - vars_size + asm_gen_t::size_of(AMT_DWORD), AR_EBP);
	cmd_list->_alloc_in_stack(vars_size);
	sym_table->asm_init_local_vars(cmd_list);
}

void stmt_block_t::asm_gen_internal_code(asm_cmd_list_ptr cmd_list, int offset) {
	offset -= vars_size;
	for each (auto stmt in statements)
		stmt->asm_generate_code(cmd_list, offset);
}

void stmt_block_t::asm_gen_exit_code(asm_cmd_list_ptr cmd_list) {
	cmd_list->_free_in_stack(vars_size);
}

void stmt_block_t::add_statement(stmt_ptr stmt) {
	statements.push_back(stmt);
}

sym_table_ptr stmt_block_t::get_sym_table() {
	return sym_table;
}

void stmt_expr_t::print_l(ostream& os, int level) {
	os << "expression: ";
	expression->short_print(os);
	os << ';';
}

void stmt_expr_t::asm_gen_internal_code(asm_cmd_list_ptr cmd_list, int offset) {
	expression->asm_gen_code(cmd_list, false);
}

void stmt_decl_t::print_l(ostream& os, int level) {
	os << "declaration: ";
	symbol->short_print_l(os, level);
	os << ';';
}

void stmt_if_t::asm_gen_internal_code(asm_cmd_list_ptr cmd_list, int offset) {
	if (!then_stmt && !else_stmt) {
		condition->asm_gen_code(cmd_list, false);
		return;
	} else
		condition->asm_gen_code(cmd_list, true);
	cmd_list->test(AR_EAX, AR_EAX);
	asm_label_ptr else_label = cmd_list->_new_label();
	asm_label_ptr exit_label = cmd_list->_new_label();
	if (then_stmt) {
		cmd_list->jz(else_stmt ? else_label : exit_label);
		then_stmt->asm_generate_code(cmd_list);
		if (else_stmt)
			cmd_list->jmp(exit_label);
	} else
		cmd_list->jnz(exit_label);
	if (else_stmt) {
		cmd_list->_insert_label(else_label);
		else_stmt->asm_generate_code(cmd_list);
	}
	cmd_list->_insert_label(exit_label);
}

void stmt_if_t::print_l(ostream& os, int level) {
	short_print_l(os, level);
	os << " (";
	condition->short_print(os);
	os << ") ";
	if (then_stmt) {
		if (typeid(*then_stmt.get()) != typeid(stmt_block_t)) {
			os << endl;
			print_level(os, level + 1);
			then_stmt->print_l(os, level + 1);
		} else
			then_stmt->print_l(os, level);
	} else
		os << ';';
	if (else_stmt) {
		if (typeid(*then_stmt.get()) != typeid(stmt_block_t)) {
			os << endl;
			print_level(os, level);
		} else
			os << ' ';
		os << "else ";
		if (typeid(*else_stmt.get()) != typeid(stmt_block_t)) {
			os << endl;
			print_level(os, level + 1);
			else_stmt->print_l(os, level + 1);
		} else 
			else_stmt->print_l(os, level);
	}
}

void stmt_loop_t::set_statement(stmt_ptr statement) {
	stmt = statement;
}

void stmt_loop_t::asm_gen_entry_code(asm_cmd_list_ptr cmd_list, int offset) {
	if (stmt)
		stmt->asm_gen_entry_code(cmd_list, offset);
}

void stmt_loop_t::asm_gen_exit_code(asm_cmd_list_ptr cmd_list) {
	if (stmt)
		stmt->asm_gen_exit_code(cmd_list);
}

void stmt_loop_t::asm_gen_jmp_to_loop(asm_cmd_list_ptr cmd_list) {
	cmd_list->jmp(loop_label);
}

void stmt_loop_t::asm_gen_jmp_to_exit_loop(asm_cmd_list_ptr cmd_list) {
	cmd_list->jmp(exit_loop_label);
}

void stmt_while_t::print_l(ostream& os, int level) {
	os << " (";
	condition->short_print(os);
	os << ") ";
	if (stmt) {
		if (typeid(*stmt.get()) != typeid(stmt_block_t)) {
			os << endl;
			print_level(os, level + 1);
			stmt->print_l(os, level + 1);
		} else
			stmt->print_l(os, level);
	} else
		os << ';' << endl;
}

void stmt_while_t::asm_gen_internal_code(asm_cmd_list_ptr cmd_list, int offset) {
	if (!stmt) {
		condition->asm_gen_code(cmd_list, false);
		return;
	}
	loop_label = cmd_list->_insert_new_nabel();
	exit_loop_label = cmd_list->_new_label();
	condition->asm_gen_code(cmd_list, true);
	cmd_list->test(AR_EAX, AR_EAX);
	cmd_list->jz(exit_loop_label);
	stmt->asm_gen_internal_code(cmd_list, offset);
	cmd_list->jmp(loop_label);
	cmd_list->_insert_label(exit_loop_label);
}

void stmt_for_t::print_l(ostream& os, int level) {
	short_print_l(os, level);
	os << " (";
	if (init_expr)
		init_expr->short_print(os);
	os << "; ";
	if (condition)
		condition->short_print(os);
	os << "; ";
	if (expr)
		expr->short_print(os);
	os << ") ";
	if (stmt) {
		if (typeid(*stmt.get()) != typeid(stmt_block_t)) {
			os << endl;
			print_level(os, level + 1);
			stmt->print_l(os, level + 1);
		} else
			stmt->print_l(os, level);
	} else
		os << ';' << endl;
}

void stmt_for_t::asm_gen_internal_code(asm_cmd_list_ptr cmd_list, int offset) {
	init_expr->asm_gen_code(cmd_list, false);
	loop_label = cmd_list->_insert_new_nabel();
	exit_loop_label = cmd_list->_new_label();
	condition->asm_gen_code(cmd_list, true);
	cmd_list->test(AR_EAX, AR_EAX);
	cmd_list->jz(exit_loop_label);
	stmt->asm_gen_internal_code(cmd_list, offset);
	expr->asm_gen_code(cmd_list, false);
	cmd_list->jmp(loop_label);
	cmd_list->_insert_label(exit_loop_label);
}

void stmt_break_t::asm_gen_internal_code(asm_cmd_list_ptr cmd_list, int offset) {
	parent->asm_gen_jmp_to_exit_loop(cmd_list);
}

void stmt_continue_t::asm_gen_internal_code(asm_cmd_list_ptr cmd_list, int offset) {
	parent->asm_gen_jmp_to_loop(cmd_list);
}

void stmt_return_t::set_ret_expr(expr_t* expr_) {
	auto func_type = parent->get_func_type();
	expr = auto_convert(expr_, func_type->get_element_type());
}

void stmt_return_t::asm_gen_internal_code(asm_cmd_list_ptr cmd_list, int offset) {
	expr->asm_gen_code(cmd_list, true);
	cmd_list->mov(AR_ESP, AR_EBP);
	cmd_list->ret();
}
