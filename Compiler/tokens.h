#pragma once

#include <string>
#include <ostream>
#include <memory>

using namespace std;

//#define register_keyword_token(name, incode_name) #ifdef LA_H

enum TOKEN {
	T_EMPTY,
	T_INTEGER,
	T_DOUBLE,
	T_CHAR,
	T_STRING,
	T_IDENTIFIER,

	T_SEMICOLON,
	T_BRACE_OPEN,
	T_BRACE_CLOSE,
	T_COLON,
	T_COMMA,
	T_QUESTION_MARK,

	T_KWRD_CHAR,
	T_KWRD_CONST,
	T_KWRD_CONTINUE,
	T_KWRD_DOUBLE,
	T_KWRD_ELSE,
	T_KWRD_FLOAT,
	T_KWRD_FOR,
	T_KWRD_IF,
	T_KWRD_INT,
	T_KWRD_LONG,
	T_KWRD_RETURN,
	T_KWRD_SHORT,
	T_KWRD_SIGNED,
	T_KWRD_SIZEOF,
	T_KWRD_UNSIGNED,
	T_KWRD_VOID,
	T_KWRD_WHILE,

	T_OP_BRACKET_OPEN,
	T_OP_BRACKET_CLOSE,
	T_OP_SQR_BRACKET_OPEN,
	T_OP_SQR_BRACKET_CLOSE,
	T_OP_ASSIGN,
	T_OP_DOT,
	T_OP_PTR,
	T_OP_NOT, 
	T_OP_INC,
	T_OP_DEC,
	T_OP_LEFT,
	T_OP_LEFT_ASSIGN,
	T_OP_RIGHT,
	T_OP_RIGHT_ASSIGN,
	T_OP_LE,
	T_OP_GE,
	T_OP_EQ,
	T_OP_NE,
	T_OP_AND,
	T_OP_OR,
	T_OP_MUL,
	T_OP_MUL_ASSIGN,
	T_OP_DIV,
	T_OP_DIV_ASSIGN,
	T_OP_ADD,
	T_OP_ADD_ASSIGN,
	T_OP_SUB,
	T_OP_SUB_ASSIGN,
	T_OP_MOD,
	T_OP_MOD_ASSIGN,
	T_OP_XOR,
	T_OP_XOR_ASSIGN,
	T_OP_BIT_AND,
	T_OP_BIT_AND_ASSIGN,
	T_OP_BIT_OR,
	T_OP_BIT_OR_ASSIGN,
	T_OP_BIT_NOT,
	T_OP_BIT_NOT_ASSIGN,
};

#define is_kwrd(x) ((x) >= T_KWRD_CHAR && (x) <= T_KWRD_WHILE)
#define is_op(x) ((x) >= T_OP_BRACKET_OPEN && (x) <= T_OP_BIT_NOT_ASSIGN)

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
	TOKEN get_token_code();
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