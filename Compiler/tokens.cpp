#include "tokens.h"
#include <sstream>

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