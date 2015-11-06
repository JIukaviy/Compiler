#include "expression_optimizer.h"
#include <assert.h>
#include <map>

/*class expr_res_t {
	SYM_TYPE expr_res_type;
public:
	bool operator==(SYM_TYPE ert);
};

template<typename T, SYM_TYPE ERT>
class expr_res_val_t {
	SYM_TYPE expr_res_type = ERT;
	T val;
public:
	T get_val();
};

template<typename T, SYM_TYPE ERT>
inline T expr_res_val_t<T, ERT>::get_val() {
	return val;
}

bool expr_res_t::operator==(SYM_TYPE ert) {
	return expr_res_type == ert;
}*/

map<TOKEN, map<SYM_TYPE, set<SYM_TYPE>>> bin_ops_operands;

void init_expr_optimizer() {
	bin_ops_operands[T_OP_MUL][ST_INTEGER].insert(ST_INTEGER);
	bin_ops_operands[T_OP_MUL][ST_INTEGER].insert(ST_DOUBLE);
	bin_ops_operands[T_OP_MUL][ST_INTEGER].insert(ST_CHAR);
	bin_ops_operands[T_OP_MUL][ST_CHAR].insert(ST_CHAR);
	bin_ops_operands[T_OP_MUL][ST_CHAR].insert(ST_INTEGER);
	bin_ops_operands[T_OP_MUL][ST_CHAR].insert(ST_DOUBLE);
	bin_ops_operands[T_OP_MUL][ST_DOUBLE].insert(ST_DOUBLE);
	bin_ops_operands[T_OP_MUL][ST_DOUBLE].insert(ST_INTEGER);
	bin_ops_operands[T_OP_MUL][ST_DOUBLE].insert(ST_CHAR);

	bin_ops_operands[T_OP_DIV] = bin_ops_operands[T_OP_MUL];
	bin_ops_operands[T_OP_ADD] = bin_ops_operands[T_OP_MUL];
	bin_ops_operands[T_OP_SUB] = bin_ops_operands[T_OP_MUL];
}

struct expr_res_t {
	type_ptr_t type;
	expr_t* expr = 0;
	bool left_hand_value = false;
	expr_res_t(expr_t* expr, type_ptr_t type, bool left_hand_value) : expr(expr), type(type), left_hand_value(left_hand_value) {}
	expr_res_t() : expr(0) {}
	operator bool() {
		return expr;
	}
};

template <class T>
expr_res_t try_valid(expr_t* expr, sym_table_t& st, expr_res_t (*func)(T*, sym_table_t&)) {
	T* t = dynamic_cast<T*>(expr);
	return t ? func(t, st) : expr_res_t();
}

#define try_valid(name) if (res = try_valid<expr_##name##_t>(expr, st, valid_##name)) return res

expr_res_t add_cast_expr(expr_res_t er, type_ptr_t type) {
	er.type = type;
	er.expr = new expr_cast_t(er.expr, er.type);
	return er;
}

expr_res_t add_cast_expr(expr_res_t er, SYM_TYPE sym_type) {
	return add_cast_expr(er, type_t::make_type(sym_type));
}

bool is_zero(expr_t* expr) {
	expr_const_t* c = dynamic_cast<expr_const_t*>(expr);
	if (!c)
		return false;
	auto d = dynamic_pointer_cast<token_with_value_t<int>>(c->get_token());
	if (d->get_value() == 0)
		return true;
	auto ch = dynamic_pointer_cast<token_with_value_t<char>>(c->get_token());
	if (ch->get_value() == 0)
		return true;
	return false;
}

expr_res_t automate_cast_expr(expr_res_t er, type_ptr_t type, pos_t pos) {
	expr_res_t res = er;
	if (er.type == type)
		return er;
	else if (er.type == ST_PTR && type == ST_PTR && dynamic_pointer_cast<sym_type_ptr_t>(type)->get_element_type() == ST_VOID)
		return res = add_cast_expr(er, type);
	else if (er.type == ST_CHAR) {
		if (type == ST_INTEGER)
			return res = add_cast_expr(er, type);
		else if (type == ST_DOUBLE)
			return res = add_cast_expr(er, type);
	} else if (er.type == ST_INTEGER) {
		if (type == ST_CHAR)
			return res = add_cast_expr(er, type);
		else if (type == ST_DOUBLE)
			return res = add_cast_expr(er, type);
	}
	throw IllegalConversion(er.type, type, pos);
}

expr_res_t automate_cast_expr(expr_res_t er, SYM_TYPE sym_type, pos_t pos) {
	return automate_cast_expr(er, type_t::make_type(sym_type), pos);
}

expr_res_t valid_expr(expr_t* expr, sym_table_t& st);

expr_res_t valid_const(expr_const_t* constant, sym_table_t& st) {
	int a;
	token_ptr_t token = constant->get_token();
	assert(token->is(T_INTEGER, T_DOUBLE, T_CHAR, T_STRING, 0));
	type_ptr_t type = type_t::make_type(symbol_t::token_to_sym_type(token->get_token_id()));
	type->set_const(true);
	return expr_res_t(constant, type, false);
}

expr_res_t valid_var(expr_var_t* var, sym_table_t& st) {
	token_ptr_t token = var->get_token();
	sym_ptr_t sym = st.get(token);
	if (!sym)
		UndefinedSymbol(var->get_token());
	if (sym == ST_ALIAS)
		throw SemanticError("Using type name is not allowed", token->get_pos());
	assert(sym == ST_VAR);
	type_ptr_t type = static_cast<sym_var_t*>(sym.get())->get_type();
	if (type == ST_ARRAY) {
		auto ptr = new sym_type_ptr_t();
		ptr->set_element_type(dynamic_pointer_cast<sym_type_array_t>(type)->get_element_type(), pos_t());
		type = type_ptr_t(ptr);
	}
	return expr_res_t(var, type, true);
}

expr_res_t valid_bin_op(expr_bin_op_t* bin_op, sym_table_t& st) {
	expr_res_t left  = valid_expr(bin_op->get_left(),  st);
	expr_res_t right = valid_expr(bin_op->get_right(), st);
	expr_res_t res;
	res.expr = bin_op;
	token_ptr_t op = bin_op->get_op();
	try {
		if (op >= T_OP_ASSIGN && op <= T_OP_RIGHT_ASSIGN) {
			if (!left.left_hand_value)
				throw ExprMustBeLeftHandValue(bin_op->get_pos());
			if (left.type->is_const())
				throw SemanticError("Left expression is constant", bin_op->get_pos());
			res.left_hand_value = true;
		}
		if (op == T_OP_ASSIGN) {
			if (left.type == ST_PTR && is_zero(right.expr))
				right = add_cast_expr(right, ST_INTEGER);
			else
				right = automate_cast_expr(right, left.type, op->get_pos());
			res.type = left.type;
		} else
		if (op->is(T_OP_ADD_ASSIGN, T_OP_SUB_ASSIGN, T_OP_MUL_ASSIGN, T_OP_DIV_ASSIGN, 0)) {
			if (op->is(T_OP_ADD_ASSIGN, T_OP_SUB_ASSIGN, 0) && left.type == ST_PTR)
				right = automate_cast_expr(right, ST_INTEGER, op->get_pos());
			else if (left.type == ST_DOUBLE || left.type == ST_INTEGER || left.type == ST_CHAR)
				right = automate_cast_expr(right, left.type, op->get_pos());
			else 
				throw InvalidBinOpOperands(left.type, right.type, bin_op);
			res.type = left.type;
		}
		else if (op->is(T_OP_ADD, T_OP_SUB, T_OP_MUL, T_OP_DIV,
						T_OP_L, T_OP_G, T_OP_LE, T_OP_GE, T_OP_EQ, T_OP_NEQ, 0)) {
				if (op->is(T_OP_ADD, T_OP_SUB, 0) && left.type == ST_PTR)
					if (right.type == ST_PTR)
						res.type = type_t::make_type(ST_INTEGER);
					else
						right = automate_cast_expr(right, ST_INTEGER, op->get_pos());
				else 
					if (left.type == ST_DOUBLE || right.type == ST_DOUBLE) {
						left = automate_cast_expr(left, ST_DOUBLE, op->get_pos());
						right = automate_cast_expr(right, ST_DOUBLE, op->get_pos());
				} else 
					if (left.type == ST_INTEGER || right.type == ST_INTEGER) {
						left = automate_cast_expr(left, ST_INTEGER, op->get_pos());
						right = automate_cast_expr(right, ST_INTEGER, op->get_pos());
				} else 
					if (left.type != ST_CHAR && right.type != ST_CHAR)
						throw InvalidBinOpOperands(left.type, right.type, bin_op);
				if (op->is(T_OP_L, T_OP_G, T_OP_LE, T_OP_GE, T_OP_EQ, T_OP_NEQ, 0))
					res.type = type_t::make_type(ST_INTEGER);
				else if (!res.type)
					res.type = left.type;
		} else if (op->is(T_OP_AND, T_OP_BIT_AND, T_OP_BIT_AND_ASSIGN, 
						  T_OP_OR, T_OP_BIT_OR, T_OP_BIT_OR_ASSIGN, 
						  T_OP_XOR, T_OP_XOR_ASSIGN, T_OP_MOD, T_OP_MOD_ASSIGN, 
						  T_OP_LEFT, T_OP_LEFT_ASSIGN, T_OP_RIGHT, T_OP_RIGHT_ASSIGN, 0)) {
			left = automate_cast_expr(left, ST_INTEGER, bin_op->get_pos());
			right = automate_cast_expr(right, ST_INTEGER, bin_op->get_pos());
			res.type = left.type;
		} else
			assert(false);
		bin_op->set_left(left.expr);
		bin_op->set_right(right.expr);
		return res;
	} catch (IllegalConversion&) {
		throw InvalidBinOpOperands(left.type, right.type, bin_op);
	}
}

expr_res_t valid_un_op(expr_un_op_t* un_op, sym_table_t& st) {
	expr_res_t expr = valid_expr(un_op->get_expr(), st);
	token_ptr_t op = un_op->get_op();
	expr_res_t res;
	res.expr = un_op;
	if (op == T_OP_INC || op == T_OP_DEC) {
		if (
				expr.type != ST_DOUBLE &&
				expr.type != ST_INTEGER &&
				expr.type != ST_CHAR &&
				expr.type != ST_PTR)
			throw InvalidUnOpOperand(expr.type, un_op);
		if (!expr.left_hand_value)
			throw ExprMustBeLeftHandValue(op->get_pos());
		if (expr.type->is_const())
			throw SemanticError("Expression is constant", op->get_pos());
		res.left_hand_value = true;
	} else if (op == T_OP_MUL) {
		if (expr.type != ST_PTR)
			throw InvalidUnOpOperand(expr.type, un_op);
		res.type = dynamic_pointer_cast<sym_type_ptr_t>(expr.type)->get_element_type();	
		res.left_hand_value = true;
	} else if (op->is(T_OP_BIT_NOT, T_OP_NOT, 0)) {
		expr = automate_cast_expr(expr, ST_INTEGER, op->get_pos());
		res.type = expr.type;
	} else if (op == T_OP_BIT_AND) {
		if (!expr.left_hand_value)
			throw SemanticError("Expression must be left-hand value or function", op->get_pos());
		shared_ptr<sym_type_ptr_t> ptr(new sym_type_ptr_t);
		ptr->set_element_type(expr.type, op->get_pos());
		res.type = ptr;
	} else
		assert(false);
	un_op->set_expr(expr.expr);
	return res;
}

expr_res_t valid_tern_op(expr_tern_op_t* tern_op, sym_table_t& st) {
	expr_res_t left = valid_expr(tern_op->get_left(), st);
	expr_res_t middle = valid_expr(tern_op->get_middle(), st);
	expr_res_t right = valid_expr(tern_op->get_right(), st);
	token_ptr_t qm = tern_op->get_question_mark_token();
	token_ptr_t c = tern_op->get_colon_token();
	expr_res_t res;
	res.expr = tern_op;

	try {
		left = automate_cast_expr(left, ST_INTEGER, qm->get_pos());
	} catch (IllegalConversion&) {
		throw SemanticError("Expression before \"?\" must be convertible to \"int\"", qm->get_pos());
	}

	if (middle.type == ST_DOUBLE || right.type == ST_DOUBLE) {
		left = automate_cast_expr(left, ST_DOUBLE, c->get_pos());
		right = automate_cast_expr(right, ST_DOUBLE, c->get_pos());
	} else
	if (middle.type == ST_INTEGER || right.type == ST_INTEGER) {
		middle = automate_cast_expr(middle, ST_INTEGER, c->get_pos());
		right = automate_cast_expr(right, ST_INTEGER, c->get_pos());
	} else
	if (left.type != right.type)
		throw InvalidTernOpOperands(left.type, right.type, tern_op);
	res.type = left.type;
	return res;
}

expr_res_t valid_arr_index(expr_arr_index_t* arr_index, sym_table_t& st) {
	expr_res_t arr = valid_expr(arr_index->get_arr(), st);
	expr_res_t index = valid_expr(arr_index->get_index(), st);
	expr_res_t res;
	pos_t op_pos = arr_index->get_sqr_bracket_token()->get_pos();
	res.expr = arr_index;
	if (arr.type != ST_PTR)
		throw SemanticError("Left operand of \"[]\" must be the pointer", op_pos);
	res.type = dynamic_pointer_cast<sym_type_ptr_t>(arr.type)->get_element_type();
	try {
		index = automate_cast_expr(index, ST_INTEGER, op_pos);
	} catch (IllegalConversion&) {
		throw SemanticError("Index must be convertible to \"int\"", op_pos);
	}
	return res;
}

expr_res_t valid_func(expr_func_t* func, sym_table_t& st) {
	return expr_res_t();
}

expr_res_t valid_struct_access(expr_struct_access_t* str, sym_table_t& st) {
	return expr_res_t();
}

expr_res_t valid_cast(expr_cast_t* cast, sym_table_t& st) {
	return expr_res_t();
}

expr_res_t valid_expr(expr_t* expr, sym_table_t& st) {
	expr_res_t res;
	//if (res = try_valid<expr_const_t>(expr, st, valid_bin_op)) return res;
	try_valid(const);
	try_valid(var);
	try_valid(bin_op);
	try_valid(un_op);
	try_valid(tern_op);
	try_valid(arr_index);
	try_valid(func);
	try_valid(struct_access);
	try_valid(cast);
	assert(false);
}

expr_t* validate_expr(expr_t* expr, sym_table_t& st) {
	return valid_expr(expr, st).expr;
}