#pragma once
#include "parser_symbol_node.h"
#include <map>

class sym_table_t : public vector<sym_ptr>, public node_t {
protected:
	map<string, sym_ptr> map_st;
	sym_table_t* parent;
	bool _insert(sym_ptr symbol);
public:
	sym_table_t();
	sym_table_t(sym_table_t* parent);
	void insert(sym_ptr s, pos_t pos);
	void insert_no_except(sym_ptr s);

	sym_ptr local_get(const string& s);
	sym_ptr local_get(const sym_ptr s);
	sym_ptr local_get(const token_ptr& token);

	sym_ptr global_get(const string& s);
	sym_ptr global_get(const sym_ptr s);
	sym_ptr global_get(const token_ptr& token);

	sym_ptr local_find(const string& s);			//Difference between "find" and "get", that "get" throws an exception when don't find a symbol
	sym_ptr local_find(const sym_ptr s);
	sym_ptr local_find(const token_ptr& token);

	sym_ptr global_find(const string& s);
	sym_ptr global_find(const sym_ptr s);
	sym_ptr global_find(const token_ptr& token);

	sym_table_t* get_parent();
	bool is_var(const token_ptr&);
	bool is_alias(const token_ptr&);
	void print(ostream& os) override;
	void print(ostream& os, int level);
};
