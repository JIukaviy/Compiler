#include "parser.h"
#include "exceptions.h"
#include "type_conversion.h"

//-----------------------------------EXPRESSIONS-----------------------------------

expr_t::expr_t(bool lvalue) : lvalue(lvalue) {}

bool expr_t::is_lvalue() {
	return lvalue;
}

void expr_t::asm_get_val(asm_cmd_list_ptr cmd_list) {
	assert(false);
}

void expr_t::asm_get_addr(asm_cmd_list_ptr cmd_list) {
	assert(false);
}

var_ptr expr_t::eval() {
	throw ExprMustBeEval(get_pos());
	return var_ptr();
}

int expr_t::get_type_size() {
	return get_type()->get_size();
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

void expr_var_t::set_symbol(shared_ptr<sym_with_type_t> symbol_, token_ptr var_token_) {
	if (symbol_->is(ST_ALIAS))
		throw ExpressionIsExpected(symbol_);
	assert(symbol_->is(ST_VAR) || symbol_->is(ST_FUNC));
	variable = symbol_;
	var_token = var_token_;
	if (variable->is(ST_FUNC))
		lvalue = false;
}

token_ptr expr_var_t::get_token() {
	return variable->get_token();
}

type_ptr expr_var_t::get_type() {
	return variable->get_type();
}

shared_ptr<sym_with_type_t> expr_var_t::get_var() {
	return variable;
}

pos_t expr_var_t::get_pos() {
	return var_token->get_pos();
}

void expr_var_t::asm_get_val(asm_cmd_list_ptr cmd_list) {
	dynamic_pointer_cast<sym_var_t>(variable)->asm_get_val(cmd_list);
}

void expr_var_t::asm_get_addr(asm_cmd_list_ptr cmd_list) {
	dynamic_pointer_cast<sym_var_t>(variable)->asm_get_addr(cmd_list);
}

var_ptr expr_var_t::eval() {
	if (variable->get_type() == ST_FUNC_TYPE ||
		variable->get_type() == ST_ARRAY)
			throw ExprMustBeEval(get_pos());
	auto init_list = static_pointer_cast<sym_var_t>(variable)->get_init_list();
	if (init_list.empty())
		throw ExprMustBeEval(get_pos());
	try {
		return init_list[0]->eval();
	} catch (...) {
		throw ExprMustBeEval(get_pos());
	}
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
	if (constant == T_STRING) {
		auto arr = shared_ptr<sym_type_array_t>(new sym_type_array_t());
		arr->set_element_type(type_t::make_type(parser_t::get_base_type(ST_CHAR), true));
		arr->set_len(static_pointer_cast<token_with_value_t<string>>(constant)->get_value().length() + 1);
		return type_t::make_type(arr);
	}
	return parser_t::get_type(symbol_t::token_to_sym_type(constant->get_token_id()));
}

pos_t expr_const_t::get_pos() {
	return constant->get_pos();
}

bool expr_const_t::is_null() {
	return static_pointer_cast<token_base_with_value_t>(constant)->is_null();
}

void expr_const_t::asm_get_val(asm_cmd_list_ptr cmd_list) {
	cmd_list->mov(AR_EAX, static_pointer_cast<token_base_with_value_t>(constant)->get_var());
}

var_ptr expr_const_t::eval() {
	return static_pointer_cast<token_base_with_value_t>(constant)->get_var();
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

expr_t* expr_un_op_t::get_expr() {
	return expr;
}

void expr_un_op_t::set_operand(expr_t* operand) {
	for each (auto type_cast in pre_check_type_convertions)
		if (type_cast(&operand))
			break;
	bool or_passed = or_conditions.empty();
	for each (auto condition in and_conditions)
		condition(operand, this);
	for each (auto condition in or_conditions)
		if (or_passed = condition(operand))
			break;
	if (!or_passed)
		throw InvalidUnOpOperand(operand->get_type(), this);
	for each (auto type_cast in type_convertions)
		if (type_cast(&operand))
			break;
	expr = operand;
}

token_ptr expr_un_op_t::get_op() {
	return op;
}

type_ptr expr_un_op_t::get_type() {
	return expr->get_type();
}

pos_t expr_un_op_t::get_pos() {
	return expr->get_pos();
}

//-----------------------------------OPERAND_CHECKERS---------------------------------

void oc_uo_is_lvalue(expr_t* operand, expr_t* op) {
	if (!operand->is_lvalue())
		throw ExprMustBeLValue(operand->get_pos());
}

void oc_uo_not_constant(expr_t* operand, expr_t* op) {
	if (operand->get_type()->is_const())
		throw AssignmentToReadOnly(operand->get_pos());
}

bool oc_uo_is_integer(expr_t* operand) {
	return operand->get_type()->is_integer();
}

bool oc_uo_is_arithmetic(expr_t* operand) {
	return operand->get_type()->is_arithmetic();
}

bool oc_uo_is_ptr(expr_t* operand) {
	return operand->get_type() == ST_PTR;
}

//-----------------------------------TYPE_CONVERTIONS---------------------------------

bool tc_uo_arr_func_to_ptr(expr_t** operand) {
	if ((*operand)->get_type() == ST_ARRAY)
		*operand = array_to_ptr(*operand);
	else if ((*operand)->get_type() == ST_FUNC_TYPE)
		*operand = func_to_ptr(*operand);
	return false;
}

bool tc_uo_integer_increase(expr_t** operand) {
	if (!(*operand)->get_type()->is_integer())
		return false;
	*operand = integer_increase(*operand);
	return true;
}

bool tc_uo_ptr_to_arithmetic(expr_t** operand) {
	if (oc_uo_is_ptr(*operand))
		*operand = auto_convert(*operand, parser_t::get_type(ST_INTEGER));
	return false;
}

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

template<class T>
expr_un_op_t* new_un_op(token_ptr op) {
	return new T(op);
}

expr_un_op_t* expr_prefix_un_op_t::make_prefix_un_op(token_ptr op) {
	return
		op == T_OP_BIT_AND ? new_un_op<expr_get_addr_un_op_t>(op) :
		op == T_OP_MUL ? new_un_op<expr_dereference_op_t>(op) :
		op->is(T_OP_INC, T_OP_DEC, 0) ? new_un_op<expr_prefix_inc_dec_op_t>(op) :
		op->is(T_OP_ADD, T_OP_SUB, 0) ? new_un_op<expr_prefix_add_sub_un_op_t>(op) :
		op == T_OP_NOT ? new_un_op<expr_prefix_not_un_op_t>(op) :
		op == T_OP_BIT_NOT ? new_un_op<expr_prefix_bit_not_un_op_t>(op) :
		op == T_KWRD_PRINTF ? new_un_op<expr_printf_op_t>(op) :
		(assert(false), nullptr);
}

var_ptr expr_prefix_un_op_t::eval() {
#define reg_un_op(o, t_name) op == t_name ? o##expr->eval() :
	return
		reg_un_op(+, T_OP_ADD)
		reg_un_op(-, T_OP_ADD)
		reg_un_op(~, T_OP_BIT_NOT)
		reg_un_op(!, T_OP_NOT)
		expr_t::eval();
#undef reg_un_op
}

//-----------------------------------PRINTF_OPERATOR-----------------------------------

expr_printf_op_t::expr_printf_op_t(token_ptr op) : expr_prefix_un_op_t(op, false) {
	or_conditions.push_back(oc_uo_is_integer);
	type_convertions.push_back(tc_uo_integer_increase);
}

void expr_printf_op_t::asm_get_val(asm_cmd_list_ptr cmd_list) {
	expr->asm_get_val(cmd_list);
	cmd_list->_push_str("invoke crt_printf, OFFSET printf_format_str, eax");
}

type_ptr expr_printf_op_t::get_type() {
	return parser_t::get_type(ST_VOID);
}

//-----------------------------------GET_ADRESS-----------------------------------

expr_get_addr_un_op_t::expr_get_addr_un_op_t(token_ptr op) : expr_prefix_un_op_t(op) {
	and_conditions.push_back(oc_uo_is_lvalue);
}

void expr_get_addr_un_op_t::asm_get_val(asm_cmd_list_ptr cmd_list) {
	expr->asm_get_addr(cmd_list);
}

type_ptr expr_get_addr_un_op_t::get_type() {
	return sym_type_ptr_t::make_ptr(expr->get_type());
}

//-----------------------------------DEREFERENCE-----------------------------------

expr_dereference_op_t::expr_dereference_op_t(token_ptr op) : expr_prefix_un_op_t(op, true) {
	pre_check_type_convertions.push_back(tc_uo_arr_func_to_ptr);
	or_conditions.push_back(oc_uo_is_ptr);
}

void expr_dereference_op_t::asm_get_val(asm_cmd_list_ptr cmd_list) {
	expr->asm_get_val(cmd_list);
	cmd_list->mov_rderef(AR_EAX, AR_EAX, get_type_size());
}

void expr_dereference_op_t::asm_get_addr(asm_cmd_list_ptr cmd_list) {
	expr->asm_get_val(cmd_list);
}

type_ptr expr_dereference_op_t::get_type() {
	return sym_type_ptr_t::dereference(expr->get_type());
}

//-----------------------------------PREFIX_INC_DEC-----------------------------------

expr_prefix_inc_dec_op_t::expr_prefix_inc_dec_op_t(token_ptr op) : expr_prefix_un_op_t(op) {
	and_conditions.push_back(oc_uo_is_lvalue);
	and_conditions.push_back(oc_uo_not_constant);
	or_conditions.push_back(oc_uo_is_arithmetic);
	or_conditions.push_back(oc_uo_is_ptr);
}

inline int get_ptr_elem_size(type_ptr type) {
	return dynamic_pointer_cast<sym_type_ptr_t>(type)->get_element_type()->get_size();
}

void expr_prefix_inc_dec_op_t::asm_get_val(asm_cmd_list_ptr cmd_list) {
	expr->asm_get_addr(cmd_list);
	if (get_type() == ST_PTR)
		cmd_list->_push_bin_oprtr_lderef(
			(op == T_OP_INC ? ABO_ADD : ABO_SUB), AR_EAX, new_var<int>(get_ptr_elem_size(get_type())), AMT_DWORD);
	else
		cmd_list->_push_un_oprtr_deref(
			(op == T_OP_INC ? AUO_INC : AUO_DEC), AR_EAX, get_type_size());
	cmd_list->mov_rderef(AR_EAX, AR_EAX, get_type_size());
}

//-----------------------------------PREFIX_ADD_SUB-----------------------------------

expr_prefix_add_sub_un_op_t::expr_prefix_add_sub_un_op_t(token_ptr op) : expr_prefix_un_op_t(op) {
	or_conditions.push_back(oc_uo_is_arithmetic);
	type_convertions.push_back(tc_uo_integer_increase);
}

void expr_prefix_add_sub_un_op_t::asm_get_val(asm_cmd_list_ptr cmd_list) {
	expr->asm_get_val(cmd_list);
	if (op == T_OP_SUB)
		cmd_list->neg(AR_EAX, get_type_size());
}

//-----------------------------------PREFIX_LOGICAL_NOT-----------------------------------

expr_prefix_not_un_op_t::expr_prefix_not_un_op_t(token_ptr op) : expr_prefix_un_op_t(op) {
	pre_check_type_convertions.push_back(tc_uo_arr_func_to_ptr);
	or_conditions.push_back(oc_uo_is_arithmetic);
	or_conditions.push_back(oc_uo_is_ptr);
}

type_ptr expr_prefix_not_un_op_t::get_type() {
	return parser_t::get_type(ST_INTEGER);
}

//-----------------------------------PREFIX_BIT_NOT-----------------------------------

expr_prefix_bit_not_un_op_t::expr_prefix_bit_not_un_op_t(token_ptr op) : expr_prefix_un_op_t(op) {
	or_conditions.push_back(oc_uo_is_integer);
	type_convertions.push_back(tc_uo_integer_increase);
}

void expr_prefix_bit_not_un_op_t::asm_get_val(asm_cmd_list_ptr cmd_list) {
	expr->asm_get_val(cmd_list);
	cmd_list->not_(AR_EAX, get_type_size());
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

expr_un_op_t * expr_postfix_un_op_t::make_postfix_un_op(token_ptr op) {
	return
		op->is(T_OP_INC, T_OP_DEC, 0) ? new_un_op<expr_postfix_inc_dec_op_t>(op) :
		(assert(false), nullptr);
}

//-----------------------------------POSTFIX_INC_DEC-----------------------------------

expr_postfix_inc_dec_op_t::expr_postfix_inc_dec_op_t(token_ptr op) : expr_postfix_un_op_t(op) {
	and_conditions.push_back(oc_uo_is_lvalue);
	and_conditions.push_back(oc_uo_not_constant);
	or_conditions.push_back(oc_uo_is_arithmetic);
	or_conditions.push_back(oc_uo_is_ptr);
}

void expr_postfix_inc_dec_op_t::asm_get_val(asm_cmd_list_ptr cmd_list) {
	expr->asm_get_addr(cmd_list);
	cmd_list->mov_rderef(AR_EBX, AR_EAX, get_type_size());
	if (get_type() == ST_PTR)
		cmd_list->_push_bin_oprtr_lderef(
			(op == T_OP_INC ? ABO_ADD : ABO_SUB), AR_EAX, new_var<int>(get_ptr_elem_size(get_type())), AMT_DWORD);
	else
		cmd_list->_push_un_oprtr_deref(
			(op == T_OP_INC ? AUO_INC : AUO_DEC), AR_EAX, get_type_size());
	cmd_list->mov(AR_EAX, AR_EBX);
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
	for (int i = pre_check_type_convertions.size() - 1; i >= 0; i--)
		if (pre_check_type_convertions[i](&left_, &right_))
			break;
	bool or_passed = or_conditions.empty();
	for each (auto operands_checker in and_conditions)
		operands_checker(left_, right_, this);
	for each (auto operands_checker in or_conditions)
		if (or_passed = operands_checker(left_, right_))
			break;
	if (!or_passed)
		throw InvalidBinOpOperands(left_->get_type(), right_->get_type(), this);
	for (int i = type_convertions.size() - 1; i >= 0; i--)
		if (type_convertions[i](&left_, &right_))
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
	return left->get_type();
}

template<class T>
inline expr_bin_op_t* new_bin_op(token_ptr op) {
	return new T(op);
}

expr_bin_op_t* expr_bin_op_t::make_bin_op(token_ptr op) {
	return
		op == T_OP_ASSIGN ? new_bin_op<expr_assign_bin_op_t>(op) :
		op->is(T_OP_MUL, T_OP_DIV, 0) ? new_bin_op<expr_arithmetic_bin_op_t>(op) :
		op->is(T_OP_MUL_ASSIGN, T_OP_DIV_ASSIGN, 0) ? new_bin_op<expr_arithmetic_assign_bin_op_t>(op) :
		op == T_OP_ADD ? new_bin_op<expr_add_bin_op_t>(op) :
		op == T_OP_SUB ? new_bin_op<expr_sub_bin_op_t>(op) :
		op == T_OP_MOD ? new_bin_op<expr_mod_bin_op_t>(op) :
		op == T_OP_ADD_ASSIGN ? new_bin_op<expr_add_assign_bin_op_t>(op) :
		op == T_OP_SUB_ASSIGN ? new_bin_op<expr_sub_assign_bin_op_t>(op) :
		op == T_OP_MOD_ASSIGN ? new_bin_op<expr_mod_assign_bin_op_t>(op) :
		op->is(T_OP_L, T_OP_LE, T_OP_G, T_OP_GE, 0) ? new_bin_op<expr_relational_bin_op_t>(op) :
		op->is(T_OP_EQ, T_OP_NEQ, 0) ? new_bin_op<expr_equality_bin_op_t>(op) :
		op->is(T_OP_AND, T_OP_OR, 0) ? new_bin_op<expr_logical_bin_op_t>(op) :
		op->is(T_OP_BIT_AND, T_OP_BIT_OR, T_OP_XOR, 0) ? new_bin_op<expr_integer_bin_op_t>(op) :
		op->is(T_OP_BIT_AND_ASSIGN, T_OP_BIT_OR_ASSIGN, T_OP_XOR_ASSIGN, 0) ? new_bin_op<expr_integer_assign_bin_op_t>(op) :
		op->is(T_OP_LEFT, T_OP_RIGHT, 0) ? new_bin_op<expr_shift_bin_op_t>(op) :
		op->is(T_OP_LEFT_ASSIGN, T_OP_RIGHT_ASSIGN, 0) ? new_bin_op<expr_shift_assign_bin_op_t>(op) :
		(assert(false), nullptr);
}

void expr_bin_op_t::_asm_get_val(asm_cmd_list_ptr cmd_list) {
	cmd_list->_push_bin_oprtr(_asm_get_operator(), AR_EAX, AR_EBX);
}

ASM_BIN_OPERATOR expr_bin_op_t::_asm_get_operator() {
	return assert(false), ABO_ADD;
}

void expr_bin_op_t::asm_get_val(asm_cmd_list_ptr cmd_list) {
	right->asm_get_val(cmd_list);
	cmd_list->push(AR_EAX);
	left->asm_get_val(cmd_list);
	cmd_list->pop(AR_EBX);
	_asm_get_val(cmd_list);
}

var_ptr expr_bin_op_t::eval() {
	var_ptr lv = left->eval();
	var_ptr rv = right->eval();
#define reg_bin_op(token, o) op == token ? lv o rv :
	return
		reg_bin_op(T_OP_ADD, +)
		reg_bin_op(T_OP_SUB, -)
		reg_bin_op(T_OP_MUL, *)
		reg_bin_op(T_OP_DIV, /)
		reg_bin_op(T_OP_MOD, %)

		reg_bin_op(T_OP_AND, &&)
		reg_bin_op(T_OP_OR, ||)
		reg_bin_op(T_OP_BIT_AND, &)
		reg_bin_op(T_OP_BIT_OR, |)
		reg_bin_op(T_OP_LEFT, <<)
		reg_bin_op(T_OP_RIGHT, >>)

		reg_bin_op(T_OP_L, <)
		reg_bin_op(T_OP_LE, <=)
		reg_bin_op(T_OP_EQ, ==)
		reg_bin_op(T_OP_NEQ, !=)
		reg_bin_op(T_OP_GE, >=)
		reg_bin_op(T_OP_G, >)
		reg_bin_op(T_OP_L, <)
		expr_t::eval();
#undef reg_bin_op
}

//-------------------------------OPERANDS_CHECKERS--------------------------------

void oc_bo_is_lvalue(expr_t* left, expr_t* right, expr_t* op) {
	oc_uo_is_lvalue(left, op);
}

void oc_bo_not_constant(expr_t* left, expr_t* right, expr_t* op) {
	oc_uo_not_constant(left, op);
}

bool oc_bo_is_integer(expr_t* left, expr_t* right) {
	return oc_uo_is_integer(left) && oc_uo_is_integer(right);
}

bool oc_bo_is_arithmetic(expr_t* left, expr_t* right) {
	return oc_uo_is_arithmetic(left) && oc_uo_is_arithmetic(right);
}

bool oc_bo_is_ptrs_to_same_types(expr_t* left, expr_t* right) {
	return 
		(oc_uo_is_ptr(left) && oc_uo_is_ptr(right)) &&
		(static_pointer_cast<sym_type_ptr_t>(left->get_type()->get_base_type())->get_element_type()->get_base_type() == 
		 static_pointer_cast<sym_type_ptr_t>(right->get_type()->get_base_type())->get_element_type()->get_base_type());
}

bool oc_bo_is_arithmetic_or_ptr(expr_t* left, expr_t* right) {
	return 
		(oc_uo_is_ptr(left) || oc_uo_is_arithmetic(left)) &&
		(oc_uo_is_ptr(right) || oc_uo_is_arithmetic(right));
}

bool oc_bo_ptr_and_integer(expr_t* left, expr_t* right) {
	return oc_uo_is_ptr(left) && oc_uo_is_integer(right);
}

bool oc_bo_integer_and_ptr(expr_t* left, expr_t* right) {
	return oc_uo_is_integer(left) && oc_uo_is_ptr(right);
}

//--------------------------------TYPE_CONVERTIONS--------------------------------

bool tc_bo_arithmetic_conversion(expr_t** left, expr_t** right) {
	arithmetic_conversion(left, right);
	return true;
}

bool tc_bo_left_to_right_type(expr_t** left, expr_t** right) {
	*right = auto_convert(*right, (*left)->get_type());
	return true;
}

bool tc_bo_integer_increase(expr_t** left, expr_t** right) {
	tc_uo_integer_increase(left);
	tc_uo_integer_increase(right);
	return true;
}

bool tc_bo_integer_and_ptr(expr_t** left, expr_t** right) {
	if (oc_uo_is_ptr(*left) || oc_uo_is_ptr(*right)) {
		tc_uo_integer_increase(left);
		tc_uo_integer_increase(right);
		return true;
	}
	return false;
}

bool tc_bo_pass_ptrs(expr_t** left, expr_t** right) {
	return oc_uo_is_ptr(*left) && oc_uo_is_ptr(*right);
}

bool tc_bo_int_to_ptr(expr_t** left, expr_t** right) {
	if (oc_uo_is_ptr(*left) || oc_uo_is_ptr(*right)) {
		type_ptr ptr = oc_uo_is_ptr(*left) ? (*left)->get_type() : (*right)->get_type();
		*left = auto_convert(*left, ptr);
		*right = auto_convert(*right, ptr);
		return true;
	}
	return false;
}

bool tc_bo_ptr_to_arithmetic(expr_t** left, expr_t** right) {
	tc_uo_ptr_to_arithmetic(left);
	tc_uo_ptr_to_arithmetic(right);
	return false;
}

bool tc_bo_arr_func_to_ptr(expr_t** left, expr_t** right) {
	tc_uo_arr_func_to_ptr(left);
	tc_uo_arr_func_to_ptr(right);
	return false;
}

bool tc_bo_right_arr_func_to_ptr(expr_t** left, expr_t** right) {
	tc_uo_arr_func_to_ptr(right);
	return false;
}

//-----------------------------------BASE_ASSIGN---------------------------------------------

expr_base_assign_bin_op_t::expr_base_assign_bin_op_t(token_ptr op) : expr_bin_op_t(op) {
	pre_check_type_convertions.push_back(tc_bo_right_arr_func_to_ptr);
	and_conditions.push_back(oc_bo_is_lvalue);
	and_conditions.push_back(oc_bo_not_constant);
}

void expr_base_assign_bin_op_t::_asm_get_val(asm_cmd_list_ptr cmd_list) {
	cmd_list->_push_bin_oprtr_lderef(_asm_get_operator(), AR_EAX, AR_EBX, get_type_size());
}

void expr_base_assign_bin_op_t::asm_get_val(asm_cmd_list_ptr cmd_list) {
	if (left->get_type() == ST_STRUCT) {
		right->asm_get_val(cmd_list);
		left->asm_get_addr(cmd_list);
		cmd_list->_push_copy_cmd(AR_ESP, AR_EAX, get_type_size());
	} else {
		right->asm_get_val(cmd_list);
		cmd_list->push(AR_EAX);
		left->asm_get_addr(cmd_list);
		cmd_list->pop(AR_EBX);
		_asm_get_val(cmd_list);
		cmd_list->mov_rderef(AR_EAX, AR_EAX, get_type_size());
	}
}

//-----------------------------------ASSIGN---------------------------------------------

expr_assign_bin_op_t::expr_assign_bin_op_t(token_ptr op) : expr_base_assign_bin_op_t(op) {
	type_convertions.push_back(tc_bo_left_to_right_type);
}

ASM_BIN_OPERATOR expr_assign_bin_op_t::_asm_get_operator() {
	return ABO_MOV;
}

//-----------------------------------INTEGER_OPERATORS-----------------------------------

expr_integer_bin_op_t::expr_integer_bin_op_t(token_ptr token) : expr_bin_op_t(token) {
	pre_check_type_convertions.push_back(tc_bo_arr_func_to_ptr);
	or_conditions.push_back(oc_bo_is_integer);
	type_convertions.push_back(tc_bo_arithmetic_conversion);
}

ASM_BIN_OPERATOR expr_integer_bin_op_t::_asm_get_operator() {
	return 
		op == T_OP_BIT_AND ? ABO_AND :
		op == T_OP_BIT_OR ? ABO_OR :
		(assert(false), ABO_AND);
}

expr_integer_assign_bin_op_t::expr_integer_assign_bin_op_t(token_ptr token) : expr_base_assign_bin_op_t(token) {
	or_conditions.push_back(oc_bo_is_integer);
}

ASM_BIN_OPERATOR expr_integer_assign_bin_op_t::_asm_get_operator() {
	return
		op == T_OP_BIT_AND_ASSIGN ? ABO_AND :
		op == T_OP_BIT_OR_ASSIGN ? ABO_OR :
		(assert(false), ABO_AND);
}

//-----------------------------------ARITHMETIC_OPERATORS-----------------------------------

void expr_arithmetic_bin_op_t::_asm_get_val(asm_cmd_list_ptr cmd_list) {
	assert(op->is(T_OP_MUL, T_OP_DIV, 0));
	if (op == T_OP_MUL)
		cmd_list->imul(AR_EAX, AR_EBX);
	else if (op == T_OP_DIV) {
		cmd_list->xor_(AR_EDX, AR_EDX);
		cmd_list->div(AR_EBX);
	}
}

expr_arithmetic_bin_op_t::expr_arithmetic_bin_op_t(token_ptr token) : expr_bin_op_t(token) {
	pre_check_type_convertions.push_back(tc_bo_arr_func_to_ptr);
	or_conditions.push_back(oc_bo_is_arithmetic);
	type_convertions.push_back(tc_bo_arithmetic_conversion);
}

void expr_arithmetic_assign_bin_op_t::_asm_get_val(asm_cmd_list_ptr cmd_list) {
	int type_size = get_type_size();
	if (op == T_OP_MUL_ASSIGN) {
		cmd_list->xor_(AR_EDX, AR_EDX);
		cmd_list->mov_rderef(AR_EDX, AR_EAX, type_size);
		cmd_list->imul(AR_EDX, AR_EBX);
		cmd_list->mov_lderef(AR_EAX, AR_EDX, type_size);
	} else if (op == T_OP_DIV_ASSIGN) {
		cmd_list->mov(AR_ECX, AR_EAX);
		if (type_size < asm_generator_t::size_of(AR_EAX)) {
			cmd_list->xor_(AR_EDX, AR_EDX);
			cmd_list->mov_rderef(AR_EDX, AR_EAX, type_size);
			cmd_list->mov(AR_EAX, AR_EDX);
		} else
			cmd_list->mov_rderef(AR_EAX, AR_EAX, type_size);
		cmd_list->xor_(AR_EDX, AR_EDX);
		cmd_list->div(AR_EBX);
		cmd_list->mov_lderef(AR_ECX, AR_EAX, type_size);
	} else
		expr_base_assign_bin_op_t::_asm_get_val(cmd_list);
}

expr_arithmetic_assign_bin_op_t::expr_arithmetic_assign_bin_op_t(token_ptr token) : expr_base_assign_bin_op_t(token) {
	or_conditions.push_back(oc_bo_is_arithmetic);
}

//--------------------------------------ADD----------------------------------------------

expr_add_bin_op_t::expr_add_bin_op_t(token_ptr op) : expr_arithmetic_bin_op_t(op) {
	or_conditions.push_back(oc_bo_ptr_and_integer);
	or_conditions.push_back(oc_bo_integer_and_ptr);
	type_convertions.push_back(tc_bo_integer_and_ptr);
	type_convertions.push_back(tc_bo_pass_ptrs);
}

inline void mul_reg_to_elem_size(asm_cmd_list_ptr cmd_list, ASM_REGISTER reg, int elem_size) {
	if (elem_size > 1)
		cmd_list->imul(reg, new_var<int>(elem_size));
}

void expr_add_bin_op_t::_asm_get_val(asm_cmd_list_ptr cmd_list) {
	if (left->get_type() == ST_PTR)
		mul_reg_to_elem_size(cmd_list, AR_EBX, get_ptr_elem_size(get_type()));
	else if (right->get_type() == ST_PTR)
		mul_reg_to_elem_size(cmd_list, AR_EAX, get_ptr_elem_size(get_type()));
	cmd_list->add(AR_EAX, AR_EBX);
}

ASM_BIN_OPERATOR expr_add_assign_bin_op_t::_asm_get_operator() {
	return ABO_ADD;
}

expr_add_assign_bin_op_t::expr_add_assign_bin_op_t(token_ptr op) : expr_arithmetic_assign_bin_op_t(op) {
	or_conditions.push_back(oc_bo_ptr_and_integer);
	or_conditions.push_back(oc_bo_integer_and_ptr);
}

void expr_add_assign_bin_op_t::_asm_get_val(asm_cmd_list_ptr cmd_list) {
	if (left->get_type() == ST_PTR)
		mul_reg_to_elem_size(cmd_list, AR_EBX, get_ptr_elem_size(get_type()));
	cmd_list->add_lderef(AR_EAX, AR_EBX, get_type_size());
}

//--------------------------------------SUB----------------------------------------------

expr_sub_bin_op_t::expr_sub_bin_op_t(token_ptr op) : expr_arithmetic_bin_op_t(op) {
	or_conditions.push_back(oc_bo_is_ptrs_to_same_types);
	or_conditions.push_back(oc_bo_ptr_and_integer);
	type_convertions.push_back(tc_bo_integer_and_ptr);
	type_convertions.push_back(tc_bo_pass_ptrs);
}

ASM_BIN_OPERATOR expr_sub_assign_bin_op_t::_asm_get_operator() {
	return ABO_SUB;
}

void expr_sub_bin_op_t::_asm_get_val(asm_cmd_list_ptr cmd_list) {
	if (left->get_type() == ST_PTR)
		mul_reg_to_elem_size(cmd_list, AR_EBX, get_ptr_elem_size(get_type()));
	cmd_list->sub(AR_EAX, AR_EBX);
}

expr_sub_assign_bin_op_t::expr_sub_assign_bin_op_t(token_ptr op) : expr_arithmetic_assign_bin_op_t(op) {
	or_conditions.push_back(oc_bo_is_ptrs_to_same_types);
	or_conditions.push_back(oc_bo_ptr_and_integer);
}

void expr_sub_assign_bin_op_t::_asm_get_val(asm_cmd_list_ptr cmd_list) {
	if (left->get_type() == ST_PTR)
		mul_reg_to_elem_size(cmd_list, AR_EBX, get_ptr_elem_size(get_type()));
	cmd_list->sub_lderef(AR_EAX, AR_EBX, get_type_size());
}

//--------------------------------------MOD----------------------------------------------

void expr_mod_bin_op_t::_asm_get_val(asm_cmd_list_ptr cmd_list) {
	cmd_list->xor_(AR_EDX, AR_EDX);
	cmd_list->div(AR_EBX);
	cmd_list->mov(AR_EAX, AR_EDX);
}

expr_mod_bin_op_t::expr_mod_bin_op_t(token_ptr op) : expr_bin_op_t(op) {
	pre_check_type_convertions.push_back(tc_bo_arr_func_to_ptr);
	or_conditions.push_back(oc_bo_is_integer);
	type_convertions.push_back(tc_bo_integer_increase);
}

void expr_mod_assign_bin_op_t::_asm_get_val(asm_cmd_list_ptr cmd_list) {
	int type_size = get_type_size();
	cmd_list->mov(AR_ECX, AR_EAX);
	if (type_size < asm_generator_t::size_of(AR_EAX)) {
		cmd_list->xor_(AR_EDX, AR_EDX);
		cmd_list->mov_rderef(AR_EDX, AR_EAX, type_size);
		cmd_list->mov(AR_EAX, AR_EDX);
	} else
		cmd_list->mov_rderef(AR_EAX, AR_EAX, type_size);
	cmd_list->xor_(AR_EDX, AR_EDX);
	cmd_list->div(AR_EBX);
	cmd_list->mov_lderef(AR_ECX, AR_EDX, type_size);
	cmd_list->mov(AR_EAX, AR_EDX);
}

expr_mod_assign_bin_op_t::expr_mod_assign_bin_op_t(token_ptr op) : expr_base_assign_bin_op_t(op) {
	or_conditions.push_back(oc_bo_is_integer);
}

//--------------------------------------RELATIONAL_OPERATORS----------------------------------------------

expr_relational_bin_op_t::expr_relational_bin_op_t(token_ptr op) : expr_arithmetic_bin_op_t(op) {
	pre_check_type_convertions.push_back(tc_bo_arr_func_to_ptr);
	or_conditions.push_back(oc_bo_is_ptrs_to_same_types);
	type_convertions.push_back(tc_bo_arithmetic_conversion);
	type_convertions.push_back(tc_bo_int_to_ptr);
	type_convertions.push_back(tc_bo_pass_ptrs);
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
	pre_check_type_convertions.push_back(tc_bo_arr_func_to_ptr);
	or_conditions.push_back(oc_bo_is_arithmetic_or_ptr);
	type_convertions.push_back(tc_bo_arithmetic_conversion);
	type_convertions.push_back(tc_bo_ptr_to_arithmetic);
}

//--------------------------------------SHIFT_OPERATORS----------------------------------------------

expr_shift_bin_op_t::expr_shift_bin_op_t(token_ptr op) : expr_bin_op_t(op) {
	pre_check_type_convertions.push_back(tc_bo_arr_func_to_ptr);
	or_conditions.push_back(oc_bo_is_integer);
	type_convertions.push_back(tc_bo_integer_increase);
}

ASM_BIN_OPERATOR expr_shift_bin_op_t::_asm_get_operator() {
	return
		op == T_OP_LEFT ? ABO_SHL :
		op == T_OP_RIGHT ? ABO_SHR :
		(assert(false), ABO_AND);
}

void expr_shift_bin_op_t::_asm_get_val(asm_cmd_list_ptr cmd_list) {
	cmd_list->mov(AR_CL, AR_BL);
	cmd_list->_push_bin_oprtr(_asm_get_operator(), AR_EAX, AR_CL);
}

expr_shift_assign_bin_op_t::expr_shift_assign_bin_op_t(token_ptr op) : expr_base_assign_bin_op_t(op) {
	or_conditions.push_back(oc_bo_is_integer);
}

ASM_BIN_OPERATOR expr_shift_assign_bin_op_t::_asm_get_operator() {
	return
		op == T_OP_LEFT_ASSIGN ? ABO_SHL :
		op == T_OP_RIGHT_ASSIGN ? ABO_SHR :
		(assert(false), ABO_AND);
}

void expr_shift_assign_bin_op_t::_asm_get_val(asm_cmd_list_ptr cmd_list) {
	cmd_list->mov(AR_CL, AR_BL);
	cmd_list->_push_bin_oprtr_lderef(_asm_get_operator(), 
		AR_EAX, AR_CL, asm_generator_t::mtype_by_size(get_type_size()));
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

expr_t* expr_tern_op_t::get_condition() {
	return condition;
}

expr_t* expr_tern_op_t::get_left() {
	return left;
}

expr_t* expr_tern_op_t::get_right() {
	return right;
}

token_ptr expr_tern_op_t::get_question_mark_token() {
	return question_mark;
}

token_ptr expr_tern_op_t::get_colon_token() {
	return colon;
}

void expr_tern_op_t::set_operands(expr_t* condition_, expr_t* left_, expr_t* right_) {
	tc_uo_arr_func_to_ptr(&condition_);
	if (condition_->get_type() == ST_PTR)
		condition_ = auto_convert(condition_, parser_t::get_type(ST_INTEGER));
	if (!condition_->get_type()->is_arithmetic())
		throw InvalidTernOpOperands(condition_->get_type(), this);
		
	condition = condition_;
	if (!(oc_bo_integer_and_ptr(left_, right_) ||
		oc_bo_ptr_and_integer(left_, right_) ||
		oc_bo_is_ptrs_to_same_types(left_, right_) ||
		oc_bo_is_arithmetic(left_, right_)))
	{
		throw InvalidTernOpOperands(left_->get_type(), right_->get_type(), this);
	}

	tc_bo_int_to_ptr(&left_, &right_) ||
	tc_bo_pass_ptrs(&left_, &right_) ||
	tc_bo_arithmetic_conversion(&left_, &right_);

	left = left_;
	right = right_;
}

type_ptr expr_tern_op_t::get_type() {
	return left->get_type();
}

pos_t expr_tern_op_t::get_pos() {
	return question_mark->get_pos();
}

var_ptr expr_tern_op_t::eval() {
	return condition->eval() ? left->eval() : right->eval();
}

//-----------------------------------ARRAY_INDEX-----------------------------------

expr_arr_index_t::expr_arr_index_t(token_ptr sqr_bracket) : expr_t(true), sqr_bracket(sqr_bracket) {}

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

void expr_arr_index_t::set_operands(expr_t* arr_, expr_t* index_) {
	if (arr_->get_type() == ST_ARRAY)
		arr_ = array_to_ptr(arr_);
	if (arr_->get_type() != ST_PTR)
		throw SemanticError("Subscripted value is neither array nor pointer", arr_->get_pos());
	if (!index_->get_type()->is_integer())
		throw SemanticError("Array subscript is not an integer", index_->get_pos());
	
	arr = arr_;
	index = index_;
}

void expr_arr_index_t::asm_get_addr(asm_cmd_list_ptr cmd_list) {
	arr->asm_get_val(cmd_list);
	cmd_list->push(AR_EAX);
	index->asm_get_val(cmd_list);
	cmd_list->pop(AR_EBX);
	mul_reg_to_elem_size(cmd_list, AR_EAX, get_type()->get_size());
	cmd_list->add(AR_EAX, AR_EBX);
}

void expr_arr_index_t::asm_get_val(asm_cmd_list_ptr cmd_list) {
	asm_get_addr(cmd_list);
	cmd_list->mov_rderef(AR_EAX, AR_EAX, get_type()->get_size());
}

type_ptr expr_arr_index_t::get_type() {
	return static_pointer_cast<sym_type_ptr_t>(arr->get_type()->get_base_type())->get_element_type();
}

pos_t expr_arr_index_t::get_pos() {
	return sqr_bracket->get_pos();
}

//-----------------------------------STRUCT_ACCESS-----------------------------------

expr_struct_access_t::expr_struct_access_t(token_ptr op) : expr_t(true), op(op) {}

void expr_struct_access_t::print_l(ostream& os, int level) {
	struct_expr->print_l(os, level + 1);
	print_level(os, level);
	op->short_print(os);
	os << endl;
	print_level(os, level + 1);
	member->get_token()->short_print(os);
	os << endl;
}

void expr_struct_access_t::short_print_l(ostream& os, int level) {
	print_level(os, level);
	struct_expr->short_print(os);
	op->short_print(os);
	member->get_token()->short_print(os);
}

expr_t* expr_struct_access_t::get_expr() {
	return struct_expr;
}

void expr_struct_access_t::set_operands(expr_t* expr_, token_ptr member_token) {
	if (op == T_OP_DOT) {
		if (expr_->get_type() != ST_STRUCT)
			throw SemanticError("Expected struct as left operand of '.'", expr_->get_pos());
		lvalue = expr_->is_lvalue();
		structure = static_pointer_cast<sym_type_struct_t>(expr_->get_type()->get_base_type());
	} else {
		assert(op == T_OP_ARROW);
		if (expr_->get_type() != ST_PTR || sym_type_ptr_t::dereference(expr_->get_type()) != ST_STRUCT)
			throw SemanticError("Expected pointer to struct as left operand of '->'", expr_->get_pos());
		structure = static_pointer_cast<sym_type_struct_t>(sym_type_ptr_t::dereference(expr_->get_type())->get_base_type());
	}
	member = structure->get_member(member_token);
	struct_expr = expr_;
}

token_ptr expr_struct_access_t::get_op() {
	return op;
}

token_ptr expr_struct_access_t::get_member() {
	return member->get_token();
}

void expr_struct_access_t::asm_get_addr(asm_cmd_list_ptr cmd_list) {
	if (op == T_OP_DOT) {
		assert(struct_expr->is_lvalue());
		struct_expr->asm_get_addr(cmd_list);
	} else
		struct_expr->asm_get_val(cmd_list);
	member->asm_get_addr(cmd_list);
}

void expr_struct_access_t::asm_get_val(asm_cmd_list_ptr cmd_list) {
	if (op == T_OP_DOT) {
		if (struct_expr->is_lvalue())
			struct_expr->asm_get_addr(cmd_list);
		else {
			struct_expr->asm_get_val(cmd_list);
			cmd_list->mov(AR_EAX, AR_ESP);
		}
	} else
		struct_expr->asm_get_val(cmd_list);
	member->asm_get_val(cmd_list);
}

type_ptr expr_struct_access_t::get_type() {
	return member->get_type();
}

pos_t expr_struct_access_t::get_pos() {
	return op->get_pos();
}

//-----------------------------------FUNCTION_CALL-----------------------------------

expr_func_t::expr_func_t(token_ptr op) : brace(op) {}

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

void expr_func_t::set_operands(expr_t* func_, vector<expr_t*> args_) {
	if (func_->get_type() != ST_FUNC_TYPE && (func_->get_type() != ST_PTR ||
		sym_type_ptr_t::dereference(func_->get_type()) != ST_FUNC))
			throw SemanticError("Called object is not a function or function pointer");
	func = func_;
	auto func_type = 
		func_->get_type() == ST_FUNC_TYPE ? dynamic_pointer_cast<sym_type_func_t>(func_->get_type()->get_base_type()) :
		dynamic_pointer_cast<sym_type_func_t>(sym_type_ptr_t::dereference(func_->get_type())->get_base_type());
	auto func_args = func_type->get_arg_types();
	if (func_args.size() != args_.size())
		throw IncorrectNumberOfArguments(args_.size(), func_type, this);
	args.resize(func_args.size());
	for (int i = 0; i < func_args.size(); i++)
		args[i] = auto_convert(args_[i], func_args[i]);
}

type_ptr expr_func_t::get_type() {
	return func->get_type();
}

pos_t expr_func_t::get_pos() {
	return brace->get_pos();
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
	if (src_type == ST_PTR && (dst_type == ST_PTR || dst_type->is_arithmetic()) ||
		src_type->is_arithmetic() && dst_type->is_arithmetic() ||
		src_type->is_integer() && dst_type == ST_PTR ||
		(src_type == ST_ARRAY || src_type == ST_FUNC_TYPE) && dst_type == ST_PTR)
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

void expr_cast_t::asm_get_val(asm_cmd_list_ptr cmd_list) {
	if (expr->get_type() == ST_CHAR && type == ST_INTEGER ||
		expr->get_type() == ST_INTEGER && type == ST_CHAR)
	{
		expr->asm_get_val(cmd_list);
		cmd_list->mov(AR_EBX, AR_EAX);
		cmd_list->xor_(AR_EAX, AR_EAX);
		cmd_list->mov(AR_AL, AR_BL);
	} else if (expr->get_type() == ST_ARRAY)
		expr->asm_get_addr(cmd_list);
}

var_ptr expr_cast_t::eval() {
	return
		type == ST_CHAR ? var_cast<char>(expr->eval()) :
		type == ST_INTEGER ? var_cast<int>(expr->eval()) :
		type == ST_DOUBLE ? var_cast<double>(expr->eval()) :
		(throw ExprMustBeEval(get_pos()), nullptr);
}
