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
	type_ptr first;
	type_ptr last;
	token_ptr identifier;
	pos_t estimated_ident_pos;
	sym_table_ptr func_sym_table;

	void update(type_ptr);
	void update(token_ptr);
	void update(type_chain_t);
	operator bool();
};

struct decl_raw_t {
	token_ptr identifier;
	token_ptr type_def; //token_ptr необходим для вывода места в коде где он встретился, в случае ошибки.
	type_ptr type;
	pos_t estimated_ident_pos;
	pos_t type_spec_pos;
	sym_table_ptr func_sym_table;
	vector<expr_t*> init_list;
	bool init_decl_is_empty = true;
	decl_raw_t();
	decl_raw_t(token_ptr, type_ptr);
	decl_raw_t(type_chain_t);
};

class parser_t {
	lexeme_analyzer_t* la;

	sym_table_ptr sym_table;
	sym_table_ptr top_sym_table;
	static sym_table_ptr prelude_sym_table;
	stack<shared_ptr<stmt_loop_t>> loop_stack;
	stack<shared_ptr<sym_func_t>> func_stack;

	sym_table_ptr  new_namespace();
	void exit_namespace();

	expr_t* left_associated_bin_op(int priority);
	expr_t* tern_op();
	expr_t* right_associated_bin_op();
	expr_t* printf_op();
	expr_t* prefix_un_op();
	expr_t* postfix_op();
	expr_t* factor();
	expr_t* parse_expr();

	vector<expr_t*> parse_func_args();
	vector<decl_raw_t> parse_func_arg_types();

	decl_raw_t parse_declaration_raw();
	type_chain_t parse_declarator();
	type_chain_t parse_init_declarator();
	type_chain_t func_arr_decl(bool in_func = false);
	vector<expr_t*> parse_initializer_list();
	expr_t* parse_initializer();
	sym_ptr parse_declaration(bool abstract_decl = false);
	sym_ptr parse_global_declaration();
	sym_ptr parse_declaration(decl_raw_t, sym_table_ptr, bool global = false, bool abstract_decl = false);
	void optimize_type(type_ptr);

	bool is_begin_of(STATEMENT stmt, token_ptr token);
	bool is_begin_of(STATEMENT stmt);
	stmt_ptr parse_statement();
	stmt_ptr parse_expr_stmt();
	void parse_decl_stmt();
	stmt_ptr parse_block_stmt();
	void parse_top_level_stmt();
	void parse_struct_decl_list(sym_table_ptr  sym_table);
	stmt_ptr parse_if_stmt();
	stmt_ptr parse_while_stmt();
	stmt_ptr parse_do_while_stmt();
	stmt_ptr parse_for_stmt();
	stmt_ptr parse_break_continue_stmt();
	stmt_ptr parse_return_stmt();
public:
	parser_t(lexeme_analyzer_t* la_);
	void print_expr(ostream&);
	void print_eval_expr(ostream&);
	void print_type(ostream&);
	void print_decl(ostream&);
	void print_statement(ostream&);
	void print_statements(ostream&);
	void print_asm_code(ostream&);
	static sym_table_ptr get_prelude_sym_table();
	static type_base_ptr get_base_type(SYM_TYPE sym_type);
	static type_ptr get_type(SYM_TYPE sym_type, bool is_const = false);
};