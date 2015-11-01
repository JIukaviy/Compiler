#pragma once

#include "tokens.h"
#include "parser_base_node.h"
#include "parser_expression_node.h"

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