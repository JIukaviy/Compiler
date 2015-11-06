#pragma once
#include <ostream>
#include <memory>

using namespace std;

class node_t {
public:
	virtual void print(ostream&) = 0;
	virtual void short_print(ostream&);
};

typedef shared_ptr<node_t> node_ptr_t;

class statement_t;

typedef shared_ptr<statement_t> stmt_ptr_t;

/*class expr_t;
class symbol_t;
class type_t;
class statement_t;*/

void print_level(ostream& os, int level);