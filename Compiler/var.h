#pragma once

#include <ostream>
#include <memory>
#include <assert.h>

using namespace std;

class var_base_t;

class var_ptr : public shared_ptr<var_base_t> {
public:
	using shared_ptr<var_base_t>::shared_ptr;
	var_ptr operator+(var_ptr);
	var_ptr operator-(var_ptr);
	var_ptr operator+();
	var_ptr operator-();
	var_ptr operator*(var_ptr);
	var_ptr operator/(var_ptr);
	var_ptr operator<<(var_ptr);
	var_ptr operator>>(var_ptr);
	friend ostream& operator<<(ostream& os, var_ptr e);
};

class var_base_t {
public:
	virtual var_ptr operator+(var_ptr) = 0;
	virtual var_ptr operator-(var_ptr) = 0;
	virtual var_ptr operator+() = 0;
	virtual var_ptr operator-() = 0;
	virtual var_ptr operator*(var_ptr) = 0;
	virtual var_ptr operator/(var_ptr) = 0;
	virtual var_ptr operator<<(var_ptr) = 0;
	virtual var_ptr operator>>(var_ptr) = 0;
	virtual void print(ostream& os) = 0;
};

template <typename T>
class var_t : public var_base_t {
	T val;
public:
	var_t(T val);
	var_ptr operator+(var_ptr) override;
	var_ptr operator-(var_ptr) override;
	var_ptr operator+() override;
	var_ptr operator-() override;
	var_ptr operator*(var_ptr) override;
	var_ptr operator/(var_ptr) override;
	var_ptr operator<<(var_ptr) override;
	var_ptr operator>>(var_ptr) override;
	T get_val();
	void print(ostream& os) override;
};

template<typename T>
inline T var_t<T>::get_val() {
	return val;
}

template<typename T>
inline void var_t<T>::print(ostream& os) {
	os << val;
}

template<>
inline void var_t<char>::print(ostream& os) {
	os << '\'' << val << '\'';
}

template<typename T>
inline var_t<T>::var_t(T val) : val(val) {}

#define define_operator(op) \
	template<typename T> \
	inline var_ptr var_t<T>::operator##op (var_ptr e) { \
		auto t = dynamic_pointer_cast<var_t<T>>(e); \
		assert(t); \
		return var_ptr(new var_t<T>(val op t->get_val())); \
	}

define_operator(+);
define_operator(-);
define_operator(*);
define_operator(/);
define_operator(<<);
define_operator(>>);

template<> 
inline var_ptr var_t<double>::operator<<(var_ptr e) {
	assert(false);
	return nullptr;
}

template<>
inline var_ptr var_t<double>::operator>>(var_ptr e) {
	assert(false);
	return nullptr;
}

#undef define_operator

template<typename T>
inline var_ptr var_t<T>::operator+() {
	return var_ptr(this); 
}

template<typename T>
inline var_ptr var_t<T>::operator-() {
	return var_ptr(new var_t<T>(-val));
}

template<typename T>
var_ptr var_cast(var_ptr e) {
	var_base_t* res = 
		typeid(*e.get()) == typeid(var_t<char>) ? new var_t<T>(T(static_pointer_cast<var_t<char>>(e)->get_val())) :
		typeid(*e.get()) == typeid(var_t<int>) ? new var_t<T>(T(static_pointer_cast<var_t<int>>(e)->get_val())) :
		typeid(*e.get()) == typeid(var_t<double>) ? new var_t<T>(T(static_pointer_cast<var_t<double>>(e)->get_val())) :
		(assert(false), nullptr);
	return var_ptr(res);
}
