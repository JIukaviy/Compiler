#pragma once
#include "parser_base_node.h"
#include "tokens.h"
#include <vector>

class expr_t : public node_t {
public:
	virtual void print(ostream&, int) = 0;
	void print(ostream&) override;
};

class expr_bin_op_t : public expr_t {
protected:
	expr_t* left;
	expr_t* right;
	token_ptr_t op;
public:
	expr_bin_op_t(expr_t* left_, expr_t* right_, token_ptr_t op);
	void print(ostream&, int) override;
	void short_print(ostream&) override;
	expr_t* get_left();
	expr_t* get_right();
	virtual void set_left(expr_t* e);
	virtual void set_right(expr_t* e);
	token_ptr_t get_op();
	pos_t get_pos();
};

class expr_const_t : public expr_t {
	token_ptr_t constant;
public:
	expr_const_t(token_ptr_t constant_);
	void print(ostream&, int) override;
	void short_print(ostream&) override;
	token_ptr_t get_token();
};

class expr_var_t : public expr_t {
	token_ptr_t variable;
public:
	expr_var_t(token_ptr_t variable_);
	void print(ostream&, int) override;
	void short_print(ostream&) override;
	token_ptr_t get_token();
};

class expr_un_op_t : public expr_t {
protected:
	expr_t* expr;
	token_ptr_t op;
public:
	expr_un_op_t(expr_t* expr, token_ptr_t op);
	void print(ostream&, int) override;
	void short_print(ostream&) override;
	expr_t* get_expr();
	void set_expr(expr_t* e);
	token_ptr_t get_op();
};

class expr_prefix_un_op_t : public expr_un_op_t {
	using expr_un_op_t::expr_un_op_t;
	void print(ostream&, int) override;
	void short_print(ostream&) override;
};

class expr_postfix_un_op_t : public expr_un_op_t {
	using expr_un_op_t::expr_un_op_t;
	void print(ostream&, int) override;
	void short_print(ostream&) override;
};

class expr_tern_op_t : public expr_t {
	expr_t* left;
	expr_t* middle;
	expr_t* right;
	token_ptr_t question_mark;
	token_ptr_t colon;
public:
	expr_tern_op_t(expr_t* left, expr_t* middle, expr_t* right, token_ptr_t qm, token_ptr_t c);
	void print(ostream&, int) override;
	void short_print(ostream&) override;
	expr_t* get_left();
	expr_t* get_middle();
	expr_t* get_right();
	token_ptr_t get_question_mark_token();
	token_ptr_t get_colon_token();
	void set_left(expr_t* e);
	void set_middle(expr_t* e);
	void set_right(expr_t* e);
};

class expr_arr_index_t : public expr_t {
	expr_t* arr;
	expr_t* index;
	token_ptr_t sqr_bracket;
public:
	expr_arr_index_t(expr_t* left_, expr_t* right_, token_ptr_t sqr_bracket);
	void print(ostream&, int) override;
	void short_print(ostream&) override;
	token_ptr_t get_sqr_bracket_token();
	expr_t* get_arr();
	expr_t* get_index();
	void set_index(expr_t* e);
	void set_arr(expr_t* e);
};

class expr_func_t : public expr_t {
	expr_t* func;
	vector<expr_t*> args;
public:
	expr_func_t(expr_t* expr, vector<expr_t*> args);
	void print(ostream&, int) override;
	void short_print(ostream&) override;
};

class expr_struct_access_t : public expr_t {
	expr_t* expr;
	token_ptr_t op;
	token_ptr_t member;
public:
	expr_struct_access_t(expr_t* expr, token_ptr_t op, token_ptr_t ident);
	void print(ostream&, int) override;
	void short_print(ostream&) override;
	expr_t* get_expr();
	void set_expr(expr_t* e);
	token_ptr_t get_op();
	token_ptr_t get_member();
};

class expr_cast_t : public expr_t {
	expr_t* expr;
	node_ptr_t type;
public: 
	expr_cast_t(expr_t* expr, node_ptr_t type);
	void print(ostream&, int) override;
	void short_print(ostream&) override;
};