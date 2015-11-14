#include "symbol_table.h"
#include "exceptions.h"
#include <assert.h>

sym_table_t::sym_table_t() : parent(nullptr) {}

sym_table_t::sym_table_t(sym_table_t* parent) : parent(parent) {}

bool sym_table_t::_insert(sym_ptr s) {
	if (!local_find(s)) {
		map_st[s->get_name()] = s;
		push_back(s);
		return true;
	}
	return false;
}

void sym_table_t::insert(sym_ptr s, pos_t pos) {
	if (!_insert(s))
		throw RedefenitionOfSymbol(s, pos);
}

void sym_table_t::insert_no_except(sym_ptr s) {
	_insert(s);
}

sym_ptr sym_table_t::global_get(const sym_ptr s) {
	sym_ptr res = global_find(s);
	if (!res)
		throw UndefinedSymbol(s, pos_t());
	return res;
}

sym_ptr sym_table_t::local_get(const sym_ptr s) {
	sym_ptr res = local_find(s);
	if (!res)
		throw UndefinedSymbol(s, pos_t());
	return res;
}

sym_ptr sym_table_t::global_get(const string& s) {
	sym_ptr res = global_find(s);
	if (!res)
		throw UndefinedSymbol(s, pos_t());
	return res;
}

sym_ptr sym_table_t::local_get(const string& s) {
	sym_ptr res = local_find(s);
	if (!res)
		throw UndefinedSymbol(s, pos_t());
	return res;
}

sym_ptr sym_table_t::global_get(const token_ptr& token) {
	sym_ptr res = global_find(token);
	if (!res)
		throw UndefinedSymbol(token);
	return res;
}

sym_ptr sym_table_t::local_get(const token_ptr& token) {
	sym_ptr res = local_find(token);
	if (!res)
		throw UndefinedSymbol(token);
	return res;
}

sym_table_t* sym_table_t::get_parent() {
	return parent;
}

sym_ptr sym_table_t::global_find(const string& s) {
	auto res = map_st.find(s);
	return res == map_st.end() ? (parent ? parent->global_find(s) : nullptr) : res->second;
}

sym_ptr sym_table_t::local_find(const string& s) {
	auto res = map_st.find(s);
	return res == map_st.end() ? nullptr : res->second;
}

sym_ptr sym_table_t::global_find(const sym_ptr s) {
	return global_find(s->get_name());
}

sym_ptr sym_table_t::local_find(const sym_ptr s) {
	return local_find(s->get_name());
}

sym_ptr sym_table_t::global_find(const token_ptr& token) {
	assert(token == T_IDENTIFIER);
	return global_find(static_pointer_cast<token_with_value_t<string>>(token)->get_value());
}

sym_ptr sym_table_t::local_find(const token_ptr& token) {
	assert(token == T_IDENTIFIER);
	return local_find(static_pointer_cast<token_with_value_t<string>>(token)->get_value());
}

bool sym_table_t::is_var(const token_ptr& token) {
	if (token != T_IDENTIFIER)
		return false;
	sym_ptr s = global_find(token);
	return s ? s == ST_VAR : false;
}

bool sym_table_t::is_alias(const token_ptr& token) {
	if (token != T_IDENTIFIER)
		return false;
	sym_ptr s = global_find(token);
	return s ? s == ST_ALIAS : false;
}

void sym_table_t::print(ostream& os) {
	print(os, 0);
}

void sym_table_t::print(ostream& os, int level) {
	for each (auto var in *this) {
		print_level(os, level);
		//os << var->get_name() << ": ";
		var->print(os);
		os << endl;
	}
}