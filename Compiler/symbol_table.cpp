#include "symbol_table.h"
#include "exceptions.h"
#include <assert.h>

sym_table_t::sym_table_t() : parent(nullptr) {}

sym_table_t::sym_table_t(sym_table_t* parent) : parent(parent) {}

void sym_table_t::insert(sym_ptr_t s) {
	if (!find(s)) {
		map_st[s->get_name()] = s;
		vec_st.push_back(s);
	}
}

sym_ptr_t sym_table_t::get(const sym_ptr_t s) {
	sym_ptr_t res = find(s);
	if (!res)
		throw UndefinedSymbol(s, pos_t());
	return res;
}

sym_ptr_t sym_table_t::get(const string& s) {
	sym_ptr_t res = find(s);
	if (!res)
		throw UndefinedSymbol(s, pos_t());
	return res;
}

sym_ptr_t sym_table_t::get(const token_ptr_t& token) {
	sym_ptr_t res = find(token);
	if (!res)
		throw UndefinedSymbol(token);
	return res;
}

sym_ptr_t sym_table_t::find(const string& s) {
	auto res = map_st.find(s);
	return res == map_st.end() ? (parent ? parent->find(s) : nullptr) : res->second;
}

sym_ptr_t sym_table_t::find(const sym_ptr_t s) {
	return find(s->get_name());
}

sym_ptr_t sym_table_t::find(const token_ptr_t& token) {
	assert(token == T_IDENTIFIER);
	return find(static_cast<token_with_value_t<string>*>(token.get())->get_value());
}

bool sym_table_t::is_var(const token_ptr_t& token) {
	if (token != T_IDENTIFIER)
		return false;
	sym_ptr_t s = find(token);
	return s ? s == ST_VAR : false;
}

bool sym_table_t::is_alias(const token_ptr_t& token) {
	if (token != T_IDENTIFIER)
		return false;
	sym_ptr_t s = find(token);
	return s ? s == ST_ALIAS : false;
}

void sym_table_t::print(ostream & os) {
	for each (auto var in vec_st) {
		os << var->get_name() << ": ";
		var->print(os);
		os << endl;
	}
}