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

/*class expr_t;
class symbol_t;
class type_t;
class statement_t;*/

void print_level(ostream& os, int level);