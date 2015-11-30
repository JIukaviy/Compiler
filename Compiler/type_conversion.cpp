#include "type_conversion.h"
#include "exceptions.h"
#include "parser.h"

#include <map>
#include <set>

bool eq_types(type_base_ptr a, type_base_ptr b) {
	return 
		a == b ||
		a == ST_ARRAY && b == ST_ARRAY &&
		static_pointer_cast<sym_type_array_t>(a)->get_element_type()->get_base_type() ==
		static_pointer_cast<sym_type_array_t>(b)->get_element_type()->get_base_type() ||
		a == ST_PTR && b == ST_PTR && 
		static_pointer_cast<sym_type_ptr_t>(a)->get_element_type()->get_base_type() ==
		static_pointer_cast<sym_type_ptr_t>(b)->get_element_type()->get_base_type();
}

expr_t* add_cast(expr_t* src, type_ptr dst) {
	if (eq_types(src->get_type()->get_base_type(),dst->get_base_type()))
		return src;
	expr_cast_t* res = new expr_cast_t();
	res->set_operand(src, dst);
	return res;
}

expr_t* auto_convert(expr_t* src, type_ptr dst_type) {
	type_ptr src_type = src->get_type();
	if (dst_type == ST_PTR && src_type->is_integer() && (typeid(*src) != typeid(expr_const_t) || !static_cast<expr_const_t*>(src)->is_null()))
		throw IllegalConversion(src_type, dst_type, src->get_pos());
	return add_cast(src, dst_type);
}

expr_t* integer_increase(expr_t* src) {
	assert(src->get_type()->is_integer());
	return add_cast(src, parser_t::get_type(ST_INTEGER));
}

void arithmetic_conversion(expr_t** left, expr_t** right) {
	type_ptr left_type = (*left)->get_type();
	type_ptr right_type = (*right)->get_type();

	assert(left_type->is_arithmetic());
	assert(right_type->is_arithmetic());

	type_ptr max_type = 
		left_type == ST_DOUBLE ? left_type :
		right_type == ST_DOUBLE ? right_type :
		left_type == ST_INTEGER ? left_type :
		right_type == ST_INTEGER ? right_type :
		parser_t::get_type(ST_INTEGER);

	*left = auto_convert(*left, max_type);
	*right = auto_convert(*right, max_type);
}

expr_t* array_to_ptr(expr_t* arr) {
	assert(arr->get_type() == ST_ARRAY);
	return add_cast(arr, static_pointer_cast<sym_type_array_t>(arr->get_type()->get_base_type())->get_ptr_to_elem_type());
}

expr_t* func_to_ptr(expr_t* func) {
	assert(func->get_type() == ST_FUNC);
	return add_cast(func, sym_type_ptr_t::make_ptr(func->get_type()));
}