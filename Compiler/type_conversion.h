#pragma once

#include "parser_expression_node.h"

expr_t* auto_convert(expr_t* src, type_ptr dst_type);
expr_t* integer_increase(expr_t* src);
void align_types(expr_t** left, expr_t** right);