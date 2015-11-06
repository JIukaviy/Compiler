#pragma once
#include "parser_symbol_node.h"
#include <map>

class sym_table_t {
protected:
	map<string, sym_ptr_t> map_st;
	vector<sym_ptr_t> vec_st;
	sym_table_t* parent;
public:
	sym_table_t();
	sym_table_t(sym_table_t* parent);
	void insert(sym_ptr_t s);
	sym_ptr_t get(const string& s);
	sym_ptr_t get(const sym_ptr_t s);
	sym_ptr_t get(const token_ptr_t& token);
	sym_ptr_t find(const string& s);			//Difference between "find" and "get", that "get" throws an exception when don't find a symbol
	sym_ptr_t find(const sym_ptr_t s);
	sym_ptr_t find(const token_ptr_t& token);
	bool is_var(const token_ptr_t&);
	bool is_alias(const token_ptr_t&);
	void print(ostream& os);
};
