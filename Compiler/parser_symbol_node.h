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
	ST_VOID
};

void init_parser_symbol_node();

class symbol_t;
class type_t;
class updatable_sym_t;

class sym_ptr : public shared_ptr<symbol_t> {
public:
	using shared_ptr<symbol_t>::shared_ptr;
	bool operator==(SYM_TYPE sym_type) const;
};

class type_ptr : public shared_ptr<type_t> {
public:
	using shared_ptr<type_t>::shared_ptr;
	bool operator==(SYM_TYPE sym_type) const;
	bool operator!=(SYM_TYPE sym_type) const;
	bool operator==(type_ptr type) const;
	bool operator!=(type_ptr type) const;
};

class updt_sym_ptr : public shared_ptr<updatable_sym_t> {
public:
	using shared_ptr<updatable_sym_t>::shared_ptr;
	bool operator==(SYM_TYPE sym_type) const;
};

class symbol_t : public node_t {
protected:
	virtual string _get_name() const = 0;
	string name;
	SYM_TYPE symbol_type;
public:
	symbol_t();
	symbol_t(SYM_TYPE symbol_type);
	virtual void print(ostream& os) = 0;
	virtual void update_name();
	const string& get_name() const;
	bool lower(sym_ptr s) const;
	bool equal(sym_ptr s) const;
	bool unequal(sym_ptr s) const;
	bool is(SYM_TYPE sym_type) const;
	SYM_TYPE get_sym_type() const;
	void short_print(ostream& os) override;
	static SYM_TYPE token_to_sym_type(TOKEN);
	static TOKEN sym_type_to_token(SYM_TYPE);
};

template<> struct less<sym_ptr> {
	bool operator()(sym_ptr a, sym_ptr b) {
		return a->lower(b);
	}
};

class type_t : public symbol_t {
protected:
	bool is_const_ = false;
public:
	//type_t();
	type_t(SYM_TYPE symbol_type);
	virtual void set_const(bool val);
	void print(ostream& os) override;
	virtual bool is_const() const;
	virtual bool completed();
	static type_ptr make_type(SYM_TYPE s);
};

class type_with_size_t : public virtual type_t {
public:
	type_with_size_t();
	virtual int get_size() = 0;
};

class updatable_sym_t : public virtual type_t {
public:
	updatable_sym_t();
	virtual void set_element_type(type_ptr type, pos_t pos) = 0;		// текущая позиция необходима в случае вывода ошибок
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
	void print(ostream& os) override;
	void short_print(ostream& os) override;
};

class sym_built_in_type : public virtual type_t {
	string _get_name() const;
public:
	sym_built_in_type();
	void print(ostream& os) override;
};

class sym_type_str_literal_t : public sym_built_in_type, public type_with_size_t {
protected:
	token_ptr str;
public:
	sym_type_str_literal_t(token_ptr str);
	int get_size();
	void print(ostream&) override;
};

class sym_type_void_t : public sym_built_in_type {
public:
	sym_type_void_t();
	bool completed() override;
};

class sym_type_int_t : public sym_built_in_type, public type_with_size_t {
public:
	sym_type_int_t();
	int get_size() override;
};

class sym_type_char_t : public sym_built_in_type, public type_with_size_t {
public:
	sym_type_char_t();
	int get_size() override;
};

class sym_type_double_t : public sym_built_in_type, public type_with_size_t {
public:
	sym_type_double_t();
	int get_size() override;
};

class sym_type_ptr : public updatable_sym_t, public type_with_size_t {
protected:
	type_ptr type;
	string _get_name() const override;
public:
	sym_type_ptr();
	virtual void update_name();
	void set_element_type(type_ptr type, pos_t pos) override;
	void print(ostream& os) override;
	type_ptr get_element_type();
	int get_size() override;
};

class sym_type_array_t : public updatable_sym_t, public type_with_size_t {
protected:
	expr_t* size;
	shared_ptr<type_with_size_t> elem_type;
	string _get_name() const override;
public:
	sym_type_array_t(expr_t* size = nullptr, bool is_const_ = false);
	virtual void update_name();
	void set_element_type(type_ptr type, pos_t pos) override;
	void print(ostream& os) override;
	bool completed() override;
	type_ptr get_element_type();
	int get_size() override;
};

class sym_type_struct_t : public type_with_size_t {
protected:
	node_t* sym_table;
	token_ptr identifier;
	string _get_name() const override;
public:
	sym_type_struct_t(node_t* sym_table, token_ptr identifier);
	sym_type_struct_t(token_ptr identifier);
	void set_sym_table(node_t* s);
	node_t* get_sym_table();
	bool completed() override;
	int get_size() override;
	void print(ostream& os) override;
};

class sym_type_func_t : public updatable_sym_t {
protected:
	type_ptr ret_type;
	vector<type_ptr> arg_types;
	sym_table_t* sym_table;
	string _get_name() const override;
public:
	sym_type_func_t(const vector<type_ptr> &at);
	sym_type_func_t(const vector<type_ptr> &at, sym_table_t* sym_table);
	virtual void update_name();
	void set_arg_types(const vector<type_ptr> &at);
	void set_element_type(type_ptr type, pos_t pos) override;
	void set_sym_table(sym_table_t* s);
	sym_table_t* get_sym_table();
	void print(ostream& os) override;
};

class sym_func_t : public symbol_t {
protected:
	node_ptr block;
	token_ptr identifier;
	shared_ptr<sym_type_func_t> func_type;
	string _get_name() const override;
public:
	sym_func_t(token_ptr ident, shared_ptr<sym_type_func_t> func_type);
	type_ptr get_type();
	void set_block(node_ptr b);
	sym_table_t* get_sym_table();
	bool defined();
	void print(ostream& os) override;
};

class sym_type_alias_t : public type_t {
protected:
	type_ptr type;
	token_ptr identifier;
	string _get_name() const override;
public:
	sym_type_alias_t::sym_type_alias_t(token_ptr identifier, type_ptr type);
	virtual void update_name();
	bool is_const() const override;
	void print(ostream& os) override;
	type_ptr get_type();
};