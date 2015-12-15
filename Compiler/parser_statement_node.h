#pragma once

#include "parser_base_node.h"
#include "parser_expression_node.h"
#include "symbol_table.h"
#include "exceptions.h"
#include "asm_generator.h"
#include <vector>

using namespace std;

enum STATEMENT {
	STMT_NONE,
	STMT_EMPTY,
	STMT_BLOCK,
	STMT_DECL,
	STMT_EXPR,
	STMT_IF,
	STMT_FOR,
	STMT_DO_WHILE,
	STMT_WHILE,
	STMT_BREAK,
	STMT_CONTINUE,
	STMT_RETURN
};

class statement_t : public node_t {
public:
	virtual void asm_generate_code(asm_cmd_list_ptr cmd_list, int offset = 0);
	virtual void asm_gen_entry_code(asm_cmd_list_ptr cmd_list, int offset = 0);
	virtual void asm_gen_internal_code(asm_cmd_list_ptr cmd_list, int offset = 0);
	virtual void asm_gen_exit_code(asm_cmd_list_ptr cmd_list);
};

class stmt_block_t : public statement_t {
	vector<stmt_ptr> statements;
	sym_table_ptr sym_table;
	int vars_size;
public:
	stmt_block_t();
	stmt_block_t(const vector<stmt_ptr>& statements, sym_table_ptr sym_table);
	void add_statement(stmt_ptr stmt);
	sym_table_ptr get_sym_table();
	void print_l(ostream& os, int level) override;
	void asm_generate_code(asm_cmd_list_ptr cmd_list, int offset = 0) override;
	void asm_gen_entry_code(asm_cmd_list_ptr cmd_list, int offset = 0);
	void asm_gen_internal_code(asm_cmd_list_ptr cmd_list, int offset = 0);
	void asm_gen_exit_code(asm_cmd_list_ptr cmd_list);
};

class stmt_expr_t : public statement_t {
	expr_t* expression;
public:
	stmt_expr_t(expr_t* expression);
	void print_l(ostream& os, int level) override;
	void asm_gen_internal_code(asm_cmd_list_ptr cmd_list, int offset = 0) override;
};

class stmt_decl_t : public statement_t {
	sym_ptr symbol;
public:
	stmt_decl_t(sym_ptr symbol);
	void print_l(ostream& os, int level) override;
};

template<TOKEN T> 
class stmt_named_t : public virtual statement_t {
public:
	void short_print_l(ostream& os, int level) override;
};

template<TOKEN T>
inline void stmt_named_t<T>::short_print_l(ostream& os, int level) {
	os << "statement: \"" << token_t::get_name_by_id(T) << "\" ";
}

class stmt_if_t : public stmt_named_t<T_KWRD_IF> {
	expr_t* condition;
	stmt_ptr then_stmt;
	stmt_ptr else_stmt;
public:
	stmt_if_t(expr_t* condition, stmt_ptr then_stmt);
	stmt_if_t(expr_t* condition, stmt_ptr then_stmt, stmt_ptr else_stmt);
	void asm_gen_internal_code(asm_cmd_list_ptr cmd_list, int offset = 0) override;
	void print_l(ostream& os, int level) override;
};

class stmt_loop_t : public virtual statement_t {
protected:
	stmt_ptr stmt;
	asm_label_ptr loop_label;
	asm_label_ptr exit_loop_label;
public:
	stmt_loop_t(stmt_ptr stmt);
	stmt_loop_t();
	void set_statement(stmt_ptr statement);
	void asm_gen_entry_code(asm_cmd_list_ptr cmd_list, int offset = 0) override;
	void asm_gen_exit_code(asm_cmd_list_ptr cmd_list) override;
	void asm_gen_jmp_to_loop(asm_cmd_list_ptr cmd_list);
	void asm_gen_jmp_to_exit_loop(asm_cmd_list_ptr cmd_list);
};

class stmt_while_t : public stmt_loop_t, public stmt_named_t<T_KWRD_WHILE> {
protected:
	expr_t* condition;
public:
	stmt_while_t(expr_t* condition, stmt_ptr stmt);
	stmt_while_t(expr_t* condition);
	void print_l(ostream& os, int level) override;
	void asm_gen_internal_code(asm_cmd_list_ptr cmd_list, int offset = 0) override;
};

class stmt_do_while_t : public stmt_while_t {
public:
	stmt_do_while_t();
	void set_condition(expr_t* condition);
	void asm_gen_internal_code(asm_cmd_list_ptr cmd_list, int offset = 0) override;
};

class stmt_for_t : public stmt_loop_t, public stmt_named_t<T_KWRD_FOR> {
	expr_t* init_expr;
	expr_t* condition;
	expr_t* expr;
public:
	stmt_for_t(expr_t* init_expr, expr_t* condition, expr_t* expr, stmt_ptr stmt);
	stmt_for_t(expr_t* init_expr, expr_t* condition, expr_t* expr);
	void print_l(ostream& os, int level) override;
	void asm_gen_internal_code(asm_cmd_list_ptr cmd_list, int offset = 0) override;
};

template<TOKEN T, typename pT> 
class stmt_jump_t : public stmt_named_t<T> {
protected:
	pT parent;
public:
	stmt_jump_t(pT parent);
	void print_l(ostream& os, int level) override;
};

template<TOKEN T, typename pT>
stmt_jump_t<T, typename pT>::stmt_jump_t(pT parent) : parent(parent) {};

template<TOKEN T, typename pT>
void stmt_jump_t<T, typename pT>::print_l(ostream& os, int level) {
	short_print_l(os, level);
	os << "for ";
	parent->short_print(os);
}

class stmt_break_t : public stmt_jump_t<T_KWRD_BREAK, shared_ptr<stmt_loop_t>> {
public:
	using stmt_jump_t<T_KWRD_BREAK, shared_ptr<stmt_loop_t>>::stmt_jump_t;
	void asm_gen_internal_code(asm_cmd_list_ptr cmd_list, int offset);
};

class stmt_continue_t : public stmt_jump_t<T_KWRD_CONTINUE, shared_ptr<stmt_loop_t>> {
public:
	using stmt_jump_t<T_KWRD_CONTINUE, shared_ptr<stmt_loop_t>>::stmt_jump_t;
	void asm_gen_internal_code(asm_cmd_list_ptr cmd_list, int offset);
};

class stmt_return_t : public stmt_jump_t<T_KWRD_RETURN, shared_ptr<sym_func_t>> {
	expr_t* expr;
public:
	using stmt_jump_t<T_KWRD_RETURN, shared_ptr<sym_func_t>>::stmt_jump_t;
	void set_ret_expr(expr_t* expr);
	void asm_gen_internal_code(asm_cmd_list_ptr cmd_list, int offset = 0) override;
};
