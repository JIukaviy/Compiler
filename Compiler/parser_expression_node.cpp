#include "parser_expression_node.h"

expr_bin_op_t::expr_bin_op_t(expr_t* left_, expr_t* right_, token_ptr op_) : left(left_), right(right_), op(op_) {}
expr_un_op_t::expr_un_op_t(expr_t* expr_, token_ptr op_) : expr(expr_), op(op_) {}
expr_func_t::expr_func_t(expr_t * expr_, vector<expr_t*> args_) : func(expr_), args(args_) {}
expr_struct_access_t::expr_struct_access_t(expr_t* expr_, token_ptr op_, token_ptr ident_) : expr(expr_), op(op_), member(ident_) {}
expr_arr_index_t::expr_arr_index_t(expr_t * left_, expr_t * right_, token_ptr sqr_bracket) : arr(left_), index(right_), sqr_bracket(sqr_bracket) {}
expr_var_t::expr_var_t(token_ptr variable_) : variable(variable_) {}
expr_const_t::expr_const_t(token_ptr constant_) : constant(constant_) {}
expr_tern_op_t::expr_tern_op_t(expr_t* left_, expr_t* middle_, expr_t* right_, token_ptr qm, token_ptr c) : left(left_), middle(middle_), right(right_), question_mark(qm), colon(c) {}
expr_cast_t::expr_cast_t(expr_t* expr, node_ptr type) : expr(expr), type(type) {}

void expr_bin_op_t::print_l(ostream& os, int level) {
	left->print_l(os, level + 1);
	print_level(os, level);
	op->short_print(os);
	os << endl;
	right->print_l(os, level + 1);
}

void expr_bin_op_t::short_print_l(ostream& os, int level) {
	print_level(os, level);
	left->short_print(os);
	op->short_print(os);
	right->short_print(os);
}

expr_t* expr_bin_op_t::get_left() {
	return left;
}

expr_t* expr_bin_op_t::get_right() {
	return right;
}

void expr_bin_op_t::set_left(expr_t* e) {
	left = e;
}

void expr_bin_op_t::set_right(expr_t* e) {
	right = e;
}

token_ptr expr_bin_op_t::get_op() {
	return op;
}

pos_t expr_bin_op_t::get_pos() {
	return op->get_pos();
}

void expr_tern_op_t::print_l(ostream& os, int level) {
	left->print_l(os, level + 1);
	print_level(os, level);
	os << "?" << endl;
	middle->print_l(os, level + 1);
	print_level(os, level);
	os << ":" << endl;
	right->print_l(os, level + 1);
}

void expr_tern_op_t::short_print_l(ostream& os, int level) {
	print_level(os, level);
	left->short_print(os);
	os << " ? ";
	middle->short_print(os);
	os << " : ";
	right->short_print(os);
}

expr_t * expr_tern_op_t::get_left() {
	return left;
}

expr_t * expr_tern_op_t::get_middle() {
	return middle;
}

expr_t * expr_tern_op_t::get_right() {
	return right;
}

token_ptr expr_tern_op_t::get_question_mark_token() {
	return question_mark;
}

token_ptr expr_tern_op_t::get_colon_token() {
	return colon;
}

void expr_tern_op_t::set_left(expr_t* e) {
	left = e;
}

void expr_tern_op_t::set_middle(expr_t* e) {
	middle = e;
}

void expr_tern_op_t::set_right(expr_t * e) {
	right = e;
}

void expr_var_t::print_l(ostream& os, int level) {
	print_level(os, level);
	variable->short_print(os);
	os << endl;
}

void expr_var_t::short_print_l(ostream& os, int level) {
	print_level(os, level);
	variable->short_print(os);
}

token_ptr expr_var_t::get_token() {
	return variable;
}

void expr_const_t::print_l(ostream& os, int level) {
	print_level(os, level);
	constant->short_print(os);
	os << endl;
}

void expr_const_t::short_print_l(ostream& os, int level) {
	print_level(os, level);
	constant->short_print(os);
}

token_ptr expr_const_t::get_token() {
	return constant;
}

void expr_un_op_t::print_l(ostream& os, int level) {
	print_level(os, level);
	op->short_print(os);
	os << endl;
	expr->print_l(os, level + 1);
}

void expr_un_op_t::short_print_l(ostream& os, int level) {
	print_level(os, level);
	op->short_print(os);
	expr->short_print(os);
}

expr_t * expr_un_op_t::get_expr() {
	return expr;
}

void expr_un_op_t::set_expr(expr_t* e) {
	expr = e;
}

token_ptr expr_un_op_t::get_op() {
	return op;
}

void expr_prefix_un_op_t::print_l(ostream& os, int level) {
	print_level(os, level);
	os << "prefix ";
	op->short_print(os);
	os << endl;
	expr->print_l(os, level + 1);
}

void expr_prefix_un_op_t::short_print_l(ostream& os, int level) {
	print_level(os, level);
	op->short_print(os);
	expr->short_print(os);
}

void expr_postfix_un_op_t::print_l(ostream& os, int level) {
	print_level(os, level);
	os << "postfix ";
	op->short_print(os);
	os << endl;
	expr->print_l(os, level + 1);
}

void expr_postfix_un_op_t::short_print_l(ostream& os, int level) {
	print_level(os, level);
	expr->short_print(os);
	op->short_print(os);
}

void expr_arr_index_t::print_l(ostream& os, int level) {
	arr->print_l(os, level + 1);
	print_level(os, level);
	os << "[]" << endl;
	index->print_l(os, level + 1);
}

void expr_arr_index_t::short_print_l(ostream& os, int level) {
	print_level(os, level);
	arr->short_print(os);
	os << "[";
	index->short_print(os);
	os << "]";
}

token_ptr expr_arr_index_t::get_sqr_bracket_token() {
	return sqr_bracket;
}

expr_t * expr_arr_index_t::get_arr() {
	return arr;
}

expr_t * expr_arr_index_t::get_index() {
	return index;
}

void expr_arr_index_t::set_index(expr_t* e) {
	index = e;
}

void expr_arr_index_t::set_arr(expr_t * e) {
	arr = e;
}

void expr_struct_access_t::print_l(ostream& os, int level) {
	expr->print_l(os, level + 1);
	print_level(os, level);
	op->short_print(os);
	os << endl;
	print_level(os, level + 1);
	member->short_print(os);
	os << endl;
}

void expr_struct_access_t::short_print_l(ostream& os, int level) {
	print_level(os, level);
	expr->short_print(os);
	op->short_print(os);
	member->short_print(os);
}

expr_t * expr_struct_access_t::get_expr() {
	return expr;
}

void expr_struct_access_t::set_expr(expr_t* e) {
	expr = e;
}

token_ptr expr_struct_access_t::get_op() {
	return op;
}

token_ptr expr_struct_access_t::get_member() {
	return member;
}

void expr_func_t::print_l(ostream &os, int level) {
	func->print_l(os, level + 1);
	print_level(os, level);
	os << "()" << endl;
	for (int i = 0; i < args.size(); i++)
		args[i]->print_l(os, level + 1);
}

void expr_func_t::short_print_l(ostream& os, int level) {
	print_level(os, level);
	func->short_print(os);
	os << "(";
	for (int i = 0; i < args.size(); i++) {
		args[i]->short_print(os);
		if (i != args.size() - 1)
			os << ", ";
	}
	os << ")";
}

void expr_cast_t::print_l(ostream& os, int level) {
	print_level(os, level);
	os << "cast to (";
	type->print(os);
	os << ')' << endl;
	expr->print_l(os, level + 1);
}

void expr_cast_t::short_print_l(ostream& os, int level) {
	print_level(os, level);
	os << "(";
	type->short_print(os);
	os << ") ";
	expr->short_print(os);
}