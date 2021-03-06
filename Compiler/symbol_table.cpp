#include "symbol_table.h"
#include "exceptions.h"
#include <assert.h>

sym_table_t::sym_table_t() : parent(nullptr) {}

sym_table_t::sym_table_t(sym_table_ptr  parent) : parent(parent) {}

void sym_table_t::_insert(sym_ptr s) {
	map_st[s->get_name()] = s;
	push_back(s);
}

void sym_table_t::insert(sym_ptr s) {
	sym_ptr a = find_local(s);
	if (a)
		throw RedefinitionOfSymbol(a, s);
	_insert(s);
}

sym_ptr sym_table_t::find_global_or_insert(sym_ptr s) {
	sym_ptr finded = find_global(s);
	return finded ? finded : (_insert(s), s);
}

sym_ptr sym_table_t::find_local_or_insert(sym_ptr s) {
	sym_ptr finded = find_local(s);
	return finded ? finded : (_insert(s), s);
}

sym_ptr sym_table_t::get_global(const sym_ptr s) {
	sym_ptr res = find_global(s);
	if (!res)
		throw UndefinedSymbol(s, pos_t());
	return res;
}

sym_ptr sym_table_t::get_local(const sym_ptr s) {
	sym_ptr res = find_local(s);
	if (!res)
		throw UndefinedSymbol(s, pos_t());
	return res;
}

sym_ptr sym_table_t::get_global(const string& s) {
	sym_ptr res = find_global(s);
	if (!res)
		throw UndefinedSymbol(s, pos_t());
	return res;
}

sym_ptr sym_table_t::get_local(const string& s) {
	sym_ptr res = find_local(s);
	if (!res)
		throw UndefinedSymbol(s, pos_t());
	return res;
}

sym_ptr sym_table_t::get_global(const token_ptr& token) {
	sym_ptr res = find_global(token);
	if (!res)
		throw UndefinedSymbol(token);
	return res;
}

sym_ptr sym_table_t::get_local(const token_ptr& token) {
	sym_ptr res = find_local(token);
	if (!res)
		throw UndefinedSymbol(token);
	return res;
}

sym_table_ptr  sym_table_t::get_parent() {
	return parent;
}

sym_ptr sym_table_t::find_global(const string& s) {
	auto res = map_st.find(s);
	return res == map_st.end() ? (parent ? parent->find_global(s) : nullptr) : res->second;
}

sym_ptr sym_table_t::find_local(const string& s) {
	auto res = map_st.find(s);
	return res == map_st.end() ? nullptr : res->second;
}

sym_ptr sym_table_t::find_global(const sym_ptr s) {
	return find_global(s->get_name());
}

sym_ptr sym_table_t::find_local(const sym_ptr s) {
	return find_local(s->get_name());
}

sym_ptr sym_table_t::find_global(const token_ptr& token) {
	assert(token == T_IDENTIFIER);
	return find_global(static_pointer_cast<token_with_value_t<string>>(token)->get_value());
}

sym_ptr sym_table_t::find_local(const token_ptr& token) {
	assert(token == T_IDENTIFIER);
	return find_local(static_pointer_cast<token_with_value_t<string>>(token)->get_value());
}

void sym_table_t::asm_set_offset_for_local_vars(int offset, ASM_REGISTER offset_reg) {
	for each (auto var in *this) {
		if (var == ST_VAR) {
			auto local_var = dynamic_pointer_cast<sym_local_var_t>(var);
			if (!local_var)
				continue;
			if (local_var->get_type_size() >= asm_gen_t::size_of(AMT_DWORD))
				offset = asm_gen_t::alignment(offset);
			local_var->asm_set_offset(offset, offset_reg);
			offset += local_var->get_type_size();
		} else if (var == ST_STRUCT) {
			auto str = dynamic_pointer_cast<sym_type_struct_t>(var);
			str->get_sym_table()->asm_set_offset_for_local_vars(0, AR_EAX);
		}
	}
}

void sym_table_t::asm_init_local_vars(asm_cmd_list_ptr cmd_list) {
	for each (auto sym in *this)
		if (sym == ST_VAR) {
			auto sym_var = dynamic_pointer_cast<sym_local_var_t>(sym);
			sym_var->asm_init(cmd_list);
		}
}

int sym_table_t::get_local_vars_size() {
	int res = 0;
	for each (auto var in *this) {
		if (var == ST_VAR) {
			auto local_var = dynamic_pointer_cast<sym_local_var_t>(var);
			if (!local_var)
				continue;
			if (local_var->get_type_size() >= asm_gen_t::size_of(AMT_DWORD))
				res = asm_gen_t::alignment(res);
			res += local_var->get_type_size();
		}
	}
	return res;
}

bool sym_table_t::is_var(const token_ptr& token) {
	if (token != T_IDENTIFIER)
		return false;
	sym_ptr s = find_global(token);
	return s ? s == ST_VAR : false;
}

bool sym_table_t::is_alias(const token_ptr& token) {
	if (token != T_IDENTIFIER)
		return false;
	sym_ptr s = find_global(token);
	return s ? s == ST_ALIAS : false;
}

void sym_table_t::print_l(ostream& os, int level) {
	for each (auto var in *this) {
		print_level(os, level);
		var->print_l(os, level);
		os << endl;
	}
}

void sym_table_t::short_print_l(ostream& os, int level) {
	for each (auto var in *this) {
		print_level(os, level);
		var->short_print_l(os, level);
		os << endl;
	}
}