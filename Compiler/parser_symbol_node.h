#pragma once
#include "tokens.h"
#include "parser_base_node.h"
#include "parser_expression_node.h"

class symbol_t : public node_t {
protected:
	bool printed = false;
	virtual string _get_name() const = 0;
	string name;
public:
	symbol_t();
	symbol_t(string name);
	virtual void print(ostream& os) = 0;
	virtual void update_name();
	const string& get_name() const;
	bool lower(symbol_t* s) const;
};

template<> struct less<symbol_t*> {
	bool operator()(symbol_t* a, symbol_t* b) {
		return a->lower(b);
	}
};

class type_t : public symbol_t {
protected:
	bool is_const_;
public:
	type_t(bool is_const = false);
	virtual void set_const(bool val);
	void print(ostream& os) override;
	virtual bool is_const();
	virtual bool completed();
};

class type_with_size_t : public virtual type_t {
public:
	type_with_size_t(bool is_const = false);
	virtual int get_size() = 0;
};

class updatable_sym_t : public virtual type_t {
public:
	updatable_sym_t(bool is_const_= false);
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
	string _get_name() const override;
public:
	sym_var_t(token_ptr_t identifier, type_t* type, vector<node_t*> init_list);
	void print(ostream& os) override;
};

template<TOKEN T>
class sym_built_in_type : public virtual type_t {
	string _get_name() const;
public:
	sym_built_in_type(bool is_const = false);
	void print(ostream& os) override;
};

template<TOKEN T>
inline string sym_built_in_type<T>::_get_name() const {
	char t[20];
	_ltoa_s(T, t, 10);
	return string(t);
}

template<TOKEN T>
inline sym_built_in_type<T>::sym_built_in_type(bool is_const) : type_t(is_const) {}

template<TOKEN T>
inline void sym_built_in_type<T>::print(ostream & os) {
	if (printed)
		return;
	else
		printed = true;
	type_t::print(os);
	os << token_t::get_name_by_id(T) << ' ';
}

class sym_type_void_t : public sym_built_in_type<T_KWRD_VOID> {
public:
	using sym_built_in_type<T_KWRD_VOID>::sym_built_in_type;
	bool completed() override;
};

class sym_type_int_t : public sym_built_in_type<T_KWRD_INT>, public type_with_size_t {
public:
	using sym_built_in_type<T_KWRD_INT>::sym_built_in_type;
	int get_size() override;
};

class sym_type_char_t : public sym_built_in_type<T_KWRD_CHAR>, public type_with_size_t {
public:
	using sym_built_in_type<T_KWRD_CHAR>::sym_built_in_type;
	int get_size() override;
};

class sym_type_double_t : public sym_built_in_type<T_KWRD_DOUBLE>, public type_with_size_t {
public:
	using sym_built_in_type<T_KWRD_DOUBLE>::sym_built_in_type;
	int get_size() override;
};

class sym_type_ptr_t : public updatable_sym_t, public type_with_size_t {
protected:
	type_t* type;
	string _get_name() const override;
public:
	//using type_with_size_t::type_with_size_t;
	sym_type_ptr_t(bool is_const_ = 0);
	void set_element_type(type_t* type, pos_t pos) override;
	void print(ostream& os) override;
	int get_size() override;
};

class sym_type_array_t : public updatable_sym_t, public type_with_size_t {
protected:
	expr_t* size;
	type_with_size_t* elem_type;
	string _get_name() const override;
public:
	sym_type_array_t(expr_t* size = nullptr, bool is_const_ = false);
	void set_element_type(type_t* type, pos_t pos) override;
	void print(ostream& os) override;
	bool completed() override;
	int get_size() override;
};

class sym_type_func_t : public updatable_sym_t {
protected:
	vector<type_t*> args;
	type_t* ret_type;
	//const string& _get_name() const override;
public:
	sym_type_func_t(vector<type_t*>& args, bool is_const_ = false);
	void set_element_type(type_t* type, pos_t pos) override;
	void print(ostream& os) override {};
};

class sym_type_alias_t : public type_t {
protected:
	type_t* type;
	token_ptr_t identifier;
	string _get_name() const override;
public:
	sym_type_alias_t::sym_type_alias_t(token_ptr_t identifier, type_t* type);
	bool is_const() override;
	void print(ostream& os) override;
	type_t* get_type();
};