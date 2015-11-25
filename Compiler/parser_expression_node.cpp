#include "parser.h"
#include "exceptions.h"
#include "type_conversion.h"

//-----------------------------------EXPRESSIONS-----------------------------------

expr_t::expr_t(bool lvalue) : lvalue(lvalue) {}

bool expr_t::is_lvalue() {
	return lvalue;
}

//-----------------------------------VARIABLE-----------------------------------

expr_var_t::expr_var_t() : expr_t(true) {}

void expr_var_t::print_l(ostream& os, int level) {
	print_level(os, level);
	variable->short_print(os);
	os << endl;
}

void expr_var_t::short_print_l(ostream& os, int level) {
	print_level(os, level);
	os << variable->get_name();
}

void expr_var_t::set_symbol(shared_ptr<sym_with_type_t> var) {
	if (var->is(ST_ALIAS))
		throw ExpressionIsExpected(var);
	assert(var->is(ST_VAR) || var->is(ST_FUNC));
	variable = var;
}

token_ptr expr_var_t::get_token() {
	return variable->get_token();
}

type_ptr expr_var_t::get_type() {
	return variable->get_type();
}

pos_t expr_var_t::get_pos() {
	return variable->get_token()->get_pos();
}

//-----------------------------------CONSTANT-----------------------------------

expr_const_t::expr_const_t(token_ptr constant_) : constant(constant_) {}

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

type_ptr expr_const_t::get_type() {
	return type_ptr(new type_t(parser_t::get_type(symbol_t::token_to_sym_type(constant->get_token_id()))));
}

pos_t expr_const_t::get_pos() {
	return constant->get_pos();
}

bool expr_const_t::is_null() {
	return static_pointer_cast<token_base_with_value_t>(constant)->is_null();
}

//-----------------------------------TERNARY_OPERATOR-----------------------------------

expr_tern_op_t::expr_tern_op_t(token_ptr qm, token_ptr c) : question_mark(qm), colon(c) {}

void expr_tern_op_t::print_l(ostream& os, int level) {
	condition->print_l(os, level + 1);
	print_level(os, level);
	os << "?" << endl;
	left->print_l(os, level + 1);
	print_level(os, level);
	os << ":" << endl;
	right->print_l(os, level + 1);
}

void expr_tern_op_t::short_print_l(ostream& os, int level) {
	print_level(os, level);
	condition->short_print(os);
	os << " ? ";
	left->short_print(os);
	os << " : ";
	right->short_print(os);
}

expr_t * expr_tern_op_t::get_condition() {
	return condition;
}

expr_t * expr_tern_op_t::get_left() {
	return left;
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

type_ptr expr_tern_op_t::get_type() {
	return left->get_type();
}


//-----------------------------------BINARY_OPERATORS-----------------------------------

expr_bin_op_t::expr_bin_op_t(token_ptr op) : left(0), right(0), op(op) {}

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

void expr_bin_op_t::set_operands(expr_t* left_, expr_t* right_) {
	bool or_passed = false;
	for each (auto operands_checker in and_conditions)
		operands_checker(left_, right_, this);
	for each (auto operands_checker in or_conditions)
		if (or_passed = operands_checker(left_, right_))
			break;
	if (!or_passed)
		throw InvalidBinOpOperands(left_->get_type(), right_->get_type(), this);
	for (int i = type_convertions.size() - 1; i >= 0; i--)
		if (type_convertions[i](&left_, &right_, this))
			break;
	left = left_;
	right = right_;
}

token_ptr expr_bin_op_t::get_op() {
	return op;
}

pos_t expr_bin_op_t::get_pos() {
	return op->get_pos();
}

type_ptr expr_bin_op_t::get_type() {
	return right->get_type();
}

template<class T>
inline expr_bin_op_t* new_bin_op(token_ptr op) {
	return new T(op);
}

expr_bin_op_t* expr_bin_op_t::make_bin_op(token_ptr op) {
	return
		op->is(T_OP_MUL, T_OP_DIV, 0) ? new_bin_op<expr_ariphmetic_bin_op_t>(op) :
		op == T_OP_ADD ? new_bin_op<expr_add_bin_op_t>(op) :
		op == T_OP_SUB ? new_bin_op<expr_sub_bin_op_t>(op) :
		op == T_OP_MOD ? new_bin_op<expr_mod_bin_op_t>(op) :
		op == T_OP_ADD_ASSIGN ? new_bin_op<expr_add_assign_bin_op_t>(op) :
		op == T_OP_SUB_ASSIGN ? new_bin_op<expr_sub_assign_bin_op_t>(op) :
		op == T_OP_MOD_ASSIGN ? new_bin_op<expr_mod_assign_bin_op_t>(op) :
		op->is(T_OP_L, T_OP_LE, T_OP_G, T_OP_GE, 0) ? new_bin_op<expr_relational_bin_op_t>(op) :
		op->is(T_OP_EQ, T_OP_NEQ, 0) ? new_bin_op<expr_equality_bin_op_t>(op) :
		op->is(T_OP_AND, T_OP_OR) ? new_bin_op<expr_logical_bin_op_t>(op) :
		op->is(T_OP_BIT_AND, T_OP_BIT_OR, T_OP_XOR) ? new_bin_op<expr_integer_bin_op_t>(op) :
		op->is(T_OP_BIT_AND_ASSIGN, T_OP_BIT_OR_ASSIGN, T_OP_XOR_ASSIGN) ? new_bin_op<expr_integer_assign_bin_op_t>(op) :
		(assert(false), nullptr);
}

//-------------------------------OPERANDS_CHECKERS--------------------------------

void oc_bo_is_lvalue(expr_t* left, expr_t* right, expr_bin_op_t* op) {
	if (!left->is_lvalue())
		throw ExprMustBeLValue(left->get_pos());
}

void oc_bo_not_constant(expr_t* left, expr_t* right, expr_bin_op_t* op) {
	if (left->get_type()->is_const())
		throw AssignmentToReadOnly(op->get_pos());
}

bool oc_bo_is_integer(expr_t* left, expr_t* right) {
	return left->get_type()->is_integer() && right->get_type()->is_integer();
}

bool oc_bo_is_ariphmetic(expr_t* left, expr_t* right) {
	return left->get_type()->is_ariphmetic() && right->get_type()->is_ariphmetic();
}

bool oc_bo_is_ptrs_to_same_types(expr_t* left, expr_t* right) {
	return 
		(left->get_type() == ST_PTR && right->get_type() == ST_PTR) && 
		(static_pointer_cast<sym_type_ptr_t>(left->get_type()->get_base_type())->get_element_type()->get_base_type() == 
		 static_pointer_cast<sym_type_ptr_t>(right->get_type()->get_base_type())->get_element_type()->get_base_type());
}

bool oc_bo_is_ariphmetic_or_ptr(expr_t* left, expr_t* right) {
	return 
		(left->get_type() == ST_PTR || left->get_type()->is_ariphmetic()) &&
		(right->get_type() == ST_PTR || right->get_type()->is_ariphmetic());
}

bool oc_bo_ptr_and_integer(expr_t* left, expr_t* right) {
	return left->get_type() == ST_PTR && right->get_type()->is_integer();
}

bool oc_bo_integer_and_ptr(expr_t* left, expr_t* right) {
	return left->get_type()->is_integer() && right->get_type() == ST_PTR;
}

//--------------------------------TYPE_CONVERTIONS--------------------------------

bool tc_bo_align_types(expr_t** left, expr_t** right, expr_bin_op_t* op) {
	align_types(left, right);
	return true;
}

bool tc_bo_left_to_right_type(expr_t** left, expr_t** right, expr_bin_op_t* op) {
	*right = auto_convert(*right, (*left)->get_type());
	return true;
}

bool tc_bo_integer_increase(expr_t** left, expr_t** right, expr_bin_op_t* op) {
	*left = integer_increase(*left);
	*right = integer_increase(*right);
	return true;
}

bool tc_bo_pass_ptrs(expr_t** left, expr_t** right, expr_bin_op_t* op) {
	return (*left)->get_type() == ST_PTR || (*right)->get_type() == ST_PTR;
}

bool tc_bo_int_to_ptr(expr_t** left, expr_t** right, expr_bin_op_t* op) {
	if ((*left)->get_type() == ST_PTR || (*right)->get_type() == ST_PTR) {
		type_ptr ptr = (*left)->get_type() == ST_PTR ? (*left)->get_type() : (*right)->get_type();
		*left = auto_convert(*left, ptr);
		*right = auto_convert(*right, ptr);
		return true;
	}
	return false;
}

bool tc_bo_ptr_to_ariphmetic(expr_t** left, expr_t** right, expr_bin_op_t* op) {
	if ((*left)->get_type() == ST_PTR)
		*left = auto_convert(*left, parser_t::get_type(ST_INTEGER));
	if ((*right)->get_type() == ST_PTR)
		*right = auto_convert(*right, parser_t::get_type(ST_INTEGER));
	return false;
}

//-----------------------------------ASSIGN---------------------------------------------

expr_assign_bin_op_t::expr_assign_bin_op_t(token_ptr op) : expr_bin_op_t(op) {
	and_conditions.push_back(oc_bo_is_lvalue);
	and_conditions.push_back(oc_bo_not_constant);
	type_convertions.push_back(tc_bo_left_to_right_type);
}

//-----------------------------------INTEGER_OPERATORS-----------------------------------

expr_integer_bin_op_t::expr_integer_bin_op_t(token_ptr token) : expr_bin_op_t(token) {
	or_conditions.push_back(oc_bo_is_integer);
	type_convertions.push_back(tc_bo_align_types);
}

expr_integer_assign_bin_op_t::expr_integer_assign_bin_op_t(token_ptr token) : expr_assign_bin_op_t(token) {
	or_conditions.push_back(oc_bo_is_integer);
}

//-----------------------------------ARIPHMETIC_OPERATORS-----------------------------------

expr_ariphmetic_bin_op_t::expr_ariphmetic_bin_op_t(token_ptr token) : expr_bin_op_t(token) {
	or_conditions.push_back(oc_bo_is_ariphmetic);
	type_convertions.push_back(tc_bo_align_types);
}

expr_ariphmetic_assign_bin_op_t::expr_ariphmetic_assign_bin_op_t(token_ptr token) : expr_assign_bin_op_t(token) {
	or_conditions.push_back(oc_bo_is_ariphmetic);
}

//--------------------------------------ADD----------------------------------------------

expr_add_bin_op_t::expr_add_bin_op_t(token_ptr op) : expr_bin_op_t(op) {
	or_conditions.push_back(oc_bo_is_ariphmetic);
	or_conditions.push_back(oc_bo_ptr_and_integer);
	or_conditions.push_back(oc_bo_integer_and_ptr);
	type_convertions.push_back(tc_bo_align_types);
	type_convertions.push_back(tc_bo_pass_ptrs);
}

expr_add_assign_bin_op_t::expr_add_assign_bin_op_t(token_ptr op) : expr_assign_bin_op_t(op) {
	or_conditions.push_back(oc_bo_is_ariphmetic);
	or_conditions.push_back(oc_bo_ptr_and_integer);
	or_conditions.push_back(oc_bo_integer_and_ptr);
	type_convertions.push_back(tc_bo_pass_ptrs);
}

//--------------------------------------SUB----------------------------------------------

expr_sub_bin_op_t::expr_sub_bin_op_t(token_ptr op) : expr_bin_op_t(op) {
	or_conditions.push_back(oc_bo_is_ariphmetic);
	or_conditions.push_back(oc_bo_is_ptrs_to_same_types);
	or_conditions.push_back(oc_bo_ptr_and_integer);
	type_convertions.push_back(tc_bo_align_types);
	type_convertions.push_back(tc_bo_pass_ptrs);
}

expr_sub_assign_bin_op_t::expr_sub_assign_bin_op_t(token_ptr op) : expr_assign_bin_op_t(op) {
	or_conditions.push_back(oc_bo_is_ariphmetic);
	or_conditions.push_back(oc_bo_is_ptrs_to_same_types);
	or_conditions.push_back(oc_bo_ptr_and_integer);
	type_convertions.push_back(tc_bo_pass_ptrs);
}

//--------------------------------------MOD----------------------------------------------

expr_mod_bin_op_t::expr_mod_bin_op_t(token_ptr op) : expr_bin_op_t(op) {
	or_conditions.push_back(oc_bo_is_integer);
	type_convertions.push_back(tc_bo_left_to_right_type);
}

expr_mod_assign_bin_op_t::expr_mod_assign_bin_op_t(token_ptr op) : expr_assign_bin_op_t(op) {
	or_conditions.push_back(oc_bo_is_integer);
}

//--------------------------------------RELATIONAL_OPERATORS----------------------------------------------

expr_relational_bin_op_t::expr_relational_bin_op_t(token_ptr op) : expr_ariphmetic_bin_op_t(op) {
	or_conditions.push_back(oc_bo_is_ptrs_to_same_types);
	type_convertions.push_back(tc_bo_int_to_ptr);
}

type_ptr expr_relational_bin_op_t::get_type() {
	return parser_t::get_type(ST_INTEGER);
}

//--------------------------------------EQUALITY_OPERATORS----------------------------------------------

expr_equality_bin_op_t::expr_equality_bin_op_t(token_ptr op) : expr_relational_bin_op_t(op) {
	or_conditions.push_back(oc_bo_ptr_and_integer);
	or_conditions.push_back(oc_bo_integer_and_ptr);
}

//--------------------------------------LOGICAL_OPERATORS----------------------------------------------

expr_logical_bin_op_t::expr_logical_bin_op_t(token_ptr op) : expr_bin_op_t(op) {
	or_conditions.push_back(oc_bo_is_ariphmetic_or_ptr);
	type_convertions.push_back(tc_bo_align_types);
	type_convertions.push_back(tc_bo_ptr_to_ariphmetic);
}

//--------------------------------------SHIFT_OPERATORS----------------------------------------------

expr_shift_bin_op_t::expr_shift_bin_op_t(token_ptr op) : expr_bin_op_t(op) {
	or_conditions.push_back(oc_bo_is_integer);
	type_convertions.push_back(tc_bo_integer_increase);
}

expr_shift_assign_bin_op_t::expr_shift_assign_bin_op_t(token_ptr op) : expr_assign_bin_op_t(op) {
	or_conditions.push_back(oc_bo_is_integer);
}

//-----------------------------------UNARY_OPERATOR-----------------------------------

expr_un_op_t::expr_un_op_t(token_ptr op, bool lvalue) : op(op), expr_t(lvalue) {}

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

void expr_un_op_t::set_operand(expr_t* operand) {
	for each (auto operands_checker in operand_checkers)
		operands_checker(operand, this);
	for each (auto type_cast in type_convertions)
		if (type_cast(&operand, this))
			break;
	expr = operand;
}

token_ptr expr_un_op_t::get_op() {
	return op;
}

type_ptr expr_un_op_t::get_type() {
	return expr->get_type();
}

//-----------------------------------OPERAND_CHECKERS---------------------------------

void oc_uo_is_lvalue(expr_t* operand, expr_un_op_t* op) {
	if (!operand->is_lvalue())
		throw ExprMustBeLValue(operand->get_pos());
}

void oc_uo_is_ariphmetic(expr_t* operand, expr_un_op_t* op) {
	if (!operand->get_type()->is_ariphmetic())
		throw InvalidUnOpOperand(operand->get_type(), op);
}

void oc_uo_not_constant(expr_t* operand, expr_un_op_t* op) {
	if (operand->get_type()->is_const())
		throw AssignmentToReadOnly(op->get_pos());
}

void oc_uo_is_ptr(expr_t* operand, expr_un_op_t* op) {
	if (!operand->get_type() != ST_PTR)
		throw InvalidUnOpOperand(operand->get_type(), op);
}

void oc_uo_is_ariphmetic_or_ptr(expr_t* operand, expr_un_op_t* op) {
	if (!operand->get_type()->is_ariphmetic() && !operand->get_type() != ST_PTR)
		throw InvalidUnOpOperand(operand->get_type(), op);
}

//-----------------------------------TYPE_CONVERTIONS---------------------------------

// пока пусто

//-----------------------------------PREFIX_UNARY_OPERATOR-----------------------------------

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

//-----------------------------------GET_ADRESS-----------------------------------

expr_addr_op_t::expr_addr_op_t(token_ptr op) : expr_prefix_un_op_t(op) {
	operand_checkers.push_back(oc_uo_is_lvalue);
}

//-----------------------------------DEREFERENCE-----------------------------------

expr_dereference_op_t::expr_dereference_op_t(token_ptr op) : expr_prefix_un_op_t(op, true) {
	operand_checkers.push_back(oc_uo_is_ptr);
}

//-----------------------------------PREFIX_INC_DEC-----------------------------------

expr_prefix_inc_dec_op_t::expr_prefix_inc_dec_op_t(token_ptr op) : expr_prefix_un_op_t(op) {
	operand_checkers.push_back(oc_uo_is_lvalue);
	operand_checkers.push_back(oc_uo_not_constant);
	operand_checkers.push_back(oc_uo_is_ariphmetic_or_ptr);
}

//-----------------------------------POSTFIX_UNARY_OPERATOR-----------------------------------

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

//-----------------------------------POSTFIX_INC_DEC-----------------------------------

expr_postfix_inc_dec_op_t::expr_postfix_inc_dec_op_t(token_ptr op) : expr_postfix_un_op_t(op) {
	operand_checkers.push_back(oc_uo_is_lvalue);
	operand_checkers.push_back(oc_uo_not_constant);
	operand_checkers.push_back(oc_uo_is_ariphmetic_or_ptr);
}

//-----------------------------------ARRAY_INDEX-----------------------------------

expr_arr_index_t::expr_arr_index_t(token_ptr sqr_bracket) : sqr_bracket(sqr_bracket) {}

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

type_ptr expr_arr_index_t::get_type() {
	return arr->get_type();
}

//-----------------------------------STRUCT_ACCESS-----------------------------------

expr_struct_access_t::expr_struct_access_t(token_ptr op) : op(op) {}

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

expr_t* expr_struct_access_t::get_expr() {
	return expr;
}

token_ptr expr_struct_access_t::get_op() {
	return op;
}

type_ptr expr_struct_access_t::get_type() {
	return member->get_type();
}

//-----------------------------------FUNCTION_CALL-----------------------------------

expr_func_t::expr_func_t(expr_t * expr_, vector<expr_t*> args_) : func(expr_), args(args_) {}

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

type_ptr expr_func_t::get_type() {
	return func->get_type();
}

//-----------------------------------CAST_OPERATOR-----------------------------------

expr_cast_t::expr_cast_t() : expr(0) {}

void expr_cast_t::print_l(ostream& os, int level) {
	print_level(os, level);
	os << "cast to (";
	type->print(os);
	os << ")(" << endl;
	expr->print_l(os, level + 1);
	os << ')';
}

void expr_cast_t::short_print_l(ostream& os, int level) {
	print_level(os, level);
	os << '(';
	type->short_print(os);
	os << ")(";
	expr->short_print(os);
	os << ')';
}

void expr_cast_t::set_operand(expr_t* expr_, type_ptr dst_type) {
	type_ptr src_type = expr_->get_type();
	if (src_type == ST_PTR && (dst_type == ST_PTR || dst_type->is_ariphmetic()) ||
		src_type->is_ariphmetic() && dst_type->is_ariphmetic() ||
		src_type->is_integer() && dst_type == ST_PTR) 
	{
		expr = expr_;
		type = dst_type;
	} else
		throw IllegalConversion(expr_->get_type(), dst_type, expr_->get_pos());
}

type_ptr expr_cast_t::get_type() {
	return type;
}

pos_t expr_cast_t::get_pos() {
	return expr->get_pos();
}