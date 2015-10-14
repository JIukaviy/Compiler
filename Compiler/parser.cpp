#include "parser.h"

parser_t::parser_t(lexeme_analyzer_t* la_): la(la_) {}

node_bin_op_t::node_bin_op_t(node_t* left_, node_t* right_, token_container_t op_) : left(left_), right(right_), op(op_) {}

void node_bin_op_t::print(ostream& os, int level) {
	os << "level: " << level << " left start of binop " << op << endl;
	left->print(os, level + 1);
	os << "level: " << level << " left_ended, right_start of binop " << op << endl;
	right->print(os, level + 1);
	os << "level: " << level << " right_ended of binop " << op << endl;
}

node_var_t::node_var_t(token_container_t variable_) : variable(variable_) {}

void node_var_t::print(ostream& os, int level) {
	os << "level: " << level <<  " variable: " << variable << endl;
}

node_const_t::node_const_t(token_container_t constant_) : constant(constant_) {}

void node_const_t::print(ostream& os, int level) {
	os << "level: " << level << " constant: " << constant << endl;
}

void node_un_op_t::print(ostream& os, int level) {
	expr->print(os, level + 1);
	os << "level: " << level << " unop: " << op << endl;
}

node_t* parser_t::expression() {
	node_t* left = term();
	token_container_t op = la->get();
	if (op == T_OP_ADD || op == T_OP_SUB) {
		if (la->eof())
			throw; // пока просто заглушки
		la->next();
		return new node_bin_op_t(left, expression(), op);
	} else 
		return left;
}

node_t* parser_t::term() {
	node_t* left = factor();
	token_container_t op = la->get();
	if (op == T_OP_MUL || op == T_OP_DIV) {
		if (la->eof())
			throw;
		la->next();
		return new node_bin_op_t(left, term(), op);
	} else
		return left;
}

node_t* parser_t::factor() {
	token_container_t t = la->get();
	if (!la->eof())
		la->next();
	if (t == T_IDENTIFIER)
		return new node_var_t(t);
	else if (t == T_INTEGER || t == T_DOUBLE)
		return new node_const_t(t);
	else if (t == T_OP_BRACKET_OPEN) {
		node_t* l = expression();
		if (la->get() != T_OP_BRACKET_CLOSE)
			throw;
		else
			if(!la->eof())
				la->next();
		return l;
	} else
		throw;
}

void parser_t::parse() {
	la->next();
	try {
		root = expression();
	} catch (...) {
		cerr << "Error appeared";
	}
}

void parser_t::print(ostream& os) {
	root->print(os, 1);
}
