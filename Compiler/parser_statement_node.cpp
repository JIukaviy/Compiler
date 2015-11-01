#include "parser_statement_node.h"

stmt_expr_t::stmt_expr_t(expr_t * expression) : expression(expression) {}
stmt_block_t::stmt_block_t(const vector<statement_t*>& statements) : statements(statements) {}
stmt_decl_t::stmt_decl_t(symbol_t * symbol) : symbol(symbol) {}
stmt_if_t::stmt_if_t(expr_t* condition, statement_t* then_stmt) : condition(condition), then_stmt(then_stmt), else_stmt(0) {}
stmt_if_t::stmt_if_t(expr_t* condition, statement_t* then_stmt, statement_t* else_stmt) : condition(condition), then_stmt(then_stmt), else_stmt(else_stmt) {}
stmt_while_t::stmt_while_t(expr_t* condition, statement_t* stmt) : condition(condition), stmt(stmt) {}
stmt_for_t::stmt_for_t(expr_t* init_expr, expr_t* condition, expr_t* expr, statement_t* stmt) : init_expr(init_expr), condition(condition), expr(expr), stmt(stmt) {}

void statement_t::print(ostream& os) {
	print(os, 0);
}

void stmt_block_t::print(ostream& os, int level) {
	print_level(os, level);
	os << '{' << endl;
	for each (auto var in statements)
		var->print(os, level + 1);
	print_level(os, level);
	os << '}' << endl;
}

void stmt_expr_t::print(ostream& os, int level) {
	print_level(os, level);
	os << "expression: ";
	expression->flat_print(os);
	os << ';' << endl;
}

void stmt_decl_t::print(ostream& os, int level) {
	print_level(os, level);
	os << "declaration: ";
	symbol->flat_print(os);
	os << ';' << endl;
}

void stmt_if_t::print(ostream& os, int level) {
	print_level(os, level);
	os << "if (";
	condition->flat_print(os);
	os << ')' << endl;
	if (then_stmt)
		then_stmt->print(os, level+1);
	else
		os << ';' << endl;
	if (else_stmt) {
		print_level(os, level);
		os << "else" << endl;
		else_stmt->print(os, level+1);
	}
}

void stmt_while_t::print(ostream& os, int level) {
	print_level(os, level);
	os << "while (";
	condition->flat_print(os);
	os << ')' << endl;
	if (stmt)
		stmt->print(os, level+1);
	else
		os << ';' << endl;
}

void stmt_for_t::print(ostream& os, int level) {
	print_level(os, level);
	os << "for (";
	if (init_expr)
		init_expr->flat_print(os);
	os << "; ";
	if (condition)
		condition->flat_print(os);
	os << "; ";
	if (expr)
		expr->flat_print(os);
	os << ')' << endl;
	if (stmt)
		stmt->print(os, level+1);
	else
		os << ';' << endl;
}