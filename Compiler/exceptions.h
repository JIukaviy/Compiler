#pragma once
#include "tokens.h"
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
	token_ptr_t op;
	token_ptr_t actually;
	set<TOKEN> expected_tokens;
	TOKEN expected_token;

	UnexpectedToken() : SyntaxError("Unexpected token") {};
	UnexpectedToken(token_ptr_t actually, TOKEN expected) : actually(actually), expected_token(expected) {};
	UnexpectedToken(token_ptr_t op, token_ptr_t actually, TOKEN expected) : op(op), actually(actually), expected_token(expected) {};
	UnexpectedToken(token_ptr_t actually) : actually(actually), expected_token(T_EMPTY) {};
	UnexpectedToken(token_ptr_t actually, const set<TOKEN>& expected) : actually(actually), expected_tokens(expected), expected_token(T_EMPTY) {};
	UnexpectedToken(token_ptr_t op, token_ptr_t actually, const set<TOKEN>& expected) : op(op), actually(actually), expected_tokens(expected), expected_token(T_EMPTY) {};

	void make_str() override {
		if (op) {
			op->print_pos(err);
			err << "Operator \"" << op->get_name() << "\" ";
		} else if (actually)
			actually->print_pos(err);
		if (expected_token != T_EMPTY || expected_tokens.size() != 0)
			err << "Instead of the \"" << actually->get_name() << '\"' << endl;
		else {
			if (op)
				err << "was not expecting \"" << actually->get_name() << '\"' << endl;
			else
				err << "Token \"" << actually->get_name() << "\" is not expected";
			return;
		}
		err << "was expecting ";
		if (expected_token != T_EMPTY)
			err << '\"' << token_t::get_name_by_id(expected_token) << '\"' << endl;
		else {
			err << "one of these tokens: " << endl;
			for (auto i = expected_tokens.begin(); i != expected_tokens.end(); ++i)
				err << " \"" << token_t::get_name_by_id(*i) << " \"\n" << endl;
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
	ExpressionIsExpected(token_ptr_t token) {
		token->print_pos(err);
		err << "Operator: \"" << token->get_name() << "\" expecting expression" << endl;
	};
};

class CloseBracketExpected : public SyntaxError {
public:
	CloseBracketExpected() : SyntaxError("Close bracket is expected") {};
};

class InvalidCombinationOfSpecifiers : public SyntaxError {
public:
	InvalidCombinationOfSpecifiers(pos_t pos) : SyntaxError("Invalid combination of specifiers", pos) {};
};

class SemanticError : public CompileError {
public:
	using CompileError::CompileError;
};


class InvalidIncompleteType : public SemanticError {
public:
	InvalidIncompleteType(pos_t pos) : SemanticError("Invalid incomplete type", pos) {};
};

/*class RedefenitionOfSymbol : public SemanticError {
RedefenitionOfSymbol(symbol_t* s, pos_t pos) {

}
};*/