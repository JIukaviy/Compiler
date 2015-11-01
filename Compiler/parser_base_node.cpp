#include "parser_base_node.h"

void node_t::flat_print(ostream& os) {
	print(os);
}

void print_level(ostream& os, int level) {
	for (int i = 0; i < level; i++)
		os << '\t';
}