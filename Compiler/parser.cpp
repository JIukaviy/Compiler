#include "parser.h"
#include <map>

parser_t::parser_t(lexeme_analyzer_t* la_): la(la_) {}

type_chain_t::type_chain_t() : last(0), first(0) {}
node_str_literal::node_str_literal(token_ptr_t str) : str(str) {}
decl_raw_t::decl_raw_t() : type(0) {}
decl_raw_t::decl_raw_t(token_ptr_t ident, type_t* type) : identifier(ident), type(type) {}
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

void type_chain_t::update(updatable_sym_t* e) {
	if (e)
		last = e;
	if (!first)
		first = e;
}

void type_chain_t::update(token_ptr_t e) {
	identifier = e;
}

void type_chain_t::update(type_chain_t e) {
	update(e.last);
	if (e.identifier)
		identifier = e.identifier;
	if (e.estimated_ident_pos)
		estimated_ident_pos = e.estimated_ident_pos;
}

type_chain_t:: operator bool() {
	return last;
}


void node_str_literal::print(ostream& os) {
	os << '"';
	str->short_print(os);
	os << '"';
}

symbol_t* sym_table_t::get(symbol_t* s) {
	auto res = find(s);
	return res == end() ? nullptr : *res;
}

/*symbol_t * sym_table_t::get(token_ptr_t token) {
	if (token != T_IDENTIFIER)
		throw;
	return static_cast<token_with_value_t<string>*>(token.get())->get_value().;
}*/

/*void sym_table_t::insert(symbol_t* s) {
	if (!get(s))
		throw SemanticError("Redefenition of symbol", )
	unordered_set<symbol_t*>::insert(s);
}*/

set<TOKEN> operators[16];

expr_t* parser_t::right_associated_bin_op() {
	expr_t* left = tern_op();
	token_ptr_t op = la->get();
	if (op->is(operators[15])) {
		la->next();
		try_parse(return new expr_bin_op_t(left, right_associated_bin_op(), op));
	} else
		return left;
}

expr_t* parser_t::tern_op() {
	expr_t* left = left_associated_bin_op(14);
	token_ptr_t op = la->get();
	if (op == T_QUESTION_MARK) {
		la->next();
		expr_t* middle = tern_op();
		la->require(op, T_COLON, T_EMPTY);
		try_parse(return new expr_tern_op_t(left, middle, right_associated_bin_op()));
	} else
		return left;
}

expr_t* parser_t::left_associated_bin_op(int p) {
	if (p == 4)
		return prefix_un_op();
	expr_t* left = left_associated_bin_op(p-1);
	token_ptr_t op = la->get();
	while (op->is(operators[p-1])) {
		la->next();
		try_parse(left = new expr_bin_op_t(left, left_associated_bin_op(p-1), op));
		op = la->get();
	}
	return left;
}

expr_t* parser_t::prefix_un_op() {
	token_ptr_t op = la->get();
	if (op->is(operators[2])) {
		la->next();
		try_parse(return new expr_prefix_un_op_t(prefix_un_op(), op));
	} else
		return postfix_op();
}

expr_t* parser_t::parser_t::postfix_op() {
	expr_t* left = factor();
	token_ptr_t op = la->get();

	while (op->is(T_OP_INC, T_OP_DEC, T_BRACKET_OPEN, T_OP_DOT, T_OP_ARROW, T_SQR_BRACKET_OPEN, T_EMPTY)) {
		la->next();
		if (op->is(T_OP_INC, T_OP_DEC, T_EMPTY)) {
			left = new expr_postfix_un_op_t(left, op);
		} else if (op == T_BRACKET_OPEN) {
			vector<expr_t*> args;
			while (la->get() != T_BRACKET_CLOSE) {
				try {
					args.push_back(right_associated_bin_op());
				} catch (UnexpectedEOF& e) {
					throw SyntaxError("Function call was expecting ')'");
				}
				if (la->get() == T_COMMA)
					la->next();
				else 
					break;
			}
			la->require(T_BRACKET_CLOSE, T_EMPTY);
			left = new expr_func_t(left, args);
		} else if (op->is(T_OP_DOT, T_OP_ARROW, T_EMPTY)) {
			token_ptr_t member = la->require(op, T_IDENTIFIER, T_EMPTY);
			left = new expr_struct_access_t(left, op, member);
		} else if (op == T_SQR_BRACKET_OPEN) {
			expr_t* index = right_associated_bin_op();
			la->require(op, T_SQR_BRACKET_CLOSE, T_EMPTY);
			left = new expr_arr_index_t(left, index);
		}
		op = la->get();
	}
	return left;
}

expr_t* parser_t::factor() {
	token_ptr_t t = la->get();
	la->next();
	if (t == T_IDENTIFIER)
		return new expr_var_t(t);
	else if (t == T_INTEGER || t == T_DOUBLE)
		return new expr_const_t(t);
	else if (t == T_BRACKET_OPEN) {
		expr_t* l = right_associated_bin_op();
		la->require(T_BRACKET_CLOSE, T_EMPTY);
		return l;
	} else if (t == T_EMPTY)
		throw UnexpectedEOF();
	else 
		throw UnexpectedToken(t);
}

//set<TOKEN> type_specifiers;
//#define in_set(set_name, key) (set_name.find(key) != set_name.end())

symbol_t* parser_t::parse_declaration() {
	decl_raw_t decl = declaration();
	symbol_t* res = nullptr;
	if (!decl.identifier)
		throw SemanticError("Identifier is expected", decl.estimated_ident_pos);

	if (decl.type->is_const() && decl.init_list.empty() && !decl.type_def)
		throw SemanticError("For constant variable initializer is needed");

	if (typeid(*decl.type) == typeid(sym_type_void_t))
		throw InvalidIncompleteType(decl.type_spec_pos);

	if (sym_table.get(decl.type))
		decl.type = (type_t*)(sym_table.get(decl.type));
	else
		sym_table.insert(decl.type);

	if (decl.type_def) {
		res = new sym_type_alias_t(decl.identifier, decl.type);
		if (!decl.init_list.empty())
			throw SemanticError("Init list not required for typedef");
	} else
		res = new sym_var_t(decl.identifier, decl.type, decl.init_list);
	
	res->update_hash();
	sym_table.insert(res);
	
	return res;
}

decl_raw_t parser_t::declaration() {
	bool has_const = false;
	token_ptr_t has_typedef;
	token_ptr_t type_spec;
	while (la->get()->is(T_KWRD_TYPEDEF, T_KWRD_CONST, T_KWRD_INT, T_KWRD_DOUBLE, T_KWRD_CHAR, T_KWRD_VOID, 0)) {
		/*if (la->get()->is(T_KWRD_TYPEDEF, T_KWRD_CONST, T_EMPTY)) {
			if (curr_specs[la->get()])
				throw InvalidCombinationOfSpecifiers(la->get());
		} else {
			if (curr_specs[T_KWRD_INT] || curr_specs[T_KWRD_DOUBLE] || curr_specs[T_KWRD_CHAR] || curr_specs[T_KWRD_VOID])
				throw InvalidCombinationOfSpecifiers(la->get());
		}
		curr_specs[la->get()] = true;*/
		if (la->get() == T_KWRD_TYPEDEF) {
			if (has_typedef)
				throw InvalidCombinationOfSpecifiers(la->get()->get_pos());
			else
				has_typedef = la->get();
		} else if (la->get() == T_KWRD_CONST) {
			if (has_const)
				throw InvalidCombinationOfSpecifiers(la->get()->get_pos());
			else
				has_const = true;
		} else if (!type_spec) {
			type_spec = la->get();
		} else
			throw InvalidCombinationOfSpecifiers(la->get()->get_pos());
		la->next();
	}
	/*type_t* type =
		curr_specs[T_KWRD_INT] ? new sym_type_int_t() :
		curr_specs[T_KWRD_DOUBLE] ? new sym_type_double_t() :
		curr_specs[T_KWRD_CHAR] ? new sym_type_char_t() :
		curr_specs[T_KWRD_VOID] ? new sym_type_void_t() : 0;*/

	/*type_t* type =
		type_spec == T_KWRD_INT ? new sym_type_int_t() :
		type_spec == T_KWRD_DOUBLE ? new sym_type_double_t() :
		type_spec == T_KWRD_CHAR ? new sym_type_char_t() :
		type_spec == T_KWRD_VOID ? new sym_type_void_t() : 0;*/

	type_t* type = 0;
	if (!type_spec)
		throw SyntaxError("Type specifier is expected", la->get()->get_pos());
	else if (type_spec == T_KWRD_INT)
		type = new sym_type_int_t();
	else if (type_spec == T_KWRD_DOUBLE)
		type = new sym_type_double_t();
	else if (type_spec == T_KWRD_CHAR)
		type = new sym_type_char_t();
	else if (type_spec == T_KWRD_VOID)
		type = new sym_type_void_t();
	
	type_chain_t chain = declarator();
	decl_raw_t res(chain);
	bool is_ptr = false;
	if (chain) {
		is_ptr = typeid(sym_type_ptr_t) == typeid(*chain.last);
		chain.last->set_element_type(type, type_spec->get_pos());
		if (is_ptr)
			type->set_const(has_const);
		type = chain.last;
	} else
		res.type = type;

	if (!is_ptr)
		type->set_const(has_const);

	res.init_list = parse_initializer_list();
	res.type_def = has_typedef;
	res.type_spec_pos = type_spec->get_pos();
	res.estimated_ident_pos = chain.estimated_ident_pos;

	return res;
}

type_chain_t parser_t::declarator() {
	token_ptr_t token = la->get();
	if (token == T_OP_MUL) {
		bool is_const_ = false;
		if (la->next() == T_KWRD_CONST) {
			is_const_ = true;
			la->next();
		}
		type_chain_t r = declarator();
		updatable_sym_t* l = new sym_type_ptr_t(is_const_);
		if (r.last)
			r.last->set_element_type(l, r.last_token_pos);
		r.update(l);
		r.last_token_pos = token->get_pos();
		return r;
	} else
		return init_declarator();
}

typedef const int int_a;

type_chain_t parser_t::init_declarator() {
	type_chain_t dcl;
	token_ptr_t token = la->get();
	if (token == T_IDENTIFIER) {
		dcl.update(la->get());
		la->next();
	} else if (token == T_BRACKET_OPEN) {
		la->next();
		dcl = declarator();
		la->require(T_BRACKET_CLOSE, T_EMPTY);
	}
	if (!dcl.estimated_ident_pos)
		dcl.estimated_ident_pos = la->get()->get_pos();
	type_chain_t r = func_arr_decl();
	if (dcl.last)
		dcl.last->set_element_type(r.first, r.last_token_pos);
	if (!dcl.first)
		dcl.first = r.first;
	if (r.last)
		dcl.last = r.last;
	dcl.last_token_pos = token->get_pos();
	return dcl;
}

type_chain_t parser_t::func_arr_decl() {
	type_chain_t dcl;
	token_ptr_t token = la->get();
	if (token == T_SQR_BRACKET_OPEN) {
		la->next();
		expr_t* expr = nullptr;
		if (la->get() != T_SQR_BRACKET_CLOSE)
			expr = parse_expr();
		la->require(T_SQR_BRACKET_CLOSE, T_EMPTY);
		updatable_sym_t* l = new sym_type_array_t(expr);
		dcl = func_arr_decl();
		l->set_element_type(dcl.last, dcl.last_token_pos);
		dcl.first = l;
		if (!dcl.last)
			dcl.last = l;
		dcl.last_token_pos = token->get_pos();
	}
	return dcl;
}

vector<node_t*> parser_t::parse_initializer_list() {
	vector<node_t*> res;
	if (la->get() == T_OP_ASSIGN) {
		if (la->next() == T_BRACE_OPEN) {
			do {
				if (la->next() == T_BRACE_CLOSE)
					break;
				res.push_back(parse_initializer());
			} while (la->get() == T_COMMA);
			la->require(T_BRACE_CLOSE, T_EMPTY);
		} else
			res.push_back(parse_initializer());
	}
	return res;
}

node_t* parser_t::parse_initializer() {
	if (la->get() == T_STRING) {
		node_t* res = new node_str_literal(la->get());
		la->next();
		return res;
	} else
		return parse_expr();
}

node_t * parser_t::parse_state() {
	return nullptr;
}

expr_t* parser_t::parse_expr() {
	return right_associated_bin_op();
}

void parser_t::print_expr(ostream& os) {
	la->next();
	if (la->get() != T_EMPTY)
		parse_expr()->print(os);
}

void parser_t::print_type(ostream& os) {
	la->next();
	if (la->get() != T_EMPTY) {
		decl_raw_t res = declaration();
		if (!res.type)
			return;
		if (res.identifier)
			res.identifier->short_print(os);
		else
			os << "abstract declaration";
		os << ": ";
		res.type->print(os);

		if (!res.init_list.empty()) {
			os << "with initializer: {";
			for (int i = 0; i < res.init_list.size(); i++) {
				res.init_list[i]->flat_print(os);
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
	for each (auto var in sym_table) {
		os << var->get_hash() << ": ";
		var->print(os);
		os << endl;
	}
}

void set_operator_priority(TOKEN op, int priority) {
	if (priority > 16 || priority < 1 || operators[priority-1].find(op) != operators[priority-1].end())
		throw;
	operators[priority-1].insert(op);
}

void set_operator_priority(TOKEN op, int priority, int priority2) {
	set_operator_priority(op, priority);
	set_operator_priority(op, priority2);
}

void parser_init() {
#define register_token(incode_name, printed_name, func_name, ...) set_operator_priority(T_##incode_name, __VA_ARGS__);
#define TOKEN_LIST
#define PRIORITY_SET
#include "token_operator.h"
#undef PRIORITY_SET
#undef TOKEN_LIST
#undef register_token
}

size_t symbol_t::get_hash() const {
	return cached_hash;
}
