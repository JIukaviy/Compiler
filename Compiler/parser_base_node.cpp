#include "parser_base_node.h"

void node_t::print(ostream& os) {
	print_l(os, 0);
}

void node_t::short_print(ostream& os) {
	short_print_l(os, 0);
}

void node_t::short_print_l(ostream& os, int level) {
	print_l(os, 0);
}

node_t::~node_t() {}

void print_level(ostream& os, int level) {
	for (int i = 0; i < level; i++)
		os << '\t';
}