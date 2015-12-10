#pragma once

#include <ostream>
#include <memory>
#include <assert.h>
#include <string>

using namespace std;

class var_base_t;

class var_ptr : public shared_ptr<var_base_t> {
public:
	using shared_ptr<var_base_t>::shared_ptr;
#define register_un_op(op) var_ptr operator##op();
#define register_bin_op(op) var_ptr operator##op(var_ptr);
#include "var_un_operators.h"
#include "var_bin_operators.h"
#undef register_un_op
#undef register_bin_op
	operator bool();
	friend ostream& operator<<(ostream& os, var_ptr e);
};

class var_base_t {
public:
#define register_un_op(op) virtual var_ptr operator##op() = 0;
#define register_bin_op(op) virtual var_ptr operator##op(var_ptr) = 0;
#include "var_un_operators.h"
#include "var_bin_operators.h"
#undef register_un_op
#undef register_bin_op
	virtual operator bool() = 0;
	virtual void print(ostream& os) = 0;
	virtual void asm_print(ostream& os) = 0;
};

template <typename T>
class var_t : public var_base_t {
	T val;
public:
	var_t(T val);
#define register_un_op(op) var_ptr operator##op() override;
#define register_bin_op(op) var_ptr operator##op(var_ptr) override;
#include "var_un_operators.h"
#include "var_bin_operators.h"
#undef register_un_op
#undef register_bin_op
	operator bool() override;
	T& get_val();
	void print(ostream& os) override;
	void asm_print(ostream& os) override;
};

template<typename T>
inline T& var_t<T>::get_val() {
	return val;
}

template<typename T>
inline void var_t<T>::print(ostream& os) {
	os << val;
}

template<typename T>
inline void var_t<T>::asm_print(ostream & os) {
	print(os);
}

template<>
inline void var_t<double>::asm_print(ostream& os) {
	os << "R8(" << scientific << val << ')';
}


template<>
inline void var_t<string>::asm_print(ostream& os) {
	os << "OFFSET STR_LITERAL(\"" << val << "\")";
}

template<>
inline void var_t<char>::print(ostream& os) {
	os << '\'' << val << '\'';
}

template<>
inline void var_t<string>::print(ostream& os) {
	os << '\"' << val << '\"';
}

template<typename T>
inline var_t<T>::var_t(T val) : val(val) {}

template<typename T>
inline var_t<T>::operator bool() {
	return (bool)val;
}

template<>
inline var_t<string>::operator bool() {
	return val.empty();
}

#define define_bin_operator(op) \
	template<typename T> \
	inline var_ptr var_t<T>::operator##op (var_ptr e) { \
		auto t = dynamic_pointer_cast<var_t<T>>(e); \
		assert(t); \
		return var_ptr(new var_t<T>(val op t->get_val())); \
	}

#define define_un_operator(op) \
	template<typename T> \
	inline var_ptr var_t<T>::operator##op () { \
		return var_ptr(new var_t<T>(op val)); \
	}

#define register_un_op(op) define_un_operator(op);
#define register_bin_op(op) define_bin_operator(op);
#include "var_un_operators.h"
#include "var_bin_operators.h"
#undef register_un_op
#undef register_bin_op

#undef define_un_operator
#undef define_bin_operator

#define forbid_bin_op(op, operand) \
template<> \
inline var_ptr var_t<operand>::operator##op(var_ptr e) {\
	assert(false);\
	return nullptr;\
}

#define forbid_un_op(op, operand) \
template<> \
inline var_ptr var_t<operand>::operator##op() {\
	assert(false);\
	return nullptr;\
}

forbid_bin_op(<<, double);
forbid_bin_op(%, double);
forbid_bin_op(>>, double)
forbid_bin_op(&&, double);
forbid_bin_op(||, double);
forbid_bin_op(&, double);
forbid_bin_op(|, double);
forbid_un_op(!, double);
forbid_un_op(~, double);

#define register_un_op(op) forbid_un_op(op, string);
#define register_bin_op(op) forbid_bin_op(op, string);
#include "var_un_operators.h"
#include "var_bin_operators.h"
#undef register_un_op
#undef register_bin_op

template<typename T>
var_ptr var_cast(var_ptr e) {
	var_base_t* res = 
		typeid(*e.get()) == typeid(var_t<char>) ? new var_t<T>(T(static_pointer_cast<var_t<char>>(e)->get_val())) :
		typeid(*e.get()) == typeid(var_t<int>) ? new var_t<T>(T(static_pointer_cast<var_t<int>>(e)->get_val())) :
		typeid(*e.get()) == typeid(var_t<double>) ? new var_t<T>(T(static_pointer_cast<var_t<double>>(e)->get_val())) :
		(assert(false), nullptr);
	return var_ptr(res);
}

template<typename T>
shared_ptr<var_t<T>> var_pointer_cast(var_ptr e) {
	return static_pointer_cast<var_t<T>>(var_cast<T>(e));
}

template<typename T>
shared_ptr<var_t<T>> new_var(T val) {
	return shared_ptr<var_t<T>>(new var_t<T>(val));
}