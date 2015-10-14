#include "tokens.h"
#include <sstream>
#include <map>

map<TOKEN, string> token_names;

ostream& operator<<(ostream& os, token_container_t& e) {
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

TOKEN token_t::get_token_id() const {
	return token;
}

string token_t::get_name() const {
	return token_names[token];
}

void token_t::print(ostream& os) const {
	os << "line: " << line << " column: " << column << " token name: " << get_name();
}

bool token_container_t::operator==(const TOKEN& token_id_) const {
	return *(get()) == token_id_;
}

bool token_container_t::operator!=(const TOKEN& token_id_) const {
	return *(get()) != token_id_;
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