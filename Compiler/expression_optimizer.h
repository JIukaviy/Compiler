#pragma once
#include "exceptions.h"
#include "parser_expression_node.h"
#include "symbol_table.h"

expr_t* validate_expr(expr_t* expr, sym_table_t& st);