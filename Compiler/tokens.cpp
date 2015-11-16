#include "tokens.h"
#include <sstream>
#include <map>
#include <stdarg.h>

map<TOKEN, string> token_names;

ostream& operator<<(ostream& os, const token_ptr& e) {
	e.get()->print(os);
	return os;
}

pos_t::pos_t() : line(0), column(0) {}
pos_t::pos_t(int line, int column) : line(line), column(column) {}

pos_t::operator bool() {
	return line || column;
}

ostream& operator<<(ostream& os, const pos_t e) {
	os << e.line << ':' << e.column << ": ";
	return os;
}

token_t::token_t(int line, int column, TOKEN token) : pos(line, column), token(token) {}
token_t::token_t() : token(T_EMPTY) {}

bool token_t::operator==(const TOKEN& token_) const {
	return token == token_;
}

bool token_t::operator!=(const TOKEN& token_) const {
	return token != token_;
}

bool token_t::operator==(const token_t& token_) const {
	return token == token_.get_token_id();
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
	return pos.line;
}

int token_t::get_column() {
	return pos.column;
}

pos_t token_t::get_pos() {
	return pos;
}

void token_t::print_l(ostream& os, int level) {
	print_level(os, level);
	os << pos << "name: " << get_name();
}

void token_t::print_pos(ostream& os) {
	os << pos;
}

void token_t::short_print_l(ostream& os, int level) {
	print_level(os, level);
	os << get_name();
}

bool token_ptr::operator==(const TOKEN& token_id_) const {
	return *(get()) == token_id_;
}

bool token_ptr::operator!=(const TOKEN& token_id_) const {
	return *(get()) != token_id_;
}

bool token_ptr::operator==(const token_ptr& token_ptr_) const {
	return *(get()) == *token_ptr_;
}

token_ptr::operator TOKEN() {
	return TOKEN(*(get()));
}

void tokens_init() {
#define register_token(incode_name, printed_name, func_name, statement, ...) token_names[T_##incode_name] = string(printed_name);
#define TOKEN_LIST
#define TOKEN_NAME_REGISTRATION
#include "token_register.h"
#undef TOKEN_NAME_REGISTRATION
#undef TOKEN_LIST
#undef register_token
}
