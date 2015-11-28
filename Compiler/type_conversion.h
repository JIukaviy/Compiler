#pragma once

#include "parser_expression_node.h"

expr_t* auto_convert(expr_t* src, type_ptr dst_type);
expr_t* integer_increase(expr_t* src);
void align_types(expr_t** left, expr_t** right);
expr_t* array_to_ptr(expr_t* arr);
expr_t* func_to_ptr(expr_t* func);
void optimize_type(type_ptr type, sym_table_ptr sym_table);