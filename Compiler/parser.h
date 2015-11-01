#pragma once

#include <ostream>
#include "tokens.h"
#include "exceptions.h"
#include "lexeme_analyzer.h"
#include "parser_base_node.h"
#include "parser_expression_node.h"
#include "parser_symbol_node.h"
#include "parser_statement_node.h"
#include <vector>
#include <unordered_set>

using namespace std;

void parser_init();

class node_str_literal : public node_t {
protected:
	token_ptr_t str;
public:
	node_str_literal(token_ptr_t str);
	void print(ostream&) override;
};

struct type_chain_t {
	type_chain_t();
	updatable_sym_t* first;
	updatable_sym_t* last;
	pos_t last_token_pos;
	token_ptr_t identifier;
	pos_t estimated_ident_pos;
	void update(updatable_sym_t*);
	void update(token_ptr_t);
	void update(type_chain_t);
	operator bool();
};

struct decl_raw_t {
	token_ptr_t identifier;
	token_ptr_t type_def; //token_ptr_t необходим для вывода места в коде где он встретился, в случае ошибки.
	type_t* type;
	pos_t estimated_ident_pos;
	pos_t type_spec_pos;
	vector<node_t*> init_list;
	decl_raw_t();
	decl_raw_t(token_ptr_t, type_t*);
	decl_raw_t(type_chain_t);
};

class sym_table_t : public unordered_set<symbol_t*> {
public:
	symbol_t* get(symbol_t*);
	symbol_t* get(token_ptr_t);
	bool is_var(token_ptr_t);
	bool is_alias(token_ptr_t);
	//void insert(symbol_t*);
};

class parser_t {
	lexeme_analyzer_t* la;

	expr_t* left_associated_bin_op(int priority);
	expr_t* tern_op();
	expr_t* right_associated_bin_op();
	expr_t* prefix_un_op();
	expr_t* postfix_op();
	expr_t* factor();
	expr_t* parse_expr();

	sym_table_t sym_table;
	decl_raw_t declaration();
	type_chain_t declarator();
	type_chain_t init_declarator();
	type_chain_t func_arr_decl();
	vector<node_t*> parse_initializer_list();
	node_t* parse_initializer();
	symbol_t* parse_declaration();

	statement_t* parse_statement();
	statement_t* stmt_expr();
	statement_t* stmt_block();
	statement_t* stmt_if();
	statement_t* stmt_while();
	statement_t* stmt_for();
	statement_t* stmt_decl();
public:
	parser_t(lexeme_analyzer_t* la_);
	void print_expr(ostream&);
	void print_type(ostream&);
	void print_decl(ostream&);
	void print_statement(ostream&);
};