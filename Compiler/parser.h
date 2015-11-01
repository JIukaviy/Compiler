#pragma once

#include <ostream>
#include "tokens.h"
#include "lexeme_analyzer.h"
#include <vector>
#include <unordered_set>

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
protected:
	token_ptr_t str;
public:
	node_str_literal(token_ptr_t str);
	void print(ostream&) override;
};

class symbol_t : public node_t {
protected:
	bool printed = false;
	size_t cached_hash = 0;
	virtual size_t _get_hash() const = 0;
public:
	virtual void print(ostream& os) = 0;
	void update_hash();
	size_t get_hash() const;
	virtual bool eq(const symbol_t*) const = 0;
};

template<> struct std::hash<symbol_t*> {
	size_t operator()(const symbol_t* s) const {
		return s->get_hash();
	}
};

template<> struct std::equal_to<symbol_t*> {
	bool operator()(const symbol_t* a, const symbol_t* b) const {
		return a->eq(b);
	}
};

class type_t : public symbol_t {
	bool is_const_;
public:
	type_t(bool is_const_ = false);
	virtual void set_const(bool val);
	void print(ostream& os) override;
	virtual bool is_const();
	virtual bool completed();
};

class type_with_size_t : public virtual type_t {
public:
	type_with_size_t(bool is_const_ = 0);
	virtual int get_size() = 0;
};

class updatable_sym_t : public virtual type_t {
public:
	updatable_sym_t(bool is_const_ = 0);
	virtual void set_element_type(type_t* symbol, pos_t pos) = 0;		// текущая позиция необходима в случае вывода ошибок
};

/*template <typename T>
class updatable_sym_t : public virtual updatable_sym_t {
protected:
	T* type;
public:
	using updatable_sym_t::updatable_sym_t;
	void set_element_type(type_t* symbol) override {
		if (!symbol)
			return;
		type = dynamic_cast<T*>(symbol);
		if (!type)
			throw InvalidIncompleteType();
	}
};*/

class sym_var_t : public symbol_t {
protected:
	token_ptr_t identifier;
	vector<node_t*> init_list;
	type_t* type;
	size_t _get_hash() const override;
public:
	sym_var_t(token_ptr_t identifier, type_t* type, vector<node_t*> init_list);
	void print(ostream& os) override;
	bool eq(const symbol_t*) const override;
};

class sym_type_void_t : public type_t {
protected:
	size_t _get_hash() const override;
public:
	void print(ostream& os) override;
	bool completed() override;
	bool eq(const symbol_t*) const override;
};

class sym_type_int_t : public type_with_size_t {
protected:
	size_t _get_hash() const override;
public:
	void print(ostream& os) override;
	int get_size() override;
	bool eq(const symbol_t*) const override;
};

class sym_type_char_t : public type_with_size_t {
protected:
	size_t _get_hash() const override;
public:
	void print(ostream& os) override;
	int get_size() override;
	bool eq(const symbol_t*) const override;
};

class sym_type_double_t : public type_with_size_t {
protected:
	size_t _get_hash() const override;
public:
	void print(ostream& os) override;
	int get_size() override;
	bool eq(const symbol_t*) const override;
};

class sym_type_ptr_t : public updatable_sym_t, public type_with_size_t {
protected:
	type_t* type;
	size_t _get_hash() const override;
public:
	//using type_with_size_t::type_with_size_t;
	sym_type_ptr_t(bool is_const_ = 0);
	void set_element_type(type_t* type, pos_t pos) override;
	void print(ostream& os) override;
	int get_size() override;
	bool eq(const symbol_t*) const override;
};

class sym_type_array_t : public updatable_sym_t, public type_with_size_t {
protected:
	expr_t* size;
	type_with_size_t* elem_type;
	size_t _get_hash() const override;
public:
	sym_type_array_t(expr_t* size = nullptr, bool is_const_ = false);
	void set_element_type(type_t* type, pos_t pos) override;
	void print(ostream& os) override;
	bool completed() override;
	int get_size() override;
	bool eq(const symbol_t*) const override;
};

class sym_type_func_t : public updatable_sym_t {
protected:
	vector<type_t*> args;
	type_t* ret_type;
	//size_t _get_hash() const override;
public:
	sym_type_func_t(vector<type_t*>& args, bool is_const_ = false);
	void set_element_type(type_t* type, pos_t pos) override;
	void print(ostream& os) override {};
};

class sym_type_alias_t : public type_t {
protected:
	type_t* type;
	token_ptr_t identifier;
	size_t _get_hash() const override;
public:
	sym_type_alias_t::sym_type_alias_t(token_ptr_t identifier, type_t* type);
	bool is_const() override;
	void set_const(bool val) override;
	void print(ostream& os) override;
	bool eq(const symbol_t*) const override;
	type_t* get_type();
};

struct type_chain_t {
	type_chain_t();
	updatable_sym_t* first;
	updatable_sym_t* last;
	pos_t last_token_pos;
	token_ptr_t identifier;
	pos_t estimated_ident_pos;
	void update(updatable_sym_t*);
	void update(token_ptr_t);
	void update(type_chain_t);
	operator bool();
};

struct decl_raw_t {
	token_ptr_t identifier;
	token_ptr_t type_def; //token_ptr_t необходим для вывода места в коде где он встретился, в случае ошибки.
	type_t* type;
	pos_t estimated_ident_pos;
	pos_t type_spec_pos;
	vector<node_t*> init_list;
	decl_raw_t();
	decl_raw_t(token_ptr_t, type_t*);
	decl_raw_t(type_chain_t);
};

class sym_table_t : public unordered_set<symbol_t*> {
public:
	symbol_t* get(symbol_t*);
	symbol_t* get(token_ptr_t);
	bool is_var(token_ptr_t);
	bool is_alias(token_ptr_t);
	//void insert(symbol_t*);
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

	sym_table_t sym_table;
	decl_raw_t declaration();
	type_chain_t declarator();
	type_chain_t init_declarator();
	type_chain_t func_arr_decl();
	vector<node_t*> parse_initializer_list();
	node_t* parse_initializer();
	symbol_t* parse_declaration();

	node_t* parse_state();
public:
	parser_t(lexeme_analyzer_t* la_);
	void print_expr(ostream&);
	void print_type(ostream&);
	void print_decl(ostream&);
};