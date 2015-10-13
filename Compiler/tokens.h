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
	value_t value;
	TOKEN token;
	int line;
	int column;
	char* copy_str(char*);
public:
	token_t(int line_, int column_, TOKEN token_);
	token_t(int line_, int column_, TOKEN token_, value_t value_);
	token_t(token_t&);
	token_t();
	~token_t();
	bool operator==(const TOKEN&) const;
	bool operator!=(const TOKEN&) const;
	token_t& operator=(const token_t&);
	friend ostream& operator<<(ostream& os, const token_t& e);
	TOKEN get_token_code() const;
	string get_name() const;
};

class token_base_t {
protected:
	int line;
	int column;
	string name = "token";
	string str;
public:
	token_base_t(int line_, int column_, string str_);
	//int get_line();
	//int get_column();
};

class token_printed_t : public token_base_t {
public:
	using token_base_t::token_base_t;
	token_printed_t(int line_, int column_, string str_) : token_base_t(line_, column_, str_) {};
	virtual void print_out(ostream& os) const;
};

class token_container_t {
protected:
	shared_ptr<token_printed_t> token;
public:
	token_container_t();
	token_container_t(token_printed_t* token_);
	friend ostream& operator<<(ostream& os, token_container_t& e);
	bool is_empty();
};

template <typename T>
class token_with_value_t : public token_printed_t {
protected:
	T value;
	void print_out(ostream& os) const override;
public:
	token_with_value_t(int line_, int column_, string str, T value_);
	T& get_value() const;
};

template<typename T>
void token_with_value_t<T>::print_out(ostream& os) const {
	token_printed_t::print_out(os);
	os << '\t' << value;
}

template<typename T>
token_with_value_t<T>::token_with_value_t(int line_, int column_, string str_, T value_) : token_printed_t(line_, column_, str_) {
	value = value_;
}


//--------------------TOKENS---------------------------

class token_identifier_t : public token_printed_t {
public:
	token_identifier_t(int line_, int column_, string str_) : token_printed_t(line_, column_, str_) {
		name = "ident";
	};
};

class token_integer_t : public token_with_value_t<int> {
public:
	token_integer_t(int line_, int column_, string str_, int value) : token_with_value_t<int>(line_, column_, str_, value) {
		name = "integer";
	};
};

class token_float_t : public token_with_value_t<double> {
public:
	token_float_t(int line_, int column_, string str_, double value) : token_with_value_t<double>(line_, column_, str_, value) {
		name = "real";
	};
};

class token_char_t : public token_with_value_t<char> {
public:
	token_char_t(int line_, int column_, string str_, char value) : token_with_value_t<char>(line_, column_, str_, value) {
		name = "char";
	};
};

class token_string_t : public token_with_value_t<string> {
public:
	token_string_t(int line_, int column_, string str_, string value) : token_with_value_t<string>(line_, column_, str_, value) {
		name = "string";
	};
};

class token_keyword_t : public token_printed_t {
public:
	token_keyword_t(int line_, int column_, string str_) : token_printed_t(line_, column_, str_) {
		name = "keyword";
	};
};

class token_operator_t : public token_printed_t {
public:
	token_operator_t(int line_, int column_, string str_) : token_printed_t(line_, column_, str_) {
		name = "op";
	};
};

class token_separator_t : public token_printed_t {
public:
	token_separator_t(int line_, int column_, string str_) : token_printed_t(line_, column_, str_) {
		name = "sep";
	};
};