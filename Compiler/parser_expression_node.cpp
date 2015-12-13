#include "parser.h"
#include "exceptions.h"
#include "type_conversion.h"
#include <map>

using namespace std;

map<TOKEN, ASM_OPERATOR> token_to_int_op_map;
map<TOKEN, ASM_OPERATOR> token_to_fp_op_map;
map<TOKEN, ASM_OPERATOR> token_to_fp_rev_op_map;

void parser_expression_node_init() {
	token_to_int_op_map[T_OP_INC] = AO_INC;
	token_to_int_op_map[T_OP_DEC] = AO_DEC;
	token_to_int_op_map[T_OP_ADD] = AO_ADD;
	token_to_int_op_map[T_OP_ADD_ASSIGN] = AO_ADD;
	token_to_int_op_map[T_OP_SUB] = AO_SUB;
	token_to_int_op_map[T_OP_SUB_ASSIGN] = AO_SUB;
	token_to_int_op_map[T_OP_MUL] = AO_IMUL;
	token_to_int_op_map[T_OP_MUL_ASSIGN] = AO_IMUL;
	token_to_int_op_map[T_OP_DIV] = AO_DIV;
	token_to_int_op_map[T_OP_DIV_ASSIGN] = AO_DIV;
	token_to_int_op_map[T_OP_MOD] = AO_DIV;
	token_to_int_op_map[T_OP_MOD_ASSIGN] = AO_DIV;
	token_to_int_op_map[T_OP_BIT_OR] = AO_OR;
	token_to_int_op_map[T_OP_BIT_OR_ASSIGN] = AO_OR;
	token_to_int_op_map[T_OP_BIT_AND] = AO_AND;
	token_to_int_op_map[T_OP_BIT_AND_ASSIGN] = AO_AND;
	token_to_int_op_map[T_OP_XOR] = AO_XOR;
	token_to_int_op_map[T_OP_XOR_ASSIGN] = AO_XOR;
	token_to_int_op_map[T_OP_LEFT] = AO_SHL;
	token_to_int_op_map[T_OP_LEFT_ASSIGN] = AO_SHL;
	token_to_int_op_map[T_OP_RIGHT] = AO_SHR;
	token_to_int_op_map[T_OP_RIGHT_ASSIGN] = AO_SHR;
	token_to_int_op_map[T_OP_ASSIGN] = AO_MOV;

	token_to_fp_op_map[T_OP_ADD] = AO_FADD;
	token_to_fp_op_map[T_OP_ADD_ASSIGN] = AO_FADD;
	token_to_fp_op_map[T_OP_SUB] = AO_FSUB;
	token_to_fp_op_map[T_OP_SUB_ASSIGN] = AO_FSUB;
	token_to_fp_op_map[T_OP_MUL] = AO_FMUL;
	token_to_fp_op_map[T_OP_MUL_ASSIGN] = AO_FMUL;
	token_to_fp_op_map[T_OP_DIV] = AO_FDIV;
	token_to_fp_op_map[T_OP_DIV_ASSIGN] = AO_FDIV;

	token_to_fp_rev_op_map[T_OP_ADD] = AO_FADD;
	token_to_fp_rev_op_map[T_OP_ADD_ASSIGN] = AO_FADD;
	token_to_fp_rev_op_map[T_OP_SUB] = AO_FSUBR;
	token_to_fp_rev_op_map[T_OP_SUB_ASSIGN] = AO_FSUBR;
	token_to_fp_rev_op_map[T_OP_MUL] = AO_FMUL;
	token_to_fp_rev_op_map[T_OP_MUL_ASSIGN] = AO_FMUL;
	token_to_fp_rev_op_map[T_OP_DIV] = AO_FDIVR;
	token_to_fp_rev_op_map[T_OP_DIV_ASSIGN] = AO_FDIVR;
}

//-----------------------------------EXPRESSIONS-----------------------------------

expr_t::expr_t(bool lvalue) : lvalue(lvalue) {}

bool expr_t::is_lvalue() {
	return lvalue;
}

void expr_t::asm_gen_code(asm_cmd_list_ptr cmd_list, bool keep_val) {
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
	try {
		return get_type()->get_size();
	} catch (...) {
		throw CantGetSize(get_pos());
	}
}

ASM_OPERATOR expr_t::token_to_fp_op(token_ptr token) {
	return token_to_fp_op_map.at(token->get_token_id());
}

ASM_OPERATOR expr_t::token_to_int_op(token_ptr token) {
	return token_to_int_op_map.at(token->get_token_id());
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

void expr_var_t::asm_gen_code(asm_cmd_list_ptr cmd_list, bool keep_val) {
	if (keep_val)
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

void expr_const_t::asm_gen_code(asm_cmd_list_ptr cmd_list, bool keep_val) {
	if (keep_val) {
		auto var = static_pointer_cast<token_base_with_value_t>(constant)->get_var();
		if (get_type() == ST_DOUBLE)
			cmd_list->fld(var);
		else
			cmd_list->mov(AR_EAX, var);
	}
}

void expr_const_t::asm_get_addr(asm_cmd_list_ptr cmd_list) {
	assert(constant == T_STRING);
	asm_gen_code(cmd_list, true);
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

void expr_un_op_t::asm_gen_code(asm_cmd_list_ptr cmd_list, bool keep_val) {
	expr->asm_gen_code(cmd_list, keep_val);
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

//-----------------------------------GET_ADRESS-----------------------------------

expr_get_addr_un_op_t::expr_get_addr_un_op_t(token_ptr op) : expr_prefix_un_op_t(op) {
	and_conditions.push_back(oc_uo_is_lvalue);
}

void expr_get_addr_un_op_t::asm_gen_code(asm_cmd_list_ptr cmd_list, bool keep_val) {
	if (keep_val)
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

void expr_dereference_op_t::asm_gen_code(asm_cmd_list_ptr cmd_list, bool keep_val) {
	if (keep_val) {
		expr->asm_gen_code(cmd_list, true);
		if (get_type() == ST_INTEGER)
			cmd_list->mov_rderef(AR_EAX, AR_EAX, get_type_size());
		else if (get_type() == ST_CHAR) {
			cmd_list->mov_rderef(AR_EBX, AR_EAX, asm_gen_t::size_of(AMT_BYTE));
			cmd_list->_cast_char_to_int(AR_EBX, AR_EAX);
		} else if (get_type() == ST_DOUBLE)
			cmd_list->fld_deref(AR_EAX, AMT_QWORD);
	}
}

void expr_dereference_op_t::asm_get_addr(asm_cmd_list_ptr cmd_list) {
	expr->asm_gen_code(cmd_list, true);
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
	type_ptr pointed_type = sym_type_ptr_t::dereference(type);
	return pointed_type == ST_VOID ? 1 : pointed_type->get_size();
}

void expr_prefix_inc_dec_op_t::asm_gen_code(asm_cmd_list_ptr cmd_list, bool keep_val) {
	expr->asm_get_addr(cmd_list);
	if (get_type() == ST_PTR)
		cmd_list->_add_op_lderef(op == T_OP_INC ? AO_ADD : AO_SUB, AR_EAX, new_var<int>(get_ptr_elem_size(get_type())), AMT_DWORD);
	else if (get_type()->is_integer())
		cmd_list->_add_op_deref(token_to_int_op(op), AR_EAX, get_type_size());
	else {
		cmd_list->fld_deref(AR_EAX, AMT_QWORD);
		cmd_list->fld1();
		cmd_list->_add_op(op == T_OP_INC ? AO_FADD : AO_FSUB);
		cmd_list->_add_op_deref(keep_val ? AO_FST : AO_FSTP, AR_EAX, AMT_QWORD);
		return;
	}
	cmd_list->mov_rderef(AR_EAX, AR_EAX, get_type_size());
}

//-----------------------------------PREFIX_ADD_SUB-----------------------------------

expr_prefix_add_sub_un_op_t::expr_prefix_add_sub_un_op_t(token_ptr op) : expr_prefix_un_op_t(op) {
	or_conditions.push_back(oc_uo_is_arithmetic);
	type_convertions.push_back(tc_uo_integer_increase);
}

void expr_prefix_add_sub_un_op_t::asm_gen_code(asm_cmd_list_ptr cmd_list, bool keep_val) {
	if (keep_val) {
		expr->asm_gen_code(cmd_list, true);
		if (op == T_OP_SUB) {
			if (get_type()->is_integer())
				cmd_list->neg(AR_EAX, get_type_size());
			else
				cmd_list->fchs();
		}
	}
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

void expr_prefix_bit_not_un_op_t::asm_gen_code(asm_cmd_list_ptr cmd_list, bool keep_val) {
	if (keep_val) {
		expr->asm_gen_code(cmd_list, true);
		cmd_list->not_(AR_EAX, get_type_size());
	}
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

void expr_postfix_inc_dec_op_t::asm_gen_code(asm_cmd_list_ptr cmd_list, bool keep_val) {
	expr->asm_get_addr(cmd_list);
	if (get_type() == ST_PTR || get_type()->is_integer()) {
		if (keep_val)
			cmd_list->mov_rderef(AR_EBX, AR_EAX, get_type_size());
		if (get_type() == ST_PTR) 
			cmd_list->_add_op_lderef(op == T_OP_INC ? AO_ADD : AO_SUB, AR_EAX, new_var<int>(get_ptr_elem_size(get_type())), AMT_DWORD);
		else if (get_type()->is_integer())
			cmd_list->_add_op_deref(token_to_int_op(op), AR_EAX, get_type_size());
		if (keep_val)
			cmd_list->mov(AR_EAX, AR_EBX, get_type_size());
	} else {
		cmd_list->fld_deref(AR_EAX, AMT_QWORD);
		if (keep_val)
			cmd_list->fld(AR_ST_0);
		cmd_list->fld1();
		cmd_list->_add_op(op == T_OP_INC ? AO_FADD : AO_FSUB);
		cmd_list->fstp_deref(AR_EAX, AMT_QWORD);
	}
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

void expr_bin_op_t::_asm_gen_code_int(asm_cmd_list_ptr cmd_list, bool keep_val) {
	if (keep_val)
		cmd_list->_add_op(token_to_int_op(op), AR_EAX, AR_EBX);
}

void expr_bin_op_t::_asm_gen_code_fp(asm_cmd_list_ptr cmd_list, bool keep_val) {
	if (keep_val)
		cmd_list->_add_op(token_to_fp_op(op));
}

void expr_bin_op_t::asm_gen_code(asm_cmd_list_ptr cmd_list, bool keep_val) {
	if (keep_val) {
		if (get_type() == ST_DOUBLE) {
			left->asm_gen_code(cmd_list, true);
			right->asm_gen_code(cmd_list, true);
			_asm_gen_code_fp(cmd_list, true);
		} else {
			right->asm_gen_code(cmd_list, true);
			cmd_list->push(AR_EAX);
			left->asm_gen_code(cmd_list, true);
			cmd_list->pop(AR_EBX);
			_asm_gen_code_int(cmd_list, true);
		}
	} else {
		left->asm_gen_code(cmd_list, false);
		right->asm_gen_code(cmd_list, false);
	}
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

void expr_base_assign_bin_op_t::_asm_gen_code_int(asm_cmd_list_ptr cmd_list, bool keep_val) {
	cmd_list->_add_op_lderef(token_to_int_op(op), AR_EBX, AR_EAX, get_type_size());
	if (keep_val)
		cmd_list->mov_rderef(AR_EAX, AR_EBX, get_type_size());
}

void expr_base_assign_bin_op_t::_asm_assign_fp_to_fp(asm_cmd_list_ptr cmd_list, bool keep_val) {}

void expr_base_assign_bin_op_t::_asm_assign_fp_to_int(asm_cmd_list_ptr cmd_list, bool keep_val) {}

void expr_base_assign_bin_op_t::asm_gen_code(asm_cmd_list_ptr cmd_list, bool keep_val) {
	if (get_type() == ST_STRUCT) {
		left->asm_get_addr(cmd_list);
		cmd_list->push(AR_EAX);
		right->asm_gen_code(cmd_list, true);
		cmd_list->pop(AR_EBX);
		cmd_list->_copy_to_mem(AR_EAX, AR_EBX, get_type_size());
	} else if (left->get_type() == ST_DOUBLE || right->get_type() == ST_DOUBLE) {
		if (left->get_type() == ST_DOUBLE && right->get_type() == ST_DOUBLE) {
			right->asm_gen_code(cmd_list, true);
			left->asm_get_addr(cmd_list);
			_asm_assign_fp_to_fp(cmd_list, keep_val);
		} else if (left->get_type() == ST_DOUBLE) {
			right->asm_gen_code(cmd_list, true);
			cmd_list->push(AR_EAX);
			left->asm_get_addr(cmd_list);
			cmd_list->pop(AR_EBX);
			if (right->get_type() == ST_CHAR) {
				cmd_list->_cast_char_to_int(AR_EBX, AR_EDX);
				cmd_list->_cast_int_to_double(AR_EDX);
			} else
				cmd_list->_cast_int_to_double(AR_EBX);
			_asm_assign_fp_to_fp(cmd_list, keep_val);
		} else {
			right->asm_gen_code(cmd_list, true);
			left->asm_get_addr(cmd_list);
			_asm_assign_fp_to_int(cmd_list, keep_val);
		}
	} else {
		left->asm_get_addr(cmd_list);
		cmd_list->push(AR_EAX);
		right->asm_gen_code(cmd_list, true);
		cmd_list->pop(AR_EBX);
		_asm_gen_code_int(cmd_list, keep_val);
	}
}

//-----------------------------------ASSIGN---------------------------------------------

expr_assign_bin_op_t::expr_assign_bin_op_t(token_ptr op) : expr_base_assign_bin_op_t(op) {
	type_convertions.push_back(tc_bo_left_to_right_type);
}

void expr_assign_bin_op_t::_asm_gen_code_int(asm_cmd_list_ptr cmd_list, bool keep_val) {
	cmd_list->mov_lderef(AR_EBX, AR_EAX, get_type_size());
}

void expr_assign_bin_op_t::_asm_assign_fp_to_fp(asm_cmd_list_ptr cmd_list, bool keep_val) {
	if (keep_val)
		cmd_list->fst_deref(AR_EAX, AMT_QWORD);
	else
		cmd_list->fstp_deref(AR_EAX, AMT_QWORD);
}

void expr_assign_bin_op_t::_asm_assign_fp_to_int(asm_cmd_list_ptr cmd_list, bool keep_val) {
	if (keep_val)
		cmd_list->fist_deref(AR_EAX, get_type_size());
	else
		cmd_list->fistp_deref(AR_EAX, get_type_size());
}

//-----------------------------------INTEGER_OPERATORS-----------------------------------

expr_integer_bin_op_t::expr_integer_bin_op_t(token_ptr token) : expr_bin_op_t(token) {
	pre_check_type_convertions.push_back(tc_bo_arr_func_to_ptr);
	or_conditions.push_back(oc_bo_is_integer);
	type_convertions.push_back(tc_bo_arithmetic_conversion);
}

expr_integer_assign_bin_op_t::expr_integer_assign_bin_op_t(token_ptr token) : expr_base_assign_bin_op_t(token) {
	or_conditions.push_back(oc_bo_is_integer);
}

//-----------------------------------ARITHMETIC_OPERATORS-----------------------------------

expr_arithmetic_bin_op_t::expr_arithmetic_bin_op_t(token_ptr token) : expr_bin_op_t(token) {
	pre_check_type_convertions.push_back(tc_bo_arr_func_to_ptr);
	or_conditions.push_back(oc_bo_is_arithmetic);
	type_convertions.push_back(tc_bo_arithmetic_conversion);
}

void expr_arithmetic_bin_op_t::_asm_gen_code_int(asm_cmd_list_ptr cmd_list, bool keep_val) {
	assert(op->is(T_OP_MUL, T_OP_DIV, 0));
	if (op == T_OP_MUL) {
		cmd_list->imul(AR_EAX, AR_EBX);
	} else if (op == T_OP_DIV) {
		cmd_list->xor_(AR_EDX, AR_EDX);
		cmd_list->div(AR_EBX);
	}
}

expr_arithmetic_assign_bin_op_t::expr_arithmetic_assign_bin_op_t(token_ptr token) : expr_base_assign_bin_op_t(token) {
	or_conditions.push_back(oc_bo_is_arithmetic);
}

void expr_arithmetic_assign_bin_op_t::_asm_gen_code_int(asm_cmd_list_ptr cmd_list, bool keep_val) {
	int type_size = get_type_size();
	if (op == T_OP_MUL_ASSIGN) {
		cmd_list->mov_rderef(AR_ECX, AR_EBX, get_type_size());
		if (left->get_type() == ST_CHAR) {
			cmd_list->_cast_char_to_int(AR_ECX, AR_EDX);
			cmd_list->mov(AR_ECX, AR_EDX);
		}
		if (right->get_type() == ST_CHAR) {
			cmd_list->_cast_char_to_int(AR_EAX, AR_EDX);
			cmd_list->mov(AR_EAX, AR_EDX);
		}
		cmd_list->imul(AR_EAX, AR_ECX);
		cmd_list->mov_lderef(AR_EBX, AR_EAX, type_size);
	} else if (op == T_OP_DIV_ASSIGN) {
		if (right->get_type() == ST_CHAR) {
			cmd_list->_cast_char_to_int(AR_EAX, AR_EDX);
			cmd_list->mov(AR_ECX, AR_EDX);
		} else
			cmd_list->mov(AR_ECX, AR_EAX);
		cmd_list->mov_rderef(AR_EAX, AR_EBX, get_type_size());
		if (left->get_type() == ST_CHAR) {
			cmd_list->_cast_char_to_int(AR_EAX, AR_EDX);
			cmd_list->mov(AR_EAX, AR_EDX);
		}
		cmd_list->xor_(AR_EDX, AR_EDX);
		cmd_list->div(AR_ECX);
		cmd_list->mov_lderef(AR_EBX, AR_EAX, type_size);
	} else
		expr_base_assign_bin_op_t::_asm_gen_code_int(cmd_list, keep_val);
}

void expr_arithmetic_assign_bin_op_t::_asm_assign_fp_to_fp(asm_cmd_list_ptr cmd_list, bool keep_val) {
	cmd_list->fld_deref(AR_EAX, AMT_QWORD);
	cmd_list->_add_op(token_to_fp_rev_op_map.at(op));
	if (keep_val)
		cmd_list->fst_deref(AR_EAX, AMT_QWORD);
	else 
		cmd_list->fstp_deref(AR_EAX, AMT_QWORD);
}

void expr_arithmetic_assign_bin_op_t::_asm_assign_fp_to_int(asm_cmd_list_ptr cmd_list, bool keep_val) {
	cmd_list->fild_deref(AR_EAX, AMT_DWORD);
	cmd_list->_add_op(token_to_fp_rev_op_map.at(op));
	if (keep_val)
		cmd_list->fist_deref(AR_EAX, AMT_QWORD);
	else
		cmd_list->fistp_deref(AR_EAX, AMT_QWORD);
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

void expr_add_bin_op_t::_asm_gen_code_int(asm_cmd_list_ptr cmd_list, bool keep_val) {
	if (left->get_type() == ST_PTR)
		mul_reg_to_elem_size(cmd_list, AR_EBX, get_ptr_elem_size(get_type()));
	else if (right->get_type() == ST_PTR)
		mul_reg_to_elem_size(cmd_list, AR_EAX, get_ptr_elem_size(get_type()));
	cmd_list->add(AR_EAX, AR_EBX);
}

expr_add_assign_bin_op_t::expr_add_assign_bin_op_t(token_ptr op) : expr_arithmetic_assign_bin_op_t(op) {
	or_conditions.push_back(oc_bo_ptr_and_integer);
	or_conditions.push_back(oc_bo_integer_and_ptr);
}

void expr_add_assign_bin_op_t::_asm_gen_code_int(asm_cmd_list_ptr cmd_list, bool keep_val) {
	if (left->get_type() == ST_PTR)
		mul_reg_to_elem_size(cmd_list, AR_EBX, get_ptr_elem_size(get_type()));
	cmd_list->add_lderef(AR_EBX, AR_EAX, get_type_size());
	if (keep_val)
		cmd_list->mov_rderef(AR_EAX, AR_EBX, get_type_size());
}

//--------------------------------------SUB----------------------------------------------

expr_sub_bin_op_t::expr_sub_bin_op_t(token_ptr op) : expr_arithmetic_bin_op_t(op) {
	or_conditions.push_back(oc_bo_is_ptrs_to_same_types);
	or_conditions.push_back(oc_bo_ptr_and_integer);
	type_convertions.push_back(tc_bo_integer_and_ptr);
	type_convertions.push_back(tc_bo_pass_ptrs);
}

void expr_sub_bin_op_t::_asm_gen_code_int(asm_cmd_list_ptr cmd_list, bool keep_val) {
	if (left->get_type() == ST_PTR)
		mul_reg_to_elem_size(cmd_list, AR_EBX, get_ptr_elem_size(get_type()));
	cmd_list->sub(AR_EAX, AR_EBX);
}

expr_sub_assign_bin_op_t::expr_sub_assign_bin_op_t(token_ptr op) : expr_arithmetic_assign_bin_op_t(op) {
	or_conditions.push_back(oc_bo_is_ptrs_to_same_types);
	or_conditions.push_back(oc_bo_ptr_and_integer);
}

void expr_sub_assign_bin_op_t::_asm_gen_code_int(asm_cmd_list_ptr cmd_list, bool keep_val) {
	if (left->get_type() == ST_PTR)
		mul_reg_to_elem_size(cmd_list, AR_EBX, get_ptr_elem_size(get_type()));
	cmd_list->sub_lderef(AR_EBX, AR_EAX, get_type_size());
	if (keep_val)
		cmd_list->mov_rderef(AR_EAX, AR_EBX, get_type_size());
}

//--------------------------------------MOD----------------------------------------------

expr_mod_bin_op_t::expr_mod_bin_op_t(token_ptr op) : expr_bin_op_t(op) {
	pre_check_type_convertions.push_back(tc_bo_arr_func_to_ptr);
	or_conditions.push_back(oc_bo_is_integer);
	type_convertions.push_back(tc_bo_integer_increase);
}

void expr_mod_bin_op_t::_asm_gen_code_int(asm_cmd_list_ptr cmd_list, bool keep_val) {
	cmd_list->xor_(AR_EDX, AR_EDX);
	cmd_list->div(AR_EBX);
	cmd_list->mov(AR_EAX, AR_EDX);
}

void expr_mod_assign_bin_op_t::_asm_gen_code_int(asm_cmd_list_ptr cmd_list, bool keep_val) {
	if (right->get_type() == ST_CHAR) {
		cmd_list->_cast_char_to_int(AR_EAX, AR_EDX);
		cmd_list->mov(AR_ECX, AR_EDX);
	} else
		cmd_list->mov(AR_ECX, AR_EAX);
	cmd_list->mov_rderef(AR_EAX, AR_EBX, get_type_size());
	if (left->get_type() == ST_CHAR) {
		cmd_list->_cast_char_to_int(AR_EAX, AR_EDX);
		cmd_list->mov(AR_EAX, AR_EDX);
	}
	cmd_list->xor_(AR_EDX, AR_EDX);
	cmd_list->div(AR_ECX);
	cmd_list->mov_lderef(AR_EBX, AR_EDX, get_type_size());
	if (keep_val)
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

void expr_shift_bin_op_t::_asm_gen_code_int(asm_cmd_list_ptr cmd_list, bool keep_val) {
	if (keep_val) {
		cmd_list->mov(AR_CL, AR_BL);
		cmd_list->_add_op(token_to_int_op(op), AR_EAX, AR_CL);
	}
}

expr_shift_assign_bin_op_t::expr_shift_assign_bin_op_t(token_ptr op) : expr_base_assign_bin_op_t(op) {
	or_conditions.push_back(oc_bo_is_integer);
}

void expr_shift_assign_bin_op_t::_asm_gen_code_int(asm_cmd_list_ptr cmd_list, bool keep_val) {
	cmd_list->mov(AR_CL, AR_AL);
	cmd_list->_add_op_lderef_ls(token_to_int_op(op),
		AR_EBX, AR_CL, asm_gen_t::mtype_by_size(get_type_size()));
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

expr_t* expr_arr_index_t::get_arr() {
	return arr;
}

expr_t* expr_arr_index_t::get_index() {
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
	arr->asm_gen_code(cmd_list, true);
	cmd_list->push(AR_EAX);
	index->asm_gen_code(cmd_list, true);
	cmd_list->pop(AR_EBX);
	mul_reg_to_elem_size(cmd_list, AR_EAX, get_type()->get_size());
	cmd_list->add(AR_EAX, AR_EBX);
}

void expr_arr_index_t::asm_gen_code(asm_cmd_list_ptr cmd_list, bool keep_val) {
	if (keep_val) {
		asm_get_addr(cmd_list);
		if (get_type() != ST_STRUCT)
			cmd_list->mov_rderef(AR_EAX, AR_EAX, get_type()->get_size());
	}
}

type_ptr expr_arr_index_t::get_type() {
	return sym_type_ptr_t::dereference(arr->get_type());
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
	try {
		member = structure->get_member(member_token);
	} catch (InvalidIncompleteType&) {
		throw InvalidIncompleteType(get_pos());
	}
	struct_expr = expr_;
}

token_ptr expr_struct_access_t::get_op() {
	return op;
}

token_ptr expr_struct_access_t::get_member() {
	return member->get_token();
}

void expr_struct_access_t::asm_get_addr(asm_cmd_list_ptr cmd_list) {
	struct_expr->asm_gen_code(cmd_list, true);
	member->asm_get_addr(cmd_list);
}

void expr_struct_access_t::asm_gen_code(asm_cmd_list_ptr cmd_list, bool keep_val) {
	struct_expr->asm_gen_code(cmd_list, keep_val);
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

inline shared_ptr<sym_type_func_t> expr_func_t::_get_func_type() {
	return
		func->get_type() == ST_FUNC_TYPE ? dynamic_pointer_cast<sym_type_func_t>(func->get_type()->get_base_type()) :
		dynamic_pointer_cast<sym_type_func_t>(sym_type_ptr_t::dereference(func->get_type())->get_base_type());
}

void expr_func_t::set_operands(expr_t* func_, vector<expr_t*> args_) {
	if (func_->get_type() != ST_FUNC_TYPE && (func_->get_type() != ST_PTR ||
		sym_type_ptr_t::dereference(func_->get_type()) != ST_FUNC))
			throw SemanticError("Called object is not a function or function pointer");
	func = func_;
	auto func_args = _get_func_type()->get_arg_types();
	if (func_args.size() != args_.size())
		throw IncorrectNumberOfArguments(args_.size(), _get_func_type(), this);
	args.resize(func_args.size());
	for (int i = 0; i < func_args.size(); i++)
		args[i] = auto_convert(args_[i], func_args[i]);
	asm_func_name = dynamic_cast<expr_var_t*>(func)->get_var()->asm_get_name();
}

type_ptr expr_func_t::get_type() {
	return _get_func_type()->get_element_type();
}

void expr_func_t::asm_gen_code(asm_cmd_list_ptr cmd_list, bool keep_val) {
	int args_size = get_args_size();
	cmd_list->push(AR_EBP);
	for (int i = args.size() - 1; i >= 0; i--) {
		args[i]->asm_gen_code(cmd_list, true);
		switch (args[i]->get_type()->get_base_type()->get_sym_type()) {
			case ST_STRUCT:
				cmd_list->_copy_to_stack(AR_EAX, args[i]->get_type_size()); 
				break;
			case ST_DOUBLE:
				cmd_list->_alloc_in_stack(args[i]->get_type_size());
				cmd_list->fstp_deref(AR_ESP, AMT_QWORD);
				break;
			case ST_CHAR:
				cmd_list->_cast_char_to_int(AR_EAX, AR_EBX);
				cmd_list->push(AR_EBX);
				break;
			default:
				cmd_list->push(AR_EAX);
		}
			
	}
	cmd_list->call(asm_func_name);
	cmd_list->_free_in_stack(args_size);
	cmd_list->pop(AR_EBP);
	if (!keep_val && get_type() == ST_DOUBLE)
		cmd_list->fdecstp();
}

int expr_func_t::get_args_size() {
	int res = 0;
	for each (auto var in args)
		res += asm_gen_t::alignment(var->get_type_size());
	return res;
}

pos_t expr_func_t::get_pos() {
	return brace->get_pos();
}

//-----------------------------------PRINTF_OPERATOR-----------------------------------

expr_printf_op_t::expr_printf_op_t(token_ptr op) : expr_func_t(op) {
	asm_func_name = "crt_printf";
}

void expr_printf_op_t::set_operands(vector<expr_t*> args_) {
	if (args_.empty())
		throw SemanticError("Printf operator requires at least one parameter", get_pos());
	if (args_[0]->get_type() != ST_PTR && args_[0]->get_type() != ST_ARRAY)
		throw SemanticError("Printf operator requires pointer to char as first parameter", get_pos());
	for (int i = 0; i < args_.size(); i++)
		tc_uo_arr_func_to_ptr(&args_[i]);
	args = args_;
}

type_ptr expr_printf_op_t::get_type() {
	return parser_t::get_type(ST_VOID);
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

void expr_cast_t::asm_gen_code(asm_cmd_list_ptr cmd_list, bool keep_val) {
	if (keep_val) {
		if (expr->get_type() == ST_CHAR && type == ST_INTEGER ||
			expr->get_type() == ST_INTEGER && type == ST_CHAR) {
			expr->asm_gen_code(cmd_list, true);
			cmd_list->mov(AR_EBX, AR_EAX);
			cmd_list->xor_(AR_EAX, AR_EAX);
			cmd_list->mov(AR_AL, AR_BL);
		} else if (expr->get_type() == ST_ARRAY && type == ST_PTR)
			expr->asm_get_addr(cmd_list);
		else if (expr->get_type() == ST_DOUBLE && type->is_integer()) {
			expr->asm_gen_code(cmd_list, true);
			cmd_list->_cast_double_to_int(AR_EAX, false);
		} else if (expr->get_type() == ST_INTEGER && type == ST_DOUBLE) {
			expr->asm_gen_code(cmd_list, true);
			cmd_list->_cast_int_to_double(AR_EAX);
		} else if (expr->get_type() == ST_CHAR && type == ST_DOUBLE) {
			expr->asm_gen_code(cmd_list, true);
			cmd_list->_cast_char_to_int(AR_EAX, AR_EBX);
			cmd_list->mov(AR_EAX, AR_EBX);
			cmd_list->_cast_int_to_double(AR_EAX);
		} else
			expr->asm_gen_code(cmd_list, true);
	} else
		expr->asm_gen_code(cmd_list, false);
}

var_ptr expr_cast_t::eval() {
	return
		type == ST_CHAR ? var_cast<char>(expr->eval()) :
		type == ST_INTEGER ? var_cast<int>(expr->eval()) :
		type == ST_DOUBLE ? var_cast<double>(expr->eval()) :
		(throw ExprMustBeEval(get_pos()), nullptr);
}
