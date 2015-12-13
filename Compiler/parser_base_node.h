#pragma once
#include <ostream>
#include <memory>

using namespace std;

class node_t {
public:
	void print(ostream& os);
	void short_print(ostream& os);
	virtual void print_l(ostream& os, int level) = 0;
	virtual void short_print_l(ostream& os, int level);
	virtual ~node_t();
};

typedef shared_ptr<node_t> node_ptr;
class statement_t;
typedef shared_ptr<statement_t> stmt_ptr;
class sym_table_t;
typedef shared_ptr<sym_table_t> sym_table_ptr;
class expr_t;

class asm_cmd_t;
class asm_cmd_list_t;
class asm_operand_t;
class asm_gen_t;
class asm_local_vars_t;

typedef shared_ptr<asm_cmd_t> asm_cmd_ptr;
typedef shared_ptr<asm_operand_t> asm_oprnd_ptr;
typedef shared_ptr<asm_cmd_list_t> asm_cmd_list_ptr;
typedef shared_ptr<asm_gen_t> asm_gen_ptr;
typedef shared_ptr<asm_local_vars_t> asm_local_vars_ptr;

void print_level(ostream& os, int level);