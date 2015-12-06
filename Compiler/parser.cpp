#include "parser.h"
#include <map>
#include <assert.h>

sym_table_ptr parser_t::prelude_sym_table;

parser_t::parser_t(lexeme_analyzer_t* la_): la(la_) {
	if (!prelude_sym_table) {
		prelude_sym_table = sym_table_ptr(new sym_table_t);

		prelude_sym_table->insert(sym_ptr(new sym_type_int_t));
		prelude_sym_table->insert(sym_ptr(new sym_type_char_t));
		prelude_sym_table->insert(sym_ptr(new sym_type_double_t));
		prelude_sym_table->insert(sym_ptr(new sym_type_void_t));
	}

	sym_table = top_sym_table = sym_table_ptr(new sym_table_t(prelude_sym_table));
}

type_chain_t::type_chain_t() : last(0), first(0) {}
decl_raw_t::decl_raw_t() {}
decl_raw_t::decl_raw_t(token_ptr ident, type_ptr type) : identifier(ident), type(type) {}
decl_raw_t::decl_raw_t(type_chain_t chain) : type(chain.first), identifier(chain.identifier) {}

#define try_parse(expr) expr /*try { \
								expr; \
							} catch (UnexpectedEOF&) { \
								throw ExpressionIsExpected(op);	\
							catch (UnexpectedToken& e) {	\
								if (e.expected_token != T_EMPTY) \
									throw; \
								throw ExpressionIsExpected(op); \
							}*/

void type_chain_t::update(type_ptr e) {
	if (e)
		last = e;
	if (!first)
		first = e;
}

void type_chain_t::update(token_ptr e) {
	identifier = e;
}

void type_chain_t::update(type_chain_t e) {
	update(e.last);
	if (e.identifier)
		identifier = e.identifier;
	if (e.estimated_ident_pos)
		estimated_ident_pos = e.estimated_ident_pos;
	if (!func_sym_table)
		func_sym_table = e.func_sym_table;
}

type_chain_t:: operator bool() {
	return (bool)last;
}

set<TOKEN> op_by_priority[16];

//-------------------EXPRESSION_PARSER-------------------------------------------

expr_t* parser_t::parse_expr() {
	//return validate_expr(right_associated_bin_op(), sym_table);
	return right_associated_bin_op();
}

expr_t* parser_t::right_associated_bin_op() {
	expr_t* left = tern_op();
	token_ptr op = la->get();
	if (op->is(op_by_priority[15])) {
		la->next();
		expr_bin_op_t* res = expr_bin_op_t::make_bin_op(op);
		res->set_operands(left, right_associated_bin_op());
		return res;
	} else
		return left;
}

expr_t* parser_t::tern_op() {
	expr_t* left = left_associated_bin_op(14);
	token_ptr op = la->get();
	if (op == T_QUESTION_MARK) {
		la->next();
		expr_t* middle = tern_op();
		token_ptr c = la->get();
		la->require(op, T_COLON, 0);
		expr_tern_op_t* res = new expr_tern_op_t(op, c);
		res->set_operands(left, middle, right_associated_bin_op());
		return res;
	} else
		return left;
}

expr_t* parser_t::left_associated_bin_op(int p) {
	if (p == 4)
		return printf_un_op();
	expr_t* left = left_associated_bin_op(p-1);
	token_ptr op = la->get();
	while (op->is(op_by_priority[p-1])) {
		la->next();
		expr_bin_op_t* bin_op = expr_bin_op_t::make_bin_op(op);
		bin_op->set_operands(left, left_associated_bin_op(p - 1));
		left = bin_op;
		op = la->get();
	}
	return left;
}

expr_t* parser_t::printf_un_op() {
	token_ptr op = la->get();
	if (op == T_KWRD_PRINTF) {
		la->next();
		la->require(T_BRACKET_OPEN, 0);
		expr_un_op_t* un_op = expr_prefix_un_op_t::make_prefix_un_op(op);
		un_op->set_operand(prefix_un_op());
		la->require(T_BRACKET_CLOSE, 0);
		return un_op;
	} else
		return prefix_un_op();
}

expr_t* parser_t::prefix_un_op() {
	token_ptr op = la->get();
	if (op->is(op_by_priority[2])) {
		la->next();
		expr_un_op_t* un_op = expr_prefix_un_op_t::make_prefix_un_op(op);
		un_op->set_operand(prefix_un_op());
		return un_op;
	} else
		return postfix_op();
}

expr_t* parser_t::parser_t::postfix_op() {
	expr_t* left = factor();
	token_ptr op = la->get();

	while (op->is(T_OP_INC, T_OP_DEC, T_BRACKET_OPEN, T_OP_DOT, T_OP_ARROW, T_SQR_BRACKET_OPEN, 0)) {
		if (op->is(T_OP_INC, T_OP_DEC, 0)) {
			la->next();
			expr_un_op_t* un_op = expr_postfix_un_op_t::make_postfix_un_op(op);
			un_op->set_operand(left);
			left = un_op;
		} else if (op == T_BRACKET_OPEN) {
			expr_func_t* func_call = new expr_func_t(op);
			func_call->set_operands(left, parse_func_args());
			left = func_call;
		} else if (op->is(T_OP_DOT, T_OP_ARROW, 0)) {
			la->next();
			expr_struct_access_t* expr_struct = new expr_struct_access_t(op);
			expr_struct->set_operands(left, la->require(op, T_IDENTIFIER, 0));
			left = expr_struct;
		} else if (op == T_SQR_BRACKET_OPEN) {
			la->next();
			expr_t* index = right_associated_bin_op();
			la->require(op, T_SQR_BRACKET_CLOSE, 0);
			expr_arr_index_t* arr = new expr_arr_index_t(op);
			arr->set_operands(left, index);
			left = arr;
		}
		op = la->get();
	}
	return left;
}

expr_t* parser_t::factor() {
	token_ptr t = la->get();
	la->next();
	if (t == T_IDENTIFIER) {
		expr_var_t* res = new expr_var_t();
		res->set_symbol(dynamic_pointer_cast<sym_with_type_t>(sym_table->get_global(t)), t);
		return res;
	} else if (t->is(T_INTEGER, T_DOUBLE, T_STRING, T_CHAR, 0))
		return new expr_const_t(t);
	else if (t == T_BRACKET_OPEN) {
		expr_t* l = right_associated_bin_op();
		la->require(T_BRACKET_CLOSE, 0);
		return l;
	} else if (t == T_EMPTY)
		throw UnexpectedEOF();
	else 
		throw UnexpectedToken(t);
}

map<STATEMENT, set<TOKEN>> starting_tokens;
#define in_set(set_name, key) (set_name.find(key) != set_name.end())

//-------------------DECLARATION_PARSER-------------------------------------------

sym_ptr parser_t::parse_declaration(bool abstract_decl) {
	return parse_declaration(parse_declaration_raw(), sym_table, false, abstract_decl);
}

sym_ptr parser_t::parse_global_declaration() {
	return parse_declaration(parse_declaration_raw(), sym_table, true);
}

sym_ptr parser_t::parse_declaration(decl_raw_t decl, sym_table_ptr sym_table, bool global, bool abstract_decl) {
	sym_ptr res;

	optimize_type(decl.type);
	 
	if (!decl.identifier)
		if ((!decl.init_decl_is_empty || !decl.init_list.empty()) && !abstract_decl)
			throw SemanticError("Identifier is expected", decl.estimated_ident_pos);
		else
			return sym_ptr();

	/*if (decl.type->is_const() && decl.init_list.empty() && !decl.type_def)
		throw SemanticError("For constant variable initializer is needed");*/

	auto func_type = dynamic_pointer_cast<sym_type_func_t>(decl.type->get_base_type());
	if (decl.type_def) {
		if (!decl.init_list.empty())
			throw SemanticError("Init list not required for typedef", decl.estimated_ident_pos);
		if (func_type)
			throw SemanticError("Can't create alias to function", decl.type_def->get_pos());
		res = sym_ptr(new sym_type_alias_t(decl.identifier, decl.type));
	} else if (func_type) {
		if (!decl.init_list.empty())
			throw SemanticError("Unexpected init list for function declaration", decl.estimated_ident_pos);
		res = sym_ptr(new sym_func_t(decl.identifier, func_type, decl.func_sym_table));
	} else {
		sym_var_t* var = global ? (sym_var_t*)(new sym_global_var_t(decl.identifier)) : (sym_var_t*)(new sym_local_var_t(decl.identifier));
		var->set_type_and_init_list(decl.type, decl.init_list);
		res = sym_ptr(var);
	}

	res->set_token(decl.identifier);
	res->update_name();
	if (res != ST_FUNC_TYPE)
		sym_table->insert(res);

	return res;
}

void parser_t::optimize_type(type_ptr type) {
	type_base_ptr base_type = type->get_base_type();
	bool struct_definition = base_type == ST_STRUCT && base_type->completed();
	type_base_ptr finded_type = dynamic_pointer_cast<type_base_t>(struct_definition ?
																	sym_table->find_local(base_type) : sym_table->find_global(base_type));
	if (finded_type) {
		shared_ptr<sym_type_alias_t> alias = dynamic_pointer_cast<sym_type_alias_t>(finded_type);
		finded_type = alias ? alias->get_type() : finded_type;
		if (struct_definition) {
			if (finded_type->completed())
				throw RedefinitionOfSymbol(finded_type, base_type);
			static_pointer_cast<sym_type_struct_t>(finded_type)->set_sym_table(static_pointer_cast<sym_type_struct_t>(base_type)->get_sym_table());
		}
		type->set_base_type(finded_type);
		return;
	}
	upd_type_ptr upd_type = dynamic_pointer_cast<updatable_base_type_t>(base_type);
	if (upd_type)
		optimize_type(upd_type->get_element_type());
	sym_table->insert(base_type);
}

decl_raw_t parser_t::parse_declaration_raw() {
	bool has_const = false;
	token_ptr has_typedef;
	token_ptr type_spec;
	token_ptr struct_ident;
	type_base_ptr base_type = 0;

	while (is_begin_of(STMT_DECL)) {
		if (la->get() == T_KWRD_TYPEDEF && !has_typedef)
			has_typedef = la->get();
		else if (la->get() == T_KWRD_CONST)
			has_const = true;
		else if (la->get() == T_IDENTIFIER && !sym_table->is_alias(la->get()))
			break;
		else if (!type_spec) {
			type_spec = la->get();

			if (type_spec->is(T_KWRD_DOUBLE, T_KWRD_INT, T_KWRD_CHAR, T_KWRD_VOID, 0))
				base_type = type_base_t::make_type(symbol_t::token_to_sym_type(type_spec));
			else if (type_spec == T_IDENTIFIER)
				base_type = dynamic_pointer_cast<type_base_t>(sym_table->get_global(type_spec));
			else if (type_spec == T_KWRD_STRUCT) {
				la->next();
				struct_ident = la->require(T_IDENTIFIER, 0);
				sym_table_ptr  struct_sym_table = nullptr;
				auto strct = shared_ptr<sym_type_struct_t>(new sym_type_struct_t(struct_ident));
				if (la->get() == T_BRACE_OPEN) {
					sym_table_ptr strct_table = sym_table_ptr(new sym_table_t(prelude_sym_table));
					parse_struct_decl_list(strct_table);
					strct->set_sym_table(strct_table);
				}
				base_type = strct;
				base_type->set_token(type_spec);
				continue;
			} else
				assert(false);
			base_type->set_token(type_spec);
		} else
			throw InvalidCombinationOfSpecifiers(la->get());
		la->next();
	}

	if (!type_spec)
		throw TypeSpecIsExpected(la->get());
	
	type_chain_t chain = parse_declarator();
	decl_raw_t res(chain);
	bool is_ptr = false;
	res.init_decl_is_empty = !(chain || chain.identifier);
	type_ptr type(new type_t(base_type));
	type->set_is_const(has_const);
	if (chain) {
		static_pointer_cast<updatable_base_type_t>(chain.last->get_base_type())->set_element_type(type);
		type = chain.last;
	} else
		res.type = type;

	res.init_list = parse_initializer_list();
	res.type_def = has_typedef;
	res.type_spec_pos = type_spec->get_pos();
	res.estimated_ident_pos = chain.estimated_ident_pos;
	res.func_sym_table = chain.func_sym_table;
	res.type->update_name();

	return res;
}

type_chain_t parser_t::parse_declarator() {
	token_ptr token = la->get();
	if (token == T_OP_MUL) {
		bool is_const_ = false;
		if (la->next() == T_KWRD_CONST) {
			is_const_ = true;
			la->next();
		}
		type_chain_t r = parse_declarator();
		type_base_ptr ptr = type_base_ptr(new sym_type_ptr_t);
		ptr->set_token(token);
		type_ptr l(new type_t(ptr));
		if (is_const_)
			l->set_is_const(is_const_);
		if (r.last)
			static_pointer_cast<updatable_base_type_t>(r.last->get_base_type())->set_element_type(l);
		r.update(l);
		return r;
	} else
		return parse_init_declarator();
}

int *(a)(char ());

type_chain_t parser_t::parse_init_declarator() {
	type_chain_t dcl;
	token_ptr token = la->get();
	if (token == T_IDENTIFIER) {
		dcl.update(la->get());
		la->next();
	} else if (token == T_BRACKET_OPEN) {
		la->next();
		dcl = parse_declarator();
		la->require(T_BRACKET_CLOSE, 0);
		if (!dcl && !dcl.identifier)
			throw SemanticError("Abstract function declaration not supported", token->get_pos());
	}
	if (!dcl.estimated_ident_pos)
		dcl.estimated_ident_pos = la->get()->get_pos();
	type_chain_t r = func_arr_decl();
	if (dcl.last)
		static_pointer_cast<updatable_base_type_t>(dcl.last->get_base_type())->set_element_type(r.first);
	if (!dcl.first)
		dcl.first = r.first;
	if (r.last)
		dcl.last = r.last;
	if (!dcl.func_sym_table)
		dcl.func_sym_table = r.func_sym_table;
	return dcl;
}

type_chain_t parser_t::func_arr_decl(bool in_func) {
	type_chain_t dcl;
	token_ptr token = la->get();
	sym_table_ptr func_sym_table;
	if (token->is(T_SQR_BRACKET_OPEN, T_BRACKET_OPEN, 0)) {
		type_base_ptr l_base;
		if (token == T_SQR_BRACKET_OPEN) {
			la->next();
			expr_t* expr = nullptr;
			if (la->get() != T_SQR_BRACKET_CLOSE)
				expr = parse_expr();
			la->require(T_SQR_BRACKET_CLOSE, 0);
			l_base = type_base_ptr(new sym_type_array_t(expr));
		} else if (token == T_BRACKET_OPEN) {
			vector<decl_raw_t> args = parse_func_arg_types();
			vector<type_ptr> arg_types;
			if (!in_func) {
				func_sym_table = sym_table_ptr(new sym_table_t(top_sym_table));
				for each (auto var in args) {
					if (var.type_def)
						throw SemanticError("Typedef not supported in function arguments declaration", var.type_def->get_pos());
					if (!var.init_list.empty())
						throw SemanticError("Default function parameters temporarily not supported", var.estimated_ident_pos);
					arg_types.push_back(var.type);
					parse_declaration(var, func_sym_table, false, true);
				}
			}
			l_base = type_base_ptr(new sym_type_func_t(arg_types));
			in_func = true;
		} else
			assert(false);
		l_base->set_token(token);
		dcl = func_arr_decl(in_func);
		dcl.func_sym_table = func_sym_table;
		type_ptr l = type_ptr(new type_t(l_base));
		static_pointer_cast<updatable_base_type_t>(l->get_base_type())->set_element_type(dcl.last);
		dcl.first = l;
		if (!dcl.last)
			dcl.last = l;
	}
	return dcl;
}

vector<expr_t*> parser_t::parse_initializer_list() {
	vector<expr_t*> res;
	if (la->get() == T_OP_ASSIGN) {
		if (la->next() == T_BRACE_OPEN) {
			do {
				if (la->next() == T_BRACE_CLOSE)
					break;
				res.push_back(parse_initializer());
			} while (la->get() == T_COMMA);
			la->require(T_BRACE_CLOSE, 0);
		} else
			res.push_back(parse_initializer());
	}
	return res;
}

expr_t* parser_t::parse_initializer() {
	return parse_expr();
}

vector<expr_t*> parser_t::parse_func_args() {
	vector<expr_t*> res;
	la->require(T_BRACKET_OPEN, 0);
	if (la->get() == T_BRACKET_CLOSE) {
		la->next();
		return res;
	}
	while (true) {
		res.push_back(parse_expr());
		if (la->get() == T_COMMA)
			la->next();
		else
			break;
	}
	la->require(T_BRACKET_CLOSE, 0);
	return res;
}

vector<decl_raw_t> parser_t::parse_func_arg_types() {
	vector<decl_raw_t> res;
	la->require(T_BRACKET_OPEN, 0);
	if (la->get() == T_BRACKET_CLOSE) {
		la->next();
		return res;
	}
	while (true) {
		res.push_back(parse_declaration_raw());
		if (la->get() == T_COMMA)
			la->next();
		else
			break;
	}
	la->require(T_BRACKET_CLOSE, 0);
	return res;
}

//-------------------STATEMENT_PARSER-------------------------------------------

sym_table_ptr parser_t::new_namespace() {
	return sym_table = sym_table_ptr(new sym_table_t(sym_table));
}

void parser_t::exit_namespace() {
	sym_table = sym_table->get_parent();
}

bool parser_t::is_begin_of(STATEMENT stmt, token_ptr token) {
	bool res = in_set(starting_tokens.at(stmt), token->get_token_id());
	return stmt == STMT_DECL ? res || sym_table->is_alias(token) : res;
}

bool parser_t::is_begin_of(STATEMENT stmt) {
	return is_begin_of(stmt, la->get());
}

stmt_ptr parser_t::parse_statement() {
	if (la->get() == T_EMPTY)
		throw SemanticError("Expected declaration or statement before end of input");
	else if (is_begin_of(STMT_NONE))
		throw UnexpectedToken(la->get());

	return
		is_begin_of(STMT_EMPTY) ? la->next(), nullptr :
		is_begin_of(STMT_DECL) ? parse_decl_stmt(), nullptr :
		is_begin_of(STMT_BLOCK) ? parse_block_stmt() :
		is_begin_of(STMT_EXPR) ? parse_expr_stmt() :
		is_begin_of(STMT_WHILE) ? parse_while_stmt() :
		is_begin_of(STMT_FOR) ? parse_for_stmt() :
		is_begin_of(STMT_IF) ? parse_if_stmt() : 
		is_begin_of(STMT_RETURN) ? parse_return_stmt() : parse_break_continue_stmt();
}

stmt_ptr parser_t::parse_block_stmt() {
	la->require(T_BRACE_OPEN, 0);
	vector<stmt_ptr> stmts;
	sym_table_ptr  sym_table = new_namespace();
	while (la->get() != T_BRACE_CLOSE) {
		stmt_ptr stmt = parse_statement();
		if (stmt)
			stmts.push_back(stmt);
	}
	la->require(T_BRACE_CLOSE, 0);
	exit_namespace();
	return stmt_ptr(new stmt_block_t(stmts, sym_table));
}

void parser_t::parse_top_level_stmt() {
	while (la->get() != T_EMPTY)
		if (la->get() == T_SEMICOLON)
			la->next();
		else
			parse_decl_stmt();
}

void parser_t::parse_struct_decl_list(sym_table_ptr sym_table) {
	la->require(T_BRACE_OPEN, 0);
	while (la->get() != T_BRACE_CLOSE) {
		if (la->get() == T_SEMICOLON) {
			la->next();
			continue;
		}
		decl_raw_t decl = parse_declaration_raw();
		la->require(T_SEMICOLON, 0);
		if (!decl.init_list.empty())
			throw SemanticError("Initializer list not supported in struct members declaration", decl.estimated_ident_pos);
		if (decl.type_def)
			throw SemanticError("Unexpected \"typedef\" in struct members declaration", decl.type_def->get_pos());
		if (decl.type == ST_FUNC_TYPE)
			throw SemanticError("Functions not supported in struct members declaration", decl.estimated_ident_pos);
		parse_declaration(decl, sym_table);
	}
	la->require(T_BRACE_CLOSE, 0);
}

void parser_t::parse_decl_stmt() {
	shared_ptr<sym_func_t> sym_func = dynamic_pointer_cast<sym_func_t>(sym_table == top_sym_table ? parse_global_declaration() : parse_declaration());
	if (sym_func) {
		sym_ptr finded_global_sym = sym_table->find_global(sym_func);
		shared_ptr<sym_func_t> finded_func = dynamic_pointer_cast<sym_func_t>(finded_global_sym);
		if (finded_global_sym && (!finded_func || !sym_func->get_func_type()->is(finded_func->get_func_type())))
			throw RedefinitionOfSymbol(finded_global_sym, sym_func);
		if (la->get() == T_BRACE_OPEN) {
			if (sym_table != top_sym_table)
				throw SemanticError("Functions can't be defined in functions", la->get()->get_pos());
			if (finded_func) {
				finded_func->set_sym_table(sym_func->get_sym_table());
				sym_func = finded_func;
			} else 
				sym_table->insert(sym_func);
			sym_table = sym_func->get_sym_table();
			func_stack.push(sym_func);
			sym_func->set_block(parse_block_stmt());
			func_stack.pop();
			exit_namespace();
			return;
		} else {
			sym_func->clear_sym_table();
			if (!finded_func)
				sym_table->insert(sym_func);
		}
	}
	la->require(T_SEMICOLON, 0);
}

stmt_ptr parser_t::parse_expr_stmt() {
	expr_t* expr = parse_expr();
	la->require(T_SEMICOLON, 0);
	return stmt_ptr(new stmt_expr_t(expr));
}

stmt_ptr parser_t::parse_if_stmt() {
	la->require(T_KWRD_IF, 0);
	la->require(T_BRACKET_OPEN, 0);
	expr_t* condition = parse_expr();
	la->require(T_BRACKET_CLOSE, 0);
	stmt_ptr then_stmt = parse_statement();
	stmt_ptr else_stmt = nullptr;
	if (la->get() == T_KWRD_ELSE) {
		la->next();
		else_stmt = parse_statement();
	}
	return stmt_ptr(new stmt_if_t(condition, then_stmt, else_stmt));
}

stmt_ptr parser_t::parse_while_stmt() {
	la->require(T_KWRD_WHILE, 0);
	la->require(T_BRACKET_OPEN, 0);
	expr_t* condition = parse_expr();
	la->require(T_BRACKET_CLOSE, 0);

	auto res = shared_ptr<stmt_loop_t>(new stmt_while_t(condition));
	loop_stack.push(res);
	res->set_statement(parse_statement());
	loop_stack.pop();
	return stmt_ptr(res);
}

stmt_ptr parser_t::parse_for_stmt() {
	la->require(T_KWRD_FOR, 0);
	la->require(T_BRACKET_OPEN, 0);

	expr_t* init_expr = nullptr;
	if (la->get() != T_SEMICOLON)
		init_expr = parse_expr();
	la->require(T_SEMICOLON, 0);

	expr_t* condition = nullptr;
	if (la->get() != T_SEMICOLON)
		condition = parse_expr();
	la->require(T_SEMICOLON, 0);

	expr_t* expr = nullptr;
	if (la->get() != T_BRACKET_CLOSE)
		expr = parse_expr();
	la->require(T_BRACKET_CLOSE, 0);

	auto res = shared_ptr<stmt_loop_t>(new stmt_for_t(init_expr, condition, expr));
	loop_stack.push(res);
	res->set_statement(parse_statement());
	loop_stack.pop();
	return res;
}

stmt_ptr parser_t::parse_break_continue_stmt() {
	token_ptr token = la->get();
	la->require(T_KWRD_BREAK, T_KWRD_CONTINUE, 0);
	la->require(T_SEMICOLON, 0);
	if (loop_stack.empty())
		throw JumpStmtNotInsideLoop(token);
	if (token == T_KWRD_BREAK)
		return stmt_ptr(new stmt_break_t(loop_stack.top()));
	else
		return stmt_ptr(new stmt_continue_t(loop_stack.top()));
}

stmt_ptr parser_t::parse_return_stmt() {
	la->require(T_KWRD_RETURN, 0);
	if (func_stack.empty())
		throw SemanticError("Return statement must be inside the function", la->get()->get_pos());
	expr_t* expr = la->get() == T_SEMICOLON ? nullptr : parse_expr();
	la->require(T_SEMICOLON);
	return stmt_ptr(new stmt_return_t(func_stack.top()));
}

//---------------------------------PRINT-------------------------------------------

void parser_t::print_expr(ostream& os) {
	la->next();
	if (la->get() != T_EMPTY)
		right_associated_bin_op()->print(os);
}

void parser_t::print_eval_expr(ostream& os) {
	if (la->next() == T_EMPTY)
		return;
	expr_t* expr = parse_expr();
	expr->eval()->print(os);
	os << endl;
}

void parser_t::print_type(ostream& os) {
	la->next();
	if (la->get() != T_EMPTY) {
		decl_raw_t res = parse_declaration_raw();
		if (!res.type)
			return;
		if (res.identifier)
			res.identifier->short_print(os);
		else
			os << "abstract declaration";
		os << ": ";
		res.type->print(os);

		if (!res.init_list.empty()) {
			os << ", with initializer: {";
			for (int i = 0; i < res.init_list.size(); i++) {
				res.init_list[i]->short_print(os);
				if (i != res.init_list.size() - 1)
					os << ", ";
			}
			os << "}";
		}
	}
	os << endl;
}

void parser_t::print_decl(ostream& os) {
	la->next();
	while (la->get() != T_EMPTY) {
		parse_declaration();
		la->require(T_SEMICOLON, 0);
	}
	sym_table->print(os);
}

void parser_t::print_statement(ostream& os) {
	if (la->next() != T_EMPTY) {
		stmt_ptr stmt = parse_statement();
		sym_table->print(os);
		if (stmt)
			stmt->print(os);
	}
}

void parser_t::print_statements(ostream& os) {
	if (la->next() != T_EMPTY) {
		parse_top_level_stmt();
		sym_table->print(os);
	}
}

void parser_t::print_asm_code(ostream& os) {
	if (la->next() != T_EMPTY) {
		asm_gen_ptr gen(new asm_generator_t);
		stmt_ptr main_block;
		parse_top_level_stmt();
		for each (auto sym in *top_sym_table) {
			if (sym == ST_FUNC) {
				auto sym_func = dynamic_pointer_cast<sym_func_t>(sym);
				if (sym_func->get_name() == "main") {
					if (!sym_func->defined())
						throw MainFuncNotFound();
					main_block = sym_func->get_block();
				}
				asm_cmd_list_ptr cmd_list(new asm_cmd_list_t);
				sym_func->asm_generate_code(cmd_list);
				gen->add_function(sym_func->get_name(), cmd_list);
			}
		}
		if (!main_block)
			throw MainFuncNotFound();
		gen->print(os);
	}
}

sym_table_ptr parser_t::get_prelude_sym_table() {
	return prelude_sym_table;
}

type_base_ptr parser_t::get_base_type(SYM_TYPE sym_type) {
	return dynamic_pointer_cast<type_base_t>(prelude_sym_table->get_global(type_base_t::make_type(sym_type)));
}

type_ptr parser_t::get_type(SYM_TYPE sym_type, bool is_const) {
	return type_t::make_type(get_base_type(sym_type), is_const);
}

void set_operator_priority(TOKEN op, int priority) {
	assert(!(priority > 16 || priority < 1 || op_by_priority[priority - 1].find(op) != op_by_priority[priority - 1].end()));
	op_by_priority[priority-1].insert(op);
}

void set_operator_priority(TOKEN op, int priority, int priority2) {
	set_operator_priority(op, priority);
	set_operator_priority(op, priority2);
}

void parser_init() {
#define register_token(incode_name, printed_name, func_name, statement, ...) set_operator_priority(T_##incode_name, __VA_ARGS__);
#define TOKEN_LIST
#define PRIORITY_SET
#include "token_operator.h"
#undef PRIORITY_SET
#undef TOKEN_LIST
#undef register_token

#define register_token(incode_name, printed_name, func_name, statement, ...) starting_tokens[statement].insert(T_##incode_name);
#define TOKEN_LIST
#include "token_register.h"
#undef TOKEN_LIST
#undef register_token

	init_parser_symbol_node();
}