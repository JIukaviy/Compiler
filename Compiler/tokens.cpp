#include "tokens.h"
#include <sstream>
#include <map>

map<TOKEN, string> token_names;

token_base_t::token_base_t(int line_, int column_, string str_) {
	line = line_;
	column = column_;
	str = str_;
}

ostream& operator<<(ostream& os, token_printed_t& e) {
	e.print_out(os);
	return os;
}

void token_printed_t::print_out(ostream& os) const {
	os << line << '\t' << column << '\t' << name << '\t' << str;
}

token_container_t::token_container_t() : token(nullptr) {}

token_container_t::token_container_t(token_printed_t * token_) : token(token_) {}

bool token_container_t::is_empty() {
	return !token.get();
}

ostream& operator<<(ostream& os, token_container_t& e) {
	e.token->print_out(os);
	return os;
}

token_t::token_t(int line_, int column_, TOKEN token_) {
	line = line_;
	column = column_;
	token = token_;
}

char* token_t::copy_str(char* str) {
	int strl;
	strl = strlen(str);
	char* new_str = new char[strl + 1];
	memcpy(new_str, str, strl + 1);
	return new_str;
}

token_t& token_t::operator=(const token_t& e) {
	line = e.line;
	column = e.column;
	token = e.token;
	value = e.value;

	if (token == T_STRING || token == T_IDENTIFIER) {
		value.str = copy_str(e.value.str);
	}

	return *this;
}

token_t::token_t(int line_, int column_, TOKEN token_, value_t value_) {
	line = line_;
	column = column_;
	token = token_;
	value = value_;

	if (token == T_STRING || token == T_IDENTIFIER) {
		value.str = copy_str(value_.str);
	}
}

token_t::token_t(token_t& e) {
	*this = e;
}

token_t::token_t() {
	line = -1;
	column = -1;
	token = T_EMPTY;
}

token_t::~token_t() {
	if (token == T_STRING || token == T_IDENTIFIER) {
		delete value.str;
	}
}

bool token_t::operator==(const TOKEN& token_) const {
	return token == token_;
}

bool token_t::operator!=(const TOKEN& token_) const {
	return token != token_;
}

TOKEN token_t::get_token_code() const {
	return token;
}

string token_t::get_name() const {
	return token_names[token];
}

void tokens_init() {
#define register_token(incode_name, printed_name, func_name) token_names[T_##incode_name] = string(printed_name);
#define TOKEN_LIST
#define TOKEN_NAME_REGISTRATION
#include "token_register.h"
#undef TOKEN_NAME_REGISTRATION
#undef TOKEN_LIST
#undef register_token
}

ostream& operator<<(ostream& os, const token_t& e) {
	os << e.get_name() << ' ';
	switch (e.token) {
		case T_IDENTIFIER: os << e.value.str; break;
		case T_STRING: os << e.value.str; break;
		case T_INTEGER: os << e.value.i; break;
		case T_DOUBLE: os << e.value.d; break;
		case T_CHAR: os << e.value.ch; break;
	}
	return os;
}