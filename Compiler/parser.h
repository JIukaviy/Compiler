#pragma once

#include <ostream>
#include "tokens.h"
#include "lexeme_analyzer.h"
#include <vector>

using namespace std;

void parser_init();

class node_t {
public:
	virtual void print(ostream&) = 0;
	virtual void flat_print(ostream&);
};

class expr_t : public node_t {
public:
	virtual void print(ostream&, int) = 0;
	void print(ostream&) override;
};

class expr_bin_op_t: public expr_t {
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

class expr_prefix_un_op_t: public expr_un_op_t {
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

class node_str_literal : public node_t {
	token_ptr_t str;
public:
	node_str_literal(token_ptr_t str);
	void print(ostream&) override;
};

class symbol_t : public node_t {
public:
	virtual void print(ostream& os) = 0;
};

class type_t : public virtual symbol_t {
	bool is_const;
public:
	type_t(bool is_const = false);
	void set_is_const(bool val);
	void print(ostream& os) override;
	virtual bool completed();
};

class type_with_size_t : public virtual type_t {
public:
	type_with_size_t(bool is_const = 0);
	virtual int get_size() = 0;
};

class base_updatable_type_t : public virtual type_t {
public:
	base_updatable_type_t(bool is_const = 0);
	virtual void add_info(type_t* symbol) = 0;
};

template <typename T>
class updatable_type_t : public virtual base_updatable_type_t {
protected:
	T* type;
public:
	using base_updatable_type_t::base_updatable_type_t;
	void add_info(type_t* symbol) override {
		if (!symbol)
			return;
		type = dynamic_cast<T*>(symbol);
		if (!type)
			throw InvalidIncompleteType();
	}
};

class sym_var_t : public updatable_type_t<type_with_size_t> {
protected:
	token_ptr_t identifier;
	vector<expr_t*> init_values;
public:
	sym_var_t(token_ptr_t identifier);
	void print(ostream& os) override;
};

class type_void_t : public type_t {
public:
	void print(ostream& os) override;
	bool completed() override;
};

class type_int_t : public type_with_size_t {
public:
	void print(ostream& os) override;
	int get_size() override;
};

class type_char_t : public type_with_size_t {
public:
	void print(ostream& os) override;
	int get_size() override;
};

class type_double_t : public type_with_size_t {
public:
	void print(ostream& os) override;
	int get_size() override;
};

class decl_ptr_t : public updatable_type_t<type_t>, public type_with_size_t {
public:
	//using type_with_size_t::type_with_size_t;
	decl_ptr_t(bool is_const = 0);
	void print(ostream& os) override;
	int get_size() override;
};

class decl_array_t : public updatable_type_t<type_with_size_t>, public type_with_size_t {
protected:
	expr_t* size;
public:
	decl_array_t(expr_t* size = nullptr, bool is_const = false);
	void add_info(type_t* type) override;
	void print(ostream& os) override;
	bool completed() override;
	int get_size() override;
};

class decl_func_t : public updatable_type_t<type_t> {
protected:
	vector<type_t*> args;
public:
	decl_func_t(vector<type_t*>& args, bool is_const = false);
	void print(ostream& os) override {};
};

class type_alias_t : public type_t {
protected:
	type_t* type;
public:
	void print(ostream& os) override {};
};

struct type_chain_t {
	type_chain_t();
	base_updatable_type_t* first;
	base_updatable_type_t* last;
	token_ptr_t identifier;
	void update(base_updatable_type_t*);
	void update(token_ptr_t);
	void update(type_chain_t);
	operator bool();
};

struct decl_raw_t {
	token_ptr_t identifier;
	type_t* type;
	vector<node_t*> init_list;
	decl_raw_t();
	decl_raw_t(token_ptr_t, type_t*);
	decl_raw_t(type_chain_t);
};

class parser_t {
	lexeme_analyzer_t* la;

	expr_t* left_associated_bin_op(int priority);
	expr_t* tern_op();
	expr_t* right_associated_bin_op();
	expr_t* prefix_un_op();
	expr_t* postfix_op();
	expr_t* factor();
	expr_t* parse_expr();

	set<type_t*> sym_table;
	decl_raw_t declaration();
	type_chain_t declarator();
	type_chain_t init_declarator();
	type_chain_t func_arr_decl();
	vector<node_t*> parse_initializer_list();
	node_t* parse_initializer();
	symbol_t* parse_declaration();
public:
	parser_t(lexeme_analyzer_t* la_);
	void print_expr(ostream&);
	void print_decl(ostream&);
};