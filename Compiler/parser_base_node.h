#pragma once
#include <ostream>

using namespace std;

class node_t {
public:
	virtual void print(ostream&) = 0;
	virtual void short_print(ostream&);
};

void print_level(ostream& os, int level);