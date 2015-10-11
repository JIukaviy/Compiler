#pragma once

#include <string>
#include <ostream>
#include <memory>
using namespace std;

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