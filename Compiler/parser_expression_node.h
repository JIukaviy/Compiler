#pragma once
#include "parser_symbol_node.h"
#include "tokens.h"
#include <vector>

class expr_t : public node_t {
	bool lvalue;
public:
	expr_t(bool lvalue = false);
	bool is_lvalue();
	virtual pos_t get_pos() = 0;
	virtual type_ptr get_type() = 0;
};

//-------------CONSTANT------------

class expr_const_t : public expr_t {
	token_ptr constant;
public:
	expr_const_t(token_ptr constant_);
	void print_l(ostream& os, int level) override;
	void short_print_l(ostream& os, int level) override;
	token_ptr get_token();
	type_ptr get_type() override;
	pos_t get_pos();
	bool is_null();
};

//------------VARIABLE_OR_FUNCTION----------

class expr_var_t : public expr_t {
	shared_ptr<sym_with_type_t> variable;
public:
	expr_var_t();
	void print_l(ostream& os, int level) override;
	void short_print_l(ostream& os, int level) override;
	void set_symbol(shared_ptr<sym_with_type_t> var);
	token_ptr get_token();
	type_ptr get_type() override;
	pos_t get_pos();
};

//-----------------UNARY_OPERATORS-----------------

class expr_un_op_t : public expr_t {
protected:
	expr_t* expr;
	token_ptr op;
	vector<void(*)(expr_t* operand, expr_un_op_t* op)> operand_checkers;
	vector<bool(*)(expr_t** operand, expr_un_op_t* op)> type_convertions;
public:
	expr_un_op_t(token_ptr op, bool lvalue = false);
	void print_l(ostream& os, int level) override;
	void short_print_l(ostream& os, int level) override;
	expr_t* get_expr();
	void set_operand(expr_t* operand);
	token_ptr get_op();
	type_ptr get_type() override;
};

class expr_prefix_un_op_t : public expr_un_op_t {
public:
	using expr_un_op_t::expr_un_op_t;
	void print_l(ostream& os, int level) override;
	void short_print_l(ostream& os, int level) override;
};

class expr_addr_op_t : public expr_prefix_un_op_t {
public:
	expr_addr_op_t(token_ptr op);
};

class expr_dereference_op_t : public expr_prefix_un_op_t {
public:
	expr_dereference_op_t(token_ptr op);
};

class expr_prefix_inc_dec_op_t : public expr_prefix_un_op_t {
public:
	expr_prefix_inc_dec_op_t(token_ptr op);
};

class expr_postfix_un_op_t : public expr_un_op_t {
public:
	using expr_un_op_t::expr_un_op_t;
	void print_l(ostream& os, int level) override;
	void short_print_l(ostream& os, int level) override;
};

class expr_postfix_inc_dec_op_t : public expr_postfix_un_op_t {
	expr_postfix_inc_dec_op_t(token_ptr op);
};


//---------------BINARY_OPERATORS-------------------

class expr_bin_op_t : public expr_t {
protected:
	expr_t* left;
	expr_t* right;
	token_ptr op;
	vector<void(*)(expr_t* left, expr_t* right, expr_bin_op_t* op)> and_conditions;
	vector<bool(*)(expr_t* left, expr_t* right)> or_conditions;
	vector<bool(*)(expr_t** left, expr_t** right, expr_bin_op_t* op)> type_convertions;
public:
	expr_bin_op_t(token_ptr op);
	void print_l(ostream& os, int level) override;
	void short_print_l(ostream& os, int level) override;
	expr_t* get_left();
	expr_t* get_right();
	void set_operands(expr_t* left, expr_t* right);
	token_ptr get_op();
	pos_t get_pos() override;
	type_ptr get_type() override;
	static expr_bin_op_t* make_bin_op(token_ptr op);
};

class expr_assign_bin_op_t : public expr_bin_op_t {
public:
	expr_assign_bin_op_t(token_ptr op);
};

class expr_integer_bin_op_t : public expr_bin_op_t {
public:
	expr_integer_bin_op_t(token_ptr token);
};

class expr_integer_assign_bin_op_t : public expr_assign_bin_op_t {
public:
	expr_integer_assign_bin_op_t(token_ptr token);
};

class expr_ariphmetic_bin_op_t : public expr_bin_op_t {
public:
	expr_ariphmetic_bin_op_t(token_ptr token);
};

class expr_ariphmetic_assign_bin_op_t : public expr_assign_bin_op_t {
public:
	expr_ariphmetic_assign_bin_op_t(token_ptr token);
};

class expr_add_bin_op_t : public expr_bin_op_t {
public:
	expr_add_bin_op_t(token_ptr op);
};

class expr_add_assign_bin_op_t : public expr_assign_bin_op_t {
public:
	expr_add_assign_bin_op_t(token_ptr op);
};

class expr_sub_bin_op_t : public expr_bin_op_t {
public:
	expr_sub_bin_op_t(token_ptr op);
};

class expr_sub_assign_bin_op_t : public expr_assign_bin_op_t {
public:
	expr_sub_assign_bin_op_t(token_ptr op);
};

class expr_mod_bin_op_t : public expr_bin_op_t {
public:
	expr_mod_bin_op_t(token_ptr op);
};

class expr_mod_assign_bin_op_t : public expr_assign_bin_op_t {
public:
	expr_mod_assign_bin_op_t(token_ptr op);
};

class expr_relational_bin_op_t : public expr_ariphmetic_bin_op_t {
public:
	expr_relational_bin_op_t(token_ptr op);
	type_ptr get_type() override;
};

class expr_equality_bin_op_t : public expr_relational_bin_op_t {
public:
	expr_equality_bin_op_t(token_ptr op);
};

class expr_logical_bin_op_t : public expr_bin_op_t {
public:
	expr_logical_bin_op_t(token_ptr op);
};

class expr_shift_bin_op_t : public expr_bin_op_t {
public:
	expr_shift_bin_op_t(token_ptr op);
};

class expr_shift_assign_bin_op_t : public expr_assign_bin_op_t {
public:
	expr_shift_assign_bin_op_t(token_ptr op);
};

//---------------TERNARY_OPERATOR-------------------

class expr_tern_op_t : public expr_t {
	expr_t* condition;
	expr_t* left;
	expr_t* right;
	token_ptr question_mark;
	token_ptr colon;
public:
	expr_tern_op_t(token_ptr qm, token_ptr c);
	void print_l(ostream& os, int level) override;
	void short_print_l(ostream& os, int level) override;
	expr_t* get_condition();
	expr_t* get_left();
	expr_t* get_right();
	token_ptr get_question_mark_token();
	token_ptr get_colon_token();
	void set_operands(expr_t* condition, expr_t* left, expr_t* right) {};
	type_ptr get_type() override;
	pos_t get_pos() override { return pos_t(); };
};

//----------------ARRAY_INDEX----------------------

class expr_arr_index_t : public expr_t {
	expr_t* arr;
	expr_t* index;
	token_ptr sqr_bracket;
public:
	expr_arr_index_t(token_ptr sqr_bracket);
	void print_l(ostream& os, int level) override;
	void short_print_l(ostream& os, int level) override;
	token_ptr get_sqr_bracket_token();
	expr_t* get_arr();
	expr_t* get_index();
	void set_operands(expr_t* arr, expr_t* index) {};
	type_ptr get_type() override;
};

//----------------FUNCTION_CALL---------------

class expr_func_t : public expr_t {
	expr_t* func;
	vector<expr_t*> args;
public:
	expr_func_t(expr_t* expr, vector<expr_t*> args);
	void print_l(ostream& os, int level) override;
	void short_print_l(ostream& os, int level) override;
	void set_operands(expr_t* f, vector<expr_t*> args_) { func = f;  args = args_; };
	type_ptr get_type() override;
};

//----------------STRUCT_ACCESS-------------------

class expr_struct_access_t : public expr_t {
	expr_t* expr;
	token_ptr op;
	shared_ptr<sym_var_t> member;
public:
	expr_struct_access_t(token_ptr op);
	void print_l(ostream& os, int level) override;
	void short_print_l(ostream& os, int level) override;
	expr_t* get_expr();
	void set_operands(expr_t* expr, token_ptr member) {};
	token_ptr get_op();
	token_ptr get_member() { return token_ptr(); };
	type_ptr get_type() override;
};

//----------------CAST-------------------

class expr_cast_t : public expr_t {
	expr_t* expr;
	type_ptr type;
public: 
	expr_cast_t();
	void print_l(ostream& os, int level) override;
	void short_print_l(ostream& os, int level) override;
	void set_operand(expr_t* expr, type_ptr type);
	type_ptr get_type() override;
	pos_t get_pos() override;
};