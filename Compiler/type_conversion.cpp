#include "type_conversion.h"
#include "exceptions.h"
#include "parser.h"

#include <map>
#include <set>

bool eq_types(type_base_ptr a, type_base_ptr b) {
	return a == b || a == ST_PTR && b == ST_PTR && 
		static_pointer_cast<sym_type_ptr_t>(a)->get_element_type() ==
		static_pointer_cast<sym_type_ptr_t>(b)->get_element_type();
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

type_ptr get_max_type(type_ptr left, type_ptr right) {
	if (left->get_base_type() == right->get_base_type())
		return left;

	assert(left->is_ariphmetic());
	assert(right->is_ariphmetic());

	return  
		left == ST_DOUBLE ? left :
		right == ST_DOUBLE ? right :
		left == ST_INTEGER ? left : right;
}

void align_types(expr_t** left, expr_t** right) {
	type_ptr left_type = (*left)->get_type();
	type_ptr right_type = (*right)->get_type();

	if (eq_types(left_type->get_base_type(), right_type->get_base_type()))
		return;

	type_ptr max_type = get_max_type(left_type, right_type);
	*left = auto_convert(*left, max_type);
	*right = auto_convert(*right, max_type);
}