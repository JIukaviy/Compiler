#pragma once
#include "parser_symbol_node.h"
#include "asm_generator.h"
#include <vector>
#include <map>

class sym_table_t : public vector<sym_ptr>, public node_t {
protected:
	map<string, sym_ptr> map_st;
	sym_table_ptr  parent;
	void _insert(sym_ptr symbol);
public:
	sym_table_t();
	sym_table_t(sym_table_ptr  parent);
	void insert(sym_ptr s);
	sym_ptr find_global_or_insert(sym_ptr s);
	sym_ptr find_local_or_insert(sym_ptr s);

	sym_ptr get_local(const string& s);
	sym_ptr get_local(const sym_ptr s);
	sym_ptr get_local(const token_ptr& token);

	sym_ptr get_global(const string& s);
	sym_ptr get_global(const sym_ptr s);
	sym_ptr get_global(const token_ptr& token);

	sym_ptr find_local(const string& s);			//Difference between "find" and "get", that "get" throws an exception when don't find a symbol
	sym_ptr find_local(const sym_ptr s);
	sym_ptr find_local(const token_ptr& token);

	sym_ptr find_global(const string& s);
	sym_ptr find_global(const sym_ptr s);
	sym_ptr find_global(const token_ptr& token);

	int asm_set_offset_for_local_vars(int offset, ASM_REGISTER offset_reg);
	void asm_init_local_vars(asm_cmd_list_ptr cmd_list);

	sym_table_ptr  get_parent();
	bool is_var(const token_ptr&);
	bool is_alias(const token_ptr&);
	void print_l(ostream& os, int level) override;
	void short_print_l(ostream& os, int level) override;
};
