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
	virtual void short_print(ostream& os, int level);
	void print(ostream& os) override;
	void short_print(ostream& os) override;
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

template<TOKEN T> 
class stmt_named_t : public virtual statement_t {
public:
	void short_print(ostream& os, int level) override;
};

template<TOKEN T>
inline void stmt_named_t<T>::short_print(ostream & os, int level) {
	print_level(os, level);
	os << "statement: \"" << token_t::get_name_by_id(T) << "\" ";
}

class stmt_if_t : public stmt_named_t<T_KWRD_IF> {
	expr_t* condition;
	statement_t* then_stmt;
	statement_t* else_stmt;
public:
	stmt_if_t(expr_t* condition, statement_t* then_stmt);
	stmt_if_t(expr_t* condition, statement_t* then_stmt, statement_t* else_stmt);
	void print(ostream&, int level) override;
};

class stmt_loop_t : public virtual statement_t {
protected:
	statement_t* stmt;
public:
	stmt_loop_t(statement_t* stmt);
	stmt_loop_t();
	void set_statement(statement_t* statement);
};

class stmt_while_t : public stmt_loop_t, public stmt_named_t<T_KWRD_WHILE> {
	expr_t* condition;
public:
	stmt_while_t(expr_t* condition, statement_t* stmt);
	stmt_while_t(expr_t* condition);
	void print(ostream&, int level) override;
};

class stmt_for_t : public stmt_loop_t, public stmt_named_t<T_KWRD_FOR> {
	expr_t* init_expr;
	expr_t* condition;
	expr_t* expr;
public:
	stmt_for_t(expr_t* init_expr, expr_t* condition, expr_t* expr, statement_t* stmt);
	stmt_for_t(expr_t* init_expr, expr_t* condition, expr_t* expr);
	void print(ostream&, int level) override;
};

template<TOKEN T> 
class stmt_jump_t : public stmt_named_t<T> {
protected:
	stmt_loop_t* loop;
public:
	stmt_jump_t(stmt_loop_t* loop);
	void print(ostream& os, int level) override;
};

template<TOKEN T>
stmt_jump_t<T>::stmt_jump_t(stmt_loop_t* loop) : loop(loop) {};

template<TOKEN T>
void stmt_jump_t<T>::print(ostream& os, int level) {
	short_print(os, level);
	os << "for ";
	loop->short_print(os);
	os << endl;
}

class stmt_break_t : public stmt_jump_t<T_KWRD_BREAK> {
	using stmt_jump_t<T_KWRD_BREAK>::stmt_jump_t;
};

class stmt_continue_t : public stmt_jump_t<T_KWRD_CONTINUE> {
	using stmt_jump_t<T_KWRD_CONTINUE>::stmt_jump_t;
};