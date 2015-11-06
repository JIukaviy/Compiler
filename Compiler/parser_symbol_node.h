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

class sym_ptr_t : public shared_ptr<symbol_t> {
public:
	using shared_ptr<symbol_t>::shared_ptr;
	bool operator==(SYM_TYPE sym_type) const;
};

class type_ptr_t : public shared_ptr<type_t> {
public:
	using shared_ptr<type_t>::shared_ptr;
	bool operator==(SYM_TYPE sym_type) const;
	bool operator!=(SYM_TYPE sym_type) const;
	bool operator==(type_ptr_t type) const;
	bool operator!=(type_ptr_t type) const;
};

class updt_sym_ptr_t : public shared_ptr<updatable_sym_t> {
public:
	using shared_ptr<updatable_sym_t>::shared_ptr;
	bool operator==(SYM_TYPE sym_type) const;
};

class symbol_t : public node_t {
protected:
	bool printed = false;
	virtual string _get_name() const = 0;
	string name;
	SYM_TYPE symbol_type;
public:
	symbol_t();
	symbol_t(SYM_TYPE symbol_type);
	virtual void print(ostream& os) = 0;
	virtual void update_name();
	const string& get_name() const;
	bool lower(sym_ptr_t s) const;
	bool equal(sym_ptr_t s) const;
	bool unequal(sym_ptr_t s) const;
	bool is(SYM_TYPE sym_type) const;
	SYM_TYPE get_sym_type() const;
	static SYM_TYPE token_to_sym_type(TOKEN);
	static TOKEN sym_type_to_token(SYM_TYPE);
};

template<> struct less<sym_ptr_t> {
	bool operator()(sym_ptr_t a, sym_ptr_t b) {
		return a->lower(b);
	}
};

class type_t : public symbol_t {
protected:
	bool is_const_;
public:
	type_t();
	type_t(SYM_TYPE symbol_type);
	virtual void set_const(bool val);
	void print(ostream& os) override;
	virtual bool is_const() const;
	virtual bool completed();
	static type_ptr_t make_type(SYM_TYPE s);
};

class type_with_size_t : public virtual type_t {
public:
	virtual int get_size() = 0;
};

class updatable_sym_t : public virtual type_t {
public:
	virtual void set_element_type(type_ptr_t type, pos_t pos) = 0;		// текущая позиция необходима в случае вывода ошибок
};

class sym_var_t : public symbol_t {
protected:
	token_ptr_t identifier;
	vector<expr_t*> init_list;
	type_ptr_t type;
	string _get_name() const override;
public:
	sym_var_t(token_ptr_t identifier, type_ptr_t type, vector<expr_t*> init_list);
	type_ptr_t get_type();
	void print(ostream& os) override;
};

class sym_built_in_type : public virtual type_t {
	string _get_name() const;
public:
	void print(ostream& os) override;
};

class sym_type_str_literal_t : public sym_built_in_type, public type_with_size_t {
protected:
	token_ptr_t str;
public:
	sym_type_str_literal_t(token_ptr_t str);
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

class sym_type_ptr_t : public updatable_sym_t, public type_with_size_t {
protected:
	type_ptr_t type;
	string _get_name() const override;
public:
	sym_type_ptr_t();
	void set_element_type(type_ptr_t type, pos_t pos) override;
	void print(ostream& os) override;
	type_ptr_t get_element_type();
	int get_size() override;
};

class sym_type_array_t : public updatable_sym_t, public type_with_size_t {
protected:
	expr_t* size;
	shared_ptr<type_with_size_t> elem_type;
	string _get_name() const override;
public:
	sym_type_array_t(expr_t* size = nullptr, bool is_const_ = false);
	void set_element_type(type_ptr_t type, pos_t pos) override;
	void print(ostream& os) override;
	bool completed() override;
	type_ptr_t get_element_type();
	int get_size() override;
};

class sym_type_struct : public type_with_size_t {
protected:
	sym_table_t* sym_table;
public:
	sym_type_struct();

};

class sym_type_func_t : public updatable_sym_t {
protected:
	vector<expr_t*> args;
	type_ptr_t ret_type;
public:
	sym_type_func_t(vector<expr_t*>& args, bool is_const_ = false);
	void set_element_type(type_ptr_t type, pos_t pos) override;
	void print(ostream& os) override {};
};

class sym_type_alias_t : public type_t {
protected:
	type_ptr_t type;
	token_ptr_t identifier;
	string _get_name() const override;
public:
	sym_type_alias_t::sym_type_alias_t(token_ptr_t identifier, type_ptr_t type);
	bool is_const() const override;
	void print(ostream& os) override;
	type_ptr_t get_type();
};