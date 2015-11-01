#include "parser_expression_node.h"

expr_bin_op_t::expr_bin_op_t(expr_t* left_, expr_t* right_, token_ptr_t op_) : left(left_), right(right_), op(op_) {}
expr_un_op_t::expr_un_op_t(expr_t* expr_, token_ptr_t op_) : expr(expr_), op(op_) {}
expr_func_t::expr_func_t(expr_t * expr_, vector<expr_t*> args_) : func(expr_), args(args_) {}
expr_struct_access_t::expr_struct_access_t(expr_t* expr_, token_ptr_t op_, token_ptr_t ident_) : expr(expr_), op(op_), ident(ident_) {}
expr_arr_index_t::expr_arr_index_t(expr_t * left_, expr_t * right_) : left(left_), right(right_) {}
expr_var_t::expr_var_t(token_ptr_t variable_) : variable(variable_) {}
expr_const_t::expr_const_t(token_ptr_t constant_) : constant(constant_) {}
expr_tern_op_t::expr_tern_op_t(expr_t* left_, expr_t* middle_, expr_t* right_) : left(left_), middle(middle_), right(right_) {}

void print_level(ostream& os, int level) {
	for (int i = 0; i < level; i++)
		os << '\t';
}

void expr_t::print(ostream& os) {
	print(os, 0);
}

void expr_bin_op_t::print(ostream& os, int level) {
	left->print(os, level + 1);
	print_level(os, level);
	op->short_print(os);
	os << endl;
	right->print(os, level + 1);
}

void expr_bin_op_t::flat_print(ostream& os) {
	left->flat_print(os);
	op->short_print(os);
	right->flat_print(os);
}

void expr_tern_op_t::print(ostream& os, int level) {
	left->print(os, level + 1);
	print_level(os, level);
	os << "?" << endl;
	middle->print(os, level + 1);
	print_level(os, level);
	os << ":" << endl;
	right->print(os, level + 1);
}

void expr_tern_op_t::flat_print(ostream& os) {
	left->flat_print(os);
	os << " ? " << endl;
	middle->flat_print(os);
	os << " : " << endl;
	right->flat_print(os);
}

void expr_var_t::print(ostream& os, int level) {
	print_level(os, level);
	variable->short_print(os);
	os << endl;
}

void expr_var_t::flat_print(ostream& os) {
	variable->short_print(os);
}

void expr_const_t::print(ostream& os, int level) {
	print_level(os, level);
	constant->short_print(os);
	os << endl;
}

void expr_const_t::flat_print(ostream& os) {
	constant->short_print(os);
}

void expr_un_op_t::print(ostream& os, int level) {
	print_level(os, level);
	op->short_print(os);
	os << endl;
	expr->print(os, level + 1);
}

void expr_un_op_t::flat_print(ostream& os) {
	op->short_print(os);
	expr->flat_print(os);
}

void expr_prefix_un_op_t::print(ostream& os, int level) {
	print_level(os, level);
	os << "prefix ";
	op->short_print(os);
	os << endl;
	expr->print(os, level + 1);
}

void expr_prefix_un_op_t::flat_print(ostream& os) {
	op->short_print(os);
	expr->flat_print(os);
}

void expr_postfix_un_op_t::print(ostream& os, int level) {
	print_level(os, level);
	os << "postfix ";
	op->short_print(os);
	os << endl;
	expr->print(os, level + 1);
}

void expr_postfix_un_op_t::flat_print(ostream& os) {
	expr->print(os);
	op->short_print(os);
}

void expr_arr_index_t::print(ostream& os, int level) {
	left->print(os, level + 1);
	print_level(os, level);
	os << "[]" << endl;
	right->print(os, level + 1);
}

void expr_arr_index_t::flat_print(ostream& os) {
	left->flat_print(os);
	os << "[";
	right->flat_print(os);
	os << "]";
}

void expr_struct_access_t::print(ostream& os, int level) {
	expr->print(os, level + 1);
	print_level(os, level);
	op->short_print(os);
	os << endl;
	print_level(os, level + 1);
	ident->short_print(os);
	os << endl;
}

void expr_struct_access_t::flat_print(ostream& os) {
	expr->flat_print(os);
	op->short_print(os);
	ident->short_print(os);
}

void expr_func_t::print(ostream &os, int level) {
	func->print(os, level + 1);
	print_level(os, level);
	os << "()" << endl;
	for (int i = 0; i < args.size(); i++)
		args[i]->print(os, level + 1);
}

void expr_func_t::flat_print(ostream &os) {
	func->flat_print(os);
	os << "(";
	for (int i = 0; i < args.size(); i++) {
		args[i]->print(os);
		if (i != args.size() - 1)
			os << ", ";
	}
	os << ")";
}