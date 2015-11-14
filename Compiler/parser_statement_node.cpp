#include "parser_statement_node.h"

stmt_expr_t::stmt_expr_t(expr_t * expression) : expression(expression) {}
stmt_block_t::stmt_block_t() {}
stmt_block_t::stmt_block_t(const vector<stmt_ptr>& statements, sym_table_t* sym_table) : statements(statements), sym_table(sym_table) {}
stmt_decl_t::stmt_decl_t(sym_ptr symbol) : symbol(symbol) {}
stmt_if_t::stmt_if_t(expr_t* condition, stmt_ptr then_stmt) : condition(condition), then_stmt(then_stmt), else_stmt(0) {}
stmt_if_t::stmt_if_t(expr_t* condition, stmt_ptr then_stmt, stmt_ptr else_stmt) : condition(condition), then_stmt(then_stmt), else_stmt(else_stmt) {}
stmt_loop_t::stmt_loop_t(stmt_ptr stmt) : stmt(stmt) {}
stmt_loop_t::stmt_loop_t() : stmt(0) {}
stmt_while_t::stmt_while_t(expr_t* condition, stmt_ptr stmt) : condition(condition), stmt_loop_t(stmt) {}
stmt_while_t::stmt_while_t(expr_t* condition) : condition(condition) {}
stmt_for_t::stmt_for_t(expr_t* init_expr, expr_t* condition, expr_t* expr, stmt_ptr stmt) : init_expr(init_expr), condition(condition), expr(expr), stmt_loop_t(stmt) {}
stmt_for_t::stmt_for_t(expr_t* init_expr, expr_t* condition, expr_t* expr) : init_expr(init_expr), condition(condition), expr(expr) {}

void statement_t::short_print(ostream& os) {
	short_print(os, 0);
}

void statement_t::short_print(ostream& os, int level) {
	print(os);
}

void statement_t::print(ostream& os) {
	print(os, 0);
}

void stmt_block_t::print(ostream& os, int level) {
	print_level(os, level);
	os << '{' << endl;
	if (!sym_table->empty()) {
		sym_table->print(os, level + 1);
		if (!statements.empty())
			os << endl;
	}
	for each (auto var in statements)
		var->print(os, level + 1);
	print_level(os, level);
	os << '}' << endl;
}

void stmt_block_t::add_statement(stmt_ptr stmt) {
	statements.push_back(stmt);
}

sym_table_t* stmt_block_t::get_sym_table() {
	return sym_table;
}

void stmt_expr_t::print(ostream& os, int level) {
	print_level(os, level);
	os << "expression: ";
	expression->short_print(os);
	os << ';' << endl;
}

void stmt_decl_t::print(ostream& os, int level) {
	print_level(os, level);
	os << "declaration: ";
	symbol->short_print(os);
	os << ';' << endl;
}

void stmt_if_t::print(ostream& os, int level) {
	short_print(os, level);
	os << " (";
	condition->short_print(os);
	os << ')';
	if (then_stmt) {
		os << endl;
		then_stmt->print(os, level + 1);
	} else
		os << ';' << endl;
	if (else_stmt) {
		print_level(os, level);
		os << "else" << endl;
		else_stmt->print(os, level+1);
	}
}

void stmt_loop_t::set_statement(stmt_ptr statement) {
	stmt = statement;
}

void stmt_while_t::print(ostream& os, int level) {
	short_print(os, level);
	os << " (";
	condition->short_print(os);
	os << ')';
	if (stmt) {
		os << endl;
		stmt->print(os, level + 1);
	} else
		os << ';' << endl;
}

void stmt_for_t::print(ostream& os, int level) {
	short_print(os, level);
	os << " (";
	if (init_expr)
		init_expr->short_print(os);
	os << "; ";
	if (condition)
		condition->short_print(os);
	os << "; ";
	if (expr)
		expr->short_print(os);
	os << ')';
	if (stmt) {
		os << endl;
		stmt->print(os, level + 1);
	} else
		os << ';' << endl;
}