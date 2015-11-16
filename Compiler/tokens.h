#pragma once

#include <string>
#include <ostream>
#include <memory>
#include <set>
#include "parser_base_node.h"

using namespace std;

//#define register_keyword_token(name, incode_name) #ifdef LA_H

#define register_token(incode_name, printed_name, func_name, statement, ...) T_##incode_name,
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

struct pos_t {
	int line;
	int column;
	pos_t();
	pos_t(int line, int column);
	operator bool();
	friend ostream& operator<<(ostream& os, const pos_t e);
};

class token_t : public node_t {
protected:
	TOKEN token;
	pos_t pos;
public:
	token_t(int line_, int column_, TOKEN token_);
	token_t();
	bool operator==(const TOKEN&) const;
	virtual bool operator==(const token_t&) const;
	bool operator!=(const TOKEN&) const;
	operator TOKEN(); 
	void print_l(ostream& os, int level) override;
	void short_print_l(ostream& os, int level) override;
	virtual void print_pos(ostream& os);
	string get_name() const;
	bool is(TOKEN *first);
	bool is(TOKEN first, ...);
	bool is(set<TOKEN>&);
	static bool is(TOKEN token, set<TOKEN>&);
	//static bool is(TOKEN token, TOKEN first, ...);
	static bool is(TOKEN token, TOKEN *first);
	static string get_name_by_id(TOKEN token_id);
	int get_line();
	int get_column();
	pos_t get_pos();
	TOKEN get_token_id() const;
};

class token_ptr : public shared_ptr<token_t> {
public:
	using shared_ptr<token_t>::shared_ptr;
	friend ostream& operator<<(ostream& os, const token_ptr& e);
	operator TOKEN();
	bool operator==(const TOKEN&) const;
	bool operator==(const token_ptr&) const;
	bool operator!=(const TOKEN&) const;
};

template <typename T>
class token_with_value_t : public token_t {
protected:
	T value;
public:
	token_with_value_t(int line_, int column_, TOKEN token_, T value_);
	bool operator==(const token_t&) const override;
	void print_l(ostream& os, int level) override;
	void short_print_l(ostream& os, int level) override;
	const T& get_value() const;
};

template<typename T>
bool token_with_value_t<T>::operator==(const token_t& token_) const {
	return token_ == token && static_cast<const token_with_value_t<T>&>(token_).get_value() == value;
}

template<typename T>
void token_with_value_t<T>::print_l(ostream& os, int level) {
	token_t::print_l(os, level);
	os << ", value: " << value;
}

template<typename T>
void token_with_value_t<T>::short_print_l(ostream& os, int level) {
	print_level(os, level);
	os << value;
}

template<typename T>
inline const T& token_with_value_t<T>::get_value() const {
	return value;
}

template<typename T>
token_with_value_t<T>::token_with_value_t(int line_, int column_, TOKEN token_, T value_) : token_t(line_, column_, token_) {
	value = value_;
}