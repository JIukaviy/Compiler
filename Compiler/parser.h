#pragma once

#include <ostream>
#include "tokens.h"
#include "lexeme_analyzer.h"
#include <vector>

using namespace std;

class node_t {
public:
	virtual void print(ostream&, int) = 0;
};

class node_bin_op_t: public node_t {
	node_t* left;
	node_t* right;
	token_container_t op;
public:
	node_bin_op_t(node_t* left_, node_t* right_, token_container_t op);
	void print(ostream&, int) override;
};

class node_const_t : public node_t {
	token_container_t constant;
public:
	node_const_t(token_container_t constant_);
	void print(ostream&, int) override;
};

class node_var_t : public node_t {
	token_container_t variable;
public:
	node_var_t(token_container_t variable_);
	void print(ostream&, int) override;
};

class node_un_op_t: public node_t {
	node_t* expr;
	token_container_t op;
public:
	void print(ostream&, int) override;
};

class node_arr_index_t : public node_bin_op_t {
public:
	void print(ostream&, int) override;
};

class node_func_t : public node_t {
	node_t* func;
	vector<token_container_t> args;
public:
	void print(ostream&, int) override;
};

class node_struct_access_t : public node_t {
	node_t* expr;
	token_container_t ident;
public:
	void print(ostream&, int) override;
};

class parser_t {
	node_t* root;
	lexeme_analyzer_t* la;
	node_t* expression();
	node_t* term();
	node_t* factor();
public:
	parser_t(lexeme_analyzer_t* la_);
	void parse();
	void print(ostream&);
};