#pragma once
#include "tokens.h"
#include "parser_base_node.h"
#include "parser_expression_node.h"
#include "base_symbol_table.h"

enum SYM_TYPE {
	ST_INTEGER,
	ST_DOUBLE,
	ST_CHAR,
	ST_STRING,
	ST_STRUCT,
	ST_ARRAY,
	ST_PTR,
	ST_FUNC,
	ST_ALIAS,
	ST_VAR,
	ST_VOID,
	ST_QL  //Quilifer list (only const in this case)
};

void init_parser_symbol_node();

class symbol_t;
class type_t;
class type_base_t;
class updatable_base_type_t;

class sym_ptr : public shared_ptr<symbol_t> {
public:
	using shared_ptr<symbol_t>::shared_ptr;
	bool operator==(SYM_TYPE sym_type) const;
	bool operator!=(SYM_TYPE sym_type) const;
};

typedef shared_ptr<updatable_base_type_t> upd_type_ptr;

class type_ptr : public shared_ptr<type_t> {
public:
	using shared_ptr<type_t>::shared_ptr;
	bool operator==(SYM_TYPE sym_type) const;
	bool operator!=(SYM_TYPE sym_type) const;
	bool operator==(type_ptr type) const;
	bool operator!=(type_ptr type) const;
};

class type_base_ptr : public shared_ptr<type_base_t> {
public:
	using shared_ptr<type_base_t>::shared_ptr;
	bool operator==(SYM_TYPE sym_type) const;
	bool operator!=(SYM_TYPE sym_type) const;
	bool operator==(type_ptr type) const;
	bool operator!=(type_ptr type) const;
};

class symbol_t : public node_t {
protected:
	virtual string _get_name() const = 0;
	string name;
	SYM_TYPE symbol_type;
	token_ptr token;
public:
	symbol_t();
	symbol_t(SYM_TYPE symbol_type);
	virtual void update_name();
	const string& get_name() const;
	bool lower(sym_ptr s) const;
	bool equal(sym_ptr s) const;
	bool unequal(sym_ptr s) const;
	virtual bool is(SYM_TYPE sym_type) const;
	virtual bool is(sym_ptr sym_type) const;
	SYM_TYPE get_sym_type() const;
	virtual token_ptr get_token();
	virtual void set_token(token_ptr token);
	void short_print_l(ostream& os, int level) override;
	static SYM_TYPE token_to_sym_type(TOKEN);
	static TOKEN sym_type_to_token(SYM_TYPE);
};

template<> struct less<sym_ptr> {
	bool operator()(sym_ptr a, sym_ptr b) {
		return a->lower(b);
	}
};

class type_base_t : public symbol_t {
public:
	type_base_t(SYM_TYPE symbol_type);
	virtual bool completed();
	static type_base_ptr make_type(SYM_TYPE s);
	virtual int get_size();
};

class updatable_base_type_t : public type_base_t {
protected:
	type_ptr elem_type;
public:
	using type_base_t::type_base_t;
	virtual void set_element_type(type_ptr type);
	virtual type_ptr get_element_type();
	void update_name() override;
};

class type_t : public type_base_t {
protected:
	type_base_ptr type;
	virtual string _get_name() const override;
	bool _is_const = false;
public:
	type_t(type_base_ptr type);
	type_t(type_base_ptr type, bool is_const);
	void print_l(ostream& os, int level) override;
	type_base_ptr get_base_type();
	void update_name() override;
	void set_base_type(type_base_ptr type);
	bool is(SYM_TYPE sym_type) const override;
	bool is_const();
	void set_is_const(bool is_const);
	bool completed() override;
	token_ptr get_token() override;
	void set_token(token_ptr token) override;
	int get_size() override;
};

class sym_var_t : public symbol_t {
protected:
	token_ptr identifier;
	vector<expr_t*> init_list;
	type_ptr type;
	string _get_name() const override;
public:
	sym_var_t(token_ptr identifier, type_ptr type, vector<expr_t*> init_list);
	type_ptr get_type();
	void print_l(ostream& os, int level) override;
	void short_print_l(ostream& os, int level) override;
};

class sym_built_in_type : public virtual type_base_t {
	string _get_name() const;
public:
	sym_built_in_type();
	void print_l(ostream& os, int level) override;
};

class sym_type_str_literal_t : public sym_built_in_type {
protected:
	token_ptr str;
public:
	sym_type_str_literal_t(token_ptr str);
	int get_size() override;
	void print_l(ostream& os, int level) override;
};

class sym_type_void_t : public sym_built_in_type {
public:
	sym_type_void_t();
	bool completed() override;
};

class sym_type_int_t : public sym_built_in_type {
public:
	sym_type_int_t();
	int get_size() override;
};

class sym_type_char_t : public sym_built_in_type {
public:
	sym_type_char_t();
	int get_size() override;
};

class sym_type_double_t : public sym_built_in_type {
public:
	sym_type_double_t();
	int get_size() override;
};

class sym_type_ptr : public updatable_base_type_t {
protected:
	string _get_name() const override;
public:
	sym_type_ptr();
	void print_l(ostream& os, int level) override;
	type_ptr get_element_type();
	int get_size() override;
};

class sym_type_array_t : public updatable_base_type_t {
protected:
	expr_t* size;
	string _get_name() const override;
public:
	sym_type_array_t(expr_t* size = nullptr, bool is_const_ = false);
	void set_element_type(type_ptr type) override;
	void print_l(ostream& os, int level) override;
	bool completed() override;
	type_ptr get_element_type();
	int get_size() override;
};

class sym_type_struct_t : public type_base_t {
protected:
	sym_table_ptr  sym_table;
	token_ptr identifier;
	string _get_name() const override;
public:
	sym_type_struct_t(sym_table_ptr  sym_table, token_ptr identifier);
	sym_type_struct_t(token_ptr identifier);
	void set_sym_table(sym_table_ptr  s);
	sym_table_ptr  get_sym_table();
	bool completed() override;
	int get_size() override;
	void print_l(ostream& os, int level) override;
	void short_print_l(ostream& os, int level) override;
};

class sym_type_func_t : public updatable_base_type_t {
protected:
	vector<type_ptr> arg_types;
	string _get_name() const override;
public:
	sym_type_func_t(const vector<type_ptr> &at);
	virtual void update_name();
	void set_arg_types(const vector<type_ptr> &at);
	void set_element_type(type_ptr type) override;
	void print_l(ostream& os, int level) override;
};

class sym_func_t : public symbol_t {
protected:
	node_ptr block;
	token_ptr identifier;
	sym_table_ptr sym_table;
	shared_ptr<sym_type_func_t> func_type;
	string _get_name() const override;
public:
	sym_func_t(token_ptr ident, shared_ptr<sym_type_func_t> func_type, sym_table_ptr sym_table);
	shared_ptr<sym_type_func_t> get_func_type();
	void set_block(node_ptr b);
	void set_sym_table(sym_table_ptr sym_table);
	sym_table_ptr get_sym_table();
	void clear_sym_table();
	bool defined();
	void print_l(ostream& os, int level) override;
};

class sym_type_alias_t : public type_base_t {
protected:
	type_ptr type;
	token_ptr identifier;
	string _get_name() const override;
public:
	sym_type_alias_t::sym_type_alias_t(token_ptr identifier, type_ptr type);
	virtual void update_name();
	void print_l(ostream& os, int level) override;
	bool completed() override;
	type_ptr get_type();
};