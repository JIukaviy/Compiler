#pragma once

#include "parser_base_node.h"
#include "parser_expression_node.h"
#include "parser_symbol_node.h"
#include "exceptions.h"
#include <vector>

using namespace std;

class statement_t : public node_t {
public:
	virtual void print(ostream& os, int level) = 0;
	void print(ostream& os) override;
};

class stmt_block_t : public statement_t {
	vector<statement_t*> statements;
public:
	stmt_block_t(const vector<statement_t*>& statements);
	void print(ostream&, int level) override;
};

class stmt_expr_t : public statement_t {
	expr_t* expression;
public:
	stmt_expr_t(expr_t* expression);
	void print(ostream&, int level) override;
};

class stmt_decl_t : public statement_t {
	symbol_t* symbol;
public:
	stmt_decl_t(symbol_t* symbol);
	void print(ostream&, int level) override;
};

class stmt_if_t : public statement_t {
	expr_t* condition;
	statement_t* then_stmt;
	statement_t* else_stmt;
public:
	stmt_if_t(expr_t* condition, statement_t* then_stmt);
	stmt_if_t(expr_t* condition, statement_t* then_stmt, statement_t* else_stmt);
	void print(ostream&, int level) override;
};

class stmt_while_t : public statement_t {
	expr_t* condition;
	statement_t* stmt;
public:
	stmt_while_t(expr_t* condition, statement_t* stmt);
	void print(ostream&, int level) override;
};

class stmt_for_t : public statement_t {
	expr_t* init_expr;
	expr_t* condition;
	expr_t* expr;
	statement_t* stmt;
public:
	stmt_for_t(expr_t* init_expr, expr_t* condition, expr_t* expr, statement_t* stmt);
	void print(ostream&, int level) override;
};