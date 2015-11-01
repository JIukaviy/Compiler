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
	void flat_print(ostream&) override;
};

class expr_const_t : public expr_t {
	token_ptr_t constant;
public:
	expr_const_t(token_ptr_t constant_);
	void print(ostream&, int) override;
	void flat_print(ostream&) override;
};

class expr_var_t : public expr_t {
	token_ptr_t variable;
public:
	expr_var_t(token_ptr_t variable_);
	void print(ostream&, int) override;
	void flat_print(ostream&) override;
};

class expr_un_op_t : public expr_t {
protected:
	expr_t* expr;
	token_ptr_t op;
public:
	expr_un_op_t(expr_t* expr, token_ptr_t op);
	void print(ostream&, int) override;
	void flat_print(ostream&) override;
};

class expr_prefix_un_op_t : public expr_un_op_t {
	using expr_un_op_t::expr_un_op_t;
	void print(ostream&, int) override;
	void flat_print(ostream&) override;
};

class expr_postfix_un_op_t : public expr_un_op_t {
	using expr_un_op_t::expr_un_op_t;
	void print(ostream&, int) override;
	void flat_print(ostream&) override;
};

class expr_tern_op_t : public expr_t {
	expr_t* left;
	expr_t* middle;
	expr_t* right;
public:
	expr_tern_op_t(expr_t* left, expr_t* middle, expr_t* right);
	void print(ostream&, int) override;
	void flat_print(ostream&) override;
};

class expr_arr_index_t : public expr_t {
	expr_t* left;
	expr_t* right;
public:
	expr_arr_index_t(expr_t* left_, expr_t* right_);
	void print(ostream&, int) override;
	void flat_print(ostream&) override;
};

class expr_func_t : public expr_t {
	expr_t* func;
	vector<expr_t*> args;
public:
	expr_func_t(expr_t* expr, vector<expr_t*> args);
	void print(ostream&, int) override;
	void flat_print(ostream&) override;
};

class expr_struct_access_t : public expr_t {
	expr_t* expr;
	token_ptr_t op;
	token_ptr_t ident;
public:
	expr_struct_access_t(expr_t* expr, token_ptr_t op, token_ptr_t ident);
	void print(ostream&, int) override;
	void flat_print(ostream&) override;
};