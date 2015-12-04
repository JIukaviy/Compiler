#include "var.h"

#define define_operator(op)\
var_ptr var_ptr::operator##op(var_ptr e) {\
	return get()->operator##op(e);\
}

define_operator(+);
define_operator(-);
define_operator(*);
define_operator(/);
define_operator(<<);
define_operator(>>);

ostream& operator<<(ostream& os, var_ptr e) {
	e->print(os);
	return os;
}
