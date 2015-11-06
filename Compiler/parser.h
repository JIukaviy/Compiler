#pragma once

#include <ostream>
#include "tokens.h"
#include "exceptions.h"
#include "lexeme_analyzer.h"
#include "parser_base_node.h"
#include "parser_expression_node.h"
#include "parser_symbol_node.h"
#include "parser_statement_node.h"
#include "symbol_table.h"
#include <vector>
#include <stack>
#include <map>

using namespace std;

void parser_init();

struct type_chain_t {
	type_chain_t();
	shared_ptr<updatable_sym_t> first;
	shared_ptr<updatable_sym_t> last;
	pos_t last_token_pos;
	token_ptr_t identifier;
	pos_t estimated_ident_pos;
	void update(shared_ptr<updatable_sym_t>);
	void update(token_ptr_t);
	void update(type_chain_t);
	operator bool();
};

struct decl_raw_t {
	token_ptr_t identifier;
	token_ptr_t type_def; //token_ptr_t необходим для вывода места в коде где он встретился, в случае ошибки.
	type_ptr_t type;
	pos_t estimated_ident_pos;
	pos_t type_spec_pos;
	vector<expr_t*> init_list;
	decl_raw_t();
	decl_raw_t(token_ptr_t, type_ptr_t);
	decl_raw_t(type_chain_t);
};

class parser_t {
	lexeme_analyzer_t* la;

	sym_table_t sym_table;
	stack<stmt_loop_t*> loop_stack;

	expr_t* left_associated_bin_op(int priority);
	expr_t* tern_op();
	expr_t* right_associated_bin_op();
	expr_t* prefix_un_op();
	expr_t* postfix_op();
	expr_t* factor();
	expr_t* parse_expr();

	decl_raw_t declaration();
	type_chain_t declarator();
	type_chain_t init_declarator();
	type_chain_t func_arr_decl();
	vector<expr_t*> parse_initializer_list();
	expr_t* parse_initializer();
	sym_ptr_t parse_declaration();

	statement_t* parse_statement();
	statement_t* stmt_expr();
	statement_t* stmt_decl();
	statement_t* stmt_block();
	statement_t* stmt_if();
	statement_t* stmt_while();
	statement_t* stmt_for();
	statement_t* stmt_break_continue();

	//expr_t* validate_expr(expr_t*);
	/*void check_is_const(expr_t*);
	expr_t* optimize_expr(expr_t*);
	int calc_expr(expr_t*);
	expr_t* optimize_bin_op(expr_bin_op_t*);
	expr_t* optimize_un_op(expr_un_op_t*);
	expr_t* optimize_tern_op(expr_tern_op_t*);
	expr_t* optimize_arr_index(expr_arr_index_t*);*/
public:
	parser_t(lexeme_analyzer_t* la_);
	void print_expr(ostream&);
	void print_type(ostream&);
	void print_decl(ostream&);
	void print_statement(ostream&);
};