#pragma once

#include <string>
#include <ostream>
#include <memory>

using namespace std;

//#define register_keyword_token(name, incode_name) #ifdef LA_H

#define register_token(incode_name, printed_name, func_name) T_##incode_name,
#define TOKEN_DECLARATION
#define TOKEN_LIST
enum TOKEN {
	T_EMPTY,
#include "token_register.h"
};
#undef TOKEN_LIST
#undef TOKEN_DECLARATION
#undef register_token

#define is_kwrd(x) ((x) >= T_KWRD_CHAR && (x) <= T_KWRD_WHILE)
#define is_op(x) ((x) >= T_OP_BRACKET_OPEN && (x) <= T_OP_BIT_NOT_ASSIGN)

void tokens_init();

union value_t {
	char   ch;
	int    i;
	double d;
	char* str;
};

class token_t {
	TOKEN token;
	int line;
	int column;
	char* copy_str(char*);
public:
	token_t(int line_, int column_, TOKEN token_);
	token_t(token_t&);
	token_t();
	bool operator==(const TOKEN&) const;
	bool operator!=(const TOKEN&) const;
	virtual void print(ostream& os) const;
	string get_name() const;
	TOKEN get_token_id() const;
};

class token_container_t : public shared_ptr<token_t> {
public:
	using shared_ptr<token_t>::shared_ptr;
	friend ostream& operator<<(ostream& os, token_container_t& e);
	bool operator==(const TOKEN&) const;
	bool operator!=(const TOKEN&) const;
};

template <typename T>
class token_with_value_t : public token_t {
protected:
	T value;
public:
	token_with_value_t(int line_, int column_, TOKEN token_, T value_);
	void print(ostream& os) const override;
	T& get_value() const;
};

template<typename T>
void token_with_value_t<T>::print(ostream& os) const {
	token_t::print(os);
	os << " token value: " << value;
}

template<typename T>
token_with_value_t<T>::token_with_value_t(int line_, int column_, TOKEN token_, T value_) : token_t(line_, column_, token_) {
	value = value_;
}