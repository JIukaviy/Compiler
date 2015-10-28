#include "tokens.h"
#include <sstream>
#include <map>
#include <stdarg.h>

map<TOKEN, string> token_names;

ostream& operator<<(ostream& os, const token_ptr_t& e) {
	e.get()->print(os);
	return os;
}

token_t::token_t(int line_, int column_, TOKEN token_) {
	line = line_;
	column = column_;
	token = token_;
}

token_t::token_t() {
	line = -1;
	column = -1;
	token = T_EMPTY;
}

bool token_t::operator==(const TOKEN& token_) const {
	return token == token_;
}

bool token_t::operator!=(const TOKEN& token_) const {
	return token != token_;
}

token_t::operator TOKEN() {
	return token;
}

TOKEN token_t::get_token_id() const {
	return token;
}

string token_t::get_name() const {
	return get_name_by_id(token);
}

bool token_t::is(TOKEN first, ...) {
	va_list list;
	va_start(list, first);
	while (first != T_EMPTY) {
		if (token == first) {
			va_end(list);
			return true;
		}
		first = va_arg(list, TOKEN);
	}
	va_end(list);
	return false;
}

bool token_t::is(set<TOKEN>& tokens) {
	return is(token, tokens);
}

bool token_t::is(TOKEN token, set<TOKEN>& tokens) {
	return tokens.find(token) != tokens.end();
}

bool token_t::is(TOKEN* first) {
	return is(token, first);
}

bool token_t::is(TOKEN token, TOKEN* first) {
	while (*first != T_EMPTY) {
		if (token == *first)
			return true;
		first++;
	}
	return false;
}

/*bool token_t::is(TOKEN token, TOKEN first, ...) {
	va_list list;
	va_start(list, first);
	TOKEN i = first;
	while ((int)i) {
		if (token == i) {
			va_end(list);
			return true;
		}
		i = va_arg(list, TOKEN);
	}
	va_end(list);
	return false;
}*/

string token_t::get_name_by_id(TOKEN token_id) {
	return token_names[token_id];
}

int token_t::get_line() {
	return line;
}

int token_t::get_column() {
	return column;
}

void token_t::print(ostream& os) const {
	os << "line: " << line << ", column: " << column << ", name: " << get_name();
}

void token_t::print_pos(ostream & os) const {
	os << line << '\t' << column << '\t';
}

void token_t::short_print(ostream & os) const {
	os << get_name();
}

bool token_ptr_t::operator==(const TOKEN& token_id_) const {
	return *(get()) == token_id_;
}

bool token_ptr_t::operator!=(const TOKEN& token_id_) const {
	return *(get()) != token_id_;
}

token_ptr_t::operator TOKEN() {
	return TOKEN(*(get()));
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