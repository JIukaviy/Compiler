#include "var.h"

#define register_bin_op(op)\
var_ptr var_ptr::operator##op(var_ptr e) {\
	return get()->operator##op(e);\
}

#define register_un_op(op)\
var_ptr var_ptr::operator##op() {\
	return get()->operator##op();\
}

#include "var_un_operators.h"
#include "var_bin_operators.h"

#undef register_bin_operator
#undef register_un_operator

var_ptr::operator bool() {
	return get()->operator bool();
}

ostream& operator<<(ostream& os, var_ptr e) {
	e->print(os);
	return os;
}