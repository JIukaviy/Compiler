#pragma once
#include "tokens.h"
#include "parser_expression_node.h"
#include <assert.h>
#include <sstream>

class CompileError {
protected:
	stringstream err;
	pos_t pos;
	virtual void make_str() {};
public:
	CompileError() : err("Unexpected error") {};
	CompileError(pos_t pos) : err("Unexpected error"), pos(pos) {};
	CompileError(string what) : err(what) {};
	CompileError(string what, pos_t pos) : err(what), pos(pos) {};

	friend ostream& operator<<(ostream& os, CompileError& e) {
		e.make_str();
		if (e.pos)
			os << e.pos;
		os << e.err.str();
		return os;
	};
};

class LexemeAnalyzeError : public CompileError {
	using CompileError::CompileError;
};

class BadNewLine : public LexemeAnalyzeError {
public:
	using LexemeAnalyzeError::LexemeAnalyzeError;
	BadNewLine(pos_t pos) : LexemeAnalyzeError("Unexpected new line", pos) {}
};

class BadEOF : public LexemeAnalyzeError {
public:
	BadEOF(pos_t pos) : LexemeAnalyzeError("BadEOF", pos) {}
};

class BadChar : public LexemeAnalyzeError {
public:
	BadChar(pos_t pos) : LexemeAnalyzeError("BadChar", pos) {}
};

class NoExp : public LexemeAnalyzeError {
public:
	NoExp(pos_t pos) : LexemeAnalyzeError("NoExp", pos) {}
};

class NoFract : public LexemeAnalyzeError {
public:
	NoFract(pos_t pos) : LexemeAnalyzeError("NoFract", pos) {}
};

class NoHex : public LexemeAnalyzeError {
public:
	NoHex(pos_t pos) : LexemeAnalyzeError("NoHex", pos) {}
};

class NoCC : public LexemeAnalyzeError {
public:
	NoCC(pos_t pos) : LexemeAnalyzeError("NoCC", pos) {}
};

class BadCC : public LexemeAnalyzeError {
public:
	BadCC(pos_t pos) : LexemeAnalyzeError("BadCC", pos) {}
};

class EOFReached : public LexemeAnalyzeError {
public:
	EOFReached() : LexemeAnalyzeError("EOF Reached") {}
};

class SyntaxError : public CompileError {
public:
	using CompileError::CompileError;
};

class UnexpectedEOF : public SyntaxError {
public:
	UnexpectedEOF() : SyntaxError("Unexpected end of file") {};
};

class UnexpectedToken : public SyntaxError {
public:
	token_ptr op;
	token_ptr actually;
	set<TOKEN> expected_tokens;
	TOKEN expected_token;

	UnexpectedToken() : SyntaxError("Unexpected token") {};
	UnexpectedToken(token_ptr actually, TOKEN expected) : actually(actually), expected_token(expected) {};
	UnexpectedToken(token_ptr op, token_ptr actually, TOKEN expected) : op(op), actually(actually), expected_token(expected) {};
	UnexpectedToken(token_ptr actually) : actually(actually), expected_token(T_EMPTY) {};
	UnexpectedToken(token_ptr actually, const set<TOKEN>& expected) : actually(actually), expected_tokens(expected), expected_token(T_EMPTY) {};
	UnexpectedToken(token_ptr op, token_ptr actually, const set<TOKEN>& expected) : op(op), actually(actually), expected_tokens(expected), expected_token(T_EMPTY) {};

	void make_str() override {
		if (op) {
			op->print_pos(err);
			err << "Operator \"" << op->get_name() << "\" ";
		} else if (actually != T_EMPTY)
			actually->print_pos(err);
		if (expected_token != T_EMPTY || expected_tokens.size() != 0) {
			err << "Expected ";
			if (expected_token != T_EMPTY)
				err << '\"' << token_t::get_name_by_id(expected_token) << "\" ";
			else {
				err << "one of: " << endl;
				for (auto i = expected_tokens.begin(); i != expected_tokens.end(); ++i)
					err << " \"" << token_t::get_name_by_id(*i) << " \" ";
			}
			if (actually != T_EMPTY)
				err << "before \"" << actually->get_name() << "\"";
			else
				err << "before end of input";
		} else {
			if (op)
				err << "was not expecting \"" << actually->get_name() << '\"' << endl;
			else
				err << "Token \"" << actually->get_name() << "\" is not expected";
		}	
	}

	/*UnexpectedToken(const set<TOKEN>& expected) {
	err << "One of these tokens is expected: \n";
	for (auto i = expected.begin(); i != expected.end(); ++i)
	err << " \"" + token_t::get_name_by_id(*i) << " \"\n";
	};*/
};

class ExpressionIsExpected : public SyntaxError {
public:
	ExpressionIsExpected() : SyntaxError("Expression is expected") {};
	ExpressionIsExpected(token_ptr token) {
		token->print_pos(err);
		err << "Operator: \"" << token->get_name() << "\" expecting expression" << endl;
	};
	ExpressionIsExpected(sym_ptr symbol) {
		symbol->get_token()->print_pos(err);
		err << "Expected expression before symbol: \"" << symbol << '\"' << endl;
	};
};

class TypeSpecIsExpected : public SyntaxError {
public:
	TypeSpecIsExpected(token_ptr token) {
		err << token->get_pos() << "Expected type specifier before '";
		token->short_print(err);
		err << '\'';
	}
};

class CloseBracketExpected : public SyntaxError {
public:
	CloseBracketExpected() : SyntaxError("Close bracket is expected") {};
};

class InvalidCombinationOfSpecifiers : public SyntaxError {
public:
	InvalidCombinationOfSpecifiers(token_ptr token) {
		err << token->get_pos() << "Invalid combination of specifiers: \"";
		token->short_print(err);
		err << "\"";
	};
};

class SemanticError : public CompileError {
public:
	using CompileError::CompileError;
};


class InvalidIncompleteType : public SemanticError {
public:
	InvalidIncompleteType(pos_t pos) : SemanticError("Invalid incomplete type", pos) {};
};

class RedefinitionOfSymbol : public SemanticError {
public:
	RedefinitionOfSymbol(token_ptr token) {
		err << token->get_pos() << "Redefinition of symbol \"";
		token->short_print(err);
		err << "\"";
	}
	RedefinitionOfSymbol(sym_ptr orig_symbol, sym_ptr redef_symbol) {
		err << redef_symbol->get_token()->get_pos() << "Redefinition of symbol \"";
		orig_symbol->short_print(err);
		err << "\", first defenition was here: ";
		pos_t pos = orig_symbol->get_token()->get_pos();
		err << pos.line << ':' << pos.column;
	}
};

class UndefinedSymbol : public SemanticError {
public:
	UndefinedSymbol(const token_ptr token) {
		err << token->get_pos() << "Undefined symbol \"";
		token->short_print(err);
		err << "\"";
	};
	UndefinedSymbol(const sym_ptr s, const pos_t pos) {
		err << pos << "Undefined symbol \"" << s->get_name() << "\"";
	};
	UndefinedSymbol(const string s, const pos_t pos) {
		err << pos << "Undefined symbol \"" << s << "\"";
	};
};

class JumpStmtNotInsideLoop : public SemanticError {
public:
	JumpStmtNotInsideLoop(token_ptr token) {
		err << token->get_pos() << "Statement \"";
		token->short_print(err);
		err << "\" must be inside the loop";
	}
};

class IllegalConversion : public SemanticError {
public:
	IllegalConversion(type_ptr a, type_ptr b, pos_t pos) {
		err << pos << "Can't convert from '";
		a->short_print(err);
		err << "' to '";
		b->short_print(err);
		err << "'";
	}
};

class InvalidTernOpOperands : public SemanticError {
public:
	InvalidTernOpOperands(type_ptr a, type_ptr b, expr_tern_op_t* tern_op) {
		err << tern_op->get_colon_token()->get_pos() << "Invalid operands for ternary: \"";
		err << " (have \'";
		a->short_print(err);
		err << "\' and \'";
		b->short_print(err);
		err << "\')";
	}
	InvalidTernOpOperands(type_ptr a, expr_tern_op_t* tern_op) {
		err << tern_op->get_colon_token()->get_pos() << "Used ";
		a->short_print(err);
		err << " where scalar is required";
	}
};

class InvalidBinOpOperands : public SemanticError {
public:
	InvalidBinOpOperands(type_ptr a, type_ptr b, expr_bin_op_t* bin_op) {
		err << bin_op->get_pos() << "Invalid operands for binary operator: \"";
		bin_op->get_op()->short_print(err);
		err << "\" (have '";
		a->short_print(err);
		err << "' and '";
		b->short_print(err);
		err << "')";
	}
};

class InvalidUnOpOperand : public SemanticError {
public:
	InvalidUnOpOperand(type_ptr a, expr_un_op_t* un_op) {
		err << un_op->get_op()->get_pos() << "Invalid operand for unary operator: \"";
		un_op->get_op()->short_print(err);
		err << "\" (have '";
		a->short_print(err);
		err << "')";
	}
};

class ExprMustBeLValue : public SemanticError {
public:
	ExprMustBeLValue(pos_t pos) : SemanticError("Expression must be lvalue", pos) {}
};

class ExprMustBeEval : public SemanticError {
public:
	ExprMustBeEval(pos_t pos) : SemanticError("Expression should be computable at compile time", pos) {}
};

class AssignmentToReadOnly : public SemanticError {
public:
	/*AssignmentToReadOnly(expr_t* expr) {
		err << expr->get_pos() << "Assignment to read only ";
		err << (typeid(*expr) == typeid(expr_bin_op_t)) ? "location" : "expression";
	}*/
	AssignmentToReadOnly(pos_t pos) {
		err << pos << "Assignment to read only expression";
	}
};

class IncorrectNumberOfArguments : public SemanticError {
public:
	IncorrectNumberOfArguments(int actually, shared_ptr<sym_type_func_t> func, expr_t* op) {
		int required = func->get_arg_types().size();
		err << op->get_pos() << "Too " << (actually < required ? "few " : "many ") << "arguments to function";
	}
};

class StructHasNoMember : public SemanticError {
public:
	StructHasNoMember(sym_type_struct_t* structure, token_ptr member) {
		err << member->get_pos() << '\'';
		structure->short_print(err);
		err << "' has no member '";
		member->short_print(err);
		err << '\'';
	}
};

class InvalidInitListSize : public SemanticError {
public:
	InvalidInitListSize(pos_t pos) : SemanticError("Too many init arguments", pos) {}
};