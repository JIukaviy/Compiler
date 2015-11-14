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
	token_ptr op;
public:
	expr_bin_op_t(expr_t* left_, expr_t* right_, token_ptr op);
	void print(ostream&, int) override;
	void short_print(ostream&) override;
	expr_t* get_left();
	expr_t* get_right();
	virtual void set_left(expr_t* e);
	virtual void set_right(expr_t* e);
	token_ptr get_op();
	pos_t get_pos();
};

class expr_const_t : public expr_t {
	token_ptr constant;
public:
	expr_const_t(token_ptr constant_);
	void print(ostream&, int) override;
	void short_print(ostream&) override;
	token_ptr get_token();
};

class expr_var_t : public expr_t {
	token_ptr variable;
public:
	expr_var_t(token_ptr variable_);
	void print(ostream&, int) override;
	void short_print(ostream&) override;
	token_ptr get_token();
};

class expr_un_op_t : public expr_t {
protected:
	expr_t* expr;
	token_ptr op;
public:
	expr_un_op_t(expr_t* expr, token_ptr op);
	void print(ostream&, int) override;
	void short_print(ostream&) override;
	expr_t* get_expr();
	void set_expr(expr_t* e);
	token_ptr get_op();
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
	token_ptr question_mark;
	token_ptr colon;
public:
	expr_tern_op_t(expr_t* left, expr_t* middle, expr_t* right, token_ptr qm, token_ptr c);
	void print(ostream&, int) override;
	void short_print(ostream&) override;
	expr_t* get_left();
	expr_t* get_middle();
	expr_t* get_right();
	token_ptr get_question_mark_token();
	token_ptr get_colon_token();
	void set_left(expr_t* e);
	void set_middle(expr_t* e);
	void set_right(expr_t* e);
};

class expr_arr_index_t : public expr_t {
	expr_t* arr;
	expr_t* index;
	token_ptr sqr_bracket;
public:
	expr_arr_index_t(expr_t* left_, expr_t* right_, token_ptr sqr_bracket);
	void print(ostream&, int) override;
	void short_print(ostream&) override;
	token_ptr get_sqr_bracket_token();
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
	token_ptr op;
	token_ptr member;
public:
	expr_struct_access_t(expr_t* expr, token_ptr op, token_ptr ident);
	void print(ostream&, int) override;
	void short_print(ostream&) override;
	expr_t* get_expr();
	void set_expr(expr_t* e);
	token_ptr get_op();
	token_ptr get_member();
};

class expr_cast_t : public expr_t {
	expr_t* expr;
	node_ptr type;
public: 
	expr_cast_t(expr_t* expr, node_ptr type);
	void print(ostream&, int) override;
	void short_print(ostream&) override;
};